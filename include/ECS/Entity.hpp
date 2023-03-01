#pragma once

namespace ECS
{

    Entity::Entity() : m_ID(0), m_Gen(0), m_Storage(nullptr) {}

    bool Entity::Valid() const { return m_Storage && m_Storage->Valid(m_ID, m_Gen); }
    void Entity::Destroy()
    {
        if (m_Storage)
        {
            m_Storage->Destroy(m_ID, m_Gen);
        }
        *this = Entity();
    }

    template <typename ComponentType> ComponentType* Entity::Add(ComponentType component)
    {
        return (m_Storage ? m_Storage->AddComponent<ComponentType>(m_ID, m_Gen, component) : nullptr);
    }
    template <typename ComponentType> ComponentType* Entity::Get()
    {
        return (m_Storage ? m_Storage->GetComponent<ComponentType>(m_ID, m_Gen) : nullptr);
    }
    template <typename ComponentType> bool Entity::Has() const
    {
        if (!m_Storage)
        {
            return false;
        }
        return m_Storage->HasComponent<ComponentType>(m_ID, m_Gen);
    }
    template <typename ComponentType> void Entity::Remove()
    {
        if (m_Storage)
        {
            m_Storage->RemoveComponent<ComponentType>(m_ID, m_Gen);
        }
    }

    bool Entity::operator==(const Entity& other) const { return m_ID == other.m_ID && m_Gen == other.m_Gen && m_Storage == other.m_Storage; }
    bool Entity::operator!=(const Entity& other) const { return m_ID != other.m_ID || m_Gen != other.m_Gen || m_Storage != other.m_Storage; }

}  // namespace ECS

namespace std
{
    template <> struct hash<ECS::Entity>
    {
        std::size_t operator()(const ECS::Entity& entity) const
        {
            return hash<uint64_t>()(((uint64_t)entity.m_ID << 32) + entity.m_Gen) ^ hash<uint64_t>()(reinterpret_cast<uint64_t>(entity.m_Storage));
        }
    };
}  // namespace std