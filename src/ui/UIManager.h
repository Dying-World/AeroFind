#pragma once
#include <windows.h>
#include <functional>
#include <string>
#include <vector>

namespace aero::ui {
class UIManager {
public:
    bool Create(HWND window, HINSTANCE instance);
    void Layout(const std::vector<std::wstring>& titles);
    void Paint(HDC dc) const;
    void CreateTabButton(size_t index, const std::wstring& title);
    void SetStatus(const std::wstring& text) const;
    void SetAddress(const std::wstring& address) const;
    std::wstring Address() const;
    HWND AddressControl() const noexcept;
    void SetAddressEnterHandler(std::function<void()> handler);
private:
    static LRESULT CALLBACK AddressProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    HWND window_ = nullptr;
    HWND address_ = nullptr;
    HWND status_ = nullptr;
    WNDPROC originalAddressProc_ = nullptr;
    std::function<void()> addressEnterHandler_;
    static UIManager* activeInstance_;
};
}