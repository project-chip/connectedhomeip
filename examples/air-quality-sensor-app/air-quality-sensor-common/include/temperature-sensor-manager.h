
#pragma once
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/temperature-measurement-server/CodegenIntegration.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId)
    {
        LogErrorOnFailure(TemperatureMeasurement::SetMeasuredValueRange(mEndpointId, -5, 60));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        LogErrorOnFailure(TemperatureMeasurement::SetMeasuredValue(mEndpointId, newValue));
        ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
