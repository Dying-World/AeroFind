#pragma once

#include <wrl.h>
#include <WebView2.h>

namespace aero::core {
class WebViewInstance {
public:
    void Attach(ICoreWebView2Controller* controller);
    void Reset();
    bool Ready() const noexcept;
    ICoreWebView2* View() const noexcept;
    ICoreWebView2Controller* Controller() const noexcept;
private:
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2> view_;
};
}