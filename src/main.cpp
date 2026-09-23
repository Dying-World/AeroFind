#include "core/BrowserApp.h"

#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    aero::core::BrowserApp app(instance);
    const int result = app.Run(showCommand);
    CoUninitialize();
    return result;
}