#include "secure_store.h"

#include <windows.h>
#include <wincrypt.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
std::filesystem::path VaultPath() {
    wchar_t localAppData[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    std::filesystem::path path = length ? std::filesystem::path(localAppData) : std::filesystem::temp_directory_path();
    path /= L"AeroFind";
    std::filesystem::create_directories(path);
    return path / L"vault.bin";
}

std::string Protect(const std::string& plain) {
    DATA_BLOB input{static_cast<DWORD>(plain.size()), reinterpret_cast<BYTE*>(const_cast<char*>(plain.data()))};
    DATA_BLOB output{};
    if (!CryptProtectData(&input, L"AeroFind vault", nullptr, nullptr, nullptr, 0, &output)) return {};
    std::string result(reinterpret_cast<char*>(output.pbData), output.cbData);
    LocalFree(output.pbData);
    return result;
}

std::string Unprotect(const std::string& encrypted) {
    DATA_BLOB input{static_cast<DWORD>(encrypted.size()), reinterpret_cast<BYTE*>(const_cast<char*>(encrypted.data()))};
    DATA_BLOB output{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, 0, &output)) return {};
    std::string result(reinterpret_cast<char*>(output.pbData), output.cbData);
    LocalFree(output.pbData);
    return result;
}

std::string ToUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int length = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
    return result;
}

std::wstring FromUtf8(const std::string& value) {
    if (value.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length);
    return result;
}

std::string ReadVault() {
    std::ifstream file(VaultPath(), std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return Unprotect(buffer.str());
}
}

namespace aero {
bool SaveSecureRecord(const std::wstring& category, const std::wstring& value) {
    std::vector<std::wstring> values = ReadSecureRecords(category);
    values.push_back(value);
    return SaveSecureRecords(category, values);
}

bool SaveSecureRecords(const std::wstring& category, const std::vector<std::wstring>& values) {
    std::string plain = ReadVault();
    std::istringstream previousLines(plain);
    std::string line;
    const std::string prefix = ToUtf8(category) + "\t";
    std::string filtered;
    while (std::getline(previousLines, line)) {
        if (line.rfind(prefix, 0) != 0) filtered += line + "\n";
    }

    for (const auto& value : values) {
        filtered += prefix + ToUtf8(value) + "\n";
    }

    const std::string encrypted = Protect(filtered);
    if (encrypted.empty()) return false;
    std::ofstream file(VaultPath(), std::ios::binary | std::ios::trunc);
    file.write(encrypted.data(), static_cast<std::streamsize>(encrypted.size()));
    return file.good();
}

std::vector<std::wstring> ReadSecureRecords(const std::wstring& category) {
    std::vector<std::wstring> records;
    std::istringstream lines(ReadVault());
    const std::string prefix = ToUtf8(category) + "\t";
    std::string line;
    while (std::getline(lines, line)) {
        if (line.rfind(prefix, 0) == 0) records.push_back(FromUtf8(line.substr(prefix.size())));
    }
    return records;
}
}