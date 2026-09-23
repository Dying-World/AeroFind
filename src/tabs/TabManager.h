#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace aero::tabs {
struct TabState {
    std::wstring url;
    std::wstring title;
    bool suspended = false;
};

class TabManager {
public:
    TabManager();
    size_t AddTab();
    bool CloseTab(size_t index);
    bool Activate(size_t index);
    size_t ActiveIndex() const noexcept;
    size_t Count() const noexcept;
    TabState& Active();
    TabState& At(size_t index);
    const TabState& At(size_t index) const;
private:
    std::vector<TabState> tabs_;
    size_t active_ = 0;
};
}