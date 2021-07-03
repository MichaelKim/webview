// WebView callback example
// To run this example, host `index.html` locally at `localhost:8080`.
// Make sure to add a loopback exception if developing on Windows
// (check the README).

#include "webview.hpp"

#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
#define to_string std::to_wstring
#else
#define to_string std::to_string
#endif

long long factorial(long long n) { return n <= 1 ? 1 : n * factorial(n - 1); }

void callback(wv::WebView &w, wv::String &arg) {
    try {
        long long num = std::stoll(arg);
        w.eval(Str("result(") + to_string(factorial(num)) + Str(")"));
    } catch (std::exception e) {
        w.eval(Str("result('Invalid number')"));
    }
}

WEBVIEW_MAIN {
    wv::WebView w{800,
                  600,
                  true,
                  true,
                  Str("WebView Callback"),
                  Str("http://localhost:8080")};

    // This can be called before or after w.init();
    w.setCallback(callback);

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}