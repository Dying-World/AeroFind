#include <windows.h>
#include <wrl.h>
#include <WebView2.h>

#include <cstddef>
#include <cwctype>
#include <string>
#include <utility>
#include <vector>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {
constexpr wchar_t kWindowClass[] = L"AeroFindWindow";
constexpr wchar_t kStartPage[] = L"https://appassets.local/index.html";
constexpr int kTabBarHeight = 34;
constexpr int kToolbarHeight = 88;
constexpr int kAddressId = 1001;
constexpr int kBackId = 1002;
constexpr int kForwardId = 1003;
constexpr int kReloadId = 1004;
constexpr int kHomeId = 1005;
constexpr int kNewTabId = 1006;
constexpr int kStatusId = 1007;
constexpr int kTabIdBase = 2000;

struct Tab {
    std::wstring url = kStartPage;
    std::wstring title = L"New tab";
};

HWND g_window = nullptr;
HWND g_address = nullptr;
HWND g_status = nullptr;
WNDPROC g_originalAddressProc = nullptr;
ComPtr<ICoreWebView2Environment> g_environment;
ComPtr<ICoreWebView2Controller> g_controller;
ComPtr<ICoreWebView2> g_webview;
std::vector<Tab> g_tabs;
size_t g_activeTab = 0;
unsigned long g_generation = 0;

std::wstring Trim(std::wstring value) {
    while (!value.empty() && iswspace(value.front())) value.erase(value.begin());
    while (!value.empty() && iswspace(value.back())) value.pop_back();
    return value;
}

std::wstring MakeTarget(std::wstring input) {
    input = Trim(std::move(input));
    if (input.empty()) return kStartPage;
    if (input.find(L"://") != std::wstring::npos || input.rfind(L"about:", 0) == 0) return input;
    return L"https://www.bing.com/search?q=" + input;
}

void SetStatus(const wchar_t* text) {
    if (g_status) SetWindowTextW(g_status, text);
}

void ResizeBrowser() {
    if (!g_window || !g_controller) return;
    RECT bounds{};
    GetClientRect(g_window, &bounds);
    RECT webBounds{0, kToolbarHeight, bounds.right, bounds.bottom};
    g_controller->put_Bounds(webBounds);
}

void UpdateAddress() {
    if (g_address && g_activeTab < g_tabs.size()) SetWindowTextW(g_address, g_tabs[g_activeTab].url.c_str());
}

void LayoutChrome() {
    RECT bounds{};
    GetClientRect(g_window, &bounds);
    for (size_t index = 0; index < g_tabs.size(); ++index) {
        HWND button = GetDlgItem(g_window, kTabIdBase + static_cast<int>(index));
        if (button) {
            SetWindowPos(button, nullptr, 8 + static_cast<int>(index) * 170, 5, 162, 25, SWP_NOZORDER);
            SetWindowTextW(button, g_tabs[index].title.c_str());
        }
    }
    HWND newTab = GetDlgItem(g_window, kNewTabId);
    if (newTab) SetWindowPos(newTab, nullptr, 12 + static_cast<int>(g_tabs.size()) * 170, 5, 28, 25, SWP_NOZORDER);
    if (g_address) SetWindowPos(g_address, nullptr, 172, kTabBarHeight + 9,
        max(220, bounds.right - 300), 28, SWP_NOZORDER);
    if (g_status) SetWindowPos(g_status, nullptr, 8, kToolbarHeight - 20, 160, 18, SWP_NOZORDER);
    ResizeBrowser();
}

void CreateTabButton(size_t index) {
    CreateWindowW(L"BUTTON", g_tabs[index].title.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        8, 5, 162, 25, g_window, reinterpret_cast<HMENU>(kTabIdBase + static_cast<int>(index)), nullptr, nullptr);
}

void DestroyActiveWebView() {
    ++g_generation;
    if (g_controller) g_controller->put_IsVisible(FALSE);
    g_webview.Reset();
    g_controller.Reset();
}

void NavigateFromAddress() {
    if (!g_webview || !g_address || g_activeTab >= g_tabs.size()) return;
    const int length = GetWindowTextLengthW(g_address);
    std::wstring input(length, L'\0');
    GetWindowTextW(g_address, input.data(), length + 1);
    g_tabs[g_activeTab].url = MakeTarget(std::move(input));
    g_webview->Navigate(g_tabs[g_activeTab].url.c_str());
    SetStatus(L"Loading...");
}

