#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include "ComponentArray.hpp"
namespace ECS
{

    class ComponentManager
    {
    public:
        ComponentManager() = default;

        template <typename ComponentType> ComponentType* AddComponent(uint32_t entity_id, const ComponentType& component)
        {
            RegisterComponent<ComponentType>();
            return m_ComponentArrays[std::type_index(typeid(ComponentType))]->AddComponent(entity_id, component);
        }
        template <typename ComponentType> ComponentType* GetComponent(uint32_t entity_id)
        {
            RegisterComponent<ComponentType>();
            return m_ComponentArrays[std::type_index(typeid(ComponentType))]->GetComponent(entity_id);
        }
        template <typename ComponentType> void RemoveComponent(uint32_t entity_id)
        {
            RegisterComponent<ComponentType>();
            m_ComponentArrays[std::type_index(typeid(ComponentType))]->RemoveComponent(entity_id);
        }
        template <typename ComponentType> bool HasComponent(uint32_t entity_id)
        {
            RegisterComponent<ComponentType>();
            return m_ComponentArrays[std::type_index(typeid(ComponentType))]->HasComponent(entity_id);
        }
        void Destroy(uint32_t entity_id)
        {
            for (const auto& [id, array] : m_ComponentArrays)
            {
                array->RemoveComponent(entity_id);
            }
        }

    private:
        template <typename ComponentType> void RegisterComponent()
        {
            auto type = std::type_index(typeid(ComponentType));
            if (m_ComponentId.find(type) == m_ComponentId.end())
            {
                m_ComponentId[type]     = m_ComponentId.size();
                m_ComponentArrays[type] = std::make_shared<ComponentArray<ComponentType>>(ComponentArray<ComponentType>());
            }
        }

        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays;
        std::unordered_map<std::type_index, uint32_t>                         m_ComponentId;
    };

}  // namespace ECS