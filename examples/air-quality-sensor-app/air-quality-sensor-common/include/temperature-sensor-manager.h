
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
        CHIP_ERROR err = TemperatureMeasurement::SetMeasuredValueRange(mEndpointId, -5, 60);
        VerifyOrReturn(
            err == CHIP_NO_ERROR,
            ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValueRange for Endpoint: %d", mEndpointId));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        CHIP_ERROR err = TemperatureMeasurement::SetMeasuredValue(mEndpointId, newValue);
        VerifyOrReturn(
            err == CHIP_NO_ERROR,
            ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue for Endpoint: %d", mEndpointId));
        ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
