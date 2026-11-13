#include "../utils/ReviewManager.h"
#include "AppControllerSupport.h"

using namespace AppControllerHelpers;

// === SẢN PHẨM ===

std::vector<ProductItemViewModel>
AppController::getProducts(const ProductFilterRequest &filter) {
  auto &store = MockDataStore::getInstance();
  std::vector<ProductItemViewModel> result;

  for (const auto &p : store.products) {
    // Keyword filter
    if (!filter.keyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(p.name, filter.keyword) &&
          !Utf8Helper::containsCaseInsensitive(p.brand, filter.keyword) &&
          !Utf8Helper::containsCaseInsensitive(p.id, filter.keyword)) {
        continue;
      }
    }

    // Brand filter
    if (!filter.brand.empty() && filter.brand != "Tất cả") {
      if (p.brand != filter.brand)
        continue;
    }

    // Price range filter
    if (filter.minPrice > 0.0 && p.sellingPrice < filter.minPrice)
      continue;
    if (filter.maxPrice > 0.0 && p.sellingPrice > filter.maxPrice)
      continue;

    // In-stock only
    if (filter.onlyInStock && p.availableStock <= 0)
      continue;

    result.push_back(p);
  }

  // Sort
  if (filter.sortBy == "price_asc") {
    std::sort(result.begin(), result.end(),
              [](const ProductItemViewModel &a, const ProductItemViewModel &b) {
                return a.sellingPrice < b.sellingPrice;
              });
  } else if (filter.sortBy == "price_desc") {
    std::sort(result.begin(), result.end(),
              [](const ProductItemViewModel &a, const ProductItemViewModel &b) {
                return a.sellingPrice > b.sellingPrice;
              });
  } else if (filter.sortBy == "stock_desc") {
    std::sort(result.begin(), result.end(),
              [](const ProductItemViewModel &a, const ProductItemViewModel &b) {
                return a.availableStock > b.availableStock;
              });
  } else { // name_asc
    std::sort(result.begin(), result.end(),
              [](const ProductItemViewModel &a, const ProductItemViewModel &b) {
                return a.name < b.name;
              });
  }

  return result;
}

ProductItemViewModel AppController::getProductById(const std::string &id) {
  auto &store = MockDataStore::getInstance();
  for (const auto &p : store.products) {
    if (p.id == id)
      return p;
  }
  return ProductItemViewModel();
}

std::vector<ProductItemViewModel>
AppController::getProductVariants(const std::string &phoneIdOrBaseModel) {
  auto &store = MockDataStore::getInstance();
  std::vector<ProductItemViewModel> variants;
  if (phoneIdOrBaseModel.empty())
    return variants;

  std::string targetBase = "";
  for (const auto &p : store.products) {
    if (p.id == phoneIdOrBaseModel || p.baseModel == phoneIdOrBaseModel ||
        p.name == phoneIdOrBaseModel) {
      targetBase = p.baseModel.empty() ? p.name : p.baseModel;
      break;
    }
  }
  if (targetBase.empty())
    targetBase = phoneIdOrBaseModel;

  for (const auto &p : store.products) {
    std::string pBase = p.baseModel.empty() ? p.name : p.baseModel;
    if (pBase == targetBase || p.id == phoneIdOrBaseModel) {
      variants.push_back(p);
    }
  }

  if (variants.empty()) {
    ProductItemViewModel single = getProductById(phoneIdOrBaseModel);
    if (!single.id.empty())
      variants.push_back(single);
  }
  return variants;
}

