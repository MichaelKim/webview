#ifndef WEBVIEW_H
#define WEBVIEW_H

#ifdef WEBVIEW_GTK
#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#endif // WEBVIEW_GTK

#include <functional>
#include <iostream>
#include <string>

#define DEFAULT_URL                                                            \
  "data:text/"                                                                 \
  "html,%3C%21DOCTYPE%20html%3E%0A%3Chtml%20lang=%22en%22%3E%0A%3Chead%3E%"    \
  "3Cmeta%20charset=%22utf-8%22%3E%3Cmeta%20http-equiv=%22X-UA-Compatible%22%" \
  "20content=%22IE=edge%22%3E%3C%2Fhead%3E%0A%3Cbody%3E%3Cdiv%20id=%22app%22%" \
  "3E%3C%2Fdiv%3E%3Cscript%20type=%22text%2Fjavascript%22%3E%3C%2Fscript%3E%"  \
  "3C%2Fbody%3E%0A%3C%2Fhtml%3E"

/*
<!DOCTYPE html>
<html lang="en">
<head><meta charset="utf-8"><meta http-equiv="X-UA-Compatible"
content="IE=edge"></head> <body><div id="app"></div><script
type="text/javascript"></script></body>
</html>
*/

namespace wv {

class WebView {
  using jscb = std::function<void(WebView &, std::string)>;

public:
  WebView(int width, int height, bool resizable, bool debug,
          std::string title = "", std::string url = DEFAULT_URL)
      : width(width), height(height), resizable(resizable), debug(debug),
        title(title), url(url) {}
  int init();                       // Initialize webview
  void setReady();                  // Done loading for eval
  void setCallback(jscb callback);  // JS callback
  void setTitle(std::string title); // Set title of window
  bool run();                       // Main loop
  void eval(std::string js);        // Eval JS
  void terminate();                 // Stop loop

  jscb js_callback;
  bool js_busy = false; // Currently in JS eval

private:
  int width;
  int height;
  bool resizable;
  bool debug;
  std::string title;
  std::string url;
  bool ready = false;       // Initial loading
  bool should_exit = false; // Close window

#ifdef WEBVIEW_GTK
  GtkWidget *window;
  GtkWidget *webview;
#endif // WEBVIEW_GTK
};

#ifdef WEBVIEW_GTK
static void external_message_received_cb(WebKitUserContentManager *m,
                                         WebKitJavascriptResult *r,
                                         gpointer arg) {
  WebView *w = static_cast<WebView *>(arg);
  if (w->js_callback) {
    JSCValue *value = webkit_javascript_result_get_js_value(r);
    std::string str = std::string(jsc_value_to_string(value));
    w->js_callback(*w, str);
  }
}

static void webview_eval_finished(GObject *object, GAsyncResult *result,
                                  gpointer arg) {
  static_cast<WebView *>(arg)->js_busy = false;
}

static void webview_load_changed_cb(WebKitWebView *webview,
                                    WebKitLoadEvent event, gpointer arg) {
  if (event == WEBKIT_LOAD_FINISHED) {
    static_cast<WebView *>(arg)->setReady();
  }
}

static void destroyWindowCb(GtkWidget *widget, gpointer arg) {
  static_cast<WebView *>(arg)->terminate();
}

// static gboolean closeWebViewCb(WebKitWebView *webView, GtkWidget *window) {
//   gtk_widget_destroy(window);
//   return TRUE;
// }

static gboolean webview_context_menu_cb(WebKitWebView *webview,
                                        GtkWidget *default_menu,
                                        WebKitHitTestResult *hit_test_result,
                                        gboolean triggered_with_keyboard,
                                        gpointer userdata) {
  // Always hide context menu if not debug
  return TRUE;
}

int WebView::init() {
  if (gtk_init_check(0, NULL) == FALSE) {
    return -1;
  }

  // Initialize GTK window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), title.c_str());

  if (resizable) {
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  } else {
    gtk_widget_set_size_request(window, width, height);
  }

  gtk_window_set_resizable(GTK_WINDOW(window), resizable);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  // Add scrolling container
  GtkWidget *scroller = gtk_scrolled_window_new(nullptr, nullptr);
  gtk_container_add(GTK_CONTAINER(window), scroller);

  // Content manager
  WebKitUserContentManager *cm = webkit_user_content_manager_new();
  webkit_user_content_manager_register_script_message_handler(cm, "external");
  g_signal_connect(cm, "script-message-received::external",
                   G_CALLBACK(external_message_received_cb), this);

  // WebView
  webview = webkit_web_view_new_with_user_content_manager(cm);
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), url.c_str());
  g_signal_connect(G_OBJECT(webview), "load-changed",
                   G_CALLBACK(webview_load_changed_cb), this);
  gtk_container_add(GTK_CONTAINER(scroller), webview);

  g_signal_connect(window, "destroy", G_CALLBACK(destroyWindowCb), this);
  // g_signal_connect(webview, "close", G_CALLBACK(closeWebViewCb), window);

  // Dev Tools if debug
  if (debug) {
    WebKitSettings *settings =
        webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
    webkit_settings_set_enable_write_console_messages_to_stdout(settings, true);
    webkit_settings_set_enable_developer_extras(settings, true);
  } else {
    g_signal_connect(G_OBJECT(webview), "context-menu",
                     G_CALLBACK(webview_context_menu_cb), nullptr);
  }

  webkit_web_view_run_javascript(
      WEBKIT_WEB_VIEW(webview),
      "window.external={invoke:function(x){"
      "window.webkit.messageHandlers.external.postMessage(x);}}",
      NULL, NULL, NULL);

  gtk_widget_grab_focus(GTK_WIDGET(webview));
  gtk_widget_show_all(window);

  return 0;
}

void WebView::setReady() { ready = true; }

void WebView::setCallback(jscb callback) { this->js_callback = callback; }

void WebView::setTitle(std::string title) {
  gtk_window_set_title(GTK_WINDOW(window), title.c_str());
}

bool WebView::run() {
  gtk_main_iteration_do(true);
  return should_exit;
}

void WebView::eval(std::string js) {
  while (!ready) {
    g_main_context_iteration(NULL, TRUE);
  }
  js_busy = true;
  webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(webview), js.c_str(), NULL,
                                 webview_eval_finished, this);
  while (js_busy) {
    g_main_context_iteration(NULL, TRUE);
  }
}

void WebView::terminate() { should_exit = true; }

#endif // WEBVIEW_GTK

} // namespace wv

#endif // WEBVIEW_H