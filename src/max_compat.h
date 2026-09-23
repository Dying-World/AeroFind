#pragma once

#include <windows.h>
#include <algorithm>

// Win32 RECT members use LONG, while the literal 220 is an int.
// Provide an exact overload so std::max(220, bounds.right - 300)
// remains valid with MSVC's strict template deduction.
namespace std {
inline int max(int left, LONG right) noexcept {
    return left > static_cast<int>(right) ? left : static_cast<int>(right);
}
}
