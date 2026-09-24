cmake_minimum_required(VERSION 3.24)
project(AeroFind VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WIN32_EXECUTABLE ON)

if(NOT WIN32)
    message(FATAL_ERROR "AeroFind is a Windows application and requires WebView2.")
endif()

set(WEBVIEW2_ROOT "" CACHE PATH "Root of the Microsoft.Web.WebView2 NuGet package")
if(NOT WEBVIEW2_ROOT)
    set(WEBVIEW2_ROOT "$ENV{WEBVIEW2_ROOT}")
endif()
if(NOT WEBVIEW2_ROOT)
    message(FATAL_ERROR "Set WEBVIEW2_ROOT to the Microsoft.Web.WebView2 package directory.")
endif()

set(WEBVIEW2_INCLUDE_DIR "${WEBVIEW2_ROOT}/build/native/include")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WEBVIEW2_ARCH x64)
else()
    set(WEBVIEW2_ARCH x86)
endif()
set(WEBVIEW2_LIB "${WEBVIEW2_ROOT}/build/native/${WEBVIEW2_ARCH}/WebView2Loader.dll.lib")
set(WEBVIEW2_DLL "${WEBVIEW2_ROOT}/build/native/${WEBVIEW2_ARCH}/WebView2Loader.dll")

if(NOT EXISTS "${WEBVIEW2_INCLUDE_DIR}/WebView2.h" OR NOT EXISTS "${WEBVIEW2_LIB}")
    message(FATAL_ERROR "WebView2 headers or libraries were not found under ${WEBVIEW2_ROOT}.")
endif()

add_executable(AeroFind WIN32
    src/main.cpp
    src/secure_store.cpp
    src/core/BrowserApp.cpp
    src/core/UrlUtils.cpp
    src/core/WebViewInstance.cpp
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
    src/tabs/TabManager.cpp
    src/storage/StorageManager.cpp
    src/network/AdBlocker.cpp
    src/network/DownloadManager.cpp
    src/ui/UIManager.cpp
)
target_include_directories(AeroFind PRIVATE "${WEBVIEW2_INCLUDE_DIR}")
target_link_libraries(AeroFind PRIVATE "${WEBVIEW2_LIB}" user32 gdi32 ole32 oleaut32 shlwapi crypt32 uxtheme)
target_compile_definitions(AeroFind PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
if(MSVC)
    target_compile_options(AeroFind PRIVATE /utf-8)
endif()

add_custom_command(TARGET AeroFind POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBVIEW2_DLL}" "$<TARGET_FILE_DIR:AeroFind>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/ui" "$<TARGET_FILE_DIR:AeroFind>/ui"
)