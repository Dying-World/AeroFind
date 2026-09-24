#include "TabManager.h"

#include "../core/AppConfig.h"
#include <algorithm>
#include <stdexcept>

namespace aero::tabs {
TabManager::TabManager() { AddTab(); }

size_t TabManager::AddTab() {
    tabs_.push_back({config::StartPage, L"New tab", false});
    if (tabs_.size() == 1) active_ = 0;
    return tabs_.size() - 1;
}

void TabManager::Append(const TabState& tab) {
    tabs_.push_back(tab);
    if (tabs_.size() == 1) active_ = 0;
}

void TabManager::Clear() {
    tabs_.clear();
    active_ = 0;
}

bool TabManager::CloseTab(size_t index) {
    if (tabs_.size() == 1 || index >= tabs_.size()) return false;
    tabs_.erase(tabs_.begin() + static_cast<std::ptrdiff_t>(index));
    if (index < active_) --active_;
    if (active_ >= tabs_.size()) active_ = tabs_.size() - 1;
    if (tabs_.empty()) {
        tabs_.push_back({config::StartPage, L"New tab", false});
        active_ = 0;
    }
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

void TabManager::RestoreFromEntries(const std::vector<std::wstring>& entries) {
    Clear();
    if (entries.empty()) {
        AddTab();
        return;
    }

    for (const auto& entry : entries) {
        TabState tab{config::StartPage, L"Saved tab", false};
        const auto split = entry.find(L'\t');
        if (split != std::wstring::npos) {
            tab.title = entry.substr(0, split);
            tab.url = entry.substr(split + 1);
        } else {
            tab.url = entry;
        }
        if (tab.title.empty()) tab.title = L"Saved tab";
        if (tab.url.empty()) tab.url = config::StartPage;
        Append(tab);
    }
    active_ = std::min(active_, tabs_.size() - 1);
}
}