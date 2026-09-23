# AeroFind

A lightweight Windows browser shell built with C++20, Win32, and Microsoft WebView2.

## Build locally

Install Visual Studio 2022 with the Desktop C++ workload, CMake, and the WebView2 Runtime. Download the `Microsoft.Web.WebView2` NuGet package and configure:

```powershell
cmake -S . -B build -A x64 -DWEBVIEW2_ROOT="C:\path\to\Microsoft.Web.WebView2.1.0.2903.40"
cmake --build build --config Release
```

The executable and `ui` folder are placed in `build/Release`. The repository workflow builds the same package on GitHub Actions and uploads `AeroFind-windows-x64.zip`.

## Resource target

AeroFind keeps the native shell idle when unused and creates one WebView2 controller. Memory and CPU still depend on the pages and extensions loaded by WebView2; no browser can guarantee 512 MB or 1% CPU for arbitrary websites. Use Windows Task Manager to measure the actual workload.