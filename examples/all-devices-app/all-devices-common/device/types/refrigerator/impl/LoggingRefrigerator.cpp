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

#include "LoggingRefrigerator.h"

#include <lib/support/CodeUtils.h>

namespace chip::app {

LoggingRefrigerator::LoggingRefrigerator(TimerDelegate & timerDelegate) : LoggingRefrigerator(timerDelegate, Config{}) {}

LoggingRefrigerator::LoggingRefrigerator(TimerDelegate & timerDelegate, Config config)
{
    mTagList = config.tagList;

    // The MA-refrigerator device type requires at least one cabinet endpoint.
    VerifyOrDie(config.cabinetCount >= 1);

    // Reserve up front: the parts capture the name pointers at construction, so the
    // name strings must not move afterwards.
    mCabinetNames.reserve(config.cabinetCount);
    mCabinets.reserve(config.cabinetCount);
    for (uint8_t i = 0; i < config.cabinetCount; i++)
    {
        mCabinetNames.push_back("Cabinet " + std::to_string(i + 1));
        mCabinets.push_back(std::make_unique<LoggingTemperatureControlledCabinetPart>(timerDelegate, config.cabinetConfig,
                                                                                      mCabinetNames.back().c_str()));
    }
}

CHIP_ERROR LoggingRefrigerator::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    for (auto & cabinet : mCabinets)
    {
        ReturnErrorOnFailure(cabinet->Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    }
    return CHIP_NO_ERROR;
}

void LoggingRefrigerator::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    for (auto it = mCabinets.rbegin(); it != mCabinets.rend(); ++it)
    {
        (*it)->Unregister(provider);
    }
}

} // namespace chip::app
