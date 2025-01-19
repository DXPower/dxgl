#include <services/UiBell.hpp>
#include <services/UiView.hpp>
#include <format>
#include <stdexcept>

using namespace services;

void UiSignal::Fire(const UiArgs& args) {
    m_ui_view->CallFunction(*m_signal_name, args);
}

const UiSignal& UiBell::RegisterSignal(std::string name) {
    if (m_signals.contains(name))
        throw std::runtime_error(std::format("Attempt to register signal {} which is already registered", name));

    auto [it, inserted] = m_signals.emplace(std::move(name), UiSignal{*m_ui_view});
    it->second.m_signal_name = &it->first;

    return it->second;
}

const UiSignal& UiBell::GetOrRegisterSignal(std::string name) {
    auto [it, inserted] = m_signals.insert_or_assign(std::move(name), UiSignal{*m_ui_view});
    it->second.m_signal_name = &it->first;
    return it->second;
}

const UiSignal& UiBell::GetSignal(const std::string& name) const {
    return m_signals.at(name);
}