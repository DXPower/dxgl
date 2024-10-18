#pragma once

#include <services/Logging.hpp>
#include <services/UiCallback.hpp>

#include <dxtl/cstring_view.hpp>

#include <functional>
#include <ranges>
#include <unordered_map>

#include <Ultralight/Ultralight.h>
#include <Ultralight/JavaScript.h>
#include <AppCore/JSHelpers.h>

namespace services {
    class JsContextStorage {
        spdlog::logger m_logger;

        ultralight::RefPtr<ultralight::View> m_view{};
        std::unordered_map<std::string, UiCallback> m_ui_callbacks{};

    public:
        JsContextStorage(ultralight::RefPtr<ultralight::View> view);
        JsContextStorage(const JsContextStorage& copy) = delete;

        ~JsContextStorage();

        // Makes a free function that is stored in the context's global object (window)
        void MakeFunction(dxtl::cstring_view js_name, UiCallback&& callback);
        void DeleteFunction(dxtl::cstring_view js_name);

        UiValue CallFunction(dxtl::cstring_view js_name, std::span<UiValue> args);

        ultralight::RefPtr<ultralight::JSContext> LockAndSetJsContext();
        
    private:
        void HandleUiCallback(std::string js_name, const ultralight::JSObject& this_obj, const ultralight::JSArgs& js_args);
    };
}