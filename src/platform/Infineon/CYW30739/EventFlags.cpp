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
 *    Provides a class that serves as a wrapper for the event system of CYW30739
 *    platform's underlying RTOS. An event instance is comprised of 32 flags.
 *    Each flag can be utilized for thread synchronization purposes.
 */
#include "EventFlags.h"

#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR EventFlags::Init(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wiced_result_t result;

    mFlags = wiced_rtos_create_event_flags();
    VerifyOrExit(mFlags != nullptr, err = CHIP_ERROR_NO_MEMORY);

    result = wiced_rtos_init_event_flags(mFlags);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_NO_MEMORY);

exit:
    return err;
}

__attribute__((section(".text_in_ram"))) CHIP_ERROR EventFlags::Set(uint32_t flags)
{
    assert(!wiced_rtos_check_for_stack_overflow());

    assert(wiced_rtos_set_event_flags(mFlags, flags) == WICED_SUCCESS);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EventFlags::WaitAnyForever(uint32_t & flags)
{
    const wiced_result_t result =
        wiced_rtos_wait_for_event_flags(mFlags, 0xffffffff, &flags, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
    if (result != WICED_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wiced_rtos_wait_for_event_flags 0x%08x", result);
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
