#pragma once
#include <cstdint>
#include <functional>
#include <json.hpp>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>

#include <iostream>

namespace Soundux
{
    namespace traits
    {
        template <typename T> struct func_traits : public func_traits<decltype(&T::operator())>
        {
        };
        template <typename ClassType, typename ReturnType, typename... Args>
        struct func_traits<ReturnType (ClassType::*)(Args...) const>
        {
            enum
            {
                arg_count = sizeof...(Args)
            };
            using arg_t = std::tuple<std::decay_t<Args>...>;
            using return_t = ReturnType;
        };
        template <typename T> struct is_optional
        {
          private:
            static std::uint8_t test(...);
            template <typename O> static auto test(std::optional<O> *) -> std::uint16_t;

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
    } // namespace traits
    namespace helpers
    {
        template <std::size_t I, typename Tuple, typename Function, std::enable_if_t<(I >= 0)> * = nullptr>
        void setTuple(Tuple &tuple, Function func)
        {
            func(I, std::get<I>(tuple));
            if constexpr (I > 0)
            {
                setTuple<I - 1>(tuple, func);
            }
        }
        template <int I, typename Tuple, typename Function, std::enable_if_t<(I < 0)> * = nullptr>
        void setTuple([[maybe_unused]] Tuple &t, [[maybe_unused]] Function f)
        {
        }
    } // namespace helpers
    class WebView
    {
        using callback_t = std::function<std::string(const nlohmann::json &)>;

      protected:
        int width;
        int height;
        bool devTools;
        std::string url;
        bool shouldExit;

        std::map<std::string, callback_t> callbacks;
        std::function<void(int, int)> resizeCallback;
        std::function<void(const std::string &)> navigateCallback;

        static inline std::string callback_code = R"js(
          async function {0}(...param)
                {
                    const seq = ++window._rpc_seq;
                    const promise = new Promise((resolve) => {
                        window._rpc[seq] = {
                            resolve: resolve
                        };
                    });
                    window.external.invoke(JSON.stringify({
                        "seq": seq,
                        "name": "{0}",
                        "params": param
                    }));
                    return JSON.parse(await promise);
                }
        )js";
        static inline std::string resolve_code = R"js(
            window._rpc[{0}].resolve(`{1}`);
            delete window._rpc[{0}];
        )js";
        static inline std::string setup_code = R"js(
            window._rpc = {}; window._rpc_seq = 0;
        )js";

        virtual void onExit();
        virtual void onResize(int, int);
        virtual void onNavigate(const std::string &);
        virtual void resolveCallback(const std::string &);

      public:
        WebView() = default;
        virtual ~WebView() = default;
        WebView(const WebView &) = delete;
        virtual WebView &operator=(const WebView &) = delete;

        virtual bool getDevToolsEnabled();
        virtual void enableDevTools(bool);

        virtual bool run() = 0;
        virtual bool setup() = 0;

        virtual void setSize(int, int);
        virtual void navigate(const std::string &);

        virtual void runCode(const std::string &) = 0;
        virtual void setTitle(const std::string &) = 0;

        virtual void setResizeCallback(const std::function<void(int, int)> &);
        virtual void setNavigateCallback(const std::function<void(const std::string &)> &);
        template <typename func_t> void addCallback(const std::string &name, func_t function)
        {
            using func_traits = traits::func_traits<decltype(function)>;
            auto func = [this, function](const nlohmann::json &j) -> std::string {
                typename func_traits::arg_t packedArgs;

                helpers::setTuple<func_traits::arg_count - 1>(packedArgs,
                                                              [&j](auto index, auto &val) { val = j.at(index); });

                if constexpr (std::is_void_v<typename func_traits::return_t>)
                {
                    auto unpackFunc = [function](auto &&...args) { function(args...); };
                    std::apply(unpackFunc, packedArgs);
                    return "null";
                }
                else if constexpr (traits::is_optional<typename func_traits::return_t>::value)
                {
                    typename func_traits::return_t rtn;

                    auto unpackFunc = [&rtn, function](auto &&...args) { rtn = std::move(function(args...)); };
                    std::apply(unpackFunc, packedArgs);

                    if (rtn)
                    {
                        return nlohmann::json(*rtn).dump();
                    }
                    return "null";
                }
                else
                {
                    typename func_traits::return_t rtn;

                    auto unpackFunc = [&rtn, function](auto &&...args) { rtn = std::move(function(args...)); };
                    std::apply(unpackFunc, packedArgs);

                    return nlohmann::json(rtn).dump();
                }
            };

            auto code = std::regex_replace(callback_code, std::regex(R"(\{0\})"), name);
            runCode(code);

            callbacks.insert({name, func});
        }
    };
} // namespace Soundux