/*
 *
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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for renesas platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/renesas/DiagnosticDataProviderImpl.h>

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    // @Neethu If you get errors on xPortGetFreeHeapSize, check if this function call is enabled in FreeRTOSConfig.h.
    // Note that there are multiple in the Smart-Bridge project. Make sure that you modify the correct one.
    size_t freeHeapSize = xPortGetFreeHeapSize();
    currentHeapFree     = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    // @Neethu If you get errors on xPortGetFreeHeapSize, check if this function call is enabled in FreeRTOSConfig.h.
    // Note that there are multiple in the Smart-Bridge project. Make sure that you modify the correct one.
    // Calculate the Heap used based on Total heap - Free heap
    int64_t heapUsed = (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(heapUsed >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapUsed = static_cast<uint64_t>(heapUsed);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    // @Neethu If you get errors on xPortGetFreeHeapSize, check if this function call is enabled in FreeRTOSConfig.h.
    // Note that there are multiple in the Smart-Bridge project. Make sure that you modify the correct one.
    // FreeRTOS records the lowest amount of available heap during runtime
    // currentHeapHighWatermark wants the highest heap usage point so we calculate it here
    int64_t HighestHeapUsageRecorded = (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(HighestHeapUsageRecorded >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapHighWatermark = static_cast<uint64_t>(HighestHeapUsageRecorded);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;
    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);
    if (err == CHIP_NO_ERROR)
    {
        // If the value overflows, return UINT16 max value to provide best-effort number.
        rebootCount = static_cast<uint16_t>(count <= UINT16_MAX ? count : UINT16_MAX);
    }
    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;
    CHIP_ERROR err  = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
