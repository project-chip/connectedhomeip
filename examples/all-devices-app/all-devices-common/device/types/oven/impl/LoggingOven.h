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

#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
#include <device/types/cooktop/impl/LoggingCooktop.h>
#include <device/types/oven/Oven.h>
#include <device/types/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.h>

namespace chip::app {

class LoggingOven : public Oven
{
public:
    struct Config
    {
        TemperatureControlledCabinetPart::Config cavityConfig;
    };

    explicit LoggingOven(TimerDelegate & timerDelegate);
    LoggingOven(TimerDelegate & timerDelegate, Config config);
    ~LoggingOven() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override
    {
        ReturnErrorOnFailure(Oven::Register(allocator, provider, composition));
        mCavityOpStateDelegate.SetCluster(&Cavity().OperationalState());
        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        mCavityOpStateDelegate.SetCluster(nullptr);
        Oven::Unregister(provider);
    }

private:
    Clusters::OperationalState::EmulatedOperationalStateDelegate mCavityOpStateDelegate;
    LoggingTemperatureControlledCabinetPart mLoggingCavity;
    LoggingCookSurfacePart mLoggingSurface;
};

} // namespace chip::app
