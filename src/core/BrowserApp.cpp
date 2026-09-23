#include "BrowserApp.h"
#include "AppConfig.h"
#include "UrlUtils.h"
#include "../utils/StringUtils.h"
#include "../utils/Logger.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace aero::core {
BrowserApp::BrowserApp(HINSTANCE instance) : instance_(instance), downloads_(storage_) {}

std::vector<std::wstring> BrowserApp::TabTitles() const { std::vector<std::wstring> result; for (size_t i = 0; i < tabs_.Count(); ++i) result.push_back(tabs_.At(i).title); return result; }

int BrowserApp::Run(int showCommand) {
    WNDCLASSW windowClass{}; windowClass.hInstance = instance_; windowClass.lpfnWndProc = WindowProc; windowClass.lpszClassName = config::WindowClass; windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW); windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); RegisterClassW(&windowClass);
    window_ = CreateWindowExW(0, config::WindowClass, L"AeroFind - Professional Web Browser", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1240, 820, nullptr, nullptr, instance_, this);
    if (!window_) return 1; ShowWindow(window_, showCommand); UpdateWindow(window_);
    MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); } return static_cast<int>(message.wParam);
}

void BrowserApp::StartEnvironment() {
    const std::wstring userData = utils::UserDataFolder();
    CreateCoreWebView2EnvironmentWithOptions(nullptr, userData.c_str(), nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT { if (FAILED(result) || !environment) { utils::LogHResult(L"Create WebView2 environment", result); return result; } environment_ = environment; CreateActiveView(); return S_OK; }).Get());
}

void BrowserApp::CreateActiveView() {
    if (!environment_) return; const size_t tabIndex = tabs_.ActiveIndex(); const unsigned long expectedGeneration = generation_;
    environment_->CreateCoreWebView2Controller(window_, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
        [this, tabIndex, expectedGeneration](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (FAILED(result) || !controller || expectedGeneration != generation_ || tabIndex != tabs_.ActiveIndex()) return result;
            webView_.Attach(controller); ResizeView(); ICoreWebView2* view = webView_.View();
            ComPtr<ICoreWebView2Settings> settings; view->get_Settings(&settings); settings->put_IsStatusBarEnabled(FALSE); settings->put_AreDefaultContextMenusEnabled(TRUE); settings->put_IsZoomControlEnabled(TRUE);
            ComPtr<ICoreWebView2_3> view3; if (SUCCEEDED(view->QueryInterface(IID_PPV_ARGS(&view3)))) view3->SetVirtualHostNameToFolderMapping(L"appassets.local", L"ui", COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
            view->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>([this, tabIndex](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                LPWSTR uri = nullptr; if (SUCCEEDED(args->get_Uri(&uri)) && uri) { tabs_.At(tabIndex).url = uri; storage_.Add(L"history", uri); ui_.SetAddress(uri); CoTaskMemFree(uri); } ui_.SetStatus(L"Loading..."); return S_OK; }).Get(), nullptr);
            view->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT { ui_.SetStatus(L"Ready"); return S_OK; }).Get(), nullptr);
            view->add_DocumentTitleChanged(Callback<ICoreWebView2DocumentTitleChangedEventHandler>([this, tabIndex](ICoreWebView2* changed, IUnknown*) -> HRESULT {
                LPWSTR title = nullptr; if (SUCCEEDED(changed->get_DocumentTitle(&title)) && title) { tabs_.At(tabIndex).title = title[0] ? title : L"New tab"; ui_.Layout(TabTitles()); CoTaskMemFree(title); } return S_OK; }).Get(), nullptr);
            ComPtr<ICoreWebView2_2> view2; if (SUCCEEDED(view->QueryInterface(IID_PPV_ARGS(&view2)))) {
                view2->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
                view2->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](ICoreWebView2*, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
                    ComPtr<ICoreWebView2WebResourceRequest> request; LPWSTR rawUri = nullptr; if (FAILED(args->get_Request(&request)) || !request || FAILED(request->get_Uri(&rawUri)) || !rawUri) return S_OK;
                    const std::wstring uri = rawUri; CoTaskMemFree(rawUri); if (!adBlocker_.IsBlocked(uri)) return S_OK; ComPtr<ICoreWebView2WebResourceResponse> response; environment_->CreateWebResourceResponse(nullptr, 204, L"Blocked", L"", &response); args->put_Response(response.Get()); return S_OK;
                }).Get(), nullptr);
            }
            ComPtr<ICoreWebView2_4> view4; if (SUCCEEDED(view->QueryInterface(IID_PPV_ARGS(&view4)))) view4->add_DownloadStarting(Callback<ICoreWebView2DownloadStartingEventHandler>([this](ICoreWebView2*, ICoreWebView2DownloadStartingEventArgs* args) -> HRESULT {
                ComPtr<ICoreWebView2DownloadOperation> operation; if (SUCCEEDED(args->get_DownloadOperation(&operation)) && operation) { LPWSTR uri = nullptr; if (SUCCEEDED(operation->get_Uri(&uri)) && uri) { downloads_.Record(uri); CoTaskMemFree(uri); } } ui_.SetStatus(L"Download started"); return S_OK;
            }).Get(), nullptr);
            view->Navigate(tabs_.At(tabIndex).url.c_str()); return S_OK;
        }).Get());
}

