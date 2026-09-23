#include "WebViewInstance.h"

namespace aero::core {
void WebViewInstance::Attach(ICoreWebView2Controller* controller) {
    controller_ = controller;
    view_.Reset();
    if (controller_) controller_->get_CoreWebView2(&view_);
}

void WebViewInstance::Reset() {
    if (controller_) controller_->put_IsVisible(FALSE);
    view_.Reset();
    controller_.Reset();
}

bool WebViewInstance::Ready() const noexcept { return view_ != nullptr && controller_ != nullptr; }
ICoreWebView2* WebViewInstance::View() const noexcept { return view_.Get(); }
ICoreWebView2Controller* WebViewInstance::Controller() const noexcept { return controller_.Get(); }
}