#include "UIManager.h"
#include "../core/AppConfig.h"
#include <algorithm>
#include <utility>
#include <uxtheme.h>

namespace aero::ui {
UIManager* UIManager::activeInstance_ = nullptr;

bool UIManager::Create(HWND window, HINSTANCE) {
    window_ = window; activeInstance_ = this;

    status_ = CreateWindowW(L"STATIC", L"Готово", WS_CHILD | WS_VISIBLE | SS_LEFT, 20, config::ToolbarHeight - 18, 180, 18, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::StatusId)), nullptr, nullptr);
    SetWindowTheme(status_, L"Explorer", nullptr);

    CreateWindowW(L"BUTTON", L"←", WS_CHILD | WS_VISIBLE | BS_FLAT, 12, 12, 34, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::BackId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"→", WS_CHILD | WS_VISIBLE | BS_FLAT, 52, 12, 34, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::ForwardId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"↻", WS_CHILD | WS_VISIBLE | BS_FLAT, 92, 12, 34, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::ReloadId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"⌂", WS_CHILD | WS_VISIBLE | BS_FLAT, 132, 12, 40, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::HomeId)), nullptr, nullptr);

    address_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", config::StartPage, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 188, 11, 660, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::AddressId)), nullptr, nullptr);
    originalAddressProc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(address_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(AddressProc)));
    SetWindowTheme(address_, L"Explorer", nullptr);

    return address_ != nullptr;
}

void UIManager::Layout(const std::vector<std::wstring>& titles) {
    RECT bounds{}; GetClientRect(window_, &bounds);
    (void)titles;
    if (address_) SetWindowPos(address_, nullptr, 188, 11, std::max(240L, static_cast<long>(bounds.right - 270)), 30, SWP_NOZORDER);
    if (status_) SetWindowPos(status_, nullptr, 20, config::ToolbarHeight - 18, 180, 18, SWP_NOZORDER);
}

void UIManager::Paint(HDC dc) const {
    RECT bounds{}; GetClientRect(window_, &bounds);
    HBRUSH toolbar = CreateSolidBrush(RGB(37, 41, 49));
    RECT toolbarBounds{0, 0, bounds.right, config::ToolbarHeight};
    FillRect(dc, &toolbarBounds, toolbar);
    DeleteObject(toolbar);
}

void UIManager::CreateTabButton(size_t index, const std::wstring& title) {
    (void)index;
    (void)title;
}
void UIManager::SetStatus(const std::wstring& text) const { if (status_) SetWindowTextW(status_, text.c_str()); }
void UIManager::SetAddress(const std::wstring& address) const { if (address_) SetWindowTextW(address_, address.c_str()); }
std::wstring UIManager::Address() const { const int length = address_ ? GetWindowTextLengthW(address_) : 0; std::wstring value(length, L'\0'); if (address_) GetWindowTextW(address_, value.data(), length + 1); return value; }
HWND UIManager::AddressControl() const noexcept { return address_; }
void UIManager::SetAddressEnterHandler(std::function<void()> handler) { addressEnterHandler_ = std::move(handler); }
void UIManager::SetToolbarVisible(bool visible) {
    EnumChildWindows(window_, [](HWND child, LPARAM parameter) -> BOOL {
        ShowWindow(child, parameter ? SW_SHOW : SW_HIDE);
        return TRUE;
    }, visible ? TRUE : FALSE);
}

LRESULT CALLBACK UIManager::AddressProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_RETURN && activeInstance_ && activeInstance_->addressEnterHandler_) { activeInstance_->addressEnterHandler_(); return 0; }
    return CallWindowProcW(activeInstance_->originalAddressProc_, window, message, wParam, lParam);
}
}