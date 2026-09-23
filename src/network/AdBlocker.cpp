#include "AdBlocker.h"

namespace aero::network {
bool AdBlocker::IsBlocked(const std::wstring& uri) const {
    static constexpr const wchar_t* hosts[] = {
        L"doubleclick.net", L"googlesyndication.com", L"adservice.google.com",
        L"adnxs.com", L"adsrvr.org", L"scorecardresearch.com", L"2mdn.net"
    };
    for (const auto* host : hosts) if (uri.find(host) != std::wstring::npos) return true;
    return false;
}
}