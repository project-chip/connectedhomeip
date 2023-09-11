/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/logging/CHIPLogging.h>

namespace {

SemaphoreHandle_t LwIPCoreLock;

}

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitLwIPCoreLock(void)
{
    if (LwIPCoreLock == NULL)
    {
        LwIPCoreLock = xSemaphoreCreateMutex();
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
    xSemaphoreTake(LwIPCoreLock, portMAX_DELAY);
}

extern "C" void unlock_lwip_core()
{
    xSemaphoreGive(LwIPCoreLock);
}
