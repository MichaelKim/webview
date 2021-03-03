#pragma once

// Headers
#include "lib/json/single_include/nlohmann/json.hpp"
#include <fmt/format.h>
#include <functional>
#include <regex>
#include <string>
#include <string_view>

#if defined(_WIN32)
#include <codecvt>
#include <locale>
#include <mutex>
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>

#include <WebView2.h>

#elif defined(__linux__)
#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#elif defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#include <objc/objc-runtime.h>

@interface WindowDelegate : NSObject <NSWindowDelegate, WKScriptMessageHandler>
@end

#ifdef MACOS_IMPL
@implementation WindowDelegate
- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)scriptMessage
{
}
@end
#endif
#endif

namespace wv
{
#if defined(_WIN32)
    using namespace Microsoft::WRL;
#endif

    class WebView
    {
        using callback_t = std::function<std::string(WebView &, const std::vector<std::string> &)>;

#if defined(__linux__) || defined(__APPLE__)
        static constexpr std::string_view INVOKE_CODE = "window.external={invoke:arg=>window.webkit."
                                                        "messageHandlers.external.postMessage(arg)};";
#elif defined(_WIN32)
        static constexpr std::string_view INVOKE_CODE =
            "window.external.invoke=arg=>window.chrome.webview.postMessage(arg);";
#endif
      private:
        bool debug;
        bool resizable;
        bool ready_early;
        int width, height;
        bool should_exit = false;

        std::string url;
        std::string title;

        bool init_done = false;
        std::map<std::string, callback_t> callbacks;
        std::function<void(int, int)> onResizeCallback;

        struct
        {
            std::uint8_t r = 255, g = 255, b = 255, a = 255;
        } background_color;

#if defined(_WIN32)
        std::mutex run_mutex;
        std::vector<std::function<void()>> run_on_init_done;
        HINSTANCE hInt = nullptr;
        HWND hwnd = nullptr;
        MSG msg = {};
        wil::com_ptr<ICoreWebView2Controller> webview_controller;
        wil::com_ptr<ICoreWebView2> webviewWindow;
        bool ready = false;

        void resize();
        static LRESULT CALLBACK WndProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#elif defined(__linux__)
        bool ready = false;
        bool javascript_busy = false;
        std::vector<std::function<void()>> run_on_load;

        GtkWidget *window;
        GtkWidget *webview;

        static void externalMessageReceived(WebKitUserContentManager *m, WebKitJavascriptResult *r, gpointer arg);
        static void webViewLoadChanged(WebKitWebView *webview, WebKitLoadEvent event, gpointer arg);
        static void webViewEvalFinished(GObject *object, GAsyncResult *result, gpointer arg);
        static void destroyWindow(GtkWidget *widget, gpointer arg);

        static gboolean webViewContextMenu(WebKitWebView *webview, GtkWidget *default_menu,
                                           WebKitHitTestResult *hit_test_result, gboolean triggered_with_keyboard,
                                           gpointer user_data);
        static gboolean webViewResize(WebKitWebView *webview, GdkEvent *event, gpointer user_data);
#elif defined(__APPLE__)
        NSAutoreleasePool *pool;
        WKWebView *webview;
        NSWindow *window;
#endif

      public:
        WebView(int width, int height, bool resizable, std::string title, std::string url, bool readyEarly = false,
                bool debug = false)
            : width(width), height(height), resizable(resizable), title(std::move(title)), url(std::move(url)),
              ready_early(readyEarly), debug(debug)
        {
        }

        bool init();
        void exit();
        bool run();

        void css(const std::string &css);
        void navigate(const std::string &url);
        void eval(const std::string &code, bool wait_for_ready = true, bool wait_for_finish = true);

        void setTitle(const std::string &t);
        void setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setResizeCallback(const std::function<void(int, int)> &callback);
        void addCallback(const std::string &name, const callback_t &callback, bool is_object = false,
                         bool execute_on_load = true);
    };

