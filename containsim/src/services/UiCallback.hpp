#pragma once

#include <any>
#include <functional>
#include <span>
#include <string>

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/fwd/unpack.hpp>

namespace services {
    struct UiCallback {
        std::function<void(std::span<std::any>)> callback{};
    };

    template<typename F, typename... Args>
    UiCallback MakeUiCallback(F&& func) {
        namespace hana = boost::hana;

        return UiCallback{
            // Make a callback that stores the passed-in callable, and accepts
            // type erased arguments from the UI. It converts them to the
            // passed-in callback's arguments' types, and then moves it from
            // the input span to the passed-in callback.
            .func = [func = std::forward<F>(func)](std::span<std::any> input_args) {
                hana::tuple<Args...> extracted_args{};

                hana::for_each(extracted_args, [&input_args, i = 0](auto& arg) mutable {
                    arg = std::move(std::any_cast<decltype(arg)>(input_args[i++]));
                });

                hana::unpack(std::move(extracted_args), func);
            }
        };
    }
}