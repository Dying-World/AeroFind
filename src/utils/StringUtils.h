#pragma once

#include <string>

namespace aero::utils {
std::wstring Trim(std::wstring value);
std::wstring EscapeHtml(const std::wstring& value);
std::wstring UserDataFolder();
}