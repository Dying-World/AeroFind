#include "Logger.h"

#include <windows.h>

namespace aero::utils {
void Log(const std::wstring& message) {
    std::wstring line = L"[AeroFind] " + message + L"\n";
    OutputDebugStringW(line.c_str());
}

void LogHResult(const std::wstring& operation, long result) {
    Log(operation + L" HRESULT=" + std::to_wstring(result));
}
}