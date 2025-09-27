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

#pragma once

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================

#ifndef CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 0
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 0
#else // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#define CHIP_SYSTEM_CONFIG_USE_LWIP 0
#define CHIP_SYSTEM_CONFIG_USE_SOCKETS 1

// ========== Platform-specific Configuration Overrides =========
// Disable Thread Local Storage (TLS) since Realtek SDK does not support it.
#define CHIP_SYSTEM_CONFIG_THREAD_LOCAL_STORAGE 0

// Disable Zephyr Socket extensions module, as the Zephyr RTOS now implements recvmsg()
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 0
