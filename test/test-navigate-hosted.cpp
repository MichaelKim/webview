#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    w.navigate(Str("http://localhost:8080/local.html"));

    w.setCallback([](wv::WebView &webview, wv::String &arg) {
        if (arg == Str("onload")) {
            webview.exit();
        }
    });

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}
