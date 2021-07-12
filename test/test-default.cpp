#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0) {
        w.exit();
    }

    return 0;
}
