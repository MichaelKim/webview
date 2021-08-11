// To run this example, host `index.html` locally at `localhost:8080`.
// Make sure to add a loopback exception if developing on Windows.
// Note: this doesn't work for WEBVIEW_MAC

#include "webview.hpp"

void callback(wv::WebView &w, wv::String &arg) {
    w.setFullscreenFromJS(arg[0] == '1');
}

WEBVIEW_MAIN {
    wv::WebView w{800,
                  600,
                  true,
                  true,
                  Str("WebView Localhost Fullscreen"),
                  Str("http://localhost:8080")};

    w.setCallback(callback);

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}