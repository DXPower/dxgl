#include <modules/core/StaffManager.hpp>
#include <modules/core/StaffTypeMeta.hpp>

#include <format>

using namespace core;

StaffManager::StaffManager(flecs::world& world)
    : m_world(&world)  {
    
    m_metas = &m_world->get<StaffTypeMetas>();
}

flecs::entity StaffManager::CreateStaffMember(StaffType type) {
    const auto& staff_type_meta = m_world->get<StaffTypeMetas>();
    const StaffTypeMeta* meta = staff_type_meta.Get(type);

    if (meta == nullptr) {
        m_logger.warn("No StaffTypeMeta found for type id {}", type);
        return flecs::entity();
    }

    auto prefab_e = m_world->lookup(meta->prefab.c_str());

    if (!prefab_e) {
        m_logger.warn("Prefab not found for staff type {} ('{}'): {}", type, meta->type, meta->prefab);
        return flecs::entity();
    }

    auto staff_e = m_world->entity()
        .is_a(prefab_e)
        .set<Staff>({type});

    return staff_e;
}