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
#include <app/clusters/temperature-measurement-server/CodegenIntegration.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(EndpointId aEndpointId) : mEndpointId(aEndpointId) {};

    void Init()
    {
        auto temperatureMeasurement = app::Clusters::TemperatureMeasurement::FindClusterOnEndpoint(mEndpointId);
        VerifyOrReturn(temperatureMeasurement != nullptr,
                       ChipLogError(NotSpecified, "Failed to find TemperatureMeasurement Cluster for Endpoint: %d", mEndpointId));

        CHIP_ERROR err = temperatureMeasurement->SetMinMeasuredValue(-500);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MinMeasuredValue attribute"));

        err = temperatureMeasurement->SetMaxMeasuredValue(6000);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(NotSpecified, "Failed to set TemperatureMeasurement MaxMeasuredValue attribute"));
    }

    void OnTemperatureChangeHandler(int16_t newValue)
    {
        auto temperatureMeasurement = app::Clusters::TemperatureMeasurement::FindClusterOnEndpoint(mEndpointId);
        VerifyOrReturn(temperatureMeasurement != nullptr,
                       ChipLogError(NotSpecified, "Failed to find TemperatureMeasurement Cluster for Endpoint: %d", mEndpointId));

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
