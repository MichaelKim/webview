#include "webview.hpp"
#include <string>

namespace Soundux
{
    void WebView::setSize(int width, int height)
    {
        this->width = width;
        this->height = height;
    }
    void WebView::navigate(const std::string &url)
    {
        this->url = url;
    }
    bool WebView::getDevToolsEnabled()
    {
        return devTools;
    }
    void WebView::enableDevTools(bool state)
    {
        devTools = state;
    }
    void WebView::setResizeCallback(const std::function<void(int, int)> &callback)
    {
        resizeCallback = callback;
    }
    void WebView::setNavigateCallback(const std::function<void(const std::string &)> &callback)
    {
        navigateCallback = callback;
    }
    void WebView::resolveCallback(const std::string &data)
    {
        auto j = nlohmann::json::parse(data, nullptr, false);
        if (!j.is_discarded())
        {
            auto params = j.at("params");
            auto seq = j.at("seq").get<std::uint32_t>();
            auto name = j.at("name").get<std::string>();

            auto code = std::regex_replace(resolve_code, std::regex(R"(\{0\})"), std::to_string(seq));
            code = std::regex_replace(code, std::regex(R"(\{1\})"), callbacks.at(name)(params));
            runCode(code);
        }
    }
    void WebView::onResize(int width, int height)
    {
        if (resizeCallback)
        {
            resizeCallback(width, height);
        }
    }
    void WebView::onExit()
    {
        shouldExit = true;
    }
    void WebView::onNavigate(const std::string &url)
    {
        if (navigateCallback)
        {
            navigateCallback(url);
        }
    }
} // namespace Soundux
