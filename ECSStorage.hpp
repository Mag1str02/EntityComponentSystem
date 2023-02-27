#pragma once

#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace DE
{
    class ECSStorage
    {
    public:
        ECSStorage() = default;

        std::pair<uint32_t, uint32_t> CreateEntity() { return m_EntityManager.CreateEntity(); }
        bool                          Valid(uint32_t id, uint32_t gen) const { return m_EntityManager.Valid(id, gen); }
        void                          Destroy(uint32_t id, uint32_t gen) const
        {
            if (m_EntityManager.Destroy(id, gen))
            {
                m_ComponentManager.DestroyEntity(id);
            }
        }

        template <typename ComponentType> ComponentType* AddComponent(uint32_t id, uint32_t gen, ComponentType& component)
        {
            return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.AddComponent(id, component) : nullptr);
        }
        template <typename ComponentType> ComponentType* RemoveComponent(uint32_t id, uint32_t gen)
        {
            if (m_EntityManager.Valid(id, gen))
            {
                m_ComponentManager.RemoveComponent(id);
            }
        }
        template <typename ComponentType> ComponentType* GetComponent(uint32_t id, uint32_t gen)
        {
            return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.GetComponent(id) : nullptr);
        }
        template <typename ComponentType> bool HasComponent(uint32_t id, uint32_t gen)
        {
            return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.HasComponent(id) : false);
        }

        template <typename ComponentType> ComponentArray<ComponentType>& GetComponentArray()
        {
            return *m_ComponentManager.GetComponentArray<ComponentType>();
        }
        template <typename SystemType> void RegisterSystem() { m_SystemManager.RegisterSystem<SystemType>()->Bind(&m_ComponentManager); }
        void                                UpdateSystems(double dt) { m_SystemManager.Update(dt); }

    private:
        EntityManager    m_EntityManager;
        ComponentManager m_ComponentManager;
        SystemManager    m_SystemManager;
    };
}  // namespace DE