    inline void WebView::setResizeCallback(const std::function<void(int, int)> &callback)
    {
        onResizeCallback = callback;
    }

#if defined(_WIN32)
    // God please forgive me my sins, writing code on windows is worse than hell
    static std::wstring charToWString(const char *text)
    {
        const size_t size = std::strlen(text);
        std::wstring wstr;
        if (size > 0)
        {
            wstr.resize(size);
            std::mbstowcs(&wstr[0], text, size);
        }
        return wstr;
    }
    static std::string ws2s(const std::wstring &wstr)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.to_bytes(wstr);
    }

    inline bool WebView::init()
    {
        hInt = GetModuleHandle(nullptr);
        if (hInt == nullptr)
        {
            return false;
        }

        // Initialize Win32 window
        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0;
        wc.lpfnWndProc = WndProcedure;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInt;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = "soundux-webview";
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClassEx(&wc))
        {
            return false;
        }

        hwnd = CreateWindow("soundux-webview", title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640,
                            480, nullptr, nullptr, hInt, nullptr);

        if (hwnd == nullptr)
        {
            return false;
        }

        // Set window size
        RECT r;
        r.left = 30;
        r.top = 70;
        r.right = width + r.left;
        r.bottom = height + r.top;
        SetWindowPos(hwnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        // Used with GetWindowLongPtr
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);
        SetFocus(hwnd);

        HRESULT hr = CreateCoreWebView2Environment(
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([&](HRESULT result,
                                                                                     ICoreWebView2Environment *env)
                                                                                     -> HRESULT {
                // Create Webview2 controller
                HRESULT hr = env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [&](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
                            if (FAILED(result))
                            {
                                return result;
                            }

                            if (controller != nullptr)
                            {
                                webview_controller = controller;
                                webview_controller->get_CoreWebView2(&webviewWindow);
                            }

                            wil::com_ptr<ICoreWebView2Settings> settings;
                            webviewWindow->get_Settings(&settings);
                            if (!debug)
                            {
                                settings->put_AreDevToolsEnabled(0);
                                settings->put_IsBuiltInErrorPageEnabled(0);
                                settings->put_AreDefaultScriptDialogsEnabled(0);
                                settings->put_IsStatusBarEnabled(0);
                                settings->put_AreHostObjectsAllowed(0);
                                settings->put_AreDefaultContextMenusEnabled(0);
                            }
                            settings->put_IsZoomControlEnabled(0);

                            resize();

                            webviewWindow->AddScriptToExecuteOnDocumentCreated(
                                charToWString(INVOKE_CODE.data()).c_str(), nullptr);

                            EventRegistrationToken token;
                            webviewWindow->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [this]([[maybe_unused]] ICoreWebView2 *webview,
                                           ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT {
                                        LPWSTR messageRaw = nullptr;
                                        args->TryGetWebMessageAsString(&messageRaw);
                                        std::wstring message(messageRaw);

                                        if (!callbacks.empty())
                                        {
                                            nlohmann::json j = nlohmann::json::parse(ws2s(message), nullptr, false);
                                            if (j.find("func") != j.end() && j.find("param") != j.end() &&
                                                j.at("func").is_string() && j.at("param").is_array() &&
                                                j.at("seq").is_number())
                                            {
                                                const auto &seq = j.at("seq").get<int>();
                                                const auto &name = j.at("func").get<std::string>();
                                                const auto &params = j.at("param").get<std::vector<std::string>>();
                                                auto rtn = callbacks.at(name)(*this, params);

                                                // clang-format off
                                                auto code = fmt::format(R"js(
                                                    window._rpc[{0}].resolve(`{1}`);
                                                    delete window._rpc[{0}];
                                                )js",seq, rtn);
                                                // clang-format on

                                                eval(code);
                                            }
                                        }
                                        CoTaskMemFree(messageRaw);
                                        return S_OK;
                                    })
                                    .Get(),
                                &token);

                            webviewWindow->AddScriptToExecuteOnDocumentCreated(
                                L"window._rpc = {}; window._rpc_seq = 0;",
                                Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
                                    [this]([[maybe_unused]] HRESULT error, [[maybe_unused]] PCWSTR id) -> HRESULT {
                                        ready = true;
                                        return S_OK;
                                    })
                                    .Get());

                            init_done = true;

                            setTitle(title);
                            setBackgroundColor(background_color.r, background_color.g, background_color.b,
                                               background_color.a);
                            navigate(url);

                            run_mutex.lock();
                            for (const auto &fn : run_on_init_done)
                            {
                                fn();
                            }
                            run_on_init_done.clear();
                            run_mutex.unlock();

                            return S_OK;
                        })
                        .Get());

                return hr;
            }).Get());

        return !FAILED(hr);
    }

    inline void WebView::addCallback(const std::string &name, const callback_t &callback, bool is_object,
                                     [[maybe_unused]] bool execute_on_load)
    {
        callbacks.insert({name, callback});
        if (is_object)
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return JSON.parse(await promise);
                }}
            )js", name);
            // clang-format on

            eval(code, true, false);
        }
        else
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return await promise;
                }}
            )js", name);
            // clang-format on

            eval(code, true, false);
        }
    }

    inline void WebView::setTitle(const std::string &t)
    {
        if (!init_done)
        {
            title = t;
        }
        else
        {
            SetWindowText(hwnd, t.c_str());
        }
    }

    inline void WebView::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (!init_done)
        {
            background_color = {r, g, b, a};
        }
    }

    inline bool WebView::run()
    {
        if (PeekMessage(&msg, nullptr, 0, 0, 1) != -1)
        {
            if (msg.message)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return !should_exit;
    }

    inline void WebView::navigate(const std::string &url)
    {
        this->url = url;
        if (init_done)
        {
            webviewWindow->Navigate(charToWString(url.c_str()).c_str());
        }
    }

    inline void WebView::eval(const std::string &code, [[maybe_unused]] bool wait_for_ready,
                              [[maybe_unused]] bool wait_for_finish)
    {
        if (!init_done)
        {
            run_mutex.lock();
            run_on_init_done.push_back([=] { eval(code); });
            run_mutex.unlock();
        }
        else
        {
            if (ready)
            {
                webviewWindow->ExecuteScript(
                    charToWString(std::regex_replace(code, std::regex(R"rgx(\\)rgx"), R"(\\)").c_str()).c_str(),
                    Callback<ICoreWebView2ExecuteScriptCompletedHandler>([](HRESULT errorCode,
                                                                            LPCWSTR resultObjectAsJson) -> HRESULT {
                        return S_OK;
                    }).Get());
            }
            else
            {
                webviewWindow->AddScriptToExecuteOnDocumentCreated(
                    charToWString(std::regex_replace(code, std::regex(R"rgx(\\)rgx"), R"(\\)").c_str()).c_str(),
                    Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
                        [this](HRESULT error, PCWSTR id) -> HRESULT { return S_OK; })
                        .Get());
            }
        }
    }

    inline void WebView::exit()
    {
        should_exit = true;
        PostQuitMessage(WM_QUIT);
    }

    inline void WebView::resize()
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        webview_controller->put_Bounds(rc);

        if (onResizeCallback)
        {
            onResizeCallback(rc.left + rc.right, rc.bottom + rc.top);
        }
    }

    inline LRESULT CALLBACK WebView::WndProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        auto *w = reinterpret_cast<WebView *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg)
        {
        case WM_SIZE:
            if (w != nullptr && w->init_done)
            {
                w->resize();
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
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
#elif defined(__linux__)
    inline bool WebView::init()
    {
        if (gtk_init_check(nullptr, nullptr) == FALSE)
        {
            return false;
        }

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        if (resizable)
        {
            gtk_window_set_default_size(GTK_WINDOW(window), width, height); // NOLINT
        }
        else
        {
            gtk_widget_set_size_request(window, width, height);
        }

        gtk_window_set_resizable(GTK_WINDOW(window), resizable);         // NOLINT
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // NOLINT

        GtkWidget *scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
        gtk_container_add(GTK_CONTAINER(window), scrolled_window); // NOLINT

        WebKitUserContentManager *cm = webkit_user_content_manager_new();
        webkit_user_content_manager_register_script_message_handler(cm, "external");

        // NOLINTNEXTLINE
        g_signal_connect(cm, "script-message-received::external", G_CALLBACK(externalMessageReceived), this);

        webview = webkit_web_view_new_with_user_content_manager(cm);
        g_signal_connect(G_OBJECT(webview), "load-changed", G_CALLBACK(webViewLoadChanged), this); // NOLINT
        gtk_container_add(GTK_CONTAINER(scrolled_window), webview);                                // NOLINT

        g_signal_connect(window, "destroy", G_CALLBACK(destroyWindow), this); // NOLINT

        if (debug)
        {
            WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview)); // NOLINT
            webkit_settings_set_enable_write_console_messages_to_stdout(settings, true);
            webkit_settings_set_enable_developer_extras(settings, true);
        }
        else
        {
            g_signal_connect(G_OBJECT(webview), "context-menu", G_CALLBACK(webViewContextMenu), nullptr); // NOLINT
        }

        g_signal_connect(GTK_WINDOW(window), "configure-event", G_CALLBACK(webViewResize), this); // NOLINT

        init_done = true;

        setTitle(title);
        setBackgroundColor(background_color.r, background_color.g, background_color.b, background_color.a);
        navigate(url);

        gtk_widget_grab_focus(GTK_WIDGET(webview)); // NOLINT
        gtk_widget_show_all(window);

        return true;
    }

    inline void WebView::addCallback(const std::string &name, const callback_t &callback, bool is_object,
                                     bool execute_on_load)
    {
        callbacks.insert({name, callback});
        if (is_object)
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return JSON.parse(await promise);
                }}
            )js", name);
            // clang-format on

            if (execute_on_load)
            {
                run_on_load.push_back([=] { eval(code, true, false); });
            }
            else
            {
                eval(code, true, false);
            }
        }
        else
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return await promise;
                }}
            )js", name);
            // clang-format on

            if (execute_on_load)
            {
                run_on_load.push_back([=] { eval(code, true, false); });
            }
            else
            {
                eval(code, true, false);
            }
        }
    }

    inline void WebView::setTitle(const std::string &title)
    {
        if (!init_done)
        {
            this->title = title;
        }
        else
        {
            gtk_window_set_title(GTK_WINDOW(window), title.c_str()); // NOLINT
        }
    }

    inline void WebView::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (!init_done)
        {
            background_color = {r, g, b, a};
        }
        else
        {
            GdkRGBA color = {r / 255.0, g / 255.0, b / 255.0, a / 255.0};
            webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &color); // NOLINT
        }
    }

    inline bool WebView::run() // NOLINT
    {
        gtk_main_iteration_do(true);
        return !should_exit;
    }

    inline void WebView::navigate(const std::string &url)
    {
        this->url = url;
        if (init_done)
        {
            webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), url.c_str()); // NOLINT
        }
    }

    inline void WebView::eval(const std::string &code, bool wait_for_ready, bool wait_for_finish)
    {
        while (!init_done || (wait_for_ready && !ready))
        {
            g_main_context_iteration(nullptr, TRUE);
        }

        javascript_busy = true;

        // NOLINTNEXTLINE
        webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(webview),
                                       std::regex_replace(code, std::regex(R"rgx(\\")rgx"), R"(\\\")").c_str(), nullptr,
                                       webViewEvalFinished, this);

        while (javascript_busy && wait_for_finish)
        {
            g_main_context_iteration(nullptr, TRUE); // NOLINT
        }
    }

    inline void WebView::exit()
    {
        should_exit = true;
    }

    inline void WebView::externalMessageReceived([[maybe_unused]] WebKitUserContentManager *manager,
                                                 WebKitJavascriptResult *result, gpointer arg)
    {
        auto *webView = static_cast<WebView *>(arg);
        if (!webView->callbacks.empty())
        {
            JSCValue *value = webkit_javascript_result_get_js_value(result);
            std::string str = std::string(jsc_value_to_string(value));

            nlohmann::json j = nlohmann::json::parse(str, nullptr, false);
            if (!j.is_discarded())
            {
                if (j.find("func") != j.end() && j.find("param") != j.end() && j.at("func").is_string() &&
                    j.at("param").is_array() && j.at("seq").is_number())
                {
                    const auto &seq = j.at("seq").get<int>();
                    const auto &name = j.at("func").get<std::string>();
                    const auto &params = j.at("param").get<std::vector<std::string>>();
                    auto rtn = webView->callbacks.at(name)(*webView, params);

                    // clang-format off
                    auto code = fmt::format(R"js(
                        window._rpc[{0}].resolve(`{1}`);
                        delete window._rpc[{0}];
                    )js",seq, rtn);
                    // clang-format on

                    webView->eval(code);
                }
            }
        }
    }

    inline void WebView::webViewEvalFinished([[maybe_unused]] GObject *object, [[maybe_unused]] GAsyncResult *result,
                                             gpointer arg)
    {
        static_cast<WebView *>(arg)->javascript_busy = false;
    }

    inline void WebView::webViewLoadChanged([[maybe_unused]] WebKitWebView *webview, WebKitLoadEvent event,
                                            gpointer arg)
    {
        auto *webView = static_cast<WebView *>(arg);
        if (webView)
        {
            if ((webView->ready_early && event == WEBKIT_LOAD_COMMITTED) ||
                (!webView->ready_early && event == WEBKIT_LOAD_FINISHED))
            {
                webView->ready = true;
                webView->eval(INVOKE_CODE.data(), false, false);
                webView->eval("window._rpc = {}; window._rpc_seq = 0;");
                for (const auto &fun : webView->run_on_load)
                {
                    fun();
                }
            }
        }
    }

    inline void WebView::destroyWindow([[maybe_unused]] GtkWidget *widget, gpointer arg)
    {
        static_cast<WebView *>(arg)->exit();
    }

    inline gboolean WebView::webViewContextMenu([[maybe_unused]] WebKitWebView *webview,
                                                [[maybe_unused]] GtkWidget *default_menu,
                                                [[maybe_unused]] WebKitHitTestResult *hit_test_result,
                                                [[maybe_unused]] gboolean triggered_with_keyboard,
                                                [[maybe_unused]] gpointer user_data)
    {
        return TRUE;
    }

    inline gboolean WebView::webViewResize([[maybe_unused]] WebKitWebView *view, GdkEvent *event, gpointer user_data)
    {
        auto *gtkEvent = reinterpret_cast<GdkEventConfigure *>(event);
        auto *webview = reinterpret_cast<WebView *>(user_data);

        if (webview && webview->onResizeCallback)
        {
            webview->onResizeCallback(gtkEvent->width, gtkEvent->height);
        }

        return FALSE;
    }
