#include "counter.h"

namespace Insights {
ESPInsightsCounter * ESPInsightsCounter::m_head = nullptr;

ESPInsightsCounter * ESPInsightsCounter::GetInstance(const char * label, const char * group)
{

    ESPInsightsCounter * current  = m_head;
    ESPInsightsCounter * previous = nullptr;

    while (current != nullptr)
    {
        if (strcmp(current->label, label) == 0 && strcmp(current->group, group) == 0)
        {
            current->instanceCount++;
            return current;
        }
        previous = current;
        current  = current->m_next;
    }

    ESPInsightsCounter * newInstance = new ESPInsightsCounter(label, group);
    newInstance->m_next              = m_head;
    m_head                           = newInstance;
    return newInstance;
}

int ESPInsightsCounter::GetInstanceCount() const
{
    return instanceCount;
}

} // namespace Insights
