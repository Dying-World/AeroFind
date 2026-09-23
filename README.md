# AeroFind

A lightweight Windows browser shell built with C++20, Win32, and Microsoft WebView2.

The shell includes a retro Windows XP / early-Chromium visual style, Chromium rendering, multiple tabs, background tab unloading, Google/Bing search, address navigation, download status, system-theme-aware start page, a small ad-domain filter, and keyboard shortcuts: `Ctrl+T`, `Ctrl+W`, `Ctrl+Tab`, `Ctrl+L`, `Ctrl+H` (history), `Ctrl+J` (downloads), and `F5`.

History and download records are stored in `%LOCALAPPDATA%\AeroFind\vault.bin` and protected with Windows DPAPI. WebView2 uses a persistent profile under `%LOCALAPPDATA%\AeroFind\WebView2`; Chromium/WebView2 protects its own cookies and site storage using the Windows user profile.

## Build locally

Install Visual Studio 2022 with the Desktop C++ workload, CMake, and the WebView2 Runtime. Download the `Microsoft.Web.WebView2` NuGet package and configure:

```powershell
cmake -S . -B build -A x64 -DWEBVIEW2_ROOT="C:\path\to\Microsoft.Web.WebView2.1.0.2903.40"
cmake --build build --config Release
```

The executable and `ui` folder are placed in `build/Release`. The repository workflow builds the same package on GitHub Actions and uploads `AeroFind-windows-x64.zip`.

The WebView2 Runtime must be installed on the target Windows machine. GitHub Actions provides the SDK and compiler, but it does not package the Evergreen Runtime itself.

## Security boundaries

The project does not copy passwords or authenticated cookies from Chrome, Edge, or Firefox. Those stores are protected by each browser and by Windows profile encryption; copying them would be unsafe and unreliable. A real VPN also requires a provider or a system tunnel and cannot be implemented by a browser shell alone. The included ad filter is intentionally a small baseline list, not a replacement for a full extension engine.

## Resource target

AeroFind keeps the native shell idle when unused and creates one WebView2 controller. Memory and CPU still depend on the pages and extensions loaded by WebView2; no browser can guarantee 512 MB or 1% CPU for arbitrary websites. Use Windows Task Manager to measure the actual workload.