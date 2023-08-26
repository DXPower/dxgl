#pragma once

#include <utility>

enum class Usable { Yes, No };

template<typename Derived, Usable IsUsable, typename... UseArgs>
class Handle {
protected:
    unsigned int handle{};

public:
    Handle() = default;

    Handle(Handle&& move) noexcept : handle(std::exchange(move.handle, 0)) { }
    Handle& operator=(Handle&& move) noexcept {
        if (this == &move)
            return *this;

        std::swap(handle, move.handle);
        return *this;
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    ~Handle() {
        Destroy();
    }

    
    void Destroy() {
        auto& derived = static_cast<const Derived&>(*this);
        derived.DestroyImpl();

        handle = 0;
    }

    void Use(UseArgs... args) const 
    requires (IsUsable == Usable::Yes) {
        auto& derived = static_cast<const Derived&>(*this);

        derived.UseImpl(std::forward<UseArgs>(args)...);
    }

    unsigned int GetHandle() const { return handle; }

protected:
    void SetHandle(unsigned int handle) { this->handle = handle; }
};