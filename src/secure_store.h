#pragma once

#include <string>
#include <vector>

namespace aero {
bool SaveSecureRecord(const std::wstring& category, const std::wstring& value);
std::vector<std::wstring> ReadSecureRecords(const std::wstring& category);
}