OperationResult
AppController::addProduct(const ProductItemViewModel &rawProduct) {
  REQUIRE_ADMIN_ROLE();
  ProductItemViewModel product = rawProduct;
  product.name = trimCopy(product.name);
  product.brand = trimCopy(product.brand);
  if (product.name.empty() || product.brand.empty()) {
    return OperationResult::Fail(
        "Tên và thương hiệu sản phẩm không được để trống!");
  }
  if (product.sellingPrice <= 0 || product.importPrice <= 0) {
    return OperationResult::Fail("Giá nhập và giá bán phải lớn hơn 0!");
  }
  if (product.listPrice > 0 && product.listPrice <= product.sellingPrice) {
    return OperationResult::Fail(
        "Giá niêm yết (giá gốc) phải lớn hơn giá bán thực tế!");
  }
  if (product.actualStock < 0) {
    return OperationResult::Fail("Số lượng tồn kho không được âm!");
  }
  if (product.warrantyMonths < 0 || product.returnDays < 0) {
    return OperationResult::Fail(
        "Thời gian bảo hành và đổi trả không được âm!");
  }

  auto &store = MockDataStore::getInstance();
  std::string generatedId;
  std::string idError;
  if (!IdGenerator::nextId("PHN", "", collectIds(store.products), generatedId,
                           idError)) {
    return OperationResult::Fail(idError);
  }

  ProductItemViewModel newItem = product;
  newItem.id = generatedId;
  newItem.reservedStock = 0;
  newItem.calculateStockStatus();

  // Ghi xuống file thông qua PhoneService và PhoneRepository
  if (phoneService != nullptr) {
    Phone phone(newItem.id, newItem.name, newItem.brand, newItem.os,
                newItem.importPrice, newItem.sellingPrice, newItem.actualStock,
                newItem.manufacturer, newItem.specs);
    if (!newItem.detailedSpecs.empty()) {
      phone.setDetailedSpecifications(
          serializeDetailedSpecs(newItem.detailedSpecs));
    }

    std::string serviceErr;
    if (!phoneService->addPhone(phone, serviceErr)) {
      return OperationResult::Fail(
          !serviceErr.empty() ? serviceErr
                              : "Lỗi khi lưu sản phẩm vào file dữ liệu!");
    }

    // Lưu ảnh nếu có
    if (!newItem.images.empty() &&
        phoneService->getProductImageRepository() != nullptr) {
      std::vector<ProductImageRecord> imgRecs;
      for (const auto &img : newItem.images) {
        ProductImageRecord r;
        r.productId = newItem.id;
        r.imagePath = img.path;
        r.isPrimary = img.isPrimary ? 1 : 0;
        r.displayOrder = img.displayOrder;
        imgRecs.push_back(r);
      }
      phoneService->getProductImageRepository()->setImagesForProduct(newItem.id,
                                                                     imgRecs);
      phoneService->getProductImageRepository()->saveToFile();
      newItem.imageUrl = phoneService->getProductImagePath(newItem.id);
    } else {
      std::string trimmedImg = trimCopy(newItem.imageUrl);
      if (!trimmedImg.empty()) {
        std::string imgErr;
        if (phoneService->updateProductImage(newItem.id, trimmedImg, imgErr)) {
          newItem.imageUrl = phoneService->getProductImagePath(newItem.id);
        }
      }
    }
  }

  // Lưu marketing & khuyến mãi
  ProductMarketingRecord mkt;
  mkt.productId = newItem.id;
  mkt.listPrice = newItem.listPrice;
  mkt.saleStart = newItem.saleStart;
  mkt.saleEnd = newItem.saleEnd;
  mkt.installmentEnabled = newItem.installmentEnabled;
  mkt.installmentNote = newItem.installmentNote;
  mkt.colors = newItem.colors;
  mkt.warrantyMonths = newItem.warrantyMonths;
  mkt.returnDays = newItem.returnDays;
  marketingRepo.setByProductId(mkt);
  marketingRepo.saveToFile();

  // Lưu mô tả chi tiết
  ProductDescriptionRecord desc;
  desc.productId = newItem.id;
  desc.shortHighlights = newItem.shortHighlights;
  desc.description = newItem.description;
  descriptionRepo.setByProductId(desc);
  descriptionRepo.saveToFile();

  store.products.push_back(newItem);

  return OperationResult::Ok("Thêm sản phẩm '" + product.name +
                             "' thành công!");
}

