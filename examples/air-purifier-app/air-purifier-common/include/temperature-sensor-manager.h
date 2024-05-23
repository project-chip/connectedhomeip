/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId){};

    void Init()
    {
        Protocols::InteractionModel::Status status = TemperatureMeasurement::Attributes::MinMeasuredValue::Set(mEndpointId, -500);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));

        status = TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 6000);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MaxMeasuredValue attribute"));
    }

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
