#ifndef WEBVIEW_H
#define WEBVIEW_H

#if !defined(WEBVIEW_WIN) && !defined(WEBVIEW_EDGE) && \
    !defined(WEBVIEW_MAC) && !defined(WEBVIEW_GTK)
#error "Define one of WEBVIEW_WIN, WEBVIEW_EDGE, WEBVIEW_MAC, or WEBVIEW_GTK"
#endif

#if defined(WEBVIEW_WIN) || defined(WEBVIEW_EDGE)
#define WEBVIEW_IS_WIN
#endif

// Helper defines
#if defined(WEBVIEW_IS_WIN)
#define WEBVIEW_MAIN int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#define GetProcNameAddress(hmod, proc) \
    reinterpret_cast<decltype(proc)*>(GetProcAddress(hmod, #proc))
#define UNICODE
#define _UNICODE
#define Str(s) L##s
#else
#define WEBVIEW_MAIN int main(int, char**)
#define Str(s) s
#endif

// Headers
#include <functional>
#include <string>

#if defined(WEBVIEW_WIN)
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "windowsapp")

#include <objbase.h>
#include <shellscalingapi.h>
#include <windows.h>
#include <winrt/Windows.Web.UI.Interop.h>

#include <memory>
#include <type_traits>

#pragma warning(push)
#pragma warning(disable : 4265)
#include <winrt/Windows.Foundation.Collections.h>
#pragma warning(pop)
#elif defined(WEBVIEW_EDGE)  // WEBVIEW_WIN
#include <WebView2.h>
#include <shellscalingapi.h>
#include <tchar.h>
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>
#elif defined(WEBVIEW_MAC)  // WEBVIEW_EDGE
#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#include <objc/objc-runtime.h>

// ObjC declarations may only appear in global scope
@interface WindowDelegate : NSObject <NSWindowDelegate, WKScriptMessageHandler>
@end

@implementation WindowDelegate
- (void)userContentController:(WKUserContentController*)userContentController
      didReceiveScriptMessage:(WKScriptMessage*)scriptMessage {
}
@end
#elif defined(WEBVIEW_GTK)  // WEBVIEW_MAC
#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#endif

constexpr auto DEFAULT_URL = Str(R"(data:text/html,
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
</head>
<body>
<div id="app"></div>
<script type="text/javascript"></script>
</body>
</html>)");

/*"data:text/html,"
        "%3C%21DOCTYPE%20html%3E%0A%3Chtml%20lang=%22en%22%3E%0A%3Chead%3E%"
        "3Cmeta%20charset=%22utf-8%22%3E%3Cmeta%20http-equiv=%22X-UA-Compatible%22%"
        "20content=%22IE=edge%22%3E%3C%2Fhead%3E%0A%3Cbody%3E%3Cdiv%20id=%22app%22%"
        "3E%3C%2Fdiv%3E%3Cscript%20type=%22text%2Fjavascript%22%3E%3C%2Fscript%3E%"
        "3C%2Fbody%3E%0A%3C%2Fhtml%3E");
*/

namespace wv {
// wv::String
#if defined(WEBVIEW_IS_WIN)
using String = std::wstring;
#else
using String = std::string;
#endif

// Namespaces
#if defined(WEBVIEW_WIN)
using namespace winrt::impl;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Web::UI::Interop;
#elif defined(WEBVIEW_EDGE)
using namespace Microsoft::WRL;
#endif

class WebView {
    using jscb = std::function<void(WebView&, String&)>;

public:
    WebView(int width_, int height_, bool resizable_, bool debug_,
            const String& title_, const String& url_ = DEFAULT_URL)
        : width(width_),
          height(height_),
          resizable(resizable_),
          debug(debug_),
          title(title_),
          url(url_) {}
    int init();                       // Initialize webview
    void setCallback(jscb callback);  // JS callback
    void setTitle(String t);          // Set title of window
    void setFullscreen(bool fs);      // Set fullscreen
    void setBgColor(uint8_t r, uint8_t g, uint8_t b,
                    uint8_t a);      // Set background color
    bool run();                      // Main loop
    void navigate(String u);         // Navigate to URL
    void preEval(const String& js);  // Eval JS before page loads
    void eval(const String& js);     // Eval JS
    void css(const String& css);     // Inject CSS
    void exit();                     // Stop loop

private:
    // Properties for init
    int width;
    int height;
    bool resizable;
    bool fullscreen = false;
    bool debug;
    String title;
    String url;

    jscb js_callback;
    bool init_done = false;  // Finished running init
    uint8_t bgR = 255, bgG = 255, bgB = 255, bgA = 255;

// Common Windows stuff
#if defined(WEBVIEW_IS_WIN)
    HINSTANCE hInt = nullptr;
    HWND hwnd = nullptr;
    MSG msg;  // Message from main loop
    bool isFullscreen = false;

    struct WindowInfo {
        LONG_PTR style;    // GWL_STYLE
        LONG_PTR exstyle;  // GWL_EXSTYLE
        RECT rect;         // GetWindowRect
    } savedWindowInfo;

    int WinInit();
    void setDPIAwareness();
    void resize();
    static LRESULT CALLBACK WndProcedure(HWND hwnd, UINT msg, WPARAM wparam,
                                         LPARAM lparam);
#endif  // WEBVIEW_WIN || WEBVIEW_EDGE

#if defined(WEBVIEW_WIN)
    String inject =
        Str("window.external.invoke=arg=>window.external.notify(arg);");
    WebViewControl webview{nullptr};
#elif defined(WEBVIEW_EDGE)  // WEBVIEW_WIN
    String inject = Str(
        "window.external.invoke=arg=>window.chrome.webview.postMessage(arg);");
    wil::com_ptr<ICoreWebView2Controller>
        webviewController;                      // Pointer to WebViewController
    wil::com_ptr<ICoreWebView2> webviewWindow;  // Pointer to WebView window
#elif defined(WEBVIEW_MAC)   // WEBVIEW_EDGE
    String inject =
        Str("window.external={invoke:arg=>window.webkit."
            "messageHandlers.webview.postMessage(arg)};");
    bool should_exit = false;  // Close window
    NSAutoreleasePool* pool;
    NSWindow* window;
    WKWebView* webview;
#elif defined(WEBVIEW_GTK)   // WEBVIEW_MAC
    String inject =
        Str("window.external={invoke:arg=>window.webkit."
            "messageHandlers.external.postMessage(arg)};");
    bool ready = false;        // Done loading page
    bool js_busy = false;      // Currently in JS eval
    bool should_exit = false;  // Close window
    GtkWidget* window;
    GtkWidget* webview;

    static void external_message_received_cb(WebKitUserContentManager* m,
                                             WebKitJavascriptResult* r,
                                             gpointer arg);
    static void webview_eval_finished(GObject* object, GAsyncResult* result,
                                      gpointer arg);
    static void webview_load_changed_cb(WebKitWebView* webview,
                                        WebKitLoadEvent event, gpointer arg);
    static void destroyWindowCb(GtkWidget* widget, gpointer arg);
    // static gboolean closeWebViewCb(WebKitWebView *webView, GtkWidget
    // *window);
    static gboolean webview_context_menu_cb(
        WebKitWebView* webview, GtkWidget* default_menu,
        WebKitHitTestResult* hit_test_result, gboolean triggered_with_keyboard,
        gpointer userdata);
#endif                       // WEBVIEW_GTK
};

// Common Windows methods
#if defined(WEBVIEW_IS_WIN)
int WebView::WinInit() {
    hInt = GetModuleHandle(nullptr);
    if (hInt == nullptr) {
        return -1;
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"webview";
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"Call to RegisterClassEx failed!", L"Error!",
                   NULL);

        return 1;
    }

    // Set default DPI awareness
    setDPIAwareness();

    hwnd = CreateWindow(L"webview", title.c_str(), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr,
                        nullptr, hInt, nullptr);

    if (hwnd == nullptr) {
        MessageBox(nullptr, L"Window Registration Failed!", L"Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    // Set window size
    RECT r{0, 0, width, height};
    SetWindowPos(hwnd, nullptr, r.left, r.top, r.right - r.left,
                 r.bottom - r.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    if (!resizable) {
        auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }

    // Used with GetWindowLongPtr in WndProcedure
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    SetFocus(hwnd);

    return 0;
}

void WebView::setTitle(std::wstring t) {
    if (!init_done) {
        title = t;
    } else {
        SetWindowText(hwnd, t.c_str());
    }
}

// Adapted from
// https://source.chromium.org/chromium/chromium/src/+/main:ui/views/win/fullscreen_handler.cc
void WebView::setFullscreen(bool fs) {
    if (isFullscreen == fs) return;

    isFullscreen = fs;

    if (fs) {
        // Store window style before going fullscreen
        savedWindowInfo.style = GetWindowLongPtr(hwnd, GWL_STYLE);
        savedWindowInfo.exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        GetWindowRect(hwnd, &savedWindowInfo.rect);

        // Set new window style
        SetWindowLongPtr(hwnd, GWL_STYLE,
                         savedWindowInfo.style & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowLongPtr(
            hwnd, GWL_EXSTYLE,
            savedWindowInfo.exstyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                                        WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

        // Get monitor size
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);
        GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST),
                       &monitorInfo);

        // Set window size to monitor size
        SetWindowPos(hwnd, nullptr, monitorInfo.rcMonitor.left,
                     monitorInfo.rcMonitor.top,
                     monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                     monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    } else {
        // Restore window style
        SetWindowLongPtr(hwnd, GWL_STYLE, savedWindowInfo.style);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, savedWindowInfo.exstyle);

        // Restore window size
        SetWindowPos(hwnd, nullptr, savedWindowInfo.rect.left,
                     savedWindowInfo.rect.top,
                     savedWindowInfo.rect.right - savedWindowInfo.rect.left,
                     savedWindowInfo.rect.bottom - savedWindowInfo.rect.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
}

bool WebView::run() {
    bool loop = GetMessage(&msg, nullptr, 0, 0) > 0;
    if (loop) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return !loop;
}

LRESULT CALLBACK WebView::WndProcedure(HWND hwnd, UINT msg, WPARAM wparam,
                                       LPARAM lparam) {
    WebView* w =
        reinterpret_cast<WebView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_SIZE:
            // WM_SIZE will first fire before the webview finishes loading
            // init() calls resize(), so this call is only for size changes
            // after fully loading.
            if (w != nullptr && w->init_done) {
                w->resize();
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        case WM_DESTROY:
            w->exit();
            break;
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void WebView::setDPIAwareness() {
    // Set default DPI awareness
    std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>
        user32(LoadLibrary(TEXT("User32.dll")), FreeLibrary);
    // WIL alternative:
    // wil::unique_hmodule user32(LoadLibrary(TEXT("User32.lib")));
    auto pSPDAC =
        GetProcNameAddress(user32.get(), SetProcessDpiAwarenessContext);
    if (pSPDAC != nullptr) {
        // Windows 10
        pSPDAC(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
        return;
    }

    std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>
        shcore(LoadLibrary(TEXT("ShCore.dll")), FreeLibrary);
    auto pSPDA = GetProcNameAddress(shcore.get(), SetProcessDpiAwareness);
    if (pSPDA != nullptr) {
        // Windows 8.1
        pSPDA(PROCESS_PER_MONITOR_DPI_AWARE);
        return;
    }

    // Windows Vista
    SetProcessDPIAware();
}
#endif

#if defined(WEBVIEW_WIN)
// Await helper
template <typename T>
auto block(T const& async) {
    if (async.Status() != AsyncStatus::Completed) {
        winrt::handle h(CreateEvent(nullptr, false, false, nullptr));
        async.Completed([h = h.get()](auto, auto) { SetEvent(h); });
        HANDLE hs[] = {h.get()};
        DWORD i;
        CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES |
                                     COWAIT_DISPATCH_CALLS |
                                     COWAIT_INPUTAVAILABLE,
                                 INFINITE, 1, hs, &i);
    }
    return async.GetResults();
}

int WebView::init() {
    if (auto res = WinInit(); res) {
        return res;
    }

    // Set to single-thread
    init_apartment(winrt::apartment_type::single_threaded);

    // Allow intranet access (and localhost)
    WebViewControlProcessOptions options;
    options.PrivateNetworkClientServerCapability(
        WebViewControlProcessCapabilityState::Enabled);

    WebViewControlProcess proc(options);
    webview = block(proc.CreateWebViewControlAsync(
        reinterpret_cast<int64_t>(hwnd), Rect()));
    webview.Settings().IsScriptNotifyAllowed(true);
    webview.ScriptNotify([=](auto const&, auto const& args) {
        if (js_callback) {
            std::string s = winrt::to_string(args.Value());
            std::wstring ws(s.begin(), s.end());
            js_callback(*this, ws);
        }
    });
    webview.NavigationStarting(
        [=](auto&&, auto&&) { webview.AddInitializeScript(inject); });

    // Set webview bounds
    resize();

    webview.IsVisible(true);

    // Done initialization, set properties
    init_done = true;

    setTitle(title);
    if (fullscreen) {
        setFullscreen(true);
    }
    setBgColor(bgR, bgG, bgB, bgA);
    navigate(url);

    return 0;
}

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

void WebView::navigate(std::wstring u) {
    if (!init_done) {
        url = u;
    } else {
        Uri uri{u};
        webview.Navigate(uri);
    }
}

void WebView::eval(const std::wstring& js) {
    auto result = block(webview.InvokeScriptAsync(
        L"eval", std::vector<winrt::hstring>({winrt::hstring(js)})));

    // if (debug) {
    // std::cout << winrt::to_string(result) << std::endl;
    //}
}

void WebView::exit() { PostQuitMessage(WM_QUIT); }

void WebView::resize() {
    RECT rc;
    GetClientRect(hwnd, &rc);
    Rect bounds((float)rc.left, (float)rc.top, (float)(rc.right - rc.left),
                (float)(rc.bottom - rc.top));
    webview.Bounds(bounds);
}
#elif defined(WEBVIEW_EDGE)  // WEBVIEW_WIN
int WebView::init() {
    if (auto res = WinInit(); res) {
        return res;
    }

    // Set to single-thread
    auto inithr = CoInitializeEx(
        nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(inithr)) {
        return -1;
    }

    auto onWebMessageReceieved =
        [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) {
            if (js_callback) {
                // Consider args->get_WebMessageAsJson?
                LPWSTR messageRaw;
                auto getMessageResult =
                    args->TryGetWebMessageAsString(&messageRaw);
                if (FAILED(getMessageResult)) {
                    return getMessageResult;
                }

                std::wstring message(messageRaw);
                js_callback(*this, message);
                CoTaskMemFree(messageRaw);
            }
            return S_OK;
        };

    auto onWebViewControllerCreate =
        [this, onWebMessageReceieved](
            HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
        if (FAILED(result)) {
            return result;
        }

        if (controller != nullptr) {
            webviewController = controller;
            webviewController->get_CoreWebView2(&webviewWindow);
        }

        wil::com_ptr<ICoreWebView2Settings> settings;
        webviewWindow->get_Settings(&settings);
        if (!debug) {
            settings->put_AreDevToolsEnabled(FALSE);
        }

        // Resize WebView
        resize();

        webviewWindow->AddScriptToExecuteOnDocumentCreated(inject.c_str(),
                                                           nullptr);

        EventRegistrationToken token;
        webviewWindow->add_WebMessageReceived(
            Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                onWebMessageReceieved)
                .Get(),
            &token);

        // Done initialization, set properties
        init_done = true;

        setTitle(title);
        if (fullscreen) {
            setFullscreen(true);
        }
        setBgColor(bgR, bgG, bgB, bgA);
        navigate(url);

        return S_OK;
    };

    auto onCreateEnvironment = [this, onWebViewControllerCreate](
                                   HRESULT result,
                                   ICoreWebView2Environment* env) -> HRESULT {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            MessageBox(nullptr, L"Could not find Edge installation.", L"Error!",
                       NULL);
            return result;
        }

        // Create Webview2 controller
        return env->CreateCoreWebView2Controller(
            hwnd,
            Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                onWebViewControllerCreate)
                .Get());
    };

    // Create WebView2 environment
    auto hr = CreateCoreWebView2Environment(
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            onCreateEnvironment)
            .Get());

    if (FAILED(hr)) {
        CoUninitialize();
        return -1;
    }

    return 0;
}

void WebView::setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!init_done) {
        bgR = r;
        bgG = g;
        bgB = b;
        bgA = a;
    } else {
        // TODO
    }
}

void WebView::navigate(std::wstring u) {
    if (!init_done) {
        url = u;
    } else {
        webviewWindow->Navigate(u.c_str());
    }
}

void WebView::eval(const std::wstring& js) {
    // Schedule an async task to get the document URL
    webviewWindow->ExecuteScript(
        js.c_str(), Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                        [](HRESULT, LPCWSTR) -> HRESULT {
                            // LPCWSTR URL = resultObjectAsJson;
                            // doSomethingWithURL(URL);
                            return S_OK;
                        })
                        .Get());

    // if (debug) {
    // std::cout << winrt::to_string(result) << std::endl;
    //}
}