void BrowserApp::ResizeView() { if (!webView_.Controller()) return; RECT bounds{}; GetClientRect(window_, &bounds); RECT webBounds{0, config::ToolbarHeight, bounds.right, bounds.bottom}; webView_.Controller()->put_Bounds(webBounds); }
void BrowserApp::NavigateFromAddress() { if (!webView_.Ready()) return; tabs_.Active().url = MakeNavigationTarget(ui_.Address()); webView_.View()->Navigate(tabs_.Active().url.c_str()); }
void BrowserApp::NewTab() { const size_t index = tabs_.AddTab(); ui_.CreateTabButton(index, tabs_.At(index).title); SwitchTab(index); }

void BrowserApp::CloseTab(size_t index) {
    if (tabs_.Count() == 1) { tabs_.Active() = {config::StartPage, L"New tab", false}; if (webView_.Ready()) webView_.View()->Navigate(config::StartPage); return; }
    const bool active = index == tabs_.ActiveIndex(); if (active) { ++generation_; webView_.Reset(); }
    DestroyWindow(GetDlgItem(window_, config::TabIdBase + static_cast<int>(index))); tabs_.CloseTab(index); ui_.Layout(TabTitles()); if (active) CreateActiveView();
}

void BrowserApp::SwitchTab(size_t index) { if (index >= tabs_.Count() || index == tabs_.ActiveIndex()) return; ++generation_; webView_.Reset(); tabs_.Activate(index); ui_.SetAddress(tabs_.Active().url); ui_.Layout(TabTitles()); CreateActiveView(); }

void BrowserApp::OpenRecords(const std::wstring& category, const wchar_t* heading) {
    if (!webView_.Ready()) return; std::wstring html = L"<!doctype html><meta charset='utf-8'><style>body{font:14px Tahoma;background:#dce8f1;padding:30px}main{max-width:900px;margin:auto;background:#fff;border:1px solid #7895aa;padding:24px}li{padding:8px;border-bottom:1px solid #c2d1dc;word-break:break-all}</style><main><h1>";
    html += heading; html += L"</h1><ul>"; for (const auto& value : storage_.Read(category)) html += L"<li>" + utils::EscapeHtml(value) + L"</li>"; html += L"</ul></main>"; webView_.View()->NavigateToString(html.c_str());
}

void BrowserApp::SaveBookmark() { storage_.Add(L"bookmarks", tabs_.Active().title + L"\t" + tabs_.Active().url); ui_.SetStatus(L"Bookmark saved"); }
void BrowserApp::OpenSettings() { if (webView_.Ready()) webView_.View()->NavigateToString(L"<h1 style='font:24px Tahoma;padding:30px'>AeroFind settings</h1><p style='font:14px Tahoma;padding:0 30px'>WebView2 profile, DPAPI storage, Google search, memory saver, and ad filtering are active.</p>"); }

void BrowserApp::CreateMenu() {
    HMENU menu = ::CreateMenu(); HMENU file = CreatePopupMenu(); AppendMenuW(file, MF_STRING, config::MenuNewTab, L"New tab\tCtrl+T"); AppendMenuW(file, MF_STRING, config::MenuCloseTab, L"Close tab\tCtrl+W"); AppendMenuW(file, MF_STRING, config::MenuExit, L"Exit"); AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"&File");
    HMENU history = CreatePopupMenu(); AppendMenuW(history, MF_STRING, config::MenuHistory, L"History\tCtrl+H"); AppendMenuW(history, MF_STRING, config::MenuDownloads, L"Downloads\tCtrl+J"); AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(history), L"Histor&y");
    HMENU bookmarks = CreatePopupMenu(); AppendMenuW(bookmarks, MF_STRING, config::MenuBookmark, L"Save bookmark\tCtrl+D"); AppendMenuW(bookmarks, MF_STRING, config::MenuBookmarks, L"Bookmark manager\tCtrl+Shift+B"); AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(bookmarks), L"&Bookmarks");
    HMENU tools = CreatePopupMenu(); AppendMenuW(tools, MF_STRING, config::MenuSettings, L"Settings"); AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(tools), L"&Tools"); SetMenu(window_, menu);
}