void CreateActiveWebView() {
    if (!g_environment || g_activeTab >= g_tabs.size()) return;
    const size_t tabIndex = g_activeTab;
    const unsigned long generation = g_generation;
    g_environment->CreateCoreWebView2Controller(g_window,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [tabIndex, generation](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                if (FAILED(result) || !controller || generation != g_generation || tabIndex != g_activeTab) return result;
                g_controller = controller;
                g_controller->get_CoreWebView2(&g_webview);
                ResizeBrowser();

                ComPtr<ICoreWebView2Settings> settings;
                g_webview->get_Settings(&settings);
                settings->put_IsStatusBarEnabled(FALSE);
                settings->put_AreDefaultContextMenusEnabled(TRUE);
                settings->put_IsZoomControlEnabled(TRUE);

                ComPtr<ICoreWebView2_3> webview3;
                if (SUCCEEDED(g_webview.As(&webview3))) {
                    webview3->SetVirtualHostNameToFolderMapping(
                        L"appassets.local", L"ui", COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
                }
                g_webview->add_NavigationStarting(
                    Callback<ICoreWebView2NavigationStartingEventHandler>(
                        [tabIndex](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                            LPWSTR uri = nullptr;
                            if (SUCCEEDED(args->get_Uri(&uri)) && uri && tabIndex < g_tabs.size()) {
                                g_tabs[tabIndex].url = uri;
                                UpdateAddress();
                                CoTaskMemFree(uri);
                            }
                            return S_OK;
                        }).Get(), nullptr);
                g_webview->add_NavigationCompleted(
                    Callback<ICoreWebView2NavigationCompletedEventHandler>(
                        [](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT {
                            SetStatus(L"Ready");
                            return S_OK;
                        }).Get(), nullptr);
                g_webview->add_DocumentTitleChanged(
                    Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
                        [tabIndex](ICoreWebView2* webview, IUnknown*) -> HRESULT {
                            LPWSTR title = nullptr;
                            if (SUCCEEDED(webview->get_DocumentTitle(&title)) && title && tabIndex < g_tabs.size()) {
                                g_tabs[tabIndex].title = title[0] ? title : L"New tab";
                                LayoutChrome();
                                CoTaskMemFree(title);
                            }
                            return S_OK;
                        }).Get(), nullptr);
                ComPtr<ICoreWebView2_4> webview4;
                if (SUCCEEDED(g_webview.As(&webview4))) {
                    webview4->add_DownloadStarting(
                        Callback<ICoreWebView2DownloadStartingEventHandler>(
                            [](ICoreWebView2*, ICoreWebView2DownloadStartingEventArgs* args) -> HRESULT {
                                args->put_Handled(FALSE);
                                SetStatus(L"Download started");
                                return S_OK;
                            }).Get(), nullptr);
                }
                g_webview->Navigate(g_tabs[tabIndex].url.c_str());
                return S_OK;
            }).Get());
}

void SwitchTab(size_t index) {
    if (index >= g_tabs.size() || index == g_activeTab) return;
    DestroyActiveWebView();
    g_activeTab = index;
    UpdateAddress();
    LayoutChrome();
    CreateActiveWebView();
}

void NewTab() {
    g_tabs.emplace_back();
    CreateTabButton(g_tabs.size() - 1);
    SwitchTab(g_tabs.size() - 1);
}

void CloseTab(size_t index) {
    if (g_tabs.size() == 1) {
        g_tabs[0] = Tab{};
        if (g_webview) g_webview->Navigate(kStartPage);
        UpdateAddress();
        return;
    }
    const bool closingActive = index == g_activeTab;
    if (closingActive) DestroyActiveWebView();
    DestroyWindow(GetDlgItem(g_window, kTabIdBase + static_cast<int>(index)));
    g_tabs.erase(g_tabs.begin() + static_cast<ptrdiff_t>(index));
    if (index < g_activeTab) --g_activeTab;
    if (g_activeTab >= g_tabs.size()) g_activeTab = g_tabs.size() - 1;
    LayoutChrome();
    if (closingActive) CreateActiveWebView();
}

