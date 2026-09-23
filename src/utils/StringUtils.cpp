#include "StringUtils.h"

#include <windows.h>
#include <cwctype>

namespace aero::utils {
std::wstring Trim(std::wstring value) {
    while (!value.empty() && iswspace(value.front())) value.erase(value.begin());
    while (!value.empty() && iswspace(value.back())) value.pop_back();
    return value;
}

std::wstring EscapeHtml(const std::wstring& value) {
    std::wstring escaped;
    for (wchar_t character : value) {
        if (character == L'&') escaped += L"&amp;";
        else if (character == L'<') escaped += L"&lt;";
        else if (character == L'>') escaped += L"&gt;";
        else if (character == L'\"') escaped += L"&quot;";
        else escaped += character;
    }
    return escaped;
}

std::wstring UserDataFolder() {
    wchar_t localAppData[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    std::wstring path = length ? localAppData : L".";
    path += L"\\AeroFind\\WebView2";
    CreateDirectoryW((path.substr(0, path.rfind(L'\\'))).c_str(), nullptr);
    CreateDirectoryW(path.c_str(), nullptr);
    return path;
}
}