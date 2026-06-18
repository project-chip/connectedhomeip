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

#include "RefrigeratorDevice.h"

#include <devices/Types.h>

namespace chip::app {

RefrigeratorDevice::RefrigeratorDevice(TimerDelegate & timerDelegate) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRefrigerator, 1)),
    mCabinet(timerDelegate,
             TemperatureControlledCabinetPart::Config{
                 .temperatureSetpoint = 400, // 4.00 °C
                 .minTemperature      = 100, // 1.00 °C
                 .maxTemperature      = 700, // 7.00 °C
                 .step                = 10,
             })
{}

CHIP_ERROR RefrigeratorDevice::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                        EndpointId parentId)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId = allocator.Allocate();

    ReturnErrorOnFailure(InitEndpointRegistration(mEndpointId, provider,
                                                  EndpointComposition(parentId, DataModel::EndpointCompositionPattern::kTree)));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mCabinet.Register(allocator, provider, mEndpointId));

    return CHIP_NO_ERROR;
}

void RefrigeratorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mCabinet.Unregister(provider);
    ShutdownEndpointRegistration(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
