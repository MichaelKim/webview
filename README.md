# webview

A tiny cross-platform webview library written in C++ using EdgeHTML (Windows) and WebkitGTK (Linux).

This is mostly a fork of zerge's [webview](https://github.com/zserge/webview) but rewritten for a more "C++"-like API and added support for Microsoft Edge on Windows.

## Support

|            | Windows        | Linux  |
| ---------- | -------------- | ------ |
| Version    | Win 10, v1803+ | ?      |
| Web Engine | EdgeHTML       | WebKit |

webview is powered by the built-in web browser engine in the OS.

### Windows

webview uses EdgeHTML (Microsoft Edge) using the C++/WinRT API.

## Usage

```c++
// main.cpp
#include "webview.h"

#ifdef WIN32
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  // Create a 800 x 600 webview that shows Google
  wv::WebView w{800, 600, true, true, "Hello world!", "http://google.com"};

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0);

  return 0;
}
```

## Build

### Windows

In order to target EdgeHTML (Microsoft Edge), `webview` uses the new C++/WinRT API. This requires some additional requirements:

**tl;dr**: Upgrade to latest version of Windows 10, install Visual Studio 2019, and add the [C++/WinRT VSIX](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264).

#### Compiler

- Requires C++17
  - Visual Studio 2017, recommended 2019
  - [C++/WinRT Visual Studio Extension](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt#visual-studio-support-for-cwinrt-xaml-the-vsix-extension-and-the-nuget-package)

As far as I know, only Visual Studio 2017+ (MSVC) will work with C++/WinRT. While not officially supported, Microsoft does use Clang internally for testing purposes. If you want to use Clang, they have some basic instructions on their website [here](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#can-i-use-llvmclang-to-compile-with-cwinrt).

Also, use `std::wstring` in place of `std::string` when using webview APIs.

#### Windows OS

- Windows 10, version 1803 (Build 17134), recommended version 1809 (Build 17763)

The C++/WinRT API is fairly new, and its webview was introduced in v1803 ([UniversalAPIContract v6](https://docs.microsoft.com/en-us/uwp/api/windows.web.ui.interop.webviewcontrol)). For more information about API contracts, read this [blog post by Microsoft](https://blogs.windows.com/buildingapps/2015/09/15/dynamically-detecting-features-with-api-contracts-10-by-10/#sRg3eXXT8oJzhUxY.97).

Also, displaying `localhost` in the webview will only work after adding a loopback exception. A simple way to enable this is to run

```
CheckNetIsolation.exe LoopbackExempt -a -n=Microsoft.Win32WebViewHost_cw5n1h2txyewy
```

This can then be checked using `CheckNetIsolation.exe LoopbackExempt -s`. Read more about network loopbacks [here](<https://docs.microsoft.com/en-us/previous-versions/windows/apps/dn640582(v=win.10)>).

### Linux

First install `gtk+-3.0` and `webkit2gtk-4.0`:

```
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.0-37 libwebkit2gtk-4.0-dev
```

Then to compile,

```
g++ main.cpp -DWEBVIEW_GTK=1 `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o webview
```

## API
