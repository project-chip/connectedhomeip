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
                       ChipLogError(NotSpecified, "AirQualitySensor App: Failed to set TemperatureMeasurement MinMeasuredValue attribute %x", (unsigned int)status));

        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 60);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "AirQualitySensor App: Failed to set TemperatureMeasurement MaxMeasuredValue attribute %x", (unsigned int)status));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        EmberAfStatus status = TemperatureMeasurement::Attributes::MeasuredValue::Set(mEndpointId, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "AirQualitySensor App: Failed to set TemperatureMeasurement MeasuredValue attribute %x", (unsigned int)status));
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
