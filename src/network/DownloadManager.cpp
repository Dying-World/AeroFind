#include "DownloadManager.h"

#include "../storage/StorageManager.h"

namespace aero::network {
DownloadManager::DownloadManager(storage::StorageManager& storage) : storage_(storage) {}
void DownloadManager::Record(const std::wstring& uri) { storage_.Add(L"downloads", uri); }
}