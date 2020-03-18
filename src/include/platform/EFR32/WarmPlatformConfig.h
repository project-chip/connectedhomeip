/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *          Platform-specific configuration overrides for the Weave
 *          Addressing and Routing Module (WARM) on EFR32 platforms
 *          using the Silcon Labs SDK.
 *
 */

#ifndef WARM_PLATFORM_CONFIG_H
#define WARM_PLATFORM_CONFIG_H

// ==================== Platform Adaptations ====================

#define WARM_CONFIG_SUPPORT_THREAD 1
#define WARM_CONFIG_SUPPORT_THREAD_ROUTING 0
#define WARM_CONFIG_SUPPORT_LEGACY6LOWPAN_NETWORK 0
#define WARM_CONFIG_SUPPORT_WIFI 0
#define WARM_CONFIG_SUPPORT_CELLULAR 0

// ========== Platform-specific Configuration Overrides =========

/* none so far */

#endif // WARM_PLATFORM_CONFIG_H
