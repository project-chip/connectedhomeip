/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lega_rtos_api.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

lega_mutex_t LwIPCoreLock;

}

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitLwIPCoreLock(void)
{
    if (LwIPCoreLock == NULL)
    {
        lega_rtos_init_mutex(&LwIPCoreLock);
        if (LwIPCoreLock == NULL)
        {
            ChipLogError(DeviceLayer, "Failed to create LwIP core lock");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

extern "C" void lock_lwip_core()
{
    lega_rtos_lock_mutex(&LwIPCoreLock, LEGA_WAIT_FOREVER);
}

extern "C" void unlock_lwip_core()
{
    lega_rtos_unlock_mutex(&LwIPCoreLock);
}
