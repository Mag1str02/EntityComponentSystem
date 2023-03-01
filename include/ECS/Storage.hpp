#pragma once

Storage::Storage() : m_SystemManager(this) {}
Entity Storage::CreateEntity()
{
    auto [id, gen] = m_EntityManager.CreateEntity();
    Entity ent;
    ent.m_ID      = id;
    ent.m_Gen     = gen;
    ent.m_Storage = this;
    return ent;
}
template <typename... Components> StorageView<Components...> Storage::View() { return StorageView<Components...>(this); }

template <typename SystemType> void Storage::RegisterSystem() { m_SystemManager.RegisterSystem<SystemType>(); }
void                                Storage::UpdateSystems(float dt) { m_SystemManager.Update(dt); }

bool Storage::Valid(uint32_t id, uint32_t gen) const { return m_EntityManager.Valid(id, gen); }
void Storage::Destroy(uint32_t id, uint32_t gen)
{
    if (m_EntityManager.Valid(id, gen))
    {
        m_EntityManager.Destroy(id);
        m_ComponentManager.Destroy(id);
    }
}

template <typename ComponentType> ComponentType* Storage::AddComponent(uint32_t id, uint32_t gen, const ComponentType& component)
{
    return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.AddComponent<ComponentType>(id, component) : nullptr);
}
template <typename ComponentType> ComponentType* Storage::GetComponent(uint32_t id, uint32_t gen)
{
    return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.GetComponent<ComponentType>(id) : nullptr);
}
template <typename ComponentType> bool Storage::HasComponent(uint32_t id, uint32_t gen) const
{
    return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.HasComponent<ComponentType>(id) : false);
}
template <typename ComponentType> void Storage::RemoveComponent(uint32_t id, uint32_t gen)
{
    if (m_EntityManager.Valid(id, gen))
    {
        m_ComponentManager.RemoveComponent<ComponentType>(id);
    }
}
