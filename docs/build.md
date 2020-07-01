# Build Steps

## Table of Contents

- Windows
    - [EdgeHTML](#edgehtml) (Microsoft Edge Legacy)
    - [Chromium](#chromium-edge) (Microsoft Edge, recommended)
- [MacOS](#macos)
- [Linux](#linux)

If you have CMake installed, the included config should work for all platforms.

There are two native web engines for Windows:
- EdgeHTML
- Chromium
    - Note: right now, the stable channel doesn't have the required version for the webview to work. As Edge continues to update, this should be fixed.

While Trident, the old Internet Explorer's engine, is possible to be used, webview does not support it due to its age, decreasing public support, and poor developer experience.

## EdgeHTML

Define `WEBVIEW_WIN` before adding `webview.h`.

In order to target EdgeHTML (Microsoft Edge), `webview` uses the new C++/WinRT API.

### Requirements

- Visual Studio 2019
    - [C++/WinRT Visual Studio Extension](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt#visual-studio-support-for-cwinrt-xaml-the-vsix-extension-and-the-nuget-package)
    - I haven't tested previous versions, but 2015 and 2017 should work as well.
- At least Windows 10, version 1809 (Build 17763)
    - The C++/WinRT API is fairly new, and its webview was introduced in v1803 ([UniversalAPIContract v6](https://docs.microsoft.com/en-us/uwp/api/windows.web.ui.interop.webviewcontrol)). This also uses `WebViewControl.AddInitializeScript` introduced in v1809. For more information about API contracts, read this [blog post by Microsoft](https://blogs.windows.com/buildingapps/2015/09/15/dynamically-detecting-features-with-api-contracts-10-by-10/#sRg3eXXT8oJzhUxY.97).

**tl;dr**: Upgrade to latest version of Windows 10, install Visual Studio 2019, install the Windows 10 SDK, and add the [C++/WinRT VSIX](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264).

A few things to note:

- To debug, install the [Microsoft Edge DevTools](https://www.microsoft.com/en-us/p/microsoft-edge-devtools-preview/9mzbfrmz0mnj).
- Use `std::wstring` in place of `std::string` when using webview APIs (with the exception of `wv::WebView::callback`).
- Displaying `localhost` in the webview will only work after adding a loopback exception. A simple way to enable this is to run

    ```
    CheckNetIsolation.exe LoopbackExempt -a -n=Microsoft.Win32WebViewHost_cw5n1h2txyewy
    ```

    This can then be checked using `CheckNetIsolation.exe LoopbackExempt -s`. Read more about network loopbacks [here](<https://docs.microsoft.com/en-us/previous-versions/windows/apps/dn640582(v=win.10)>).

<details><summary><strong>I don't like Visual Studio!</strong></summary>
<p>
While not officially supported, Microsoft does use Clang internally for testing purposes. If you want to use Clang, they have some basic instructions on <a href="https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#can-i-use-llvmclang-to-compile-with-cwinrt" rel="nofollow">their website</a>.

I've gotten `clang-cl` to compile with the following steps:

1. Download / clone this repo and navigate to it.
2. Install the Windows 10 SDK. Make sure to install at least version 1809.
   1. (Optional) Add `cppwinrt.exe` to your PATH (located in `C:\Program Files (x86)\Windows Kits\10\{version}\bin\x86\cppwinrt.exe`).
3. Run `cppwinrt.exe -in sdk`. (Optionally, you can add the `-verbose` flag.)

   1. This should generate a local directory called `winrt` containing a bunch of headers. These are WinRT projection headers that you can use to consume from C++ code. You will be needing these headers for compilation.
   2. For more on `cppwinrt.exe`, check out this [blog post](https://moderncpp.com/2017/11/15/cppwinrt-exe-in-the-windows-sdk/) by the creator of C++/WinRT.

4. Install LLVM 8.0.0. (I've tested it with 8.0.0, but Microsoft says LLVM 6.0.0 should work too.)
   1. (Optional) Add LLVM to your PATH, specifically `clang-cl.exe`.
5. Compile by running `clang-cl main.cpp /EHsc /I "." -Xclang -std=c++17 -Xclang -Wno-delete-non-virtual-dtor -o my_webview.exe /link "WindowsApp.lib" "user32.lib" "kernel32.lib"`.

If your `winrt/` directory is located somewhere else, change the `/I "."` argument above.

You may result in some compiler errors in some of the `winrt::` headers. I fixed them by manually editing the headers in the `winrt/` subdirectory.

</p>
</details>

## Chromium (Edge)

Define `WEBVIEW_EDGE` before adding `webview.h`.

To target the new Chromium Edge, `webview` uses the new WebView2 SDK.

Follow the instructions in the [official WebView2 docs](https://docs.microsoft.com/en-us/microsoft-edge/webview2/gettingstarted/win32).

To summarize:
- Visual Studio 2017 or 2019
- Microsoft Edge (Chromium)
    - Install the Canary channel as the minimum required version is 82.0.488.0 (currently the Beta channel is too old).
    - With Windows 10 versions 1803+, the new Edge comes already installed. However, its version is too old for WebView2 (currently at 81.0.416.81).
- Add the following NuGet packages to the solution:
    - Microsoft.Windows.ImplementationLibrary
    - Microsoft.Web.WebView2

## MacOS

webview depends on the Cocoa and Webkit frameworks. Also, make sure your compiler supports Objective-C (g++ and clang++ should both work).

Use the provided CMake config to compile. Otherwise, to manually compile,
- Define `WEBVIEW_MAC`
- Enable Objective-C++ compilation
- Add frameworks Cocoa and Webkit

For example,
```
clang++ main.cpp -DWEBVIEW_MAC -ObjC++ -std=c++11 -framework Cocoa -framework Webkit -o my_webview
```

## Linux

webview depends on `gtk+-3.0` and `webkit2gtk-4.0`:
```
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.0-37 libwebkit2gtk-4.0-dev
```

Use the provided CMake config to compile. Otherwise, to manually compile,
- Define `WEBVIEW_GTK`
- Add `gtk+-3.0` and `webkit2gtk-4.0`

For example,
```sh
g++ main.cpp -DWEBVIEW_GTK `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o my_webview
```
