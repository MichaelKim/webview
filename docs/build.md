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
  - Right now, the stable channel doesn't have the required version for the webview to work. As Edge continues to update, this should be fixed.

## EdgeHTML (Edge Legacy)

Define `WEBVIEW_WIN` before adding `webview.hpp`.

In order to target EdgeHTML (Microsoft Edge), `webview` uses the new C++/WinRT API.

### Requirements

- Visual Studio 2019
  - [C++/WinRT Visual Studio Extension](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264)
  - I haven't tested previous versions, but 2015 and 2017 should work as well.
- At least Windows 10, version 1809 (Build 17763)
  - The C++/WinRT API is fairly new, and its webview was introduced in v1803 ([UniversalAPIContract v6](https://docs.microsoft.com/en-us/uwp/api/windows.web.ui.interop.webviewcontrol)). This also uses `WebViewControl.AddInitializeScript` introduced in v1809. For more information about API contracts, read this [blog post by Microsoft](https://blogs.windows.com/buildingapps/2015/09/15/dynamically-detecting-features-with-api-contracts-10-by-10/#sRg3eXXT8oJzhUxY.97).

A few things to note:

- To debug, install the [Microsoft Edge DevTools](https://www.microsoft.com/en-us/p/microsoft-edge-devtools-preview/9mzbfrmz0mnj).
- Displaying `localhost` in the webview will only work after adding a loopback exception. A simple way to enable this is to run

  ```
  CheckNetIsolation.exe LoopbackExempt -a -n=Microsoft.Win32WebViewHost_cw5n1h2txyewy
  ```

  This can then be checked using `CheckNetIsolation.exe LoopbackExempt -s`. Read more about network loopbacks [here](<https://docs.microsoft.com/en-us/previous-versions/windows/apps/dn640582(v=win.10)>).

<details><summary><strong>Build with cl.exe (Edge Legacy)</strong></summary>

To use `cl.exe` directly, you'd need to grab the NuGet packages manually.

1. Download / clone this repo and navigate to it.
2. Get the [C++/WinRT package](https://www.nuget.org/packages/Microsoft.Windows.CppWinRT/) either by using the [NuGet CLI](https://www.nuget.org/downloads) or downloading them from the NuGet website.
3. Run `.\bin\cppwinrt.exe -in sdk`. (Optionally, you can add the `-verbose` flag.)
   - This should generate a local directory called `winrt` containing a bunch of headers. These are WinRT projection headers that you can use to consume from C++ code. You will be needing these headers for compilation.
4. Compile by running `cl main.cpp /DWEBVIEW_WIN /EHsc /I "." /std:c++17 /link WindowsApp.lib user32.lib kernel32.lib`.

If your `winrt` directory is located somewhere else, change the `/I "."` argument above.

</details>

<details><summary><strong>Build with Clang (Edge Legacy)</strong></summary>

While not officially supported, Microsoft does use Clang internally for testing purposes. If you want to use Clang, they have some basic instructions on <a href="https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#can-i-use-llvm-clang-to-compile-with-c---winrt-" rel="nofollow">their website</a>.

I've gotten `clang-cl` to compile with the following steps:

1. Download / clone this repo and navigate to it.
2. Get the [C++/WinRT package](https://www.nuget.org/packages/Microsoft.Windows.CppWinRT/) either by using the [NuGet CLI](https://www.nuget.org/downloads) or downloading them from the NuGet website.
3. Run `.\bin\cppwinrt.exe -in sdk`. (Optionally, you can add the `-verbose` flag.)
4. Install LLVM 8.0.0. (I've tested it with 8.0.0, but Microsoft says LLVM 6.0.0 should work too.)
   - (Optional) Add LLVM to your PATH, specifically `clang-cl.exe`.
5. Compile by running `clang-cl main.cpp /DWEBVIEW_WIN /EHsc /I "." -Xclang -std=c++17 -Xclang -Wno-delete-non-virtual-dtor /link "WindowsApp.lib" "user32.lib" "kernel32.lib"`.

If your `winrt` directory is located somewhere else, change the `/I "."` argument above.

</details>

## Chromium (Edge)

Define `WEBVIEW_EDGE` before adding `webview.hpp`.

To target the new Chromium Edge, `webview` uses the new WebView2 SDK.

Follow the instructions in the [official WebView2 docs](https://docs.microsoft.com/en-us/microsoft-edge/webview2/gettingstarted/win32).

To summarize:

- Visual Studio 2015 or later
- Windows 7, 8.1, 10
- Microsoft Edge (Chromium)
  - Install a non-stable channel (Beta, Dev, or Canary) or the [WebView2 Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/#download-section)
- Add the following NuGet packages to the solution:
  - [Microsoft.Windows.ImplementationLibrary](https://www.nuget.org/packages/Microsoft.Windows.ImplementationLibrary/)
  - [Microsoft.Web.WebView2](https://www.nuget.org/packages/Microsoft.Web.WebView2)

<details><summary><strong>Build with cl.exe (Edge Chromium)</strong></summary>

To use `cl.exe` directly, you'd need to grab the NuGet packages manually.

1. Download / clone this repo and navigate to it.
2. Make sure you have the new Edge installed (beta, dev, or canary) or the runtime.
3. Get the WebView2 package and the Windows Implementation Libraries (WIL) package either by using the [NuGet CLI](https://www.nuget.org/downloads) or downloading them from the NuGet website.
   - From WebView2, you need the following files:
     - `.\build\native\include\WebView2.h`
     - `.\build\native\x86\WebView2LoaderStatic.lib`.
     - For dynamic linking, use `WebView2Loader.dll.lib` and make sure `WebView2Loader.dll` is located with your executable when running.
   - From WIL, you need `.\include\wil\`.
4. Compile by running `cl main.cpp /DWEBVIEW_EDGE /EHsc /std:c++17 /link WebView2LoaderStatic.lib user32.lib Version.lib Advapi32.lib Shell32.lib`.

</details>

## MacOS

webview depends on the Cocoa and Webkit frameworks. Also, make sure your compiler supports Objective-C++ (g++ and clang++ should both work).

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
