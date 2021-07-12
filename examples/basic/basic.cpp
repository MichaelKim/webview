// Basic webview example

#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    w.navigate(Str("http://www.google.com"));

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}