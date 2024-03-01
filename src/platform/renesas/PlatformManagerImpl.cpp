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
 * @file
 * Stub for PlatformManagerImpl for renesas platform.
 */
#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {

/**
 * Singleton instance of the KeyValueStoreManager implementation object.
 */
PlatformManagerImpl PlatformManagerImpl::sInstance;

// CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
// {
//     CHIP_ERROR err = CHIP_NO_ERROR;

// //     // Initialize the configuration system.
// //     err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
// //     SuccessOrExit(err);

// // #if CHIP_SYSTEM_CONFIG_USE_LWIP
// //     // Initialize LwIP.
// //     tcpip_init(NULL, NULL);
// // #endif // CHIP_SYSTEM_CONFIG_USE_LWIP

// //     //ReturnErrorOnFailure(System::Clock::InitClock_RealTime());

// //     // Call _InitChipStack() on the generic implementation base class
// //     // to finish the initialization process.
// //     err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
// //     SuccessOrExit(err);

// //exit:
//     return err;
// }

} // namespace DeviceLayer
} // namespace chip
