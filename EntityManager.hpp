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
                m_AvailableEntities.push(m_Generations.size());
            }
            auto id = m_AvailableEntities.front();
            m_AvailableEntities.pop();
            if (++m_Generations[id] == 0)
            {
                ++m_Generations[id];
            }
            return std::make_pair(id, m_Generations[id]);
        }
        bool Destroy(uint32_t id, uint32_t gen)
        {
            if (m_Generations.find(id) != m_Generations.end() && m_Generations[id] == gen)
            {
                m_AvailableEntities.push(id);
                return true;
            }
            return false;
        }
        bool Valid(uint32_t id, uint32_t gen) const { return m_Generations.find(id) != m_Generations.end() && m_Generations.at(id) == gen; }

    private:
        std::unordered_map<uint32_t, uint32_t> m_Generations;
        std::queue<uint32_t>                   m_AvailableEntities;
    };
}  // namespace ECS