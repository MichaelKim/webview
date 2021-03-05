#if defined(_WIN32)
#include "../webview.hpp"
#include <mutex>
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>

#include <WebView2.h>

namespace Soundux
{
    using namespace Microsoft::WRL;

    class WebView2 : public WebView
    {
        HINSTANCE instance = nullptr;
        HWND hwnd = nullptr;
        MSG msg = {};

        wil::com_ptr<ICoreWebView2Controller> webViewController;
        wil::com_ptr<ICoreWebView2> webViewWindow;

        static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

      public:
        bool run() override;
        bool setup() override;

        void setSize(int width, int height) override;

        void enableDevTools(bool enable) override;
        void runCode(const std::string &code) override;
        void navigate(const std::string &url) override;
        void setTitle(const std::string &title) override;
    };
} // namespace Soundux
#endif