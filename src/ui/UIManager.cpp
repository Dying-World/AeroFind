#include "UIManager.h"
#include "../core/AppConfig.h"
#include <algorithm>
#include <utility>

namespace aero::ui {
UIManager* UIManager::activeInstance_ = nullptr;

bool UIManager::Create(HWND window, HINSTANCE) {
    window_ = window; activeInstance_ = this;
    status_ = CreateWindowW(L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE | SS_LEFT, 8, config::ToolbarHeight - 20, 160, 18, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::StatusId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE, 8, config::TabBarHeight + 9, 35, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::BackId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE, 48, config::TabBarHeight + 9, 35, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::ForwardId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"R", WS_CHILD | WS_VISIBLE, 88, config::TabBarHeight + 9, 35, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::ReloadId)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"Home", WS_CHILD | WS_VISIBLE, 128, config::TabBarHeight + 9, 40, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::HomeId)), nullptr, nullptr);
    address_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", config::StartPage, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 172, config::TabBarHeight + 9, 600, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::AddressId)), nullptr, nullptr);
    originalAddressProc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(address_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(AddressProc)));
    CreateWindowW(L"BUTTON", L"+", WS_CHILD | WS_VISIBLE, 185, 5, 28, 25, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::NewTabId)), nullptr, nullptr);
    return address_ != nullptr;
}

void UIManager::Layout(const std::vector<std::wstring>& titles) {
    RECT bounds{}; GetClientRect(window_, &bounds);
    for (size_t index = 0; index < titles.size(); ++index) {
        HWND tab = GetDlgItem(window_, config::TabIdBase + static_cast<int>(index));
        if (tab) { SetWindowPos(tab, nullptr, 8 + static_cast<int>(index) * 170, 5, 162, 25, SWP_NOZORDER); SetWindowTextW(tab, titles[index].c_str()); }
    }
    HWND newTab = GetDlgItem(window_, config::NewTabId);
    if (newTab) SetWindowPos(newTab, nullptr, 12 + static_cast<int>(titles.size()) * 170, 5, 28, 25, SWP_NOZORDER);
    if (address_) SetWindowPos(address_, nullptr, 172, config::TabBarHeight + 9, std::max(220L, static_cast<long>(bounds.right - 300)), 28, SWP_NOZORDER);
    if (status_) SetWindowPos(status_, nullptr, 8, config::ToolbarHeight - 20, 160, 18, SWP_NOZORDER);
}

void UIManager::Paint(HDC dc) const {
    RECT bounds{}; GetClientRect(window_, &bounds);
    HBRUSH tabs = CreateSolidBrush(RGB(205, 214, 223)); HBRUSH toolbar = CreateSolidBrush(RGB(235, 239, 243));
    RECT tabBounds{0, 0, bounds.right, config::TabBarHeight}; RECT toolbarBounds{0, config::TabBarHeight, bounds.right, config::ToolbarHeight};
    FillRect(dc, &tabBounds, tabs); FillRect(dc, &toolbarBounds, toolbar); DeleteObject(tabs); DeleteObject(toolbar);
}

void UIManager::CreateTabButton(size_t index, const std::wstring& title) { CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, 5, 162, 25, window_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(config::TabIdBase + static_cast<int>(index))), nullptr, nullptr); }
void UIManager::SetStatus(const std::wstring& text) const { if (status_) SetWindowTextW(status_, text.c_str()); }
void UIManager::SetAddress(const std::wstring& address) const { if (address_) SetWindowTextW(address_, address.c_str()); }
std::wstring UIManager::Address() const { const int length = address_ ? GetWindowTextLengthW(address_) : 0; std::wstring value(length, L'\0'); if (address_) GetWindowTextW(address_, value.data(), length + 1); return value; }
HWND UIManager::AddressControl() const noexcept { return address_; }
void UIManager::SetAddressEnterHandler(std::function<void()> handler) { addressEnterHandler_ = std::move(handler); }

LRESULT CALLBACK UIManager::AddressProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_RETURN && activeInstance_ && activeInstance_->addressEnterHandler_) { activeInstance_->addressEnterHandler_(); return 0; }
    return CallWindowProcW(activeInstance_->originalAddressProc_, window, message, wParam, lParam);
}
}