OperationResult
AppController::updateProduct(const ProductItemViewModel &rawProduct) {
  REQUIRE_ADMIN_ROLE();
  ProductItemViewModel product = rawProduct;
  product.name = trimCopy(product.name);
  product.brand = trimCopy(product.brand);
  if (!IdGenerator::isValidId(product.id, "PHN") || product.name.empty()) {
    return OperationResult::Fail("Thông tin sản phẩm không hợp lệ!");
  }
  if (product.sellingPrice <= 0) {
    return OperationResult::Fail("Giá bán phải lớn hơn 0!");
  }
  if (product.listPrice > 0 && product.listPrice <= product.sellingPrice) {
    return OperationResult::Fail(
        "Giá niêm yết (giá gốc) phải lớn hơn giá bán thực tế!");
  }
  if (product.actualStock < 0) {
    return OperationResult::Fail("Số lượng tồn kho không được âm!");
  }
  if (product.warrantyMonths < 0 || product.returnDays < 0) {
    return OperationResult::Fail(
        "Thời gian bảo hành và đổi trả không được âm!");
  }

  // Cập nhật xuống file qua PhoneService
  if (phoneService != nullptr) {
    Phone phone(product.id, product.name, product.brand, product.os,
                product.importPrice, product.sellingPrice, product.actualStock,
                product.manufacturer, product.specs);
    if (!product.detailedSpecs.empty()) {
      phone.setDetailedSpecifications(
          serializeDetailedSpecs(product.detailedSpecs));
    }

    std::string serviceErr;
    if (!phoneService->updatePhone(phone, serviceErr)) {
      return OperationResult::Fail(
          !serviceErr.empty() ? serviceErr
                              : "Lỗi khi cập nhật sản phẩm vào file dữ liệu!");
    }

    // Cập nhật thư viện ảnh
    if (!product.images.empty() &&
        phoneService->getProductImageRepository() != nullptr) {
      std::vector<ProductImageRecord> imgRecs;
      for (const auto &img : product.images) {
        ProductImageRecord r;
        r.productId = product.id;
        r.imagePath = img.path;
        r.isPrimary = img.isPrimary ? 1 : 0;
        r.displayOrder = img.displayOrder;
        imgRecs.push_back(r);
      }
      phoneService->getProductImageRepository()->setImagesForProduct(product.id,
                                                                     imgRecs);
      phoneService->getProductImageRepository()->saveToFile();
      product.imageUrl = phoneService->getProductImagePath(product.id);
    } else {
      std::string currentImg = phoneService->getProductImagePath(product.id);
      std::string trimmedImg = trimCopy(product.imageUrl);
      if (!trimmedImg.empty() && trimmedImg != currentImg &&
          trimmedImg != ("data/" + currentImg)) {
        std::string imgErr;
        if (!phoneService->updateProductImage(product.id, trimmedImg, imgErr)) {
          return OperationResult::Fail(
              !imgErr.empty() ? imgErr : "Lỗi khi cập nhật ảnh sản phẩm!");
        }
        product.imageUrl = phoneService->getProductImagePath(product.id);
      }
    }
  }

  // Lưu marketing & khuyến mãi
  ProductMarketingRecord mkt;
  mkt.productId = product.id;
  mkt.listPrice = product.listPrice;
  mkt.saleStart = product.saleStart;
  mkt.saleEnd = product.saleEnd;
  mkt.installmentEnabled = product.installmentEnabled;
  mkt.installmentNote = product.installmentNote;
  mkt.colors = product.colors;
  mkt.warrantyMonths = product.warrantyMonths;
  mkt.returnDays = product.returnDays;
  marketingRepo.setByProductId(mkt);
  marketingRepo.saveToFile();

  // Lưu mô tả chi tiết
  ProductDescriptionRecord desc;
  desc.productId = product.id;
  desc.shortHighlights = product.shortHighlights;
  desc.description = product.description;
  descriptionRepo.setByProductId(desc);
  descriptionRepo.saveToFile();

  auto &store = MockDataStore::getInstance();
  for (auto &p : store.products) {
    if (p.id == product.id) {
      p.name = product.name;
      p.brand = product.brand;
      p.os = product.os;
      p.sellingPrice = product.sellingPrice;
      p.importPrice = product.importPrice;
      p.actualStock = product.actualStock;
      p.manufacturer = product.manufacturer;
      p.specs = product.specs;
      p.imageUrl = product.imageUrl;
      p.listPrice = product.listPrice;
      p.saleStart = product.saleStart;
      p.saleEnd = product.saleEnd;
      p.installmentEnabled = product.installmentEnabled;
      p.installmentNote = product.installmentNote;
      p.colors = product.colors;
      p.warrantyMonths = product.warrantyMonths;
      p.returnDays = product.returnDays;
      p.shortHighlights = product.shortHighlights;
      p.description = product.description;
      p.detailedSpecs = product.detailedSpecs;
      p.images = product.images;
      p.calculateStockStatus();
      return OperationResult::Ok("Cập nhật sản phẩm '" + product.name +
                                 "' thành công!");
    }
  }

  return OperationResult::Fail("Không tìm thấy sản phẩm có mã '" + product.id +
                               "' để cập nhật!");
}