LRESULT CALLBACK BrowserApp::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    BrowserApp* app = reinterpret_cast<BrowserApp*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) { const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam); app = static_cast<BrowserApp*>(create->lpCreateParams); SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app)); app->window_ = window; }
    return app ? app->HandleMessage(window, message, wParam, lParam) : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT BrowserApp::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: ui_.Create(window, instance_); ui_.SetAddressEnterHandler([this] { NavigateFromAddress(); }); CreateMenu();
        RegisterHotKey(window, 1, MOD_CONTROL | MOD_NOREPEAT, 'T'); RegisterHotKey(window, 2, MOD_CONTROL | MOD_NOREPEAT, 'W');
        RegisterHotKey(window, 3, MOD_CONTROL | MOD_NOREPEAT, VK_TAB); RegisterHotKey(window, 4, MOD_CONTROL | MOD_NOREPEAT, 'L');
        RegisterHotKey(window, 5, MOD_NOREPEAT, VK_F5); RegisterHotKey(window, 6, MOD_CONTROL | MOD_NOREPEAT, 'H');
        RegisterHotKey(window, 7, MOD_CONTROL | MOD_NOREPEAT, 'J'); RegisterHotKey(window, 8, MOD_CONTROL | MOD_NOREPEAT, 'D');
        RegisterHotKey(window, 9, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'B'); StartEnvironment(); return 0;
    case WM_SIZE: ui_.Layout(TabTitles()); ResizeView(); return 0;
    case WM_PAINT: { PAINTSTRUCT paint{}; HDC dc = BeginPaint(window, &paint); ui_.Paint(dc); EndPaint(window, &paint); return 0; }
    case WM_COMMAND: { const int id = LOWORD(wParam); if (id == config::BackId && webView_.Ready()) webView_.View()->GoBack(); else if (id == config::ForwardId && webView_.Ready()) webView_.View()->GoForward(); else if (id == config::ReloadId && webView_.Ready()) webView_.View()->Reload(); else if (id == config::HomeId && webView_.Ready()) webView_.View()->Navigate(config::StartPage); else if (id == config::NewTabId || id == config::MenuNewTab) NewTab(); else if (id == config::MenuCloseTab) CloseTab(tabs_.ActiveIndex()); else if (id == config::MenuExit) PostMessageW(window, WM_CLOSE, 0, 0); else if (id == config::MenuHistory) OpenRecords(L"history", L"Browsing history"); else if (id == config::MenuDownloads) OpenRecords(L"downloads", L"Downloads"); else if (id == config::MenuBookmark) SaveBookmark(); else if (id == config::MenuBookmarks) OpenRecords(L"bookmarks", L"Bookmarks"); else if (id == config::MenuSettings) OpenSettings(); else if (id >= config::TabIdBase && id < config::TabIdBase + static_cast<int>(tabs_.Count())) SwitchTab(id - config::TabIdBase); return 0; }
    case WM_HOTKEY: if (wParam == 1) NewTab(); else if (wParam == 2) CloseTab(tabs_.ActiveIndex()); else if (wParam == 3) SwitchTab((tabs_.ActiveIndex() + 1) % tabs_.Count()); else if (wParam == 4) { SetFocus(ui_.AddressControl()); SendMessageW(ui_.AddressControl(), EM_SETSEL, 0, -1); } else if (wParam == 5 && webView_.Ready()) webView_.View()->Reload(); else if (wParam == 6) OpenRecords(L"history", L"Browsing history"); else if (wParam == 7) OpenRecords(L"downloads", L"Downloads"); else if (wParam == 8) SaveBookmark(); else if (wParam == 9) OpenRecords(L"bookmarks", L"Bookmarks"); return 0;
    case WM_DESTROY: for (int id = 1; id <= 9; ++id) UnregisterHotKey(window, id); webView_.Reset(); PostQuitMessage(0); return 0;
    default: return DefWindowProcW(window, message, wParam, lParam);
    }
}
}