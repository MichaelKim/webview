#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    w.setCallback([](wv::WebView &webview, wv::String &arg) {
        if (arg == Str("done")) {
            webview.exit();
        }
    });

    w.preEval(Str(R"(
        window.onload = function() {
            window.external.invoke("done");
        };
    )"));

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}
