/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <system/SystemEvent.h>

namespace chip {
namespace System {

class Layer;
class Object;

class PlatformEventing
{
public:
    static CHIP_ERROR PostEvent(System::Layer & aLayer, Object & aTarget, EventType aType, uintptr_t aArgument);
    static CHIP_ERROR DispatchEvents(System::Layer & aLayer);
    static CHIP_ERROR DispatchEvent(System::Layer & aLayer, Event aEvent);
    static CHIP_ERROR StartTimer(System::Layer & aLayer, uint32_t aMilliseconds);
};

} // namespace System
} // namespace chip
