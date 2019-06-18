#ifndef WEBVIEW_H
#define WEBVIEW_H

// Headers
#include <functional>
#include <string>

#if defined(WEBVIEW_WIN)
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "windowsapp")

#include <objbase.h>
#include <windows.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.UI.Interop.h>

constexpr auto DEFAULT_URL =
    L"data:text/"
    "html,%3C%21DOCTYPE%20html%3E%0A%3Chtml%20lang=%22en%22%3E%0A%3Chead%3E%"
    "3Cmeta%20charset=%22utf-8%22%3E%3Cmeta%20http-equiv=%22X-UA-Compatible%22%"
    "20content=%22IE=edge%22%3E%3C%2Fhead%3E%0A%3Cbody%3E%3Cdiv%20id=%22app%22%"
    "3E%3C%2Fdiv%3E%3Cscript%20type=%22text%2Fjavascript%22%3E%3C%2Fscript%3E%"
    "3C%2Fbody%3E%0A%3C%2Fhtml%3E";
#elif defined(WEBVIEW_GTK) // WEBVIEW_WIN
#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

constexpr auto DEFAULT_URL =
    "data:text/"
    "html,%3C%21DOCTYPE%20html%3E%0A%3Chtml%20lang=%22en%22%3E%0A%3Chead%3E%"
    "3Cmeta%20charset=%22utf-8%22%3E%3Cmeta%20http-equiv=%22X-UA-Compatible%22%"
    "20content=%22IE=edge%22%3E%3C%2Fhead%3E%0A%3Cbody%3E%3Cdiv%20id=%22app%22%"
    "3E%3C%2Fdiv%3E%3Cscript%20type=%22text%2Fjavascript%22%3E%3C%2Fscript%3E%"
    "3C%2Fbody%3E%0A%3C%2Fhtml%3E";
#endif // WEBVIEW_GTK

/*
<!DOCTYPE html>
<html lang="en">
<head><meta charset="utf-8"><meta http-equiv="X-UA-Compatible"
content="IE=edge"></head> <body><div id="app"></div><script
type="text/javascript"></script></body>
</html>
*/

namespace wv {
#if defined(WEBVIEW_WIN)
using string = std::wstring;
using string = std::wstring;
using namespace winrt::impl;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Web::UI::Interop;
#elif defined(WEBVIEW_GTK)
using string = std::string;
#endif

class WebView {
  using jscb = std::function<void(WebView &, std::string &)>;

public:
  WebView(int width, int height, bool resizable, bool debug, string title,
          string url = DEFAULT_URL)
      : width(width), height(height), resizable(resizable), debug(debug),
        title(title), url(url) {}
  int init();                      // Initialize webview
  void setCallback(jscb callback); // JS callback
  void setTitle(string t);         // Set title of window
  void setFullscreen(bool fs);     // Set fullscreen
  void setBgColor(uint8_t r, uint8_t g, uint8_t b,
                  uint8_t a); // Set background color
  bool run();                 // Main loop
  void navigate(string u);    // Navigate to URL
  void eval(string js);       // Eval JS
  void css(string css);       // Inject CSS
  void exit();                // Stop loop

private:
  // Properties for init
  int width;
  int height;
  bool resizable;
  bool fullscreen = false;
  bool debug;
  string title;
  string url;

  jscb js_callback;
  bool init_done = false; // Finished running init

#if defined(WEBVIEW_WIN)
  HINSTANCE hInt = nullptr;
  HWND hwnd = nullptr;
  WebViewControl webview{nullptr};
  MSG msg; // Message from main loop
  uint8_t bgR = 255, bgG = 255, bgB = 255, bgA = 0;

  static LRESULT CALLBACK WndProcedure(HWND hwnd, UINT msg, WPARAM wparam,
                                       LPARAM lparam);
#elif defined(WEBVIEW_GTK) // WEBVIEW_WIN
  bool ready = false;       // Done loading page
  bool js_busy = false;     // Currently in JS eval
  bool should_exit = false; // Close window
  GdkRGBA bgColor = {0, 0, 0, 0};
  GtkWidget *window;
  GtkWidget *webview;

