#include <windows.h>
#include <wrl.h>
#include <WebView2.h>

#include <cwctype>
#include <string>
#include <utility>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {
constexpr wchar_t kWindowClass[] = L"AeroFindWindow";
constexpr int kToolbarHeight = 52;
constexpr int kControlIdAddress = 1001;
constexpr int kControlIdBack = 1002;
constexpr int kControlIdForward = 1003;
constexpr int kControlIdReload = 1004;

HWND g_window = nullptr;
HWND g_address = nullptr;
WNDPROC g_originalAddressProc = nullptr;
ComPtr<ICoreWebView2Controller> g_controller;
ComPtr<ICoreWebView2> g_webview;

std::wstring MakeNavigationTarget(std::wstring input) {
    while (!input.empty() && iswspace(input.front())) input.erase(input.begin());
    while (!input.empty() && iswspace(input.back())) input.pop_back();
    if (input.empty()) return L"https://appassets.local/index.html";
    if (input.find(L"://") != std::wstring::npos || input.rfind(L"about:", 0) == 0) return input;
    return L"https://www.bing.com/search?q=" + input;
}

void NavigateFromAddress() {
    if (!g_webview || !g_address) return;
    const int length = GetWindowTextLengthW(g_address);
    std::wstring input(length, L'\0');
    GetWindowTextW(g_address, input.data(), length + 1);
    const auto target = MakeNavigationTarget(std::move(input));
    g_webview->Navigate(target.c_str());
}

void ResizeBrowser() {
    if (!g_window || !g_controller) return;
    RECT bounds{};
    GetClientRect(g_window, &bounds);
    RECT webBounds{0, kToolbarHeight, bounds.right, bounds.bottom};
    g_controller->put_Bounds(webBounds);
}

LRESULT CALLBACK AddressProc(HWND address, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_RETURN) {
        NavigateFromAddress();
        return 0;
    }
    return CallWindowProcW(g_originalAddressProc, address, message, wParam, lParam);
}

void CreateBrowser() {
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                if (FAILED(result) || !environment) return result;
                return environment->CreateCoreWebView2Controller(g_window,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT controllerResult, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(controllerResult) || !controller) return controllerResult;
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
                            g_webview->Navigate(L"https://appassets.local/index.html");
                            return S_OK;
                        }).Get());
            }).Get());
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        g_window = window;
        CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE, 12, 12, 28, 28, window,
            reinterpret_cast<HMENU>(kControlIdBack), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE, 44, 12, 28, 28, window,
            reinterpret_cast<HMENU>(kControlIdForward), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"R", WS_CHILD | WS_VISIBLE, 76, 12, 36, 28, window,
            reinterpret_cast<HMENU>(kControlIdReload), nullptr, nullptr);
        g_address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"https://appassets.local/index.html",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 122, 12, 520, 28, window,
            reinterpret_cast<HMENU>(kControlIdAddress), nullptr, nullptr);
        g_originalAddressProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            g_address, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(AddressProc)));
        CreateBrowser();
        return 0;
    case WM_SIZE:
        if (g_address) {
            RECT bounds{};
            GetClientRect(window, &bounds);
            SetWindowPos(g_address, nullptr, 122, 12, max(200, bounds.right - 134), 28, SWP_NOZORDER);
        }
        ResizeBrowser();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kControlIdBack && g_webview) g_webview->GoBack();
        if (LOWORD(wParam) == kControlIdForward && g_webview) g_webview->GoForward();
        if (LOWORD(wParam) == kControlIdReload && g_webview) g_webview->Reload();
        return 0;
    case WM_KEYDOWN:
        if (wParam == 'L' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SetFocus(g_address);
            SendMessageW(g_address, EM_SETSEL, 0, -1);
        }
        return 0;
    case WM_DESTROY:
        g_webview.Reset();
        g_controller.Reset();
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

    HWND window = CreateWindowExW(0, kWindowClass, L"AeroFind", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 780, nullptr, nullptr, instance, nullptr);
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