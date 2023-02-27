#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include "ComponentArray.hpp"

namespace DE
{
    class ComponentManager
    {
    private:
        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays;
        std::unordered_map<std::type_index, uint32_t>                         m_ComponentId;

        template <typename ComponentType> void RegisterComponent()
        {
            const char* component_name = typeid(ComponentType).name();

            m_ComponentId[component_name]     = m_ComponentId.size();
            m_ComponentArrays[component_name] = std::make_shared<ComponentArray<ComponentType>>(ComponentArray<ComponentType>(m_EntityAmount));

            // Logger::Info("ECS", "ComponentManager", "Registered Component (" + NormalTypeName(component_name) + ")");
        }

    public:
        ComponentManager() : m_EntityAmount(0) {}

        void CopyEntity(EntityId from, EntityId to)
        {
            for (auto& [name, component_array] : m_ComponentArrays)
            {
                component_array->CopyEntity(from, to);
            }
        }

        template <typename ComponentType> void AddComponent(EntityId entity_id, ComponentType component = ComponentType())
        {
            GetComponentArray<ComponentType>()->InsertComponent(entity_id, component);
        }
        template <typename ComponentType> ComponentType& GetComponent(EntityId entity_id) { return (*GetComponentArray<ComponentType>())[entity_id]; }
        template <typename ComponentType> void RemoveComponent(EntityId entity_id) { GetComponentArray<ComponentType>()->RemoveComponent(entity_id); }

        void EntityDestroyed(EntityId entity_id)
        {
            for (auto& [name, component_array] : m_ComponentArrays)
            {
                component_array->EntityDestroyed(entity_id);
            }
        }

        template <typename ComponentType> ComponentId GetComponentId(ComponentType component = ComponentType())
        {
            if (m_ComponentId.find(typeid(ComponentType).name()) != m_ComponentId.end())
            {
                return m_ComponentId[typeid(ComponentType).name()];
            }
            return -1;
        }

        template <typename ComponentType> Ref<ComponentArray<ComponentType>> GetComponentArray()
        {
            if (m_ComponentArrays.find(typeid(ComponentType).name()) == m_ComponentArrays.end())
            {
                RegisterComponent<ComponentType>();
            }
            return std::static_pointer_cast<ComponentArray<ComponentType>>(m_ComponentArrays[typeid(ComponentType).name()]);
        }
        void ExtendArrays()
        {
            ++m_EntityAmount;
            for (auto& [name, component_array] : m_ComponentArrays)
            {
                component_array->ExtendArray();
            }
        }
    };
}  // namespace DE