void WebView::exit() {
    PostQuitMessage(WM_QUIT);
    CoUninitialize();
}

void WebView::resize() {
    RECT rc;
    GetClientRect(hwnd, &rc);
    webviewController->put_Bounds(rc);
}
#elif defined(WEBVIEW_MAC)   // WEBVIEW_EDGE
int WebView::init() {
    // Initialize autorelease pool
    pool = [NSAutoreleasePool new];

    // Window style: titled, closable, minimizable
    uint style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                 NSWindowStyleMaskMiniaturizable;

    // Set window to be resizable
    if (resizable) {
        style |= NSWindowStyleMaskResizable;
    }

    // Initialize Cocoa window
    window = [[NSWindow alloc]
        // Initial window size
        initWithContentRect:NSMakeRect(0, 0, width, height)
                  // Window style
                  styleMask:style
                    backing:NSBackingStoreBuffered
                      defer:NO];

    // Minimum window size
    [window setContentMinSize:NSMakeSize(width, height)];

    // Position window in center of screen
    [window center];

    // Initialize WKWebView
    WKWebViewConfiguration* config = [WKWebViewConfiguration new];
    WKPreferences* prefs = [config preferences];
    [prefs setJavaScriptCanOpenWindowsAutomatically:NO];
    if (debug) {
        [prefs setValue:@YES forKey:@"developerExtrasEnabled"];
    }

    WKUserContentController* controller = [config userContentController];
    // Add inject script
    WKUserScript* userScript = [WKUserScript alloc];
    [userScript initWithSource:[NSString stringWithUTF8String:inject.c_str()]
                 injectionTime:WKUserScriptInjectionTimeAtDocumentStart
              forMainFrameOnly:NO];
    [controller addUserScript:userScript];

    webview = [[WKWebView alloc] initWithFrame:NSZeroRect configuration:config];

    // Add delegate methods manually in order to capture "this"
    class_replaceMethod(
        [WindowDelegate class], @selector(windowWillClose:),
        imp_implementationWithBlock([=](id, SEL, id) { this->exit(); }),
        "v@:@");

    class_replaceMethod(
        [WindowDelegate class],
        @selector(userContentController:didReceiveScriptMessage:),
        imp_implementationWithBlock(
            [=](id, SEL, WKScriptMessage* scriptMessage) {
                if (this->js_callback) {
                    id body = [scriptMessage body];
                    if (![body isKindOfClass:[NSString class]]) {
                        return;
                    }

                    std::string msg = [body UTF8String];
                    this->js_callback(*this, msg);
                }
            }),
        "v@:@");

    WindowDelegate* delegate = [WindowDelegate alloc];
    [controller addScriptMessageHandler:delegate name:@"webview"];
    // Set delegate to window
    [window setDelegate:delegate];

    // Initialize application
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Sets the app as the active app
    [NSApp activateIgnoringOtherApps:YES];

    // Add webview to window
    [window setContentView:webview];

    // Display window
    [window makeKeyAndOrderFront:nil];

    // Done initialization, set properties
    init_done = true;

    setTitle(title);
    if (fullscreen) {
        setFullscreen(true);
    }
    setBgColor(bgR, bgG, bgB, bgA);
    navigate(url);

    return 0;
}

