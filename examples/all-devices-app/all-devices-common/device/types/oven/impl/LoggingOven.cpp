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

namespace chip::app {

LoggingOven::LoggingOven(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify) :
    LoggingOven(timerDelegate, platformIdentify, Config{})
{}

LoggingOven::LoggingOven(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify, Config config) :
    mCavity(timerDelegate, platformIdentify, config.cavityConfig, "Cavity"),
    mSurface(timerDelegate, platformIdentify, "Top Surface")
{}

CHIP_ERROR LoggingOven::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    ReturnErrorOnFailure(mCavity.Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    ReturnErrorOnFailure(mSurface.Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    return CHIP_NO_ERROR;
}

void LoggingOven::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    mSurface.Unregister(provider);
    mCavity.Unregister(provider);
}

} // namespace chip::app
