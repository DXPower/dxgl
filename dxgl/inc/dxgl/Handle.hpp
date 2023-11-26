#pragma once

#include <utility>

namespace dxgl {
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

    template<typename T, bool Mutable = false>
    class HandleRef {
        using Handle_t = std::conditional_t<Mutable, T*, const T*>;
        Handle_t handle{};

    public:
        HandleRef() noexcept = default;
        HandleRef(std::conditional_t<Mutable, T&, const T&> handle) : handle(&handle) { }
        // HandleRef(const HandleRef<T, false>& other) requires (Mutable) : handle(other.GetHandle()) { }
        
        HandleRef(const HandleRef& copy) = default;
        HandleRef(T&& handle) = delete;

        HandleRef& operator=(const HandleRef& copy) = default;

        operator HandleRef<T, false>() const requires Mutable {
            return HandleRef<T, false>(*handle);
        }


        T& operator*() requires (Mutable) {
            return *handle;
        }

        const T& operator*() const {
            return *handle;
        }

        const T* operator->() const {
            return handle;
        }

        T* operator->() requires (Mutable) {
            return handle;
        }

        operator bool() const {
            return handle != nullptr;
        }

        bool HasValue() const {
            return handle != nullptr;
        }

        auto& GetOwner() const {
            return *handle;
        }

        HandleRef<T, false> View() const requires Mutable {
            return HandleRef<T, false>(*handle);
        }

    };
}