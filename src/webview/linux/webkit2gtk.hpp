#if defined(__linux__)
#include "../webview.hpp"
#include <JavaScriptCore/JavaScript.h>
#include <cassert>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

namespace Soundux
{
    class WebKit2Gtk : public WebView
    {
        GtkWidget *window;
        GtkWidget *webview;

        static void messageReceived(WebKitUserContentManager *, WebKitJavascriptResult *, gpointer);
        static void loadChanged(WebKitWebView *, WebKitLoadEvent, gpointer);
        static gboolean resize(WebKitWebView *, GdkEvent *, gpointer);
        static void destroy(GtkWidget *, gpointer);

        static gboolean contextMenu(WebKitWebView *, GtkWidget *, WebKitHitTestResultContext *, gboolean, gpointer);

      public:
        bool run() override;
        bool setup(int width, int height) override;

        void setSize(int width, int height) override;

        void enableDevTools(bool enable) override;
        void runCode(const std::string &code) override;
        void navigate(const std::string &url) override;
        void setTitle(const std::string &title) override;
    };
} // namespace Soundux
#endif