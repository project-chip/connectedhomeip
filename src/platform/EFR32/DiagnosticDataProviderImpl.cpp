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
 *          for EFR32 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/EFR32/DiagnosticDataProviderImpl.h>

#include <lwip/tcpip.h>

#include "AppConfig.h"
#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

/*
 * The following Heap stats are compiled values done by the FreeRTOS Heap4 implementation.
 * See /examples/platform/efr32/heap_4_silabs.c
 * It keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation.
 */
CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize = xPortGetFreeHeapSize();
    currentHeapFree     = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    // Calculate the Heap used based on Total heap - Free heap
    int64_t heapUsed = (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(heapUsed >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapUsed = static_cast<uint64_t>(heapUsed);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    // FreeRTOS records the lowest amount of available heap during runtime
    // currentHeapHighWatermark wants the highest heap usage point so we calculate it here
    int64_t HighestHeapUsageRecorded = (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(HighestHeapUsageRecorded >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapHighWatermark = static_cast<uint64_t>(HighestHeapUsageRecorded);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
