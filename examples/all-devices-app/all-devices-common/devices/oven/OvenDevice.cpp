/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OvenDevice.h"

#include <devices/Types.h>

namespace chip::app {

OvenDevice::OvenDevice(TimerDelegate & timerDelegate) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOven, 1)),
    mCavity(timerDelegate,
            TemperatureControlledCabinetPart::Config{
                .temperatureSetpoint = 18000, // 180.00 °C
                .minTemperature      = 5000,  // 50.00 °C
                .maxTemperature      = 25000, // 250.00 °C
                .step                = 500,   // 5.00 °C
            }),
    mSurface(timerDelegate)
{}

CHIP_ERROR OvenDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId = endpoint;

    ReturnErrorOnFailure(InitEndpointRegistration(endpoint, provider,
                                                  EndpointComposition(parentId, DataModel::EndpointCompositionPattern::kTree)));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mCavity.Register(endpoint + 1, provider, endpoint));
    ReturnErrorOnFailure(mSurface.Register(endpoint + 2, provider, endpoint));

    return CHIP_NO_ERROR;
}

void OvenDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mSurface.Unregister(provider);
    mCavity.Unregister(provider);
    ShutdownEndpointRegistration(mEndpointId, provider);
}

} // namespace chip::app
