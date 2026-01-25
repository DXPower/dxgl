#pragma once

#include <common/IdRegistry.hpp>
#include <common/Resource.hpp>

#include <format>
#include <generator>
#include <memory>
#include <typeindex>
#include <unordered_map>

class ResourceRegistry {
    std::unordered_map<ResourceId, std::unique_ptr<detail::ResourceStorage>> m_resources{};
    std::unordered_map<std::type_index, std::vector<detail::ResourceStorage*>> m_type_indexed_resources{};

    IdRegistry m_ids{};

public:
    ResourceRegistry() = default;
    ResourceRegistry(const ResourceRegistry&) = delete;
    ResourceRegistry& operator=(const ResourceRegistry&) = delete;
    ResourceRegistry(ResourceRegistry&&) noexcept = default;
    ResourceRegistry& operator=(ResourceRegistry&&) = default;  

    template<StoredResourceType T>
    ResourceOf<T> Register(std::string name, T&& resource) {
        using Resource_t = detail::GetStoredResourceType<T>;
        ResourceId rid = m_ids.AddId(std::move(name));

        auto storage = std::make_unique<detail::TypedResourceStorage<Resource_t>>(rid, std::forward<T>(resource));
        ResourceOf<T> result(storage.get());
        
        m_type_indexed_resources[std::type_index(typeid(Resource_t))].push_back(storage.get());
        m_resources.emplace(rid, std::move(storage));

        return result;
    }

    template<ResourceTypeOrVoid T = void>
    Resource<T> TryGetById(ResourceId rid) const {
        Resource<> result{};

        auto it = m_resources.find(rid);
        if (it != m_resources.end()) {
            result = Resource<>(it->second.get());
        }

        return result.TryAs<T>();
    }

    template<ResourceTypeOrVoid T = void>
    Resource<T> TryGetByName(std::string_view name) const {
        const auto& ids = m_ids.GetIds();
        auto id_it = ids.right.find(std::string(name));

        if (id_it == ids.right.end()) {
            return Resource<T>();
        }

        return TryGetById<T>(id_it->second);
    }

    template<ResourceTypeOrVoid T = void>
    Resource<T> GetById(ResourceId rid) const {
        auto res = TryGetById<T>(rid);

        if (!res.IsValid()) {
            throw std::runtime_error(std::format("Resource with given ID {} does not exist", rid));
        }

        return res;
    }

    template<ResourceTypeOrVoid T = void>
    Resource<T> GetByName(std::string_view name) const {
        auto res = TryGetByName<T>(name);

        if (!res.IsValid()) {
            throw std::runtime_error(std::format("Resource with given name {} does not exist", name));
        }

        return res;
    }

    template<StoredResourceType T>
    std::generator<Resource<T>> GetAllOfType() const {
        auto it = m_type_indexed_resources.find(std::type_index(typeid(T)));

        if (it != m_type_indexed_resources.end()) {
            for (auto* storage : it->second) {
                co_yield Resource<T>(static_cast<detail::TypedResourceStorage<T>*>(storage));
            }
        }
    }
};