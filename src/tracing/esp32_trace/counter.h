#include <esp_diagnostics_metrics.h>
#include <iostream>
#include <string.h>

#define PATH1 "sys.cnt"
namespace Insights {
class ESPInsightsCounter
{
private:
    static ESPInsightsCounter * m_head;
    char label[50];
    char group[50];
    int instanceCount;
    ESPInsightsCounter * m_next;
    bool registered = false;

    ESPInsightsCounter(const char * labelParam, const char * groupParam) : instanceCount(1), m_next(nullptr)
    {
        strncpy(label, labelParam, sizeof(label));
        strncpy(group, groupParam, sizeof(group));
    }

public:
    static ESPInsightsCounter * GetInstance(const char * label, const char * group);

    int GetInstanceCount() const;

    void ReportMetrics()
    {
        std::cout << "Trace instant: Label=" << label << ", Group=" << group << ", Instance Count=" << instanceCount << std::endl;
        if (!registered)
        {
            esp_diag_metrics_register("SYS_CNT", label, label, PATH1, ESP_DIAG_DATA_TYPE_UINT);
            registered = true;
        }
        std::cout << "Label ------" << label << "Count---" << instanceCount;
        esp_diag_metrics_add_uint(label, instanceCount);
    }
};

} // namespace Insights
