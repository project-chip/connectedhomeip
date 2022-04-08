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
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on Telink platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#ifndef INET_CONFIG_ENABLE_IPV4
#error Inet IPv4 configuration should be configured at build generation time
#endif

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 4
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 4
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

// Although Zephyr defines SO_BINDTODEVICE, it's currently unimplemented
#ifdef HAVE_SO_BINDTODEVICE
#undef HAVE_SO_BINDTODEVICE
#endif // HAVE_SO_BINDTODEVICE
