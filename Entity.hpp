#pragma once

#include "ComponentManager.hpp"
#include "EntityManager.hpp"
// #include "SystemManager.hpp"
namespace ECS
{
    class Entity;
    class Storage
    {
    public:
        Storage()                          = default;
        ~Storage()                         = default;
        Storage(const Storage&)            = delete;
        Storage(Storage&&)                 = delete;
        Storage& operator=(const Storage&) = delete;
        Storage& operator=(Storage&&)      = delete;

        Entity CreateEntity();

        // template <typename ComponentType> ComponentArray<ComponentType>& GetComponentArray()
        // {
        //     return *m_ComponentManager.GetComponentArray<ComponentType>();
        // }
        // template <typename SystemType> void RegisterSystem() { m_SystemManager.RegisterSystem<SystemType>()->Bind(&m_ComponentManager); }
        // void                                UpdateSystems(double dt) { m_SystemManager.Update(dt); }

    private:
        friend class Entity;

        void Destroy(uint32_t id, uint32_t gen);
        bool Valid(uint32_t id, uint32_t gen) const;

        template <typename ComponentType> ComponentType* AddComponent(uint32_t id, uint32_t gen, ComponentType& component);
        template <typename ComponentType> ComponentType* RemoveComponent(uint32_t id, uint32_t gen);
        template <typename ComponentType> ComponentType* GetComponent(uint32_t id, uint32_t gen);
        template <typename ComponentType> bool           HasComponent(uint32_t id, uint32_t gen) const;

        EntityManager    m_EntityManager;
        ComponentManager m_ComponentManager;
        // SystemManager    m_SystemManager;
    };

    class Entity
    {
    public:
        Entity() : m_ID(0), m_Gen(0), m_Storage(nullptr) {}

        bool Valid() const;
        void Destroy();

        template <typename ComponentType> ComponentType* Add(ComponentType component = ComponentType());
        template <typename ComponentType> void           Remove();
        template <typename ComponentType> ComponentType* Get();
        template <typename ComponentType> bool           Has() const;

        bool operator==(const Entity& other) const;
        bool operator!=(const Entity& other) const;

    private:
        friend struct std::hash<Entity>;
        friend class Storage;

        uint32_t m_ID;
        uint32_t m_Gen;
        Storage* m_Storage;
    };

    Entity Storage::CreateEntity()
    {
        auto [id, gen] = m_EntityManager.CreateEntity();
        Entity ent;
        ent.m_ID      = id;
        ent.m_Gen     = gen;
        ent.m_Storage = this;
        return ent;
    }
    bool Storage::Valid(uint32_t id, uint32_t gen) const { return m_EntityManager.Valid(id, gen); }
    void Storage::Destroy(uint32_t id, uint32_t gen)
    {
        if (m_EntityManager.Destroy(id, gen))
        {
            m_ComponentManager.Destroy(id);
        }
    }

    template <typename ComponentType> ComponentType* Storage::AddComponent(uint32_t id, uint32_t gen, ComponentType& component)
    {
        return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.AddComponent<ComponentType>(id, component) : nullptr);
    }
    template <typename ComponentType> ComponentType* Storage::RemoveComponent(uint32_t id, uint32_t gen)
    {
        if (m_EntityManager.Valid(id, gen))
        {
            m_ComponentManager.RemoveComponent<ComponentType>(id);
        }
    }
    template <typename ComponentType> ComponentType* Storage::GetComponent(uint32_t id, uint32_t gen)
    {
        return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.GetComponent<ComponentType>(id) : nullptr);
    }
    template <typename ComponentType> bool Storage::HasComponent(uint32_t id, uint32_t gen) const
    {
        return (m_EntityManager.Valid(id, gen) ? m_ComponentManager.HasComponent<ComponentType>(id) : false);
    }

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
        return m_Storage->AddComponent<ComponentType>(m_ID, m_Gen, component);
    }
    template <typename ComponentType> void           Entity::Remove() { m_Storage->RemoveComponent<ComponentType>(m_ID, m_Gen); }
    template <typename ComponentType> ComponentType* Entity::Get() { return m_Storage->GetComponent<ComponentType>(m_ID, m_Gen); }
    template <typename ComponentType> bool           Entity::Has() const
    {
        if (!m_Storage)
        {
            return false;
        }
        return m_Storage->HasComponent<ComponentType>(m_ID, m_Gen);
    }

    bool Entity::operator==(const Entity& other) const { return m_ID == other.m_ID && m_Gen == other.m_Gen && m_Storage == other.m_Storage; }
    bool Entity::operator!=(const Entity& other) const { return m_ID != other.m_ID || m_Gen != other.m_Gen || m_Storage != other.m_Storage; }

}  // namespace ECS
namespace std
{
    template <> struct hash<ECS::Entity>
    {
        std::size_t operator()(const ECS::Entity& entity) const { return hash<uint64_t>()((uint64_t)entity.m_ID << 32 + entity.m_Gen); }
    };
}  // namespace std