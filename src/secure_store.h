#pragma once

#include <string>
#include <vector>

namespace aero {
bool SaveSecureRecord(const std::wstring& category, const std::wstring& value);
bool SaveSecureRecords(const std::wstring& category, const std::vector<std::wstring>& values);
std::vector<std::wstring> ReadSecureRecords(const std::wstring& category);
}