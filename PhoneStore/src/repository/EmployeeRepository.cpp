#include "EmployeeRepository.h"

EmployeeRepository::EmployeeRepository(FileManager& fm)
    : fileManager(fm), fileName("employees.txt"), hasSnapshot(false) {}

EmployeeRepository::~EmployeeRepository() {
    destroyAll(items);
    destroyAll(snapshotItems);
}

void EmployeeRepository::destroyAll(Vector<Employee*>& list) {
    for (int i = 0; i < list.getSize(); ++i) delete list[i];
    list.clear();
}

Vector<Employee*> EmployeeRepository::cloneAll(const Vector<Employee*>& list) {
    Vector<Employee*> copy;
    for (int i = 0; i < list.getSize(); ++i) copy.push_back(list[i]->clone());
    return copy;
}

void EmployeeRepository::rebuildIndexes() {
    idIndex.clear();
    usernameIndex.clear();
    for (int i = 0; i < items.getSize(); ++i) {
        idIndex.insert(items[i]->getID(), i);
        // TODO(logic): usernameIndex.insert(items[i]->getAccount().getUsername(), i);
    }
}

Result<void> EmployeeRepository::load() {
    destroyAll(items);
    report = LoadReport();

    Result<Vector<std::string>> read = fileManager.readAllLines(fileName);
    if (!read.isOk()) {
        rebuildIndexes();
        if (read.code() == ErrorCode::NotFound) return Result<void>::ok(); // file chưa có = rỗng
        return Result<void>::fail(read.code(), read.message());
    }

    const Vector<std::string>& lines = read.value();
    for (int i = 0; i < lines.getSize(); ++i) {
        const std::string& line = lines[i];
        if (line.empty() || line[0] == '#') continue;
        ++report.totalLines;

        Employee* e = nullptr;
        try {
            e = Employee::fromFileLine(line);
        } catch (...) {
            e = nullptr;
        }
        if (e == nullptr) {
            report.addError(i + 1, "Dòng nhân viên không hợp lệ");
            continue;
        }
        bool duplicate = false;
        for (int j = 0; j < items.getSize(); ++j) {
            if (items[j]->getID() == e->getID()) { duplicate = true; break; }
        }
        if (e->getID().empty() || duplicate) {
            report.addError(i + 1, e->getID().empty() ? "ID rỗng" : "trùng ID: " + e->getID());
            delete e;
            continue;
        }
        items.push_back(e);
        ++report.loadedLines;
    }
    rebuildIndexes();
    return Result<void>::ok();
}

Result<void> EmployeeRepository::save() { return flush(); }

Vector<std::unique_ptr<Employee>> EmployeeRepository::getAll() const {
    Vector<std::unique_ptr<Employee>> copy;
    for (int i = 0; i < items.getSize(); ++i) copy.push_back(std::unique_ptr<Employee>(items[i]->clone()));
    return copy;
}

Result<std::unique_ptr<Employee>> EmployeeRepository::findById(const std::string& id) const {
    const int* pos = idIndex.find(id);
    if (pos == nullptr) {
        return Result<std::unique_ptr<Employee>>::fail(ErrorCode::NotFound, "Không tìm thấy nhân viên: " + id);
    }
    return Result<std::unique_ptr<Employee>>::ok(std::unique_ptr<Employee>(items[*pos]->clone()));
}

Result<std::unique_ptr<Employee>> EmployeeRepository::findByUsername(const std::string&) const {
    // TODO(logic): tra usernameIndex rồi trả bản clone.
    return Result<std::unique_ptr<Employee>>::fail(ErrorCode::NotImplemented,
                                                   "TODO: EmployeeRepository::findByUsername");
}

Result<void> EmployeeRepository::add(const Employee& employee) {
    const std::string id = employee.getID();
    if (id.empty()) return Result<void>::fail(ErrorCode::InvalidInput, "ID không được để trống");
    if (idIndex.contains(id)) return Result<void>::fail(ErrorCode::Duplicate, "Trùng ID: " + id);
    items.push_back(employee.clone());
    rebuildIndexes();
    return Result<void>::ok();
}

Result<void> EmployeeRepository::update(const Employee& employee) {
    const int* pos = idIndex.find(employee.getID());
    if (pos == nullptr) return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy ID: " + employee.getID());
    int index = *pos;
    delete items[index];
    items[index] = employee.clone();
    rebuildIndexes();
    return Result<void>::ok();
}

Result<void> EmployeeRepository::remove(const std::string& id) {
    const int* pos = idIndex.find(id);
    if (pos == nullptr) return Result<void>::fail(ErrorCode::NotFound, "Không tìm thấy ID: " + id);
    int index = *pos;
    delete items[index];
    items.removeAt(index);
    rebuildIndexes();
    return Result<void>::ok();
}

size_t EmployeeRepository::count() const { return static_cast<size_t>(items.getSize()); }

const LoadReport& EmployeeRepository::loadReport() const { return report; }

void EmployeeRepository::snapshot() {
    destroyAll(snapshotItems);
    snapshotItems = cloneAll(items);
    hasSnapshot = true;
}

Result<void> EmployeeRepository::flush() {
    Vector<std::string> lines;
    lines.push_back("#v1");
    for (int i = 0; i < items.getSize(); ++i) lines.push_back(items[i]->toFileString());
    return fileManager.writeAllLinesAtomic(fileName, lines);
}

void EmployeeRepository::restore() {
    if (!hasSnapshot) return;
    destroyAll(items);
    items = cloneAll(snapshotItems);
    rebuildIndexes();
}