OperationResult AppController::deleteProduct(const std::string &id) {
  REQUIRE_ADMIN_ROLE();
  if (!IdGenerator::isValidId(id, "PHN")) {
    return OperationResult::Fail(
        "Mã sản phẩm không đúng định dạng PHN-000001!");
  }
  auto &store = MockDataStore::getInstance();

  // Preserve products referenced by transaction and warranty history.
  for (const auto &ord : store.orders) {
    for (const auto &d : ord.details) {
      if (d.phoneId == id) {
        return OperationResult::Fail(
            "Không thể xóa sản phẩm '" + id +
            "' vì đang được tham chiếu trong đơn hàng (" + ord.id + ")!");
      }
    }
  }
  for (const auto &invoice : store.invoices) {
    for (const auto &d : invoice.details) {
      if (d.phoneId == id) {
        return OperationResult::Fail(
            "Không thể xóa sản phẩm '" + id +
            "' vì đang được tham chiếu trong hóa đơn (" + invoice.id + ")!");
      }
    }
  }
  for (const auto &receipt : store.receipts) {
    for (const auto &d : receipt.details) {
      if (d.phoneId == id) {
        return OperationResult::Fail(
            "Không thể xóa sản phẩm '" + id +
            "' vì đang được tham chiếu trong phiếu nhập (" + receipt.id + ")!");
      }
    }
  }
  for (const auto &warranty : store.warranties) {
    if (warranty.phoneId == id) {
      return OperationResult::Fail(
          "Không thể xóa sản phẩm '" + id +
          "' vì đang được tham chiếu trong yêu cầu bảo hành (" + warranty.id +
          ")!");
    }
  }

  // Xóa khỏi file qua PhoneService
  if (phoneService != nullptr) {
    std::string serviceErr;
    if (!phoneService->deletePhone(id, serviceErr)) {
      return OperationResult::Fail(
          !serviceErr.empty() ? serviceErr
                              : "Lỗi khi xóa sản phẩm khỏi file dữ liệu!");
    }
    if (phoneService->getProductImageRepository() != nullptr) {
      phoneService->getProductImageRepository()->removeImagePath(id);
      phoneService->getProductImageRepository()->saveToFile();
    }
  }

  marketingRepo.removeByProductId(id);
  marketingRepo.saveToFile();

  descriptionRepo.removeByProductId(id);
  descriptionRepo.saveToFile();

  for (auto it = store.products.begin(); it != store.products.end(); ++it) {
    if (it->id == id) {
      std::string name = it->name;
      store.products.erase(it);
      return OperationResult::Ok("Đã xóa sản phẩm '" + name + "'!");
    }
  }

  return OperationResult::Fail("Không tìm thấy sản phẩm để xóa!");
}

