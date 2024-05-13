
#pragma once
#include <app-common/zap-generated/attributes/Accessors.h>

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId)
    {
        Protocols::InteractionModel::Status status = TemperatureMeasurement::Attributes::MinMeasuredValue::Set(mEndpointId, -5);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));

        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 60);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MaxMeasuredValue attribute"));
    };

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        Protocols::InteractionModel::Status status = TemperatureMeasurement::Attributes::MeasuredValue::Set(mEndpointId, newValue);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MeasuredValue attribute"));
        ChipLogDetail(NotSpecified, "The new TemperatureMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
