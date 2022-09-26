/*
 *
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

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on the Beken platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define INET_CONFIG_ERROR_TYPE int32_t
#define INET_CONFIG_NO_ERROR 0
#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999

// ==================== General Configuration Overrides ====================

// NOTE: Values that are mapped to CONFIG_ #defines are settable via the Kconfig mechanism.

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 10
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 10
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

#define INET_CONFIG_ENABLE_IPV4 1
