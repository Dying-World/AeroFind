#include "TabManager.h"

#include "../core/AppConfig.h"
#include <stdexcept>

namespace aero::tabs {
TabManager::TabManager() { AddTab(); }

size_t TabManager::AddTab() {
    tabs_.push_back({config::StartPage, L"New tab", false});
    return tabs_.size() - 1;
}

bool TabManager::CloseTab(size_t index) {
    if (tabs_.size() == 1 || index >= tabs_.size()) return false;
    tabs_.erase(tabs_.begin() + static_cast<std::ptrdiff_t>(index));
    if (index < active_) --active_;
    if (active_ >= tabs_.size()) active_ = tabs_.size() - 1;
    return true;
}

bool TabManager::Activate(size_t index) {
    if (index >= tabs_.size()) return false;
    active_ = index;
    return true;
}

size_t TabManager::ActiveIndex() const noexcept { return active_; }
size_t TabManager::Count() const noexcept { return tabs_.size(); }
TabState& TabManager::Active() { return tabs_.at(active_); }
TabState& TabManager::At(size_t index) { return tabs_.at(index); }
const TabState& TabManager::At(size_t index) const { return tabs_.at(index); }
}