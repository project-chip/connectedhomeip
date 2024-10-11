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
#include <functional>

namespace chip {
namespace app {
namespace Clusters {

class ThermostatManager
{
public:
    using HeatingCallbackType = std::function<void()>;

    ThermostatManager(EndpointId aEndpointId, const HeatingCallbackType & callback) :
        mEndpointId(aEndpointId), heatingCallback(callback)
    {}

    void Init();

    void HeatingSetpointWriteCallback(int16_t newValue);
    void SystemModeWriteCallback(uint8_t newValue);
    void OnLocalTemperatureChangeCallback(int16_t temperature);
    void SetHeatMode(bool heat);

private:
    EndpointId mEndpointId;

    void SetHeating(bool isHeating);
    HeatingCallbackType heatingCallback;
};

} // namespace Clusters
} // namespace app
} // namespace chip
