#include <services/JsContextStorage.hpp>
#include <dxtl/overloaded.hpp>

using namespace services;
using namespace ultralight;

static UiValue JsToUiValue(const JSValue& js_arg) {
    if (js_arg.IsUndefined()) {
        return UiValue::Make<UiTypes::Undefined>();
    } else if (js_arg.IsNull()) {
        return UiValue::Make<UiTypes::Null>();
    } else if (js_arg.IsBoolean()) {
        return UiValue::Make<UiTypes::Bool>(js_arg.ToBoolean());
    } else if (js_arg.IsNumber()) {
        return UiValue::Make<UiTypes::Double>(js_arg.ToNumber());
    } else if (js_arg.IsString()) {
        JSString js_str = js_arg.ToString();
        String ul_str = static_cast<String>(js_str);

        return UiValue::Make<UiTypes::String>(std::string(ul_str.utf8().data(), ul_str.utf8().sizeBytes()));
    }

    throw std::runtime_error("Unimplemented JSValue possibility");
}

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

static JSValue UiValueToJsValue(const UiValue& arg) {
    auto ConvertUiObject = [](const auto& fields) -> JSValue {
        JSObject js_object{};
        
        for (const auto& [name, value] : fields) {
            js_object[name.c_str()] = UiValueToJsValue(value);
        }

        return (JSObjectRef) js_object;
    };

    auto ConvertUiArray = [](std::span<const UiValue> elements) -> JSValue {
            JSArray js_array{};

            for (const auto& e : elements) {
                js_array.push(UiValueToJsValue(e));
            }

            return (JSObjectRef) js_array;
    };

    return arg.Visit(dxtl::overloaded(
        [&](UiUndefined) -> JSValue { return JSValueUndefinedTag{}; },
        [&](UiNull) -> JSValue { return JSValueNullTag{}; },
        [&](bool o) -> JSValue { return o; },
        [&](double o) -> JSValue { return o; },
        [&](const std::string& o) -> JSValue { return o.c_str(); },
        [&](dxtl::cstring_view o) -> JSValue { return o.c_str(); },
        [&](const UiObject& o) -> JSValue { return ConvertUiObject(o.fields); },
        [&](UiObjectView o) -> JSValue { return ConvertUiObject(*o.fields); },
        [&](const UiArray& o) -> JSValue { return ConvertUiArray(o.elements); },
        [&](UiArrayView o) -> JSValue { return ConvertUiArray(o.elements); }
    ));
}

UiValue JsContextStorage::CallFunction(dxtl::cstring_view js_name, std::span<UiValue> args) {
    auto scoped_context = LockAndSetJsContext();

    JSObject window = JSGlobalObject();

    // if (window.HasProperty(js_name.c_str()))
    //     throw std::runtime_error(std::format("Attempt to call function '{}' that doesn't exist in window", js_name.c_str()));

    JSPropertyValue window_func = window[js_name.c_str()];

    if (!window_func.IsFunction())
        throw std::runtime_error(std::format("Attempt to call non-function '{}' in window", js_name.c_str()));

    if (std::size_t expected_args = window_func.ToObject()["length"].ToInteger(); expected_args != args.size())
        throw std::runtime_error(std::format("Function '{}' expects {} args, but {} were provided", js_name.c_str(), expected_args, args.size()));

    JSFunction js_func = window_func.ToFunction();
    JSArgs js_args{};

    for (const auto& arg : args) {
        js_args.push_back(UiValueToJsValue(arg));
    }

    return JsToUiValue(js_func(js_args));
}

RefPtr<JSContext> JsContextStorage::LockAndSetJsContext() {
    auto scoped_context = m_view->LockJSContext();
    SetJSContext(*scoped_context);

    return scoped_context;
}

void JsContextStorage::HandleUiCallback(std::string js_name, const JSObject& this_obj [[maybe_unused]], const JSArgs& js_args) {
    try {
        std::vector<UiValue> cpp_args{};
        cpp_args.reserve(js_args.size());

        for (const auto& js_arg : std::ranges::subrange(js_args.data(), js_args.data() + js_args.size())) {
            cpp_args.push_back(JsToUiValue(js_arg));
        }

        m_ui_callbacks.at(js_name).callback(cpp_args);
    } catch (const std::exception& e) {
        m_logger.error("Caught exception in UI Callback {}: {}", js_name, e.what());
    }
}