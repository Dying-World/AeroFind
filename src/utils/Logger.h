#pragma once

#include <string>

namespace aero::utils {
void Log(const std::wstring& message);
void LogHResult(const std::wstring& operation, long result);
}