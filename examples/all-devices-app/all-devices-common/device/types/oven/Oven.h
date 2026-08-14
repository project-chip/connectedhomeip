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

#include <device/api/Interface.h>
#include <device/types/cooktop/Cooktop.h>
#include <device/types/temperature-controlled-cabinet/TemperatureControlledCabinetPart.h>

namespace chip::app {

class Oven : public DeviceInterface
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & cavityOperationalStateDelegate;
        TemperatureControlledCabinetPart::Config cavityConfig;
    };

    Oven(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & surfaceOnOff, Clusters::IdentifyDelegate & cavityIdentify,
         Clusters::IdentifyDelegate & surfaceIdentify, const Config & config);
    ~Oven() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Composition getters to expose child endpoints
    TemperatureControlledCabinetPart & Cavity() { return mCavity; }
    CookSurfacePart & Surface() { return mSurface; }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    TemperatureControlledCabinetPart mCavity;
    CookSurfacePart mSurface;
};

} // namespace chip::app