LRESULT CALLBACK AddressProc(HWND address, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_RETURN) {
        NavigateFromAddress();
        return 0;
    }
    return CallWindowProcW(g_originalAddressProc, address, message, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_window = window;
        g_tabs.emplace_back();
        g_status = CreateWindowW(L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE | SS_LEFT,
            8, kToolbarHeight - 20, 160, 18, window, reinterpret_cast<HMENU>(kStatusId), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE, 8, kTabBarHeight + 9, 35, 28, window,
            reinterpret_cast<HMENU>(kBackId), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE, 48, kTabBarHeight + 9, 35, 28, window,
            reinterpret_cast<HMENU>(kForwardId), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"R", WS_CHILD | WS_VISIBLE, 88, kTabBarHeight + 9, 35, 28, window,
            reinterpret_cast<HMENU>(kReloadId), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Home", WS_CHILD | WS_VISIBLE, 128, kTabBarHeight + 9, 40, 28, window,
            reinterpret_cast<HMENU>(kHomeId), nullptr, nullptr);
        g_address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", kStartPage,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 172, kTabBarHeight + 9, 600, 28, window,
            reinterpret_cast<HMENU>(kAddressId), nullptr, nullptr);
        g_originalAddressProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            g_address, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(AddressProc)));
        CreateTabButton(0);
        CreateWindowW(L"BUTTON", L"+", WS_CHILD | WS_VISIBLE, 185, 5, 28, 25, window,
            reinterpret_cast<HMENU>(kNewTabId), nullptr, nullptr);
        RegisterHotKey(window, 1, MOD_CONTROL | MOD_NOREPEAT, 'T');
        RegisterHotKey(window, 2, MOD_CONTROL | MOD_NOREPEAT, 'W');
        RegisterHotKey(window, 3, MOD_CONTROL | MOD_NOREPEAT, VK_TAB);
        RegisterHotKey(window, 4, MOD_CONTROL | MOD_NOREPEAT, 'L');
        RegisterHotKey(window, 5, MOD_NOREPEAT, VK_F5);
        CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                    if (FAILED(result) || !environment) return result;
                    g_environment = environment;
                    CreateActiveWebView();
                    return S_OK;
                }).Get());
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT bounds{};
        GetClientRect(window, &bounds);
        HBRUSH tabBrush = CreateSolidBrush(RGB(49, 91, 128));
        HBRUSH toolbarBrush = CreateSolidBrush(RGB(204, 218, 231));
        RECT tabs{0, 0, bounds.right, kTabBarHeight};
        RECT toolbar{0, kTabBarHeight, bounds.right, kToolbarHeight};
        FillRect(dc, &tabs, tabBrush);
        FillRect(dc, &toolbar, toolbarBrush);
        DeleteObject(tabBrush);
        DeleteObject(toolbarBrush);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_SIZE:
        LayoutChrome();
        return 0;
    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        if (id == kBackId && g_webview) g_webview->GoBack();
        else if (id == kForwardId && g_webview) g_webview->GoForward();
        else if (id == kReloadId && g_webview) g_webview->Reload();
        else if (id == kHomeId && g_webview) g_webview->Navigate(kStartPage);
        else if (id == kNewTabId) NewTab();
        else if (id >= kTabIdBase && id < kTabIdBase + static_cast<int>(g_tabs.size())) SwitchTab(id - kTabIdBase);
        return 0;
    }
    case WM_HOTKEY:
        if (wParam == 1) NewTab();
        else if (wParam == 2) CloseTab(g_activeTab);
        else if (wParam == 3 && !g_tabs.empty()) SwitchTab((g_activeTab + 1) % g_tabs.size());
        else if (wParam == 4) {
            SetFocus(g_address);
            SendMessageW(g_address, EM_SETSEL, 0, -1);
        } else if (wParam == 5 && g_webview) g_webview->Reload();
        return 0;
    case WM_DESTROY:
        for (int id = 1; id <= 5; ++id) UnregisterHotKey(window, id);
        DestroyActiveWebView();
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    WNDCLASSW windowClass{};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.lpszClassName = kWindowClass;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&windowClass);
    HWND window = CreateWindowExW(0, kWindowClass, L"AeroFind - Retro Web Browser", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1240, 820, nullptr, nullptr, instance, nullptr);
    if (!window) {
        CoUninitialize();
        return 1;
    }
    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    CoUninitialize();
    return static_cast<int>(message.wParam);
}