#elif defined(__APPLE__)
    inline bool WebView::init()
    {
        pool = [NSAutoreleasePool new];
        uint style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

        if (resizable)
        {
            style |= NSWindowStyleMaskResizable;
        }

        window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
                                             styleMask:style
                                               backing:NSBackingStoreBuffered
                                                 defer:NO];

        [window setContentMinSize:NSMakeSize(width, height)];

        [window center];

        WKWebViewConfiguration *config = [WKWebViewConfiguration new];
        WKPreferences *prefs = [config preferences];
        [prefs setJavaScriptCanOpenWindowsAutomatically:NO];
        [prefs setValue:@YES forKey:@"developerExtrasEnabled"];

        WKUserContentController *controller = [config userContentController];
        WKUserScript *userScript = [WKUserScript alloc];
        [userScript initWithSource:[NSString stringWithUTF8String:INVOKE_CODE.data()]
                     injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                  forMainFrameOnly:NO];
        [controller addUserScript:userScript];

        webview = [[WKWebView alloc] initWithFrame:NSZeroRect configuration:config];

        class_replaceMethod([WindowDelegate class], @selector(windowWillClose:),
                            imp_implementationWithBlock([=](id self, SEL cmd, id notification) { this->exit(); }),
                            "v@:@");

        class_replaceMethod([WindowDelegate class], @selector(userContentController:didReceiveScriptMessage:),
                            imp_implementationWithBlock([=](id self, SEL cmd, WKScriptMessage *scriptMessage) {
                                if (!callbacks.empty())
                                {
                                    id body = [scriptMessage body];
                                    if (![body isKindOfClass:[NSString class]])
                                    {
                                        return;
                                    }

                                    std::string str = [body UTF8String];
                                    nlohmann::json j = nlohmann::json::parse(str, nullptr, false);
                                    if (!j.is_discarded())
                                    {
                                        if (j.find("func") != j.end() && j.find("param") != j.end() &&
                                            j.at("func").is_string() && j.at("param").is_array() &&
                                            j.at("seq").is_number())
                                        {
                                            const auto &seq = j.at("seq").get<int>();
                                            const auto &name = j.at("func").get<std::string>();
                                            const auto &params = j.at("param").get<std::vector<std::string>>();
                                            auto rtn = callbacks.at(name)(*this, params);

                                            // clang-format off
                                            auto code = fmt::format(R"js(
                                                window._rpc[{0}].resolve(`{1}`);
                                                delete window._rpc[{0}];
                                            )js",seq, rtn);
                                            // clang-format on

                                            eval(code);
                                        }
                                    }
                                }
                            }),
                            "v@:@");

        WindowDelegate *delegate = [WindowDelegate alloc];
        [controller addScriptMessageHandler:delegate name:@"soundux-webview"];
        [window setDelegate:delegate];

        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        [window setContentView:webview];
        [window makeKeyAndOrderFront:nil];

        init_done = true;

        setTitle(title);
        setBackgroundColor(background_color.r, background_color.g, background_color.b, background_color.a);
        navigate(url);

        return true;
    }

    inline void WebView::addCallback(const std::string &name, const callback_t &callback, bool is_object,
                                     [[maybe_unused]] bool execute_on_load)
    {
        callbacks.insert({name, callback});
        if (is_object)
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return JSON.parse(await promise);
                }}
            )js", name);
            // clang-format on

            eval(code);
        }
        else
        {
            // clang-format off
            auto code = fmt::format(R"js(
                async function {0}(...param)
                {{
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {{
                        window._rpc[seq] = {{
                            resolve: resolve
                        }};
                    }});
                    window.external.invoke(JSON.stringify({{
                        "seq": seq,
                        "func": "{0}",
                        "param": Array.from(param).map(x => `${{x}}`)
                    }}));
                    return await promise;
                }}
            )js", name);
            // clang-format on

            eval(code);
        }
    }

    inline void WebView::setTitle(const std::string &title)
    {
        if (!init_done)
        {
            this->title = title;
        }
        else
        {
            [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
        }
    }

    inline void WebView::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (!init_done)
        {
            background_color = {r, g, b, a};
        }
        else
        {
            [window setBackgroundColor:[NSColor colorWithCalibratedRed:r / 255.0
                                                                 green:g / 255.0
                                                                  blue:b / 255.0
                                                                 alpha:a / 255.0]];
        }
    }

    inline bool WebView::run()
    {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantFuture]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:true];
        if (event)
        {
            [NSApp sendEvent:event];
        }

        return should_exit;
    }

    inline void WebView::navigate(const std::string &url)
    {
        this->url = url;
        if (init_done)
        {
            [webview loadRequest:[NSURLRequest
                                     requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]]];
        }
    }

    inline void WebView::eval(const std::string &code, [[maybe_unused]] bool wait_for_ready,
                              [[maybe_unused]] bool wait_for_finish)
    {
        [webview evaluateJavaScript:[NSString stringWithUTF8String:code.c_str()] completionHandler:nil];
    }

    inline void WebView::exit()
    {
        should_exit = true;
        [NSApp terminate:nil];
    }
#endif

} // namespace wv