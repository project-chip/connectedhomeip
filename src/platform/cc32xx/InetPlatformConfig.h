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
 *          Platform-specific configuration overrides for the Openchip Inet
 *          Layer on CC32XX platforms using the Texas Instruments SDK.
 *
 */

#ifndef INET_PLATFORM_CONFIG_H
#define INET_PLATFORM_CONFIG_H

// XXX: Seth update config?

// ==================== Platform Adaptations ====================

// XXX: Seth is there a proper platform error type??
#define INET_CONFIG_ERROR_TYPE uint32_t
#define INET_CONFIG_NO_ERROR (0)

#define INET_CONFIG_ERROR_MIN 1000000
#define INET_CONFIG_ERROR_MAX 1000999

#define INET_CONFIG_ENABLE_IPV4 1

// ========== Platform-specific Configuration Overrides =========

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 4
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 8
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

#endif // INET_PLATFORM_CONFIG_H
