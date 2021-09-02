// WebView callback example
// This won't work on EdgeHTML as those webviews can't navigate to local files.

#include <filesystem>

#include "webview.hpp"

#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
#define to_string std::to_wstring
#else
#define to_string std::to_string
#endif

long long factorial(long long n) { return n <= 1 ? 1 : n * factorial(n - 1); }

void callback(wv::WebView &w, const wv::String &arg) {
    try {
        long long num = std::stoll(arg);
        w.eval(Str("result(") + to_string(factorial(num)) + Str(")"));
    } catch (std::exception &) {
        w.eval(Str("result('Invalid number')"));
    }
}

WEBVIEW_MAIN {
    auto cwd = std::filesystem::current_path();

    wv::WebView w{800,
                  600,
                  true,
                  true,
                  Str("WebView Callback"),
                  Str("file:///") + wv::String(cwd / "index.html")};

    // This can be called before or after w.init();
    w.setCallback(callback);

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}