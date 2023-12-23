#pragma once

#include <stdexcept>
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

        struct ArityMismatchError : std::runtime_error {
            std::size_t expected{};
            std::size_t actual{};

            ArityMismatchError(std::size_t expected, std::size_t actual);
        };

        struct ArgTypeMismatchError : std::runtime_error {
            std::size_t arg_index{};

            ArgTypeMismatchError(std::size_t arg_index);
        };
    };


    template<typename... Args>
    UiCallback MakeUiCallback(auto&& func) {
        namespace mp = boost::mp11;

        return UiCallback{
            // Make a callback that stores the passed-in callable, and accepts
            // type erased arguments from the UI. It converts them to the
            // passed-in callback's arguments' types, and then moves it from
            // the input span to the passed-in callback.

            // Because we store the func as a capture, we need to mark ourselves as mutable in-case the
            // stored function is
            .callback = [func = std::forward<decltype(func)>(func)](std::span<UiCallbackArg> input_args) mutable {
                if (input_args.size() != sizeof...(Args)) {
                    throw UiCallback::ArityMismatchError(sizeof...(Args), input_args.size());
                }

                std::tuple<std::remove_cvref_t<Args>...> extracted_args{};

                mp::tuple_for_each(extracted_args, [&input_args, i = 0](auto&& output_arg) mutable {
                    using OutputArgType = std::remove_cvref_t<decltype(output_arg)>;
                    auto& input_arg = input_args[i];

                    if (!std::holds_alternative<OutputArgType>(input_arg)) {
                        throw UiCallback::ArgTypeMismatchError(i);
                    }

                    output_arg = std::move(std::get<OutputArgType>(input_arg));
                    i++;
                });

                std::apply(func, std::move(extracted_args));
            }
        };
    }
}