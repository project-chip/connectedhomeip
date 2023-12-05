#include <app-common/zap-generated/attributes/Accessors.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId)
    {
        EmberAfStatus status = TemperatureMeasurement::Attributes::MinMeasuredValue::Set(mEndpointId, -5);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));

        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 60);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MaxMeasuredValue attribute"));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        EmberAfStatus status = TemperatureMeasurement::Attributes::MeasuredValue::Set(mEndpointId, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue attribute"));
        ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