void WebView::setTitle(std::string t) {
    if (!init_done) {
        title = t;
    } else {
        [window setTitle:[NSString stringWithUTF8String:t.c_str()]];
    }
}

void WebView::setFullscreen(bool fs) {
    if (!init_done) {
        fullscreen = fs;
    } else {
        // TODO: replace toggle with set
        [window toggleFullScreen:nil];
    }
}

void WebView::setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!init_done) {
        bgR = r;
        bgG = g;
        bgB = b;
        bgA = a;
    } else {
        [window setBackgroundColor:[NSColor colorWithCalibratedRed:r / 255.0
                                                             green:g / 255.0
                                                              blue:b / 255.0
                                                             alpha:a / 255.0]];
    }
}

bool WebView::run() {
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:true];
    if (event) {
        [NSApp sendEvent:event];
    }

    return should_exit;
}

void WebView::navigate(std::string u) {
    if (!init_done) {
        url = u;
    } else if (u.rfind("data:", 0) == 0) {
        [webview loadHTMLString:[NSString stringWithUTF8String:u.c_str()]
                        baseURL:nil];
    } else {
        [webview
            loadRequest:[NSURLRequest
                            requestWithURL:
                                [NSURL URLWithString:[NSString
                                                         stringWithUTF8String:
                                                             u.c_str()]]]];
    }
}

