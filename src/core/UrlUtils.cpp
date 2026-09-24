#include "UrlUtils.h"

#include "AppConfig.h"
#include "../utils/StringUtils.h"

#include <utility>

namespace aero::core {
std::wstring MakeNavigationTarget(std::wstring input) {
    input = utils::Trim(std::move(input));
    if (input.empty()) return config::StartPage;
    if (input.rfind(L"http://", 0) == 0 || input.rfind(L"https://", 0) == 0 || input.rfind(L"about:", 0) == 0) return input;
    if (input.rfind(L"www.", 0) == 0) return L"https://" + input;
    if (input.rfind(L"localhost", 0) == 0 || input.rfind(L"appassets.local", 0) == 0) return L"https://" + input;
    return std::wstring(config::DefaultSearchEngine) + input;
}
}