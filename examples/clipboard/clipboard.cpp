#include <filesystem>

#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w{800,
                  600,
                  true,
                  true,
                  Str("WebView Clipboard Example"),
                  Str("file:///") + wv::String(std::filesystem::current_path() /
                                               "index.html")};

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}