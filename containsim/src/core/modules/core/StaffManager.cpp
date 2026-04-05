#include <modules/core/StaffManager.hpp>
#include <modules/core/StaffTypeMeta.hpp>

#include <format>

using namespace core;

StaffManager::StaffManager(flecs::world& world, const ResourceRegistry& resources)
    : m_world(&world), m_resources(&resources)  {
    
}

flecs::entity StaffManager::CreateStaffMember(StaffType type_id) {
    const auto& meta = m_resources->TryGetById<StaffTypeMeta>(type_id);

    if (!meta) {
        m_logger.warn("Invalid StaffType: {}", std::to_underlying(type_id));
        return flecs::entity();
    }

    auto prefab_e = m_world->lookup(meta->prefab.c_str());

    if (!prefab_e) {
        m_logger.warn("Prefab not found for staff type {} ('{}'): {}", std::to_underlying(type_id), meta->type, meta->prefab);
        return flecs::entity();
    }

    auto staff_e = m_world->entity()
        .is_a(prefab_e)
        .set<Staff>({type_id});

    return staff_e;
}