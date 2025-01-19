#include <common/UiValue.hpp>

#include <span>
#include <unordered_map>

namespace services {
    class UiView;

    class UiSignal {
        UiView* m_ui_view{};
        const std::string* m_signal_name{};

        UiSignal(UiView& ui_view) : m_ui_view(&ui_view) { }

    public:
        template<typename... T>
        void Fire(std::in_place_t, T&&... values) {
            UiArgs args{};
            std::array ui_values = {UiValue::Infer(std::forward<T>(values))...};
            
            for (auto& v : ui_values) {
                args.push_back(v);
            }

            Fire(std::move(args));
        }

        void Fire(const UiArgs& args);

        friend class UiBell;
    };

    class UiBell {
        UiView* m_ui_view{};
        std::unordered_map<std::string, UiSignal> m_signals{};

    public:
        UiBell(UiView& ui_view) : m_ui_view(&ui_view) { }

        const UiSignal& RegisterSignal(std::string name);
        const UiSignal& GetOrRegisterSignal(std::string name);
        const UiSignal& GetSignal(const std::string& name) const;
    };
}