void WebView::eval(const std::string& js) {
    [webview evaluateJavaScript:[NSString stringWithUTF8String:js.c_str()]
              completionHandler:nil];
}

void WebView::exit() {
    // Distinguish window closing with app exiting
    should_exit = true;
    [NSApp terminate:nil];
}

#elif defined(WEBVIEW_GTK)  // WEBVIEW_MAC
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
    GtkWidget* scroller = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(window), scroller);

    // Content manager
    WebKitUserContentManager* cm = webkit_user_content_manager_new();
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
        WebKitSettings* settings =
            webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
        webkit_settings_set_enable_write_console_messages_to_stdout(settings,
                                                                    true);
        webkit_settings_set_enable_developer_extras(settings, true);
    } else {
        g_signal_connect(G_OBJECT(webview), "context-menu",
                         G_CALLBACK(webview_context_menu_cb), nullptr);
    }

    webkit_user_content_manager_add_script(
        cm, webkit_user_script_new(
                inject.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL, NULL));

    // Done initialization, set properties
    init_done = true;

    setTitle(title);
    if (fullscreen) {
        setFullscreen(true);
    }
    setBgColor(bgR, bgG, bgB, bgA);
    navigate(url);

    // Finish
    gtk_widget_grab_focus(GTK_WIDGET(webview));
    gtk_widget_show_all(window);

    return 0;
}

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
    if (!init_done) {
        bgR = r;
        bgG = g;
        bgB = b;
        bgA = a;
    } else {
        GdkRGBA color = {r / 255.0, g / 255.0, b / 255.0, a / 255.0};
        webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &color);
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

