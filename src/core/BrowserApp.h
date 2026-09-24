#pragma once

#include "WebViewInstance.h"
#include "../network/AdBlocker.h"
#include "../network/DownloadManager.h"
#include "../storage/StorageManager.h"
#include "../tabs/TabManager.h"
#include "../ui/UIManager.h"
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>

namespace aero::core {
class BrowserApp {
public:
    explicit BrowserApp(HINSTANCE instance);
    int Run(int showCommand);
    LRESULT HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
private:
    void StartEnvironment(); void CreateActiveView(); void ResizeView(); void NavigateFromAddress();
    void NewTab(); void CloseTab(size_t index); void SwitchTab(size_t index); void OpenRecords(const std::wstring& category, const wchar_t* heading);
    void OpenHistoryPage(); void OpenDownloadsPage(); void OpenMediaPage(); void SaveBookmark(); void OpenSettings(); void CreateMenu(); void SaveSession(); void LoadSession(); std::vector<std::wstring> TabTitles() const;
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    HINSTANCE instance_ = nullptr; HWND window_ = nullptr;
    Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
    WebViewInstance webView_; tabs::TabManager tabs_; ui::UIManager ui_; storage::StorageManager storage_;
    network::AdBlocker adBlocker_; network::DownloadManager downloads_; unsigned long generation_ = 0;
};
}