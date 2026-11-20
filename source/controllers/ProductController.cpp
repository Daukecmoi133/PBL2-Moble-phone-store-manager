#include "../utils/ReviewManager.h"
#include "AppControllerSupport.h"

using namespace AppControllerHelpers;

namespace {
bool beginProductTransaction(FileTransaction &tx, PhoneService *phoneService,
                             ProductMarketingRepository &marketingRepo,
                             ProductDescriptionRepository &descriptionRepo) {
  if (phoneService != nullptr) {
    tx.addParticipant(phoneService->getRepository().getFilePath());
    tx.addParticipant(phoneService->getRepository().getSpecificationsFilePath());
    if (phoneService->getProductImageRepository() != nullptr) {
      tx.addParticipant(
          phoneService->getProductImageRepository()->getResolvedFilePath());
    }
  }
  tx.addParticipant(marketingRepo.getResolvedFilePath());
  tx.addParticipant(descriptionRepo.getResolvedFilePath());

  tx.onRollback([phoneService, &marketingRepo, &descriptionRepo]() {
    if (phoneService != nullptr) {
      phoneService->reload();
      if (phoneService->getProductImageRepository() != nullptr) {
        phoneService->getProductImageRepository()->loadFromFile();
      }
    }
    marketingRepo.loadFromFile();
    descriptionRepo.loadFromFile();
  });

  return tx.begin();
}

ProductMarketingRecord makeMarketingRecord(const ProductItemViewModel &vm) {
  ProductMarketingRecord mkt;
  mkt.productId = vm.id;
  mkt.listPrice = vm.listPrice;
  mkt.saleStart = vm.saleStart;
  mkt.saleEnd = vm.saleEnd;
  mkt.installmentEnabled = vm.installmentEnabled;
  mkt.installmentNote = vm.installmentNote;
  mkt.colors = vm.colors;
  mkt.warrantyMonths = vm.warrantyMonths;
  mkt.returnDays = vm.returnDays;
  return mkt;
}

ProductDescriptionRecord makeDescriptionRecord(const ProductItemViewModel &vm) {
  ProductDescriptionRecord desc;
  desc.productId = vm.id;
  desc.shortHighlights = vm.shortHighlights;
  desc.description = vm.description;
  return desc;
}

bool persistProductImages(ProductItemViewModel &vm, PhoneService *phoneService,
                          bool isUpdate, std::string &outErr) {
  if (phoneService == nullptr) return true;
  if (!vm.images.empty() &&
      phoneService->getProductImageRepository() != nullptr) {
    std::vector<ProductImageRecord> imgRecs;
    for (const auto &img : vm.images) {
      ProductImageRecord r;
      r.productId = vm.id;
      r.imagePath = img.path;
      r.isPrimary = img.isPrimary ? 1 : 0;
      r.displayOrder = img.displayOrder;
      imgRecs.push_back(r);
    }
    phoneService->getProductImageRepository()->setImagesForProduct(vm.id,
                                                                   imgRecs);
    if (!phoneService->getProductImageRepository()->saveToFile()) {
      outErr = "Lỗi khi lưu thư viện ảnh sản phẩm!";
      return false;
    }
    vm.imageUrl = phoneService->getProductImagePath(vm.id);
    return true;
  }

  std::string trimmedImg = trimCopy(vm.imageUrl);
  if (trimmedImg.empty()) return true;
  if (isUpdate) {
    std::string currentImg = phoneService->getProductImagePath(vm.id);
    if (trimmedImg == currentImg || trimmedImg == ("data/" + currentImg)) {
      return true;
    }
  }
  if (!phoneService->updateProductImage(vm.id, trimmedImg, outErr)) {
    if (outErr.empty()) outErr = "Lỗi khi cập nhật ảnh sản phẩm!";
    return false;
  }
  vm.imageUrl = phoneService->getProductImagePath(vm.id);
  return true;
}
} // namespace

// === SẢN PHẨM ===

