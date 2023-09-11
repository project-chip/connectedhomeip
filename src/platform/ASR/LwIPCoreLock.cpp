/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
