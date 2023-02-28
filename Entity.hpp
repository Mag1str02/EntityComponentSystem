#pragma once

#include "ComponentManager.hpp"
#include "EntityManager.hpp"
// #include "SystemManager.hpp"

//*___ENTITY___________________________________________________________________________________________________________________________________________

namespace ECS
{
    class Entity;
    template <typename... Components> class StorageView;
    class Storage
    {
    public:
        Storage()                          = default;
        ~Storage()                         = default;
        Storage(const Storage&)            = delete;
        Storage(Storage&&)                 = delete;
        Storage& operator=(const Storage&) = delete;
        Storage& operator=(Storage&&)      = delete;

        Entity                                                       CreateEntity();
        template <typename... Components> StorageView<Components...> View();

        // template <typename ComponentType> ComponentArray<ComponentType>& GetComponentArray()
        // {
        //     return *m_ComponentManager.GetComponentArray<ComponentType>();
        // }
        // template <typename SystemType> void RegisterSystem() { m_SystemManager.RegisterSystem<SystemType>()->Bind(&m_ComponentManager); }
        // void                                UpdateSystems(double dt) { m_SystemManager.Update(dt); }

    private:
        template <typename... Components> friend class StorageView;
        friend class Entity;

        bool Valid(uint32_t id, uint32_t gen) const;
        void Destroy(uint32_t id, uint32_t gen);

        template <typename ComponentType> ComponentType* AddComponent(uint32_t id, uint32_t gen, const ComponentType& component);
        template <typename ComponentType> ComponentType* GetComponent(uint32_t id, uint32_t gen);
        template <typename ComponentType> bool           HasComponent(uint32_t id, uint32_t gen) const;
        template <typename ComponentType> void           RemoveComponent(uint32_t id, uint32_t gen);

        EntityManager    m_EntityManager;
        ComponentManager m_ComponentManager;
        // SystemManager    m_SystemManager;
    };

    //*___ENTITY___________________________________________________________________________________________________________________________________________

    class Entity
    {
    public:
        Entity() : m_ID(0), m_Gen(0), m_Storage(nullptr) {}

        bool Valid() const;
        void Destroy();

        template <typename ComponentType> ComponentType* Add(ComponentType component = ComponentType());
        template <typename ComponentType> ComponentType* Get();
        template <typename ComponentType> bool           Has() const;
        template <typename ComponentType> void           Remove();

        bool operator==(const Entity& other) const;
        bool operator!=(const Entity& other) const;

    private:
        template <typename... Components> friend class StorageView;
        friend struct std::hash<Entity>;
        friend class Storage;

        uint32_t m_ID;
        uint32_t m_Gen;
        Storage* m_Storage;
    };

    //*___VIEW___________________________________________________________________________________________________________________________________________

    template <typename... Components> class StorageView
    {
    public:
        class Iterator
        {
        public:
            Iterator(uint32_t id, StorageView* v) : m_ID(id), m_View(v) {}

            bool      operator==(const Iterator& other) const { return m_View == other.m_View && m_ID == other.m_ID; }
            bool      operator!=(const Iterator& other) const { return m_View != other.m_View || m_ID != other.m_ID; }
            Iterator& operator++();
            Iterator  operator++(int);
            Entity    operator*();

        private:
            friend class StorageView;
            uint32_t     m_ID;
            StorageView* m_View;
        };
        Iterator begin();
        Iterator end();

    private:
        friend class Storage;
        StorageView(Storage* storage);

        Signature m_Signature;
        Storage*  m_Storage;
    };

    //*___STORAGE___________________________________________________________________________________________________________________________________________

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

    //*___ENTITY___________________________________________________________________________________________________________________________________________

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

    //*___VIEW___________________________________________________________________________________________________________________________________________

    template <typename... Components> StorageView<Components...>::StorageView(Storage* storage) : m_Storage(storage)
    {
        m_Signature = m_Storage->m_ComponentManager.BuildSignature<Components...>();
    }
    template <typename... Components> typename StorageView<Components...>::Iterator StorageView<Components...>::begin()
    {
        Iterator res(m_Storage->m_EntityManager.BeginEntity(), this);
        if (!m_Storage->m_ComponentManager.Matches(res.m_ID, m_Signature))
        {
            ++res;
        }
        return res;
    }
    template <typename... Components> typename StorageView<Components...>::Iterator StorageView<Components...>::end()
    {
        return Iterator(m_Storage->m_EntityManager.EndEntity(), this);
    }

    template <typename... Components> typename StorageView<Components...>::Iterator& StorageView<Components...>::Iterator::operator++()
    {
        do
        {
            m_ID = m_View->m_Storage->m_EntityManager.NextEntity(m_ID);
        }
        while (m_ID != m_View->m_Storage->m_EntityManager.EndEntity() && !m_View->m_Storage->m_ComponentManager.Matches(m_ID, m_View->m_Signature));
        return *this;
    }
    template <typename... Components> typename StorageView<Components...>::Iterator StorageView<Components...>::Iterator::operator++(int)
    {
        auto res = *this;
        do
        {
            m_ID = m_View->m_Storage->m_EntityManager.NextEntity(m_ID);
        }
        while (m_ID != m_View->m_Storage->m_EntityManager.EndEntity() && m_View->m_Storage->m_ComponentManager.Matches(m_ID, m_View->m_Signature));
        return res;
    }
    template <typename... Components> Entity StorageView<Components...>::Iterator::operator*()
    {
        Entity res;
        res.m_ID      = m_ID;
        res.m_Gen     = m_View->m_Storage->m_EntityManager.Generation(m_ID);
        res.m_Storage = m_View->m_Storage;
        return res;
    }

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