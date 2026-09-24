#include "StorageManager.h"

#include "../secure_store.h"

namespace aero::storage {
bool StorageManager::Add(const std::wstring& category, const std::wstring& value) const {
    return SaveSecureRecord(category, value);
}

bool StorageManager::Set(const std::wstring& category, const std::vector<std::wstring>& values) const {
    return SaveSecureRecords(category, values);
}

std::vector<std::wstring> StorageManager::Read(const std::wstring& category) const {
    return ReadSecureRecords(category);
}
}