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
 *          for k32w0 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/nxp/mw320/DiagnosticDataProviderImpl.h>

#include <lwip/tcpip.h>

//#include <openthread/platform/entropy.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize;

    freeHeapSize    = xPortGetFreeHeapSize();
    currentHeapFree = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

// ToDo: Find a suitable for HEAP_SIZE
#define HEAP_SIZE 0x200

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    /*
        size_t freeHeapSize;
        size_t usedHeapSize;

        freeHeapSize = xPortGetFreeHeapSize();
        usedHeapSize = HEAP_SIZE - freeHeapSize;

        currentHeapUsed = static_cast<uint64_t>(usedHeapSize);
    */
    currentHeapUsed = HEAP_SIZE - xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    /*
        size_t highWatermarkHeapSize;

        highWatermarkHeapSize    = HEAP_SIZE - xPortGetMinimumEverFreeHeapSize();
        currentHeapHighWatermark = static_cast<uint64_t>(highWatermarkHeapSize);
    */
    currentHeapHighWatermark = HEAP_SIZE - xPortGetMinimumEverFreeHeapSize();
    return CHIP_NO_ERROR;
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
