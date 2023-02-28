#pragma once

#include <vector>
#include <unordered_map>
namespace ECS
{
    class IComponentArray
    {
    public:
        virtual ~IComponentArray()                               = default;
        virtual void* AddComponent(uint32_t id, void* component) = 0;
        virtual void* GetComponent(uint32_t id)                  = 0;
        virtual void  RemoveComponent(uint32_t id)               = 0;
    };

    template <typename ComponentType> class ComponentArray : public IComponentArray
    {
    public:
        ComponentArray() = default;

        virtual void* AddComponent(uint32_t id, void* component)
        {
            if (m_ComponentArray.size() < id + 1)
            {
                m_EntityToIndex[id] = m_ComponentArray.size();
                m_ComponentArray.push_back(*reinterpret_cast<ComponentType*>(component));
            }
            else
            {
                m_ComponentArray[m_EntityToIndex[id]] = *reinterpret_cast<ComponentType*>(component);
            }
            return &m_ComponentArray[m_EntityToIndex[id]];
        }
        virtual void* GetComponent(uint32_t id) { return &m_ComponentArray[m_EntityToIndex[id]]; }
        virtual void  RemoveComponent(uint32_t id)
        {
            if (m_EntityToIndex[id] != m_ComponentArray.size() - 1)
            {
                m_ComponentArray[m_EntityToIndex[id]] = m_ComponentArray.back();
            }
            m_ComponentArray.pop_back();
        }

    private:
        std::unordered_map<uint32_t, uint32_t> m_EntityToIndex;
        std::vector<ComponentType>             m_ComponentArray;
    };

}  // namespace ECS