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

#include "LoggingOven.h"

#include <lib/support/CodeUtils.h>

namespace chip::app {

LoggingOven::LoggingOven(TimerDelegate & timerDelegate) : LoggingOven(timerDelegate, Config{}) {}

LoggingOven::LoggingOven(TimerDelegate & timerDelegate, Config config) : mSurface(timerDelegate, "Top Surface")
{
    // The MA-oven device type requires at least one cavity endpoint.
    VerifyOrDie(config.cavityCount >= 1);

    // Reserve up front: the parts capture the name pointers at construction, so the
    // name strings must not move afterwards.
    mCavityNames.reserve(config.cavityCount);
    mCavities.reserve(config.cavityCount);
    for (uint8_t i = 0; i < config.cavityCount; i++)
    {
        mCavityNames.push_back("Cavity " + std::to_string(i + 1));
        mCavities.push_back(std::make_unique<LoggingTemperatureControlledCabinetPart>(timerDelegate, config.cavityConfig,
                                                                                      mCavityNames.back().c_str()));
    }
}

CHIP_ERROR LoggingOven::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    for (auto & cavity : mCavities)
    {
        ReturnErrorOnFailure(cavity->Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    }
    ReturnErrorOnFailure(mSurface.Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    return CHIP_NO_ERROR;
}

void LoggingOven::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    mSurface.Unregister(provider);
    for (auto it = mCavities.rbegin(); it != mCavities.rend(); ++it)
    {
        (*it)->Unregister(provider);
    }
}

} // namespace chip::app
