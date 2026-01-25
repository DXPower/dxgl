#pragma once

#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <utility>

using ResourceId = int;

template<typename T>
concept StoredResourceType = std::is_class_v<T> && !std::is_void_v<T> && !std::is_reference_v<T> && !std::is_const_v<T>;

template<typename T>
concept ResourceTypeOrVoid = StoredResourceType<T> || std::is_void_v<T>;

namespace detail {
template<typename T>
using GetStoredResourceType = std::remove_cvref_t<T>;

class ResourceStorage {
    ResourceId m_id{};

public:
    ResourceStorage() = default;
    ResourceStorage(ResourceId rid) : m_id(rid) {}

    virtual ~ResourceStorage() = default;

    ResourceId GetId() const { return m_id; }
};

template<StoredResourceType T>
class TypedResourceStorage : public ResourceStorage {
    T m_resource;

public:
    TypedResourceStorage(ResourceId rid, T resource)
        : ResourceStorage(rid), m_resource(std::move(resource)) {}
    
    const T& GetResource() const { return m_resource; }
};
}

template<ResourceTypeOrVoid T>
class Resource;

namespace detail {
struct VoidResourceOps {
protected:
    using Storage_t = ResourceStorage;
    Storage_t* m_storage{};

public:
    template<typename U>
    Resource<U> TryAs(this const auto& self) {
        if constexpr (std::is_void_v<U>) {
            return self;
        }

        if (self.m_storage) {
            auto derived_storage = dynamic_cast<TypedResourceStorage<U>*>(self.m_storage);

            if (derived_storage) {
                return Resource<U>(derived_storage);
            } else {
                return Resource<U>();
            }
        }  else {
            return Resource<U>();
        }
    }

    template<typename U>
    Resource<U> As(this const auto& self) {
        auto res = self.template TryAs<U>();

        if (!res.IsValid()) {
            throw std::runtime_error("Failed to cast Resource to desired type");
        }

        return res;
    }

    template<typename U>
    explicit operator Resource<U>(this const auto& self) {
        return self.template As<U>();
    }
};

template<typename T>
struct TypedResourceOps {
protected:
    using Storage_t = TypedResourceStorage<T>;
    Storage_t* m_storage{};

public:
    
    const T& GetResource(this const Resource<T>& self) {
        if (self.m_storage) {
            return self.m_storage->GetResource();
        } else {
            throw std::runtime_error("Attempt to get resource from uninitialized Resource");
        }
    }

    const T* operator->(this const Resource<T>& self) {
        return &self.GetResource();
    }

    const T& operator*(this const Resource<T>& self) {
        return self.GetResource();
    }
};
}

template<ResourceTypeOrVoid T = void>
class Resource : public std::conditional_t<std::is_void_v<T>, detail::VoidResourceOps, detail::TypedResourceOps<T>> {
public:
    Resource() = default;

    ResourceId GetId() const {
        if (this->m_storage) {
            return this->m_storage->GetId();
        } else {
            throw std::runtime_error("Attempt to get Id of uninitialized Resource");
        }
    }

    explicit operator bool() const {
        return this->m_storage != nullptr;
    }
    
    bool IsValid() const {
        return static_cast<bool>(*this);
    }

private:
    explicit Resource(Resource::Storage_t* storage) {
        this->m_storage = storage;
    }

    friend class ResourceRegistry;

    template<typename>
    friend struct detail::TypedResourceOps;

    friend struct detail::VoidResourceOps;
};

using UntypedResource = Resource<>;

template<typename T>
using ResourceOf = Resource<detail::GetStoredResourceType<T>>;