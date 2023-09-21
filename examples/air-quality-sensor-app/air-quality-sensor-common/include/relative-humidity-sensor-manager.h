#include <app-common/zap-generated/attributes/Accessors.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

class RelativeHumiditySensorManager
{
public:
    RelativeHumiditySensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId)
    {
        EmberAfStatus status = RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(mEndpointId, 0);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MinMeasuredValue attribute"));

        status = RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 100);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MaxMeasuredValue attribute"));
    };

    void OnHumidityChangeHandler(uint16_t newValue)
    {
        EmberAfStatus status = RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(mEndpointId, newValue);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MeasuredValue attribute"));
        ChipLogDetail(NotSpecified, "The new RelativeHumidityMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
