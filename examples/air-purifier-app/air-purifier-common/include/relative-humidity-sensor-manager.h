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

class RelativeHumiditySensorManager
{
public:
    RelativeHumiditySensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId){};

    void Init()
    {
        Protocols::InteractionModel::Status status = RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(mEndpointId, 0);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MinMeasuredValue attribute"));

        status = RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(mEndpointId, 10000);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MaxMeasuredValue attribute"));
    };

    void OnHumidityChangeHandler(uint16_t newValue)
    {
        Protocols::InteractionModel::Status status =
            RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(mEndpointId, newValue);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set RelativeHumidityMeasurement MeasuredValue attribute"));
        ChipLogDetail(NotSpecified, "The new RelativeHumidityMeasurement value: %d", newValue);
    }

private:
    EndpointId mEndpointId;
};

} // namespace Clusters
} // namespace app
} // namespace chip
