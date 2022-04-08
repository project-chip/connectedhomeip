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
 *          for CYW30739 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CYW30739/DiagnosticDataProviderImpl.h>
#include <platform/DiagnosticDataProvider.h>

#include <hal/wiced_memory.h>
#include <malloc.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapFree = wiced_memory_get_free_bytes() + mallocInfo.fordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapUsed = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapHighWatermark = mallocInfo.arena;

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
