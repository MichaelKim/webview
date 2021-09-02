#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    w.setCallback([](wv::WebView &webview, const wv::String &arg) {
        if (arg == Str("ready")) {
            webview.eval(Str("window._onready();"));
        } else if (arg == Str("exit")) {
            webview.exit();
        }
    });

    w.preEval(Str(R"(
        window.onload = function() {
            window.external.invoke("ready");
        };

        window._onready = function() {
            window.external.invoke("exit");
        }
    )"));

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}
