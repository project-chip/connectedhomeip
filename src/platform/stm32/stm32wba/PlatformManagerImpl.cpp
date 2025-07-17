/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *           Provides an implementation of the PlatformManager object
 *            for the stm32 platform.
 */

#include <crypto/CHIPCryptoPAL.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/stm32/stm32wba/DiagnosticDataProviderImpl.h>
#include <platform/CommissionableDataProvider.h>


namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

extern "C" int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen);

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    System::Clock::InitClock_RealTime();
	chip::Crypto::add_entropy_source(mbedtls_hardware_poll, NULL, 16);
	ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack());
    // Start timer to increment TotalOperationalHours every hour
    SystemLayer().StartTimer(System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_RunEventLoop(void)
{
	Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_RunEventLoop();
}

void PlatformManagerImpl::UpdateOperationalHours(System::Layer * systemLayer, void * appState)
{
    uint32_t totalOperationalHours = 0;

    if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + 1);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    SystemLayer().StartTimer(System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);
}

void PlatformManagerImpl::_Shutdown()
{
     Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