void WebView::eval(const std::string& js) {
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

void WebView::exit() { should_exit = true; }

void WebView::external_message_received_cb(WebKitUserContentManager*,
                                           WebKitJavascriptResult* r,
                                           gpointer arg) {
    WebView* w = static_cast<WebView*>(arg);
    if (w->js_callback) {
        JSCValue* value = webkit_javascript_result_get_js_value(r);
        std::string str = std::string(jsc_value_to_string(value));
        w->js_callback(*w, str);
    }
}

void WebView::webview_eval_finished(GObject*, GAsyncResult*, gpointer arg) {
    static_cast<WebView*>(arg)->js_busy = false;
}

void WebView::webview_load_changed_cb(WebKitWebView*, WebKitLoadEvent event,
                                      gpointer arg) {
    if (event == WEBKIT_LOAD_FINISHED) {
        static_cast<WebView*>(arg)->ready = true;
    }
}

void WebView::destroyWindowCb(GtkWidget*, gpointer arg) {
    static_cast<WebView*>(arg)->exit();
}

// gboolean WebView::closeWebViewCb(WebKitWebView *webView, GtkWidget *window) {
//   gtk_widget_destroy(window);
//   return TRUE;
// }

gboolean WebView::webview_context_menu_cb(WebKitWebView*, GtkWidget*,
                                          WebKitHitTestResult*, gboolean,
                                          gpointer) {
    // Always hide context menu if not debug
    return TRUE;
}
#endif                      // WEBVIEW_GTK

void WebView::css(const wv::String& css) {
    eval(Str(R"js(
    (
      function (css) {
        if (document.styleSheets.length === 0) {
          var s = document.createElement('style');
          s.type = 'text/css';
          document.head.appendChild(s);
        }
        document.styleSheets[0].insertRule(css);
      }
    )(')js") +
         css + Str("')"));
}

void WebView::setCallback(jscb callback) { js_callback = callback; }

void WebView::preEval(const wv::String& js) {
    inject += Str("(()=>{") + js + Str("})()");
}

}  // namespace wv

#endif  // WEBVIEW_H