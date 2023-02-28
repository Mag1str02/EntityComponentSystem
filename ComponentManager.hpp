#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include "ComponentArray.hpp"

namespace ECS
{
    class Signature
    {
    public:
        bool Get(uint64_t id) const { return (m_Data.size() <= (id >> 6) ? false : (m_Data[id >> 6] >> (id & 63)) & 1); }
        void Set(uint64_t id, bool value)
        {
            if (m_Data.size() <= id >> 6)
            {
                m_Data.resize((id >> 6) + 1);
            }
            m_Data[id >> 6] &= ~(1 << (id & 63));
            m_Data[id >> 6] |= ((uint64_t)value << (id & 63));
        }

        bool Matches(const Signature& required) const
        {
            size_t i;
            size_t mn = std::min(required.m_Data.size(), m_Data.size());
            for (i = 0; i < mn; ++i)
            {
                if ((m_Data[i] & required.m_Data[i]) != required.m_Data[i])
                {
                    return false;
                }
            }
            while (i < required.m_Data.size())
            {
                if (required.m_Data[i++])
                {
                    return false;
                }
            }
            return true;
        }

    private:
        std::vector<uint64_t> m_Data;
    };
    class ComponentManager
    {
    public:
        ComponentManager() = default;

        template <typename ComponentType> ComponentType* AddComponent(uint32_t entity_id, const ComponentType& component)
        {
            RegisterComponent<ComponentType>();
            ValidateSignature(entity_id);
            m_Signatures[entity_id].Set(m_ComponentId[std::type_index(typeid(ComponentType))], true);
            return reinterpret_cast<ComponentType*>(
                m_ComponentArrays[std::type_index(typeid(ComponentType))]->AddComponent(entity_id, const_cast<ComponentType*>(&component)));
        }
        template <typename ComponentType> ComponentType* GetComponent(uint32_t entity_id)
        {
            ValidateSignature(entity_id);
            if (!HasComponent<ComponentType>(entity_id))
            {
                return nullptr;
            }
            return reinterpret_cast<ComponentType*>(m_ComponentArrays[std::type_index(typeid(ComponentType))]->GetComponent(entity_id));
        }
        template <typename ComponentType> void RemoveComponent(uint32_t entity_id)
        {
            ValidateSignature(entity_id);
            if (HasComponent<ComponentType>(entity_id))
            {
                m_Signatures[entity_id].Set(m_ComponentId[std::type_index(typeid(ComponentType))], false);
                m_ComponentArrays[std::type_index(typeid(ComponentType))]->RemoveComponent(entity_id);
            }
        }
        template <typename ComponentType> bool HasComponent(uint32_t entity_id) const
        {
            auto type = std::type_index(typeid(ComponentType));
            if (m_Signatures.size() < entity_id + 1 || m_ComponentId.find(type) == m_ComponentId.end())
            {
                return false;
            }

            return m_Signatures.at(entity_id).Get(m_ComponentId.at(type));
        }
        void Destroy(uint32_t entity_id)
        {
            for (const auto& [id, array] : m_ComponentArrays)
            {
                array->RemoveComponent(entity_id);
            }
            m_Signatures[entity_id] = Signature();
        }

        template <typename... Components> Signature BuildSignature()
        {
            return (ValidateComponents<Components...>() ? GetSignature<Components...>() : Signature());
        }
        bool Matches(uint32_t id, const Signature& signature) const
        {
            if (m_Signatures.size() <= id)
            {
                return false;
            }
            // printf("Cheking %u...\n", id);
            // m_Signatures.at(id).Print();
            return m_Signatures.at(id).Matches(signature);
        }

    private:
        template <typename T, typename... Components> bool ValidateComponentsHelper() const
        {
            if (m_ComponentId.find(std::type_index(typeid(T))) == m_ComponentId.end())
            {
                return false;
            }
            return ValidateComponents<Components...>();
        }
        template <typename... Components> bool ValidateComponents() const { return ValidateComponentsHelper<Components...>(); }
        template <> bool                       ValidateComponents<>() const { return true; }

        template <typename T, typename... Components> Signature GetSignatureHelper() const
        {
            Signature res = GetSignature<Components...>();
            res.Set(m_ComponentId.at(std::type_index(typeid(T))), true);
            return res;
        }
        template <typename... Components> Signature GetSignature() const { return GetSignatureHelper<Components...>(); }
        template <> Signature                       GetSignature<>() const { return Signature(); }

        void ValidateSignature(uint32_t entity_id)
        {
            while (m_Signatures.size() < entity_id + 1)
            {
                m_Signatures.push_back(Signature());
            }
        }
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
        std::vector<Signature>                                                m_Signatures;
    };
}  // namespace ECS