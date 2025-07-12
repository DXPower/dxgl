#pragma once

#include <concepts>
#include <functional>

template<typename T>
class Command {
public:
    using Executor = std::function<void(T&)>;

private:
    Executor m_executor{};

public:
    // The null command
    Command() : Command([](T&) {}) {}
    Command(Executor executor) : m_executor(std::move(executor)) {}
    template<typename F>
    requires std::constructible_from<Executor, F>
    Command(F&& func) : m_executor(std::forward<F>(func)) {}

    void Execute(T& target) const {
        m_executor(target);
    }
};

template<typename T>
class Commandable {
protected:
    void ProcessCommand(const Command<T>& cmd) {
        cmd.Execute(static_cast<T&>(*this));
    }
};