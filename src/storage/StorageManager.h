#pragma once

#include <string>
#include <vector>

namespace aero::storage {
class StorageManager {
public:
    bool Add(const std::wstring& category, const std::wstring& value) const;
    bool Set(const std::wstring& category, const std::vector<std::wstring>& values) const;
    std::vector<std::wstring> Read(const std::wstring& category) const;
};
}