/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *    Provides an implementation of the event flags for the platform.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <wiced_rtos.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the platform.
 */
class EventFlags
{
public:
    CHIP_ERROR Init(void);
    CHIP_ERROR Set(uint32_t flags);
    CHIP_ERROR WaitAnyForever(uint32_t & flags);

private:
    wiced_event_flags_t * mFlags;
};

} // namespace DeviceLayer
} // namespace chip
