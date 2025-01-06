#pragma once

#include <variant>
#include <vector>
#include <span>
#include <string>
#include <dxtl/cstring_view.hpp>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>
#include <boost/intrusive/list.hpp>

namespace detail {
    template<typename T> 
    concept IsVector = requires(T t) { 
        typename T::value_type;
        requires std::is_same_v<T, std::vector<typename T::value_type>>;
    };

    template<typename T> 
    concept IsSpan = requires(T t) { 
        typename T::value_type;
        requires std::is_same_v<T, std::span<typename T::value_type>>;
    };
}

class UiValue;

struct UiNull { };
struct UiUndefined { };

struct UiObject {
    std::unordered_map<std::string, UiValue> fields{};
};

struct UiObjectView {
    std::unordered_map<std::string, UiValue>* fields{};
};

struct UiArray {
    std::vector<UiValue> elements{};
};

struct UiArrayView {
    std::span<const UiValue> elements{};
};

enum class UiTypes {
    Undefined,
    Null,
    Bool,
    Double,
    String,
    StringView,
    Object,
    ObjectView,
    Array,
    ArrayView
};

class UiValue {
    using Data = std::variant<UiUndefined, UiNull, bool, double, std::string, dxtl::cstring_view, UiObject, UiObjectView, UiArray, UiArrayView>;
    Data m_data{};

    UiValue(Data&& data) : m_data(std::move(data)) { }
public:
    template<UiTypes Type, typename... Args>
    static UiValue Make(Args&&... args) {
        using enum UiTypes;

        if constexpr (Type == Undefined) {
            return UiValue(UiUndefined{});
        } else if constexpr (Type == Null) {
            return UiValue(UiNull{});
        } else if constexpr (Type == Bool) {
            return UiValue(Data(std::in_place_type<bool>, std::forward<Args>(args)...));
        } else if constexpr (Type == Double) {
            return UiValue(Data(std::in_place_type<double>, std::forward<Args>(args)...));
        } else if constexpr (Type == String) {
            return UiValue(Data(std::in_place_type<std::string>, std::forward<Args>(args)...));
        } else if constexpr (Type == StringView) {
            return UiValue(Data(std::in_place_type<dxtl::cstring_view >, std::forward<Args>(args)...));
        } else if constexpr (Type == Object) {
            return UiValue(Data(std::in_place_type<UiObject>, std::forward<Args>(args)...));
        } else if constexpr (Type == ObjectView) {
            return UiValue(Data(std::in_place_type<UiObjectView>, std::forward<Args>(args)...));
        } else if constexpr (Type == Array) {
            return UiValue(Data(std::in_place_type<UiArray>, std::forward<Args>(args)...));
        } else if constexpr (Type == ArrayView) {
            return UiValue(Data(std::in_place_type<UiArrayView>, std::forward<Args>(args)...));
        }
    }
    
    template<typename T>
    static UiValue Infer(T&& o) {
        using enum UiTypes;
        using D = std::decay_t<T>;

        if constexpr (std::is_same_v<D, UiUndefined>) {
            return UiValue::Make<UiTypes::Undefined>();
        } else if constexpr (std::is_same_v<D, UiNull>) {
            return UiValue::Make<UiTypes::Null>();
        } else if constexpr (std::is_same_v<D, bool>) {
            return UiValue::Make<UiTypes::Bool>(o);
        } else if constexpr (std::is_same_v<D, double>) {
            return UiValue::Make<UiTypes::Double>();
        } else if constexpr (std::is_same_v<D, std::string>) {
            return UiValue::Make<UiTypes::String>(std::forward<T>(o));
        } else if constexpr (std::is_constructible_v<dxtl::cstring_view, T>) {
            return UiValue::Make<UiTypes::StringView>(std::forward<T>(o));
        } else if constexpr (std::is_same_v<D, UiObject>) {
            return UiValue::Make<UiTypes::Object>(std::forward<T>(o));
        } else if constexpr (std::is_same_v<D, UiObjectView>) {
            return UiValue::Make<UiTypes::ObjectView>(std::forward<T>(o));
        } else if constexpr (::detail::IsVector<D> || std::is_same_v<UiArray, D>) {
            return UiValue::Make<UiTypes::Array>(std::forward<T>(o));
        } else if constexpr (::detail::IsSpan<D> || std::is_same_v<UiArrayView, D>) {
            return UiValue::Make<UiTypes::ArrayView>(std::forward<T>(o));
        }

        // TODO: Replace with static_assert once DR goes into Clang
        throw std::runtime_error("Could not infer type");
    }

