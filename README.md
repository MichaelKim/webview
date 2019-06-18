# webview

A tiny cross-platform webview library written in C++ using EdgeHTML / WinRT (Windows) and WebkitGTK (Linux).

This is mostly a fork of zerge's [webview](https://github.com/zserge/webview) but rewritten for a more "C++"-like API and added support for Microsoft Edge on Windows.

## Support

|            | Windows            | Linux                         |
| ---------- | ------------------ | ----------------------------- |
| Version    | Windows 10, v1803+ | Tested on Ubuntu 18.04.02 LTS |
| Web Engine | EdgeHTML           | WebKit                        |

## Usage

```c++
// main.cpp
#include "webview.h"

#ifdef WIN32 // To support Windows
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  // Create a 800 x 600 webview that shows Google
  wv::WebView w{800, 600, true, true, Str("Hello world!"), Str("http://google.com")};

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0);

  return 0;
}
```

Since Windows (WinAPI) uses `std::wstring`s, all string literals should be wrapped in the macro `Str(s)`.

Check out example programs in the [`examples/`](examples/) directory in this repo.

## Build

### Windows

First define `WEBVIEW_WIN` before adding `webview.h`.

In order to target EdgeHTML (Microsoft Edge), `webview` uses the new C++/WinRT API. This requires some additional requirements:

**tl;dr**: Upgrade to latest version of Windows 10, install Visual Studio 2019, install the Windows 10 SDK, and add the [C++/WinRT VSIX](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264).

#### Compiler

