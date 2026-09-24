# AeroFind

AeroFind is a lightweight Windows browser shell built with C++20, Win32, and Microsoft WebView2. It is designed to feel modern and familiar, while keeping the native shell small, stable, and easy to maintain.

## Overview

AeroFind combines a native desktop window, Chromium rendering via WebView2, tab management, local start-page experiences, and persistent local data storage. The project is intentionally compact: it focuses on a clean browser shell rather than trying to recreate the full complexity of a large browser engine.

The app includes:
- tabbed browsing and simple session preservation
- Google/Bing search routing
- local app pages for start, settings, history, downloads, and media
- encrypted local storage using Windows DPAPI
- a lightweight ad-domain filter baseline
- keyboard shortcuts for fast navigation
- Chrome-like source viewer and WebView2 DevTools
- distraction-free Focus mode for the active page

## Project structure

```text
src/
    core/       BrowserApp, WebViewInstance, URL routing, app config
    ui/         Win32 controls, address bar, tab chrome, toolbar painting
    tabs/       Tab state and active-tab management
    network/    AdBlocker and download tracking
    storage/    Storage facade over encrypted local records
    utils/      String helpers, path helpers, logging
ui/
    index.html      modern start page
    settings.html   settings panel
    history.html    history view
    downloads.html  downloads view
    media.html      media/demo hub
```

## Features

- modern Chrome-like browser chrome
- Google/Bing search integration
- custom local landing page with search, news, and bookmarks
- persistent tab/session state in encrypted local storage
- built-in local pages for settings, history, downloads, and media
- lightweight resource profile compared to full browser suites
- Windows-native integration with WebView2

## Build locally

Requirements:
- Windows 10/11
- Visual Studio 2022 with Desktop C++ workload
- CMake 3.24+
- Microsoft WebView2 SDK / NuGet package

Use the following commands:

```powershell
cmake -S . -B build -A x64 -DWEBVIEW2_ROOT="C:\path\to\Microsoft.Web.WebView2.1.0.2903.40"
cmake --build build --config Release
```

The executable and local UI assets are copied into the build output directory. The GitHub Actions workflow in [.github/workflows/build.yml](.github/workflows/build.yml) builds the same configuration automatically and uploads a release ZIP.

## Standard commands

- `Ctrl+T` / `Ctrl+W`: new tab / close active tab
- `Ctrl+L`: focus the address bar
- `Ctrl+R` or `F5`: reload the page
- `Alt+F4`: close AeroFind
- `Ctrl+U`: view the current page source
- `Ctrl+Shift+I`: open WebView2 developer tools
- `Ctrl+Shift+Space`: toggle AeroFind Focus mode
- `Ctrl+H`, `Ctrl+J`, `Ctrl+D`: history, downloads, and save bookmark

## GitHub release workflow

This project is prepared for release automation through GitHub Actions:
- WebView2 SDK is restored via NuGet
- the app is configured with CMake
- a Release build is produced
- the final ZIP is uploaded as an artifact
- tagged releases are published automatically

## Security and limitations

AeroFind does not copy cookies, saved credentials, or browser profile data from Chrome, Edge, or Firefox. Those are protected by each browser and by Windows account security. A real VPN requires a system-level tunnel or provider service; a browser shell alone cannot provide a full VPN implementation.

The ad filter included in the project is a lightweight baseline filter and is not a replacement for a full browser extension engine. Memory and CPU use still depend on the pages loaded by Chromium; the shell itself is designed to stay compact, but no browser can guarantee fixed low-resource usage for arbitrary websites.

## Resource target

The application is intended to stay light and responsive by keeping the native shell lean and reusing a single WebView2 environment. In real use, the actual memory and CPU footprint depends on the active page content and media workload, but the shell is built for a low-overhead desktop experience.

## License

This project is ready for personal and open-source use as a browser shell prototype and release candidate. Use it as a base for extension, redesign, and feature work on Windows.