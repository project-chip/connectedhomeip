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
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on nRF Connect SDK platform.
 *
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

#ifndef CONFIG_ARCH_POSIX
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 15
#endif

#ifndef CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
#ifdef CONFIG_WIFI_NRF70
// Minimal mDNS uses Matter packet buffers, so as long as minimal mDNS is used
// in Nordic's Wi-Fi solution, the packet buffers must be a bit bigger than what
// is required by Matter.
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX CONFIG_NRF_WIFI_IFACE_MTU
#else
// Matter specification requires that Matter messages fit in IPV6 MTU of 1280B
// unless for large messages that can be sent over BTP or TCP. But those will
// likely require a separate buffer pool or employ chained buffers.
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 1280
#endif // CONFIG_WIFI_NRF70
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX

// ========== Platform-specific Configuration Overrides =========

// Disable Zephyr Socket extensions module, as the Zephyr RTOS now implements recvmsg()
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 0