    #define CHECK_GET_FAIL(o, kind) do { if (!(o)) throw std::runtime_error("Attempt to get " kind " of UiValue did not store such object.");} while (false) // NOLINT

    template<UiTypes Type>
    decltype(auto) Get() { // NOLINT
        using enum UiTypes;

        if constexpr (Type == Undefined) {
            bool success = std::holds_alternative<UiUndefined>(m_data);
            CHECK_GET_FAIL(success, "Undefined");
            return UiUndefined{};
        } else if constexpr (Type == Null) {
            bool success = std::holds_alternative<UiNull>(m_data);
            CHECK_GET_FAIL(success, "Null");
            return UiNull{};
        } else if constexpr (Type == Bool) {
            auto* o = std::get_if<bool>(&m_data);
            CHECK_GET_FAIL(o, "Bool");
            return *o;
        } else if constexpr (Type == Double) {
            auto* o = std::get_if<double>(&m_data);
            CHECK_GET_FAIL(o, "Double");
            return *o;
        } else if constexpr (Type == String) {
            auto* o = std::get_if<std::string>(&m_data);
            CHECK_GET_FAIL(o, "String");
            return *o;
        } else if constexpr (Type == StringView) {
            if (auto* o = std::get_if<std::string>(&m_data)) {
                return dxtl::cstring_view(*o);
            } else if (auto* o = std::get_if<dxtl::cstring_view>(&m_data)) {
                return *o;
            }
            
            CHECK_GET_FAIL(false, "StringView");
        } else if constexpr (Type == Object) {
            auto* o = std::get_if<UiObject>(&m_data);
            CHECK_GET_FAIL(false, "Object");
            return *o;
        } else if constexpr (Type == ObjectView) {
            if (auto* o = std::get_if<UiObject>(&m_data)) {
                return const_cast<const UiObject&>(*o);
            } else if (const auto* o = std::get_if<UiObjectView>(&m_data)) {
                return *o;
            }
            
            CHECK_GET_FAIL(false, "ObjectView");
        }  else if constexpr (Type == Array) {
            auto* o = std::get_if<UiArray>(&m_data);
            CHECK_GET_FAIL(false, "Array");
            return *o;
        } else if constexpr (Type == ArrayView) {
            if (auto* o = std::get_if<UiArray>(&m_data)) {
                return const_cast<const UiArray&>(*o);
            } else if (auto* o = std::get_if<UiArrayView>(&m_data)) {
                return *o;
            }
            
            CHECK_GET_FAIL(false, "ArrayView");
        } 
    }

    #undef CHECK_GET_FAIL

    template<UiTypes Type>
    decltype(auto) Get() const {
        auto&& ret = const_cast<UiValue*>(this)->Get<Type>(); // NOLINT

        if constexpr (std::is_reference_v<decltype(ret)>) {
            return std::as_const(ret);
        } else {
            return ret;
        }
    }

    bool Holds(UiTypes type) const {
        using enum UiTypes;

        switch (type) {
            case Undefined: return std::holds_alternative<UiUndefined>(m_data);
            case Null: return std::holds_alternative<UiNull>(m_data);
            case Bool: return std::holds_alternative<bool>(m_data);
            case Double: return std::holds_alternative<double>(m_data);
            case String: return std::holds_alternative<std::string>(m_data);
            case StringView: return std::holds_alternative<dxtl::cstring_view>(m_data);
            case Object: return std::holds_alternative<UiObject>(m_data);
            case ObjectView: return std::holds_alternative<UiObjectView>(m_data);
            case Array: return std::holds_alternative<UiArray>(m_data);
            case ArrayView: return std::holds_alternative<UiArrayView>(m_data);
        }
    }

    template<typename T>
    decltype(auto) Visit(auto&& v) {
        return std::visit(m_data, std::forward<T>(v));
    }

    template<typename T>
    decltype(auto) Visit(T&& v) const {
        return std::visit(std::forward<T>(v), m_data);
    }

    auto& GetVariant() { return m_data; }
    const auto& GetVariant() const { return m_data; }
};

struct UiArg : boost::intrusive::list_base_hook<> { };

class UiArgs {

};
