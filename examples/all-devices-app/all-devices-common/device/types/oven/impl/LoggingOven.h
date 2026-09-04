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

#include <device/types/cooktop/impl/LoggingCooktop.h>
#include <device/types/oven/Oven.h>
#include <device/types/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.h>

#include <lib/support/CodeUtils.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace chip::app {

class LoggingOven : public Oven
{
public:
    struct Config
    {
        TemperatureControlledCabinetPart::Config cavityConfig;
        uint8_t cavityCount = 1;
    };

    explicit LoggingOven(TimerDelegate & timerDelegate);
    LoggingOven(TimerDelegate & timerDelegate, Config config);
    ~LoggingOven() override = default;

    size_t GetCavityCount() const { return mCavities.size(); }
    LoggingTemperatureControlledCabinetPart & GetCavity(size_t index)
    {
        VerifyOrDie(index < mCavities.size());
        return *mCavities[index];
    }
    LoggingCookSurfacePart & Surface() { return mSurface; }

protected:
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider) override;
    void UnregisterParts(CodeDrivenDataModelProvider & provider) override;

private:
    // Cavity names are owned here because the parts keep only a const char pointer to them.
    std::vector<std::string> mCavityNames;
    std::vector<std::unique_ptr<LoggingTemperatureControlledCabinetPart>> mCavities;
    LoggingCookSurfacePart mSurface;
};

} // namespace chip::app
