/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides an implementation of the PlatformManager object
 *          for Qorvo QPG platforms
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.cpp>

#include <lwip/tcpip.h>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::QPGConfig::Init();
    SuccessOrExit(err);
    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());

    // Initialize LwIP.
    tcpip_init(NULL, NULL);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize;
    size_t usedHeapSize;
    size_t highWatermarkHeapSize;

    qvCHIP_GetHeapStats(&freeHeapSize, &usedHeapSize, &highWatermarkHeapSize);
    currentHeapFree = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    size_t freeHeapSize;
    size_t usedHeapSize;
    size_t highWatermarkHeapSize;

    qvCHIP_GetHeapStats(&freeHeapSize, &usedHeapSize, &highWatermarkHeapSize);
    currentHeapUsed = static_cast<uint64_t>(usedHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    size_t freeHeapSize;
    size_t usedHeapSize;
    size_t highWatermarkHeapSize;

    qvCHIP_GetHeapStats(&freeHeapSize, &usedHeapSize, &highWatermarkHeapSize);
    currentHeapHighWatermark = static_cast<uint64_t>(highWatermarkHeapSize);
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
