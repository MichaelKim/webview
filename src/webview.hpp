#if defined(__linux__)
#include "webview/linux/webkit2gtk.hpp"
using SounduxWebView = Soundux::WebKit2Gtk;
#elif defined(_WIN32)
#include "webview/windows/webview2.hpp"
using SounduxWebView = Soundux::WebView2;
#endif
