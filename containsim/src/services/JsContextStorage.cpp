#include <services/JsContextStorage.hpp>

using namespace services;
using namespace ultralight;

JsContextStorage::JsContextStorage(RefPtr<View> view) : m_view(std::move(view)) { }

JsContextStorage::~JsContextStorage() {
    auto scoped_context = LockAndSetJsContext();
    JSObject window = JSGlobalObject();

    // Delete the registered functions associated with the ui_callbacks
    for (const auto& func_name : m_ui_callbacks | std::views::elements<0>) {
        window.DeleteProperty(func_name.c_str());
    }
}

void JsContextStorage::MakeFunction(dxtl::cstring_view js_name, UiCallback&& callback) {
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

void JsContextStorage::HandleUiCallback(std::string js_name, const JSObject& this_obj [[maybe_unused]], const JSArgs& args [[maybe_unused]]) {
    m_ui_callbacks.at(js_name).callback({});
}