  void setBgColor(GdkRGBA color);
  static void external_message_received_cb(WebKitUserContentManager *m,
                                           WebKitJavascriptResult *r,
                                           gpointer arg);
  static void webview_eval_finished(GObject *object, GAsyncResult *result,
                                    gpointer arg);
  static void webview_load_changed_cb(WebKitWebView *webview,
                                      WebKitLoadEvent event, gpointer arg);
  static void destroyWindowCb(GtkWidget *widget, gpointer arg);
  static gboolean webview_context_menu_cb(WebKitWebView *webview,
                                          GtkWidget *default_menu,
                                          WebKitHitTestResult *hit_test_result,
                                          gboolean triggered_with_keyboard,
                                          gpointer userdata);
#endif                     // WEBVIEW_GTK
};

#if defined(WEBVIEW_WIN)
int WebView::init() {
  hInt = GetModuleHandle(nullptr);
  if (hInt == nullptr) {
    return -1;
  }

  if (debug) {
    AllocConsole();
    FILE *out, *err;
    freopen_s(&out, "CONOUT$", "w", stdout);
    freopen_s(&err, "CONOUT$", "w", stderr);
  }

  // Initialize Win32 window
  WNDCLASSEX wc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WndProcedure;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInt;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = L"webview";
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  RegisterClassEx(&wc);
  hwnd = CreateWindow(L"webview", title.c_str(), WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr,
                      hInt, nullptr);

  if (hwnd == nullptr) {
    MessageBox(NULL, L"Window Registration Failed!", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

  ShowWindow(hwnd, SW_SHOWDEFAULT);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  // Initialize WinRT WebView
  init_apartment(winrt::apartment_type::single_threaded);

  // Allow intranet access (and localhost)
  WebViewControlProcessOptions options;
  options.PrivateNetworkClientServerCapability(
      WebViewControlProcessCapabilityState::Enabled);

  WebViewControlProcess proc(options);
  auto op =
      proc.CreateWebViewControlAsync(reinterpret_cast<int64_t>(hwnd), Rect());
  if (op.Status() != AsyncStatus::Completed) {
    winrt::handle h(CreateEvent(nullptr, false, false, nullptr));
    op.Completed([h = h.get()](auto, auto) { SetEvent(h); });
    HANDLE hs[] = {h.get()};
    DWORD i;
    winrt::check_hresult(CoWaitForMultipleHandles(
        COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS |
            COWAIT_INPUTAVAILABLE,
        INFINITE, 1, hs, &i));
  }
  webview = op.GetResults();
  webview.Settings().IsScriptNotifyAllowed(true);
  webview.ScriptNotify([=](auto const &sender, auto const &args) {
    if (js_callback) {
      std::string s = winrt::to_string(args.Value());
      js_callback(*this, s);
    }
  });
  webview.NavigationStarting([=](auto &&, auto &&) {
    webview.AddInitializeScript(L"(function(){window.external.invoke = s => "
                                L"window.external.notify(s)})();");
  });

  // Set window bounds
  RECT r;
  GetClientRect(hwnd, &r);
  Rect bounds(r.left, r.top, r.right - r.left, r.bottom - r.top);
  webview.Bounds(bounds);

  webview.IsVisible(true);

  // Done initialization, set properties
  init_done = true;

  setTitle(title);
  setBgColor(bgR, bgG, bgB, bgA);
  navigate(url);
}

void WebView::setCallback(jscb callback) { js_callback = callback; }

void WebView::setTitle(std::wstring t) {
  if (!init_done) {
    title = t;
  } else {
    SetWindowText(hwnd, t.c_str());
  }
}

void WebView::setFullscreen(bool fs) {}

void WebView::setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  if (!init_done) {
    bgR = r;
    bgG = g;
    bgB = b;
    bgA = a;
  } else {
    webview.DefaultBackgroundColor({a, r, g, b});
  }
}

bool WebView::run() {
  bool loop = GetMessage(&msg, NULL, 0, 0) > 0;
  if (loop) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return loop;
}

void WebView::navigate(std::wstring u) {
  if (!init_done) {
    url = u;
  } else {
    Uri uri{u};
    webview.Navigate(uri);
  }
}

void WebView::eval(std::wstring js) {
  auto op = webview.InvokeScriptAsync(
      L"eval", std::vector<winrt::hstring>({winrt::hstring(js)}));

  if (debug) {
    if (op.Status() != AsyncStatus::Completed) {
      winrt::handle h(CreateEvent(nullptr, false, false, nullptr));
      op.Completed([h = h.get()](auto, auto) { SetEvent(h); });
      HANDLE hs[] = {h.get()};
      DWORD i;
      winrt::check_hresult(CoWaitForMultipleHandles(
          COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS |
              COWAIT_INPUTAVAILABLE,
          INFINITE, 1, hs, &i));
    }
    auto result = op.GetResults();
    // std::cout << winrt::to_string(result) << std::endl;
  }
}

void WebView::css(std::wstring css) {
  eval(LR"js(
  (
    function (css) {
      if (document.styleSheets.length === 0) {
        var s = document.createElement('style');
        s.type = 'text/css';
        document.head.appendChild(s);
      }
      document.styleSheets[0].insertRule(css);
    }
  )(')js" +
       css + L"')");
}

void WebView::exit() { PostQuitMessage(WM_QUIT); }

LRESULT CALLBACK WebView::WndProcedure(HWND hwnd, UINT msg, WPARAM wparam,
                                       LPARAM lparam) {
  WebView *w =
      reinterpret_cast<WebView *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  switch (msg) {
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    w->exit();
    break;
  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}
#elif defined(WEBVIEW_GTK) // WEBVIEW_WIN
int WebView::init() {
  if (gtk_init_check(0, NULL) == FALSE) {
    return -1;
  }

  // Initialize GTK window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

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

  // Done initialization, set properties
  init_done = true;

  setTitle(title);
  if (fullscreen) {
    setFullscreen(true);
  }
  setBgColor(bgColor);
  navigate(url);

  // Finish
  gtk_widget_grab_focus(GTK_WIDGET(webview));
  gtk_widget_show_all(window);

  return 0;
}

void WebView::setCallback(jscb callback) { this->js_callback = callback; }

void WebView::setTitle(std::string t) {
  if (!init_done) {
    title = t;
  } else {
    gtk_window_set_title(GTK_WINDOW(window), t.c_str());
  }
}

void WebView::setFullscreen(bool fs) {
  if (!init_done) {
    fullscreen = fs;
  } else if (fs) {
    gtk_window_fullscreen(GTK_WINDOW(window));
  } else {
    gtk_window_unfullscreen(GTK_WINDOW(window));
  }
}

void WebView::setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  GdkRGBA color = {r / 255.0, g / 255.0, b / 255.0, a / 255.0};
  if (!init_done) {
    bgColor = color;
  } else {
    setBgColor(color);
  }
}

bool WebView::run() {
  gtk_main_iteration_do(true);
  return should_exit;
}

void WebView::navigate(std::string u) {
  if (!init_done) {
    url = u;
  } else {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), u.c_str());
  }
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

void WebView::css(std::string css) {
  eval(R"js(
    (
      function (css) {
        if (document.styleSheets.length === 0) {
          var s = document.createElement('style');
          s.type = 'text/css';
          document.head.appendChild(s);
        }
        document.styleSheets[0].insertRule(css);
      }
    )(')js" +
       css + "')");
}

void WebView::exit() { should_exit = true; }

void WebView::setBgColor(GdkRGBA color) {
  webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &color);
}

void WebView::external_message_received_cb(WebKitUserContentManager *m,
                                           WebKitJavascriptResult *r,
                                           gpointer arg) {
  WebView *w = static_cast<WebView *>(arg);
  if (w->js_callback) {
    JSCValue *value = webkit_javascript_result_get_js_value(r);
    std::string str = std::string(jsc_value_to_string(value));
    w->js_callback(*w, str);
  }
}

void WebView::webview_eval_finished(GObject *object, GAsyncResult *result,
                                    gpointer arg) {
  static_cast<WebView *>(arg)->js_busy = false;
}

void WebView::webview_load_changed_cb(WebKitWebView *webview,
                                      WebKitLoadEvent event, gpointer arg) {
  if (event == WEBKIT_LOAD_FINISHED) {
    static_cast<WebView *>(arg)->ready = true;
  }
}

void WebView::destroyWindowCb(GtkWidget *widget, gpointer arg) {
  static_cast<WebView *>(arg)->exit();
}

// static gboolean closeWebViewCb(WebKitWebView *webView, GtkWidget *window) {
//   gtk_widget_destroy(window);
//   return TRUE;
// }

gboolean WebView::webview_context_menu_cb(WebKitWebView *webview,
                                          GtkWidget *default_menu,
                                          WebKitHitTestResult *hit_test_result,
                                          gboolean triggered_with_keyboard,
                                          gpointer userdata) {
  // Always hide context menu if not debug
  return TRUE;
}
#endif                     // WEBVIEW_GTK

} // namespace wv

#endif // WEBVIEW_H