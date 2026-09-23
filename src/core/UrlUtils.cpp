#include "UrlUtils.h"

#include "AppConfig.h"
#include "../utils/StringUtils.h"

#include <utility>

namespace aero::core {
std::wstring MakeNavigationTarget(std::wstring input) {
    input = utils::Trim(std::move(input));
    if (input.empty()) return config::StartPage;
    if (input.find(L"://") != std::wstring::npos || input.rfind(L"about:", 0) == 0) return input;
    return L"https://www.google.com/search?q=" + input;
}
}