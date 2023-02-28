#pragma once

#include <queue>
#include <cstdint>
#include <unordered_map>

namespace ECS
{
    template <typename... Components> class StorageView;
    class EntityManager
    {
    public:
        EntityManager() = default;

        std::pair<uint32_t, uint32_t> CreateEntity()
        {
            if (m_AvailableEntities.empty())
            {
                m_AvailableEntities.push(m_States.size());
                m_States.push_back(false);
                m_Generations.push_back(0);
            }
            auto id = m_AvailableEntities.front();
            m_AvailableEntities.pop();
            m_States[id] = true;
            ++m_Generations[id];
            return std::make_pair(id, m_Generations[id]);
        }
        void Destroy(uint32_t id)
        {
            m_States[id] = false;
            m_AvailableEntities.push(id);
        }
        bool Valid(uint32_t id, uint32_t gen) const { return m_States[id] && m_Generations[id] == gen; }

    private:
        template <typename... Components> friend class StorageView;

        uint32_t NextEntity(uint32_t id) const
        {
            if (id >= m_States.size())
            {
                return m_States.size();
            }
            do
            {
                ++id;
            }
            while (id < m_States.size() && !m_States[id]);
            return id;
        }
        uint32_t Generation(uint32_t id) const { return m_Generations[id]; }
        uint32_t BeginEntity() const
        {
            uint32_t res = 0;
            while (res < m_States.size() && !m_States[res])
            {
                ++res;
            }
            return res;
        }
        uint32_t EndEntity() const { return m_States.size(); }

        std::vector<uint32_t> m_Generations;
        std::vector<bool>     m_States;
        std::queue<uint32_t>  m_AvailableEntities;
    };
}  // namespace ECS