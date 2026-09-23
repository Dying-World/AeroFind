#pragma once

#include <string>

namespace aero::storage { class StorageManager; }

namespace aero::network {
class DownloadManager {
public:
    explicit DownloadManager(storage::StorageManager& storage);
    void Record(const std::wstring& uri);
private:
    storage::StorageManager& storage_;
};
}