/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
 *          Layer on Telink platform.
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

#if defined(CONFIG_SOC_RISCV_TELINK_TL321X) || defined(CONFIG_SOC_SERIES_RISCV_TELINK_B9X_RETENTION)
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP 1
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL 0
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 1280
#endif

#define CHIP_SYSTEM_CONFIG_USE_LWIP 0
#define CHIP_SYSTEM_CONFIG_USE_SOCKETS 1

// Reduce packet buffer pool size (default 15) to reduce ram consumption
#if defined(CONFIG_PM) || defined(CONFIG_SOC_RISCV_TELINK_TL321X)
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 0
#else
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8
#endif

// ========== Platform-specific Configuration Overrides =========

// Disable Zephyr Socket extensions module, as the Zephyr RTOS now implements recvmsg()
#if !defined(CONFIG_ZEPHYR_VERSION_3_3)
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 0
#endif
