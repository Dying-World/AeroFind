#pragma once

#include <string>

namespace aero::network {
class AdBlocker {
public:
    bool IsBlocked(const std::wstring& uri) const;
};
}