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
        HINSTANCE hInt = nullptr;
        HWND hwnd = nullptr;
        MSG msg = {};
        wil::com_ptr<ICoreWebView2Controller> webview_controller;
        wil::com_ptr<ICoreWebView2> webviewWindow;

        static LRESULT CALLBACK WndProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

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