- Requires C++17
  - Visual Studio 2017, recommended 2019
  - [C++/WinRT Visual Studio Extension](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt#visual-studio-support-for-cwinrt-xaml-the-vsix-extension-and-the-nuget-package)

Also, use `std::wstring` in place of `std::string` when using webview APIs (with the exception of `wv::WebView::callback`).

#### I don't like Visual Studio!

While not officially supported, Microsoft does use Clang internally for testing purposes. If you want to use Clang, they have some basic instructions on their website [here](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#can-i-use-llvmclang-to-compile-with-cwinrt).

I've gotten `clang-cl` to compile with the following steps:

1. Download / clone this repo and navigate to it.
2. Install the Windows 10 SDK. Make sure to install at least version 1809.
   1. (Optional) Add `cppwinrt.exe` to your PATH (located in `C:\Program Files (x86)\Windows Kits\10\{version}\bin\x86\cppwinrt.exe`).
3. Run `cppwinrt.exe -in sdk`. (Optionally, you can add the `-verbose` flag.)

   1. This should generate a local directory called `winrt` containing a bunch of headers. These are WinRT projection headers that you can use to consume from C++ code. You will be needing these headers for compilation.
   2. For more on `cppwinrt.exe`, check out this [blog post](https://moderncpp.com/2017/11/15/cppwinrt-exe-in-the-windows-sdk/) by the creator of C++/WinRT.

4. Install LLVM 8.0.0. (I've tested it with 8.0.0, but Microsoft says LLVM 6.0.0 should work too.)
   1. (Optional) Add LLVM to your PATH, specifically `clang-cl.exe`.
5. Compile by running `clang-cl examples\main.cpp /EHsc /I "." -Xclang -std=c++17 -Xclang -Wno-delete-non-virtual-dtor -o webview.exe /link "WindowsApp.lib" "user32.lib" "kernel32.lib"`.

You may result in some compiler errors in some of the `winrt::` headers. I fixed them by manually editing the headers in the `winrt/` subdirectory.

#### Windows OS

- Requires Windows 10, version 1803 (Build 17134), recommended version 1809 (Build 17763)

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

Make sure to define `WEBVIEW_GTK`.

## API (JavaScript)

To communicate from JavaScript to C++, use the method `window.external.invoke`:

```js
// JavaScript
window.exteral.invoke('hello world!');
```

```c++
// C++
void callback(WebView &w, std::string &arg) {
  // arg = "hello world!"
}
```

This method can only accept strings, so objects must be serialized in some way:

```js
// JavaScript
window.external.invoke(JSON.stringify({ foo: 'bar' }));
```

## API (C++)

Note: On Windows, all strings are `std::wstring` due to the underlying WinRT API. On Linux, they are `std::string`. The only exception is `WebView::setCallback`, which uses `std::string` even on Windows.

If you need to support Windows, make sure to use the following `main` declaration:

```c++
#ifdef WIN32
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  // ...
}
```

### Constructor

```c++
WebView(int width, int height, bool resizable, bool debug, string title,
        string url = DEFAULT_URL)
```

The default url (`DEFAULT_URL`) will render a blank page with a single root div:

```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
  </head>
  <body>
    <div id="app"></div>
    <script type="text/javascript"></script>
  </body>
</html>
```

### `init`

```c++
int init();
```

Initializes the webview window.

#### Returns

0 if successful, otherwise -1.

#### Example

```c++
#include "webview.h"

int main() {
  wv::WebView w{800, 600, true, true, "My WebView"};

  // Initialize the webview
  if (w.init() == -1) {
    return 1;
  }

  // Do stuff with the webview

  return 0;
}
```

### `setCallback`

```c++
void setCallback(std::function<void(WebView &, std::string &)> callback);
```

Attaches a function that will run when receiving messages from JavaScript.

#### Params

- callback: A callback function

#### Example

```c++
#include "webview.h"

void callback(WebView &w, std::string &arg) {
  if (arg == "color") {
    w.css("body { background-color: red; }");
  }
}

// ...

w.setCallback(callback);
```

### `setTitle`

```c++
void setTitle(string title);
```

Sets the title of the webview window.

#### Params

- title: The new title of the webview

### `setFullscreen`

```c++
void setFullscreen(bool fs);
```

Sets the webview window to fullscreen or windowed mode.

#### Params

- fs: True if setting to fullscreen, false if windowed

### `setBgColor`

Sets the background color of the webview. If the webpage has a background color, it will take precedence over this color.

```c++
void setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
```

#### Params

- r: Red component of the color (0 to 255)
- g: Green component of the color (0 to 255)
- b: Blue component of the color (0 to 255)
- a: Alpha component of the color (0 to 255)

### `run`

```c++
bool run();
```

Runs one iteration of the main loop. This method is blocking if no events are pending.

#### Returns

True if the webview window will be closed, otherwise false.

#### Example

```c++
#include "webview.h"

int main() {
  wv::WebView w{800, 600, true, true, "My WebView"};

  if (w.init() == -1) {
    return 1;
  }

  // Keep running until the window will close
  while (w.run() == 0);

  return 0;
}
```

### `navigate`

```c++
void navigate(string uri);
```

Navigates the webview to the specified URI.

#### Params

- uri: URI to the webpage

### `run`

```c++
void eval(string js);
```

Executes the JavaScript string in the current webpage.

#### Params

- js: JavaScript string to execute

#### Example

```c++
#include "webview.h"

void callback(WebView &w, std::string &arg) {
  if (arg == "eval") {
    w.eval("alert('boo!')");
  }
}

int main() {
  wv::WebView w{800, 600, true, true, "My WebView"};

  if (w.init() == -1) {
    return 1;
  }

  w.setCallback(callback);

  while (w.run() == 0);

  return 0;
}
```

The `alert` will display when this is executed in the webpage:

```js
window.external.invoke('eval');
```

### `css`

```c++
void css(string css);
```

Applies the CSS string to the current webpage.

#### Params

- css: CSS string to apply

#### Example

This method works similar to `WebView::eval`.

```c++
#include "webview.h"

void callback(WebView &w, std::string &arg) {
  if (arg == "style") {
    w.css("p { color: red; }");
  }
}

int main() {
  wv::WebView w{800, 600, true, true, "My WebView"};

  if (w.init() == -1) {
    return 1;
  }

  w.setCallback(callback);

  while (w.run() == 0);

  return 0;
}
```

All `p` tags will be colored red when this is executed in the webpage:

```js
window.external.invoke('style');
```

### `exit`

```c++
void exit();
```

Closes the webview window. This will cause the next invocation of `WebView::run` to return `true`.