std::vector<ProductItemViewModel>
AppController::getProducts(const ProductFilterRequest &filter) {
  std::vector<ProductItemViewModel> result;
  if (phoneService == nullptr) {
    return result;
  }

  const auto reservedMap = computeReservedStockMap();
  const Vector<Phone> &phones = phoneService->getAllPhones();
  const bool hasBrandFilter =
      !filter.brand.empty() && filter.brand != "Tất cả";
  const bool hasAnyFilter =
      !filter.keyword.empty() || hasBrandFilter ||
      filter.minPrice > 0.0 || filter.maxPrice > 0.0 || filter.onlyInStock;
  if (!hasAnyFilter) {
    result.reserve(static_cast<size_t>(phones.getSize()));
  }

  for (int i = 0; i < phones.getSize(); ++i) {
    const Phone &phone = phones[i];

    // Pre-filter raw Phone fields before constructing full ViewModel
    if (hasBrandFilter && phone.getBrand() != filter.brand) {
      continue;
    }
    if (filter.minPrice > 0.0 && phone.getSellingPrice() < filter.minPrice) {
      continue;
    }
    if (filter.maxPrice > 0.0 && phone.getSellingPrice() > filter.maxPrice) {
      continue;
    }
    if (!filter.keyword.empty()) {
      if (!Utf8Helper::containsCaseInsensitive(phone.getName(),
                                               filter.keyword) &&
          !Utf8Helper::containsCaseInsensitive(phone.getBrand(),
                                               filter.keyword) &&
          !Utf8Helper::containsCaseInsensitive(phone.getID(), filter.keyword)) {
        continue;
      }
    }

    auto resIt = reservedMap.find(phone.getID());
    const int reserved = (resIt != reservedMap.end()) ? resIt->second : 0;
    if (filter.onlyInStock && (phone.getQuantity() - reserved) <= 0) {
      continue;
    }

    ProductItemViewModel vm = phoneToViewModel(
        phone, phoneService, &marketingRepo, &descriptionRepo, reserved);
    result.push_back(std::move(vm));
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
  if (phoneService == nullptr) {
    return ProductItemViewModel();
  }
  Phone *p = phoneService->getPhoneById(id);
  if (p == nullptr) {
    return ProductItemViewModel();
  }
  return phoneToViewModel(*p, phoneService, &marketingRepo, &descriptionRepo,
                          computeReservedStockForPhone(id));
}

std::vector<ProductItemViewModel>
AppController::getProductVariants(const std::string &phoneIdOrBaseModel) {
  std::vector<ProductItemViewModel> variants;
  if (phoneIdOrBaseModel.empty() || phoneService == nullptr)
    return variants;

  const std::vector<ProductItemViewModel> allProducts = getProducts();
  std::string targetBase = "";
  for (const auto &p : allProducts) {
    if (p.id == phoneIdOrBaseModel || p.baseModel == phoneIdOrBaseModel ||
        p.name == phoneIdOrBaseModel) {
      targetBase = p.baseModel.empty() ? p.name : p.baseModel;
      break;
    }
  }
  if (targetBase.empty())
    targetBase = phoneIdOrBaseModel;

  for (const auto &p : allProducts) {
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

  if (phoneService == nullptr) {
    return OperationResult::Fail("Hệ thống quản lý sản phẩm chưa sẵn sàng!");
  }

  std::string generatedId;
  std::string idError;
  if (!IdGenerator::nextId("PHN", "",
                           collectModelIds(phoneService->getAllPhones()),
                           generatedId, idError)) {
    return OperationResult::Fail(idError);
  }

  ProductItemViewModel newItem = product;
  newItem.id = generatedId;
  newItem.reservedStock = 0;
  newItem.calculateStockStatus();

  // Bắt đầu giao dịch đa tệp nguyên tử cho toàn bộ 5 tệp sản phẩm
  FileTransaction tx;
  if (!beginProductTransaction(tx, phoneService, marketingRepo, descriptionRepo)) {
    return OperationResult::Fail("Không thể khởi tạo giao dịch lưu trữ sản phẩm!");
  }

  // Ghi xuống file thông qua PhoneService và PhoneRepository
  Phone phone(newItem.id, newItem.name, newItem.brand, newItem.os,
              newItem.importPrice, newItem.sellingPrice, newItem.actualStock,
              newItem.manufacturer, newItem.specs);
  if (!newItem.detailedSpecs.empty()) {
    phone.setDetailedSpecifications(
        serializeDetailedSpecs(newItem.detailedSpecs));
  }

  std::string serviceErr;
  if (!phoneService->addPhone(phone, serviceErr)) {
    tx.rollback();
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi lưu sản phẩm vào file dữ liệu!");
  }

  std::string imgErr;
  if (!persistProductImages(newItem, phoneService, false, imgErr)) {
    tx.rollback();
    return OperationResult::Fail(imgErr);
  }

  // Lưu marketing & khuyến mãi
  marketingRepo.setByProductId(makeMarketingRecord(newItem));
  if (!marketingRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi lưu thông tin marketing sản phẩm!");
  }

  // Lưu mô tả chi tiết
  descriptionRepo.setByProductId(makeDescriptionRecord(newItem));
  if (!descriptionRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi lưu mô tả chi tiết sản phẩm!");
  }

  if (!tx.commit()) {
    tx.rollback();
    return OperationResult::Fail("Không thể xác nhận giao dịch lưu sản phẩm!");
  }

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

  if (phoneService == nullptr ||
      phoneService->getPhoneById(product.id) == nullptr) {
    return OperationResult::Fail("Không tìm thấy sản phẩm có mã '" + product.id +
                                 "' để cập nhật!");
  }

  FileTransaction tx;
  if (!beginProductTransaction(tx, phoneService, marketingRepo, descriptionRepo)) {
    return OperationResult::Fail("Không thể khởi tạo giao dịch cập nhật sản phẩm!");
  }

  // Cập nhật xuống file qua PhoneService
  Phone phone(product.id, product.name, product.brand, product.os,
              product.importPrice, product.sellingPrice, product.actualStock,
              product.manufacturer, product.specs);
  if (!product.detailedSpecs.empty()) {
    phone.setDetailedSpecifications(
        serializeDetailedSpecs(product.detailedSpecs));
  }

  std::string serviceErr;
  if (!phoneService->updatePhone(phone, serviceErr)) {
    tx.rollback();
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi cập nhật sản phẩm vào file dữ liệu!");
  }

  std::string imgErr;
  if (!persistProductImages(product, phoneService, true, imgErr)) {
    tx.rollback();
    return OperationResult::Fail(imgErr);
  }

  // Lưu marketing & khuyến mãi
  marketingRepo.setByProductId(makeMarketingRecord(product));
  if (!marketingRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi cập nhật thông tin marketing sản phẩm!");
  }

  // Lưu mô tả chi tiết
  descriptionRepo.setByProductId(makeDescriptionRecord(product));
  if (!descriptionRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi cập nhật mô tả chi tiết sản phẩm!");
  }

  if (!tx.commit()) {
    tx.rollback();
    return OperationResult::Fail("Không thể xác nhận giao dịch cập nhật sản phẩm!");
  }

  return OperationResult::Ok("Cập nhật sản phẩm '" + product.name +
                             "' thành công!");
}

OperationResult AppController::deleteProduct(const std::string &id) {
  REQUIRE_ADMIN_ROLE();
  if (!IdGenerator::isValidId(id, "PHN")) {
    return OperationResult::Fail(
        "Mã sản phẩm không đúng định dạng PHN-000001!");
  }
  if (phoneService == nullptr) {
    return OperationResult::Fail("Hệ thống quản lý sản phẩm chưa sẵn sàng!");
  }

  // Bảo toàn sản phẩm đã phát sinh giao dịch hoặc bảo hành
  if (orderService != nullptr) {
    const Vector<Order> &orders = orderService->getAllOrders();
    for (int i = 0; i < orders.getSize(); ++i) {
      const Vector<OrderDetail> &details = orders[i].getDetails();
      for (int j = 0; j < details.getSize(); ++j) {
        if (details[j].getPhoneId() == id) {
          return OperationResult::Fail(
              "Không thể xóa sản phẩm '" + id +
              "' vì đang được tham chiếu trong đơn hàng (" + orders[i].getID() +
              ")!");
        }
      }
    }
  }
  if (invoiceService != nullptr) {
    const Vector<Invoice> &invoices = invoiceService->getAll();
    for (int i = 0; i < invoices.getSize(); ++i) {
      const Vector<OrderDetail> &details = invoices[i].getDetails();
      for (int j = 0; j < details.getSize(); ++j) {
        if (details[j].getPhoneId() == id) {
          return OperationResult::Fail(
              "Không thể xóa sản phẩm '" + id +
              "' vì đang được tham chiếu trong hóa đơn (" + invoices[i].getID() +
              ")!");
        }
      }
    }
  }
  if (supplierService != nullptr) {
    const Vector<ImportReceipt> &receipts = supplierService->getAllReceipts();
    for (int i = 0; i < receipts.getSize(); ++i) {
      const Vector<OrderDetail> &details = receipts[i].getDetails();
      for (int j = 0; j < details.getSize(); ++j) {
        if (details[j].getPhoneId() == id) {
          return OperationResult::Fail(
              "Không thể xóa sản phẩm '" + id +
              "' vì đang được tham chiếu trong phiếu nhập (" +
              receipts[i].getID() + ")!");
        }
      }
    }
  }
  if (warrantyService != nullptr) {
    const Vector<WarrantyRequest> &warranties = warrantyService->getAll();
    for (int i = 0; i < warranties.getSize(); ++i) {
      if (warranties[i].getPhoneId() == id) {
        return OperationResult::Fail(
            "Không thể xóa sản phẩm '" + id +
            "' vì đang được tham chiếu trong yêu cầu bảo hành (" +
            warranties[i].getId() + ")!");
      }
    }
  }

  Phone *existingPhone = phoneService->getPhoneById(id);
  if (existingPhone == nullptr) {
    return OperationResult::Fail("Không tìm thấy sản phẩm để xóa!");
  }
  const std::string deletedName = existingPhone->getName();

  FileTransaction tx;
  if (!beginProductTransaction(tx, phoneService, marketingRepo, descriptionRepo)) {
    return OperationResult::Fail("Không thể khởi tạo giao dịch xóa sản phẩm!");
  }

  std::string serviceErr;
  if (!phoneService->deletePhone(id, serviceErr)) {
    tx.rollback();
    return OperationResult::Fail(
        !serviceErr.empty() ? serviceErr
                            : "Lỗi khi xóa sản phẩm khỏi file dữ liệu!");
  }
  if (phoneService->getProductImageRepository() != nullptr) {
    phoneService->getProductImageRepository()->removeImagePath(id);
    if (!phoneService->getProductImageRepository()->saveToFile()) {
      tx.rollback();
      return OperationResult::Fail("Lỗi khi cập nhật file ảnh sau khi xóa sản phẩm!");
    }
  }

  marketingRepo.removeByProductId(id);
  if (!marketingRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi cập nhật file marketing sau khi xóa sản phẩm!");
  }

  descriptionRepo.removeByProductId(id);
  if (!descriptionRepo.saveToFile()) {
    tx.rollback();
    return OperationResult::Fail("Lỗi khi cập nhật file mô tả sau khi xóa sản phẩm!");
  }

  if (!tx.commit()) {
    tx.rollback();
    return OperationResult::Fail("Không thể xác nhận giao dịch xóa sản phẩm!");
  }

  return OperationResult::Ok("Đã xóa sản phẩm '" +
                             (deletedName.empty() ? id : deletedName) + "'!");
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
  if (customerId.empty() || phoneId.empty() || orderService == nullptr)
    return false;

  const Vector<Order> &orders = orderService->getAllOrders();
  for (int i = 0; i < orders.getSize(); ++i) {
    if (orders[i].getCustomerId() == customerId &&
        orders[i].getStatus() == OrderStatus::COMPLETED) {
      const Vector<OrderDetail> &details = orders[i].getDetails();
      for (int j = 0; j < details.getSize(); ++j) {
        if (details[j].getPhoneId() == phoneId)
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
