#pragma once

#include <app/clusters/relative-humidity-measurement-server/CodegenIntegration.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {

class RelativeHumiditySensorManager
{
public:
    RelativeHumiditySensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId){};

    void OnHumidityChangeHandler(uint16_t newValue)
    {
        CHIP_ERROR err = RelativeHumidityMeasurement::SetMeasuredValue(mEndpointId, DataModel::MakeNullable<uint16_t>(newValue));
        VerifyOrReturn(CHIP_NO_ERROR == err,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MeasuredValue: %" CHIP_ERROR_FORMAT,
                                    err.Format()));
        ChipLogDetail(NotSpecified, "The new RelativeHumidityMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