OperationResult
AppController::updateProductImage(const std::string &productId,
                                  const std::string &imageFilePath) {
  REQUIRE_ADMIN_ROLE();
  if (!IdGenerator::isValidId(productId, "PHN")) {
    return OperationResult::Fail(
        "Mã sản phẩm không đúng định dạng PHN-000001!");
  }
  if (phoneService == nullptr) {
    return OperationResult::Fail("Hệ thống quản lý sản phẩm chưa sẵn sàng!");
  }

  std::string serviceErr;
  if (!phoneService->updateProductImage(productId, imageFilePath, serviceErr)) {
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr : "Không thể cập nhật ảnh sản phẩm!");
  }

  std::string newPath = phoneService->getProductImagePath(productId);
  auto &store = MockDataStore::getInstance();
  for (auto &p : store.products) {
    if (p.id == productId) {
      p.imageUrl = newPath;
      break;
    }
  }
  return OperationResult::Ok("Cập nhật ảnh sản phẩm thành công!");
}

std::vector<ProductReviewViewModel>
AppController::getProductReviews(const std::string &phoneId) {
  if (phoneId.empty()) return {};
  std::vector<ProductReviewViewModel> direct = ReviewManager::getReviewsForProduct(phoneId);
  ProductItemViewModel currentProd = getProductById(phoneId);
  std::string base = currentProd.baseModel.empty() ? currentProd.name : currentProd.baseModel;
  if (base.empty()) return direct;

  auto all = ReviewManager::loadAllReviews();
  std::vector<ProductReviewViewModel> res = direct;
  std::vector<std::string> seenCustomers;
  for (const auto &r : direct) {
    seenCustomers.push_back(r.customerId);
  }

  for (const auto &r : all) {
    if (r.phoneId == phoneId) continue;
    if (std::find(seenCustomers.begin(), seenCustomers.end(), r.customerId) != seenCustomers.end()) continue;
    ProductItemViewModel otherProd = getProductById(r.phoneId);
    std::string otherBase = otherProd.baseModel.empty() ? otherProd.name : otherProd.baseModel;
    if (!otherBase.empty() && otherBase == base) {
      res.push_back(r);
      seenCustomers.push_back(r.customerId);
    }
  }
  return res;
}

bool AppController::canCustomerReviewProduct(const std::string &customerId,
                                             const std::string &phoneId) {
  if (customerId.empty() || phoneId.empty())
    return false;
  auto &store = MockDataStore::getInstance();

  for (const auto &ord : store.orders) {
    if (ord.customerId == customerId && ord.status == OrderStatus::COMPLETED) {
      for (const auto &d : ord.details) {
        if (d.phoneId == phoneId)
          return true;
      }
    }
  }
  return false;
}

OperationResult AppController::addProductReview(const std::string &phoneId,
                                                int ratingStars,
                                                const std::string &comment) {
  if (phoneId.empty()) {
    return OperationResult::Fail("Mã sản phẩm không hợp lệ!");
  }
  if (ratingStars < 1 || ratingStars > 5) {
    return OperationResult::Fail("Số sao đánh giá phải từ 1 đến 5 sao!");
  }
  std::string trimmedComment = StringHelper::trim(comment);
  if (trimmedComment.empty()) {
    return OperationResult::Fail(
        "Vui lòng nhập lời nhận xét hoặc chia sẻ trải nghiệm của bạn!");
  }

  std::string cId = currentSession.userId;
  if (cId.empty() || !currentSession.isAuthenticated) {
    return OperationResult::Fail("Vui lòng đăng nhập để đánh giá sản phẩm!");
  }

  if (!canCustomerReviewProduct(cId, phoneId)) {
    return OperationResult::Fail("Bạn chỉ có thể đánh giá sản phẩm này sau khi "
                                 "đã mua và hoàn thành đơn hàng!");
  }

  std::string cName =
      currentSession.fullName.empty() ? "Khách hàng" : currentSession.fullName;
  std::string timeStr = ReviewManager::getCurrentTimeString();

  ProductReviewViewModel rev(phoneId, cId, cName, ratingStars, trimmedComment,
                             timeStr);
  if (ReviewManager::upsertReview(rev)) {
    return OperationResult::Ok(
        "Đánh giá sản phẩm thành công! Cảm ơn nhận xét của bạn.");
  }
  return OperationResult::Fail("Không thể lưu trữ đánh giá sản phẩm!");
}
