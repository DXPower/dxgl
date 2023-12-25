#include <services/JsContextStorage.hpp>

using namespace services;
using namespace ultralight;

JsContextStorage::JsContextStorage(RefPtr<View> view)
    : m_logger(logging::CreateLogger("JsContextStorage")),
      m_view(std::move(view)) 
{ }

JsContextStorage::~JsContextStorage() {
    auto scoped_context = LockAndSetJsContext();
    JSObject window = JSGlobalObject();

    // Delete the registered functions associated with the ui_callbacks
    for (const auto& func_name : m_ui_callbacks | std::views::elements<0>) {
        window.DeleteProperty(func_name.c_str());
    }
}

void JsContextStorage::MakeFunction(dxtl::cstring_view js_name, UiCallback&& callback) {
    m_logger.info("Binding JS function {} to callback", js_name.c_str());
    m_ui_callbacks.emplace(js_name, std::move(callback));

    auto scoped_context = LockAndSetJsContext();

    JSObject window = JSGlobalObject();
    window[js_name.c_str()] = std::bind_front(&JsContextStorage::HandleUiCallback, this, std::string(js_name));
}

void JsContextStorage::DeleteFunction(dxtl::cstring_view js_name) {
    auto scoped_context = LockAndSetJsContext();

    JSObject window = JSGlobalObject();
    window.DeleteProperty(js_name.c_str());
}

RefPtr<JSContext> JsContextStorage::LockAndSetJsContext() {
    auto scoped_context = m_view->LockJSContext();
    SetJSContext(*scoped_context);

    return scoped_context;
}

void JsContextStorage::HandleUiCallback(std::string js_name, const JSObject& this_obj [[maybe_unused]], const JSArgs& js_args) {
    try {
        std::vector<UiCallbackArg> cpp_args{};
        cpp_args.reserve(js_args.size());

        for (const auto& js_arg : std::ranges::subrange(js_args.data(), js_args.data() + js_args.size())) {
            UiCallbackArg cpp_arg{};

            if (js_arg.IsUndefined()) {
                cpp_arg = UiUndefined{};
            } else if (js_arg.IsNull()) {
                cpp_arg = UiNull{};
            } else if (js_arg.IsBoolean()) {
                cpp_arg = js_arg.ToBoolean();
            } else if (js_arg.IsNumber()) {
                cpp_arg = js_arg.ToNumber();
            } else if (js_arg.IsString()) {
                JSString js_str = js_arg.ToString();
                String ul_str = static_cast<String>(js_str);

                cpp_arg = std::string(ul_str.utf8().data(), ul_str.utf8().sizeBytes());
            }

            cpp_args.push_back(std::move(cpp_arg));
        }

        m_ui_callbacks.at(js_name).callback(cpp_args);
    } catch (const std::exception& e) {
        m_logger.error("Caught exception in UI Callback {}: {}", js_name, e.what());
    }
}