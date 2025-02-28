/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8
#endif

#if CHIP_DEVICE_LAYER_TARGET_BL616
// #define CHIP_CONFIG_SHA256_CONTEXT_SIZE sizeof(mbedtls_sha256_context) in hw_acc/sha256_alt.h
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE (32 + 64 + 64 + 19 * 32)
#define CHIP_CONFIG_SHA256_CONTEXT_ALIGN 32
#else

// #define CHIP_CONFIG_SHA256_CONTEXT_SIZE sizeof(bl_sha_ctx_t)
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE ((7 + 1 + 5 + 18 + 16 + 16 + 7) * sizeof(unsigned int))
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE                                                                                     \
    (PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN + CHIP_SYSTEM_HEADER_RESERVE_SIZE)
#endif
