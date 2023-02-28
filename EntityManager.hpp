#pragma once

#include <queue>
#include <cstdint>
#include <unordered_map>

namespace ECS
{
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
        std::vector<std::pair<uint32_t, uint32_t>> GetValidEntities() const
        {
            std::vector<std::pair<uint32_t, uint32_t>> res;
            for (size_t i = 0; i < m_States.size(); ++i)
            {
                if (m_States[i])
                {
                    res.emplace_back(i, m_Generations[i]);
                }
            }
            return res;
        }
        void Destroy(uint32_t id)
        {
            m_States[id] = false;
            m_AvailableEntities.push(id);
        }
        bool Valid(uint32_t id, uint32_t gen) const { return m_States[id] && m_Generations[id] == gen; }

    private:
        std::vector<uint32_t> m_Generations;
        std::vector<bool>     m_States;
        std::queue<uint32_t>  m_AvailableEntities;
    };
}  // namespace ECS