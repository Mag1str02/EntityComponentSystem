#pragma once

#include "ECSStorage.hpp"

namespace DE
{
    class Entity
    {
    public:
        Entity() : m_ID(0), m_Gen(0), m_Storage(nullptr) {}

        bool Valid() { return m_Storage && m_Storage->Valid(m_ID, m_Gen); }
        void Destroy()
        {
            if (m_Storage)
            {
                m_Storage->Destroy(m_ID, m_Gen);
            }
            *this = Entity();
        }
        template <typename ComponentType> ComponentType* Add(ComponentType component = ComponentType())
        {
            return m_Storage->AddComponent<ComponentType>(m_ID, m_Gen, component);
        }
        template <typename ComponentType> void           Remove() { m_Storage->RemoveComponent<ComponentType>(m_ID, m_Gen); }
        template <typename ComponentType> ComponentType* Get() { return m_Storage->GetComponent<ComponentType>(m_ID, m_Gen); }
        template <typename ComponentType> bool           Has()
        {
            if (!m_Storage)
            {
                return false;
            }
            return m_Storage->HasComponent<ComponentType>(m_ID, m_Gen);
        }

        bool operator==(const Entity& other) const { return m_ID == other.m_ID && m_Gen == other.m_Gen && m_Storage == other.m_Storage; }
        bool operator!=(const Entity& other) const { return m_ID != other.m_ID || m_Gen != other.m_Gen || m_Storage != other.m_Storage; }

    private:
        friend struct std::hash<Entity>;
        uint32_t    m_ID;
        uint32_t    m_Gen;
        ECSStorage* m_Storage;
    };
}  // namespace DE
namespace std
{
    template <> struct hash<DE::Entity>
    {
        std::size_t operator()(const DE::Entity& entity) const { return hash<uint64_t>()((uint64_t)entity.m_ID << 32 + entity.m_Gen); }
    };
}  // namespace std