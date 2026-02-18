
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
        auto temperatureMeasurement = app::Clusters::TemperatureMeasurement::FindClusterOnEndpoint(mEndpointId);
        VerifyOrReturn(temperatureMeasurement != nullptr);

        CHIP_ERROR err = temperatureMeasurement->SetMinMeasuredValue(-5);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));

        err = temperatureMeasurement->SetMaxMeasuredValue(60);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MaxMeasuredValue attribute"));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        auto temperatureMeasurement = app::Clusters::TemperatureMeasurement::FindClusterOnEndpoint(mEndpointId);
        VerifyOrReturn(temperatureMeasurement != nullptr);

        CHIP_ERROR err = temperatureMeasurement->SetMeasuredValue(newValue);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue attribute"));
        ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
