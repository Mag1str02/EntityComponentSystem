#pragma once

namespace ECS
{
    bool Signature::Get(uint64_t id) const { return (m_Data.size() <= (id >> 6) ? false : (m_Data[id >> 6] >> (id & 63)) & 1); }
    void Signature::Set(uint64_t id, bool value)
    {
        if (m_Data.size() <= id >> 6)
        {
            m_Data.resize((id >> 6) + 1);
        }
        m_Data[id >> 6] &= ~(1 << (id & 63));
        m_Data[id >> 6] |= ((uint64_t)value << (id & 63));
    }
    bool Signature::Matches(const Signature& required) const
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

    template <typename ComponentType> ComponentType* ComponentManager::AddComponent(uint32_t entity_id, const ComponentType& component)
    {
        RegisterComponent<ComponentType>();
        ValidateSignature(entity_id);
        m_Signatures[entity_id].Set(m_ComponentId[INDEX(ComponentType)], true);
        return reinterpret_cast<ComponentType*>(
            m_ComponentArrays[INDEX(ComponentType)]->AddComponent(entity_id, const_cast<ComponentType*>(&component)));
    }
    template <typename ComponentType> ComponentType* ComponentManager::GetComponent(uint32_t entity_id)
    {
        ValidateSignature(entity_id);
        if (!HasComponent<ComponentType>(entity_id))
        {
            return nullptr;
        }
        return reinterpret_cast<ComponentType*>(m_ComponentArrays[INDEX(ComponentType)]->GetComponent(entity_id));
    }
    template <typename ComponentType> void ComponentManager::RemoveComponent(uint32_t entity_id)
    {
        ValidateSignature(entity_id);
        if (HasComponent<ComponentType>(entity_id))
        {
            m_Signatures[entity_id].Set(m_ComponentId[INDEX(ComponentType)], false);
            m_ComponentArrays[INDEX(ComponentType)]->RemoveComponent(entity_id);
        }
    }
    template <typename ComponentType> bool ComponentManager::HasComponent(uint32_t entity_id) const
    {
        if (m_Signatures.size() < entity_id + 1 || m_ComponentId.find(INDEX(ComponentType)) == m_ComponentId.end())
        {
            return false;
        }

        return m_Signatures.at(entity_id).Get(m_ComponentId.at(INDEX(ComponentType)));
    }
    void ComponentManager::Destroy(uint32_t entity_id)
    {
        for (const auto& [id, array] : m_ComponentArrays)
        {
            array->RemoveComponent(entity_id);
        }
        m_Signatures[entity_id] = Signature();
    }

    template <typename... Components> Signature ComponentManager::BuildSignature()
    {
        return (ValidateComponents<Components...>() ? GetSignature<Components...>() : Signature());
    }
    bool ComponentManager::Matches(uint32_t id, const Signature& signature) const
    {
        if (m_Signatures.size() <= id)
        {
            return false;
        }
        // printf("Cheking %u...\n", id);
        // m_Signatures.at(id).Print();
        return m_Signatures.at(id).Matches(signature);
    }

    template <typename T, typename... Components> bool ComponentManager::ValidateComponentsHelper() const
    {
        if (m_ComponentId.find(INDEX(T)) == m_ComponentId.end())
        {
            return false;
        }
        return ValidateComponents<Components...>();
    }
    template <typename... Components> bool ComponentManager::ValidateComponents() const { return ValidateComponentsHelper<Components...>(); }
    template <> bool                       ComponentManager::ValidateComponents<>() const { return true; }

    template <typename T, typename... Components> Signature ComponentManager::GetSignatureHelper() const
    {
        Signature res = GetSignature<Components...>();
        res.Set(m_ComponentId.at(INDEX(T)), true);
        return res;
    }
    template <typename... Components> Signature ComponentManager::GetSignature() const { return GetSignatureHelper<Components...>(); }
    template <> Signature                       ComponentManager::GetSignature<>() const { return Signature(); }

    void ComponentManager::ValidateSignature(uint32_t entity_id)
    {
        while (m_Signatures.size() < entity_id + 1)
        {
            m_Signatures.push_back(Signature());
        }
    }
    template <typename ComponentType> void ComponentManager::RegisterComponent()
    {
        if (m_ComponentId.find(INDEX(ComponentType)) == m_ComponentId.end())
        {
            m_ComponentId[INDEX(ComponentType)]     = m_ComponentId.size();
            m_ComponentArrays[INDEX(ComponentType)] = std::make_shared<ComponentArray<ComponentType>>(ComponentArray<ComponentType>());
        }
    }
}  // namespace ECS