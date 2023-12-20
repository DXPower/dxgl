#pragma once

#include <any>
#include <functional>
#include <span>
#include <string>
#include <variant>

#include <boost/mp11/tuple.hpp>

namespace services {
    struct UiNull { };
    struct UiUndefined { };

    using UiCallbackArg = std::variant<UiUndefined, UiNull, bool, double, std::string>;

    struct UiCallback {
        std::function<void(std::span<UiCallbackArg>)> callback{};
    };

    template<typename... Args>
    UiCallback MakeUiCallback(auto&& func) {
        namespace mp = boost::mp11;

        return UiCallback{
            // Make a callback that stores the passed-in callable, and accepts
            // type erased arguments from the UI. It converts them to the
            // passed-in callback's arguments' types, and then moves it from
            // the input span to the passed-in callback.
            .callback = [func = std::forward<decltype(func)>(func)](std::span<UiCallbackArg> input_args) mutable {
                std::tuple<Args...> extracted_args{};

                mp::tuple_for_each(extracted_args, [&input_args, i = 0](auto&& arg) mutable {
                    arg = std::move(std::get<decltype(arg)>(input_args[i++]));
                });

                std::apply(func, std::move(extracted_args));
            }
        };
    }
}