#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <queue>

#define INDEX(type) std::type_index(typeid(type))

namespace ECS
{
    //*___CLASS_DECLARATIONS___________________________________________________________________________________________________________________________________________________________________________________________________

    class Entity;
    class Storage;
    class System;
    class IComponentArray;
    class ComponentManager;
    class EntityManager;
    class SystemManager;
    template <typename... Components> class StorageView;
    template <typename ComponentType> class ComponentArray;

    //*___COMPONENT_MANAGER___________________________________________________________________________________________________________________________________________________________________________________________

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

        virtual void* AddComponent(uint32_t id, void* component);
        virtual void* GetComponent(uint32_t id);
        virtual void  RemoveComponent(uint32_t id);

    private:
        std::unordered_map<uint32_t, uint32_t> m_EntityToIndex;
        std::vector<ComponentType>             m_ComponentArray;
    };

    class Signature
    {
    public:
        bool   Get(uint64_t id) const;
        void   Set(uint64_t id, bool value);
        bool   Matches(const Signature& required) const;
        size_t Size() const
        {
            size_t size = 0;
            for (size_t i = 0; i < 64 * m_Data.size(); ++i)
            {
                if (Get(i))
                {
                    ++size;
                }
            }
            return size;
        }

    private:
        std::vector<uint64_t> m_Data;
    };
    class ComponentManager
    {
    public:
        ComponentManager() = default;

        template <typename ComponentType> ComponentType* AddComponent(uint32_t entity_id, const ComponentType& component);
        template <typename ComponentType> ComponentType* GetComponent(uint32_t entity_id);
        template <typename ComponentType> void           RemoveComponent(uint32_t entity_id);
        template <typename ComponentType> bool           HasComponent(uint32_t entity_id) const;
        void                                             Destroy(uint32_t entity_id);

        template <typename... Components> Signature BuildSignature();
        bool                                        Matches(uint32_t id, const Signature& signature) const;

    private:
        template <typename... Components> typename std::enable_if<sizeof...(Components) == 0, bool>::type ValidateComponents() const;
        template <typename T, typename... Components> bool                                                ValidateComponents() const;

        template <typename... Components> typename std::enable_if<sizeof...(Components) == 0, Signature>::type GetSignature() const;
        template <typename T, typename... Components> Signature                                                GetSignature() const;

        void                                   ValidateSignature(uint32_t entity_id);
        template <typename ComponentType> void RegisterComponent();

        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays;
        std::unordered_map<std::type_index, uint32_t>                         m_ComponentId;
        std::vector<Signature>                                                m_Signatures;
    };

    //*___ENTITY_MANAGER___________________________________________________________________________________________________________________________________________________________________________________________

    class EntityManager
    {
    public:
        EntityManager() = default;

        std::pair<uint32_t, uint32_t> CreateEntity();
        void                          Destroy(uint32_t id);
        bool                          Valid(uint32_t id, uint32_t gen) const;

    private:
        template <typename... Components> friend class StorageView;

        uint32_t NextEntity(uint32_t id) const;
        uint32_t Generation(uint32_t id) const;
        uint32_t BeginEntity() const;
        uint32_t EndEntity() const;

        std::vector<uint32_t> m_Generations;
        std::vector<bool>     m_States;
        std::queue<uint32_t>  m_AvailableEntities;
    };

    //*___SYSTEM_MANAGER___________________________________________________________________________________________________________________________________________________________________________________________

    class System
    {
    public:
        virtual ~System()                    = default;
        virtual void        Update(float dt) = 0;
        virtual std::string GetName() const  = 0;

    protected:
        template <typename... Components> StorageView<Components...> View();

    private:
        friend class SystemManager;
        void Bind(Storage* storage);

        Storage* m_Storage;
    };
    class SystemManager
    {
    public:
        SystemManager(Storage* storage);
        void                                            Update(float dt);
        template <typename Before, typename After> void AddDependency();
        template <typename SystemType> void             RegisterSystem();

    private:
        std::vector<std::shared_ptr<System>>          m_Systems;
        std::unordered_map<std::type_index, uint32_t> m_SystemId;
        std::vector<std::vector<uint32_t>>            m_DependencyGraph;
        Storage*                                      m_Storage;
    };

    //*___ENTITY____________________________________________________________________________________________________________________________________________________________________________________________________

    class Entity
    {
    public:
        Entity();

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

    //*___STORAGE___________________________________________________________________________________________________________________________________________________________________________________________

    class Storage
    {
    public:
        Storage();
        ~Storage()                         = default;
        Storage(const Storage&)            = delete;
        Storage(Storage&&)                 = delete;
        Storage& operator=(const Storage&) = delete;
        Storage& operator=(Storage&&)      = delete;

        Entity                                                       CreateEntity();
        template <typename... Components> StorageView<Components...> View();

        template <typename SystemType> void RegisterSystem();
        void                                UpdateSystems(float dt);

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
        SystemManager    m_SystemManager;
    };
    template <typename... Components> class StorageView
    {
    public:
        class Iterator
        {
        public:
            Iterator(uint32_t id, StorageView* v);

            bool      operator==(const Iterator& other) const;
            bool      operator!=(const Iterator& other) const;
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
        bool     Empty();

    private:
        friend class Storage;
        StorageView(Storage* storage);

        Signature m_Signature;
        Storage*  m_Storage;
    };

}  // namespace ECS

#include "ComponentArray.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Storage.hpp"
#include "StorageView.hpp"
#include "System.hpp"
#include "SystemManager.hpp"