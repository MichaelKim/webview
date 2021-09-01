# API Reference

## Table of Contents

- [JavaScript API](#javascript-api)
- [C++ API](#c-api)
  - [Constructor](#constructor)
  - [webview.init](#init)
  - [webview.setCallback](#setcallback)
  - [webview.setTitle](#settitle)
  - [webview.setFullscreen](#setfullscreen)
  - [webview.setFullscreenFromJS](#setfullscreenfromjs)
  - [webview.setBgColor](#setbgcolor)
  - [webview.run](#run)
  - [webview.navigate](#navigate)
  - [webview.preEval](#preeval)
  - [webview.eval](#eval)
  - [webview.css](#css)
  - [webview.exit](#exit)

## JavaScript API

To communicate from JavaScript to C++, use the method `window.external.invoke`:

```js
// JavaScript
window.exteral.invoke('hello world!');
```

```c++
// C++
void callback(WebView &w, wv::String &arg) {
  // arg = "hello world!"
}
```

This method can only accept strings, so objects must be serialized in some way:

```js
// JavaScript
window.external.invoke(JSON.stringify({ foo: 'bar' }));
```

## C++ API

Note: On Windows, all strings are `std::wstring`. On MacOS and Linux, they are `std::string`.

If you need to support Windows, make sure to use Win32 `WinMain` entry point:

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

### `setFullscreenFromJS`

```c++
void setFullscreenFromJS(bool allow);
```

Sets whether JavaScript can directly switch between fullscreen and windowed mode. By default, JavaScript cannot change the window using `requestFullscreen()` and `exitFullscreen()`. Any fullscreen element will take up the size of the window itself rather than the screen.

Note: on Mac (`WEBVIEW_MAC`), this method will only apply when called before [`init`](#init). After [`init`](#init), this will be a no-op. For fine-grained control, consider polyfilling the fullscreen API with a call to C++ and manually call [`setFullscreen`](#setfullscreen). For example,

```js
Element.prototype.requestFullscreen = () => window.external.invoke('full');
Document.prototype.exitFullscreen = () => window.external.invoke('exit');
```

```c++
WEBVIEW_MAIN {
  bool allow = false;
  wv::WebView w{};

  if (w.init() == -1) return 1;
  w.setCallback([](wv::WebView& w, wv::String& arg) {
    if (allow) {
      if (arg == "full") w.setFullscreen(true);
      else if (arg == "exit") w.setFullscreen(false);
    }
  });

  while (w.run() == 0);

  return 0;
}
```

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

### `preEval`

```c++
void preEval(string js);
```

Injects the JavaScript string into the webpage before it loads.

Note: in some cases, the JavaScript will be run before the DOM loads. If you need access to the DOM, make sure to wait for it to load:

```js
window.onload = () => {
  const el = document.getElementById('id');
};

// or

window.addEventListener('DOMContentLoaded', () => {
  // or the 'load' event
  const el = document.getElementById('id');
});
```

### `eval`

```c++
void eval(string js);
```

Executes the JavaScript string in the current webpage.

#### Params

- js: JavaScript string to execute

#### Example

```c++
#include "webview.h"

void callback(wv::WebView &w, std::string &arg) {
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

void callback(wv::WebView &w, std::string &arg) {
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
