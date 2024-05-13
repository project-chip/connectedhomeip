/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
 *    Copyright 2023 NXP
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on NXP platforms using the NXP SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

/* Default NXP Platform adaptations are used */

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the NXP platform.
// These can be overridden by the application as needed.

// ========== Platform-specific Configuration Overrides =========

/* Default NXP Platform configuration overrides are used */

// Include default nxp platform configurations
#include "platform/nxp/common/CHIPDeviceNXPPlatformDefaultConfig.h"
