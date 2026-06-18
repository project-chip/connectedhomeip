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

#pragma once

#include <devices/cooktop/CooktopDevice.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/temperature-controlled-cabinet/TemperatureControlledCabinetPart.h>

namespace chip::app {

class OvenDevice : public DeviceInterface
{
public:
    explicit OvenDevice(TimerDelegate & timerDelegate);
    ~OvenDevice() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    TemperatureControlledCabinetPart mCavity;
    CookSurfacePart mSurface;
};

} // namespace chip::app
