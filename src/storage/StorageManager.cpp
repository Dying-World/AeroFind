#include "StorageManager.h"

#include "../secure_store.h"

namespace aero::storage {
bool StorageManager::Add(const std::wstring& category, const std::wstring& value) const {
    return SaveSecureRecord(category, value);
}

std::vector<std::wstring> StorageManager::Read(const std::wstring& category) const {
    return ReadSecureRecords(category);
}
}