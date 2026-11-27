/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>
#include <lib/core/CHIPError.h>

namespace chip::NXP::App::LowPower {

/**
 * @brief Low-power related init function.
 *
 * For example, it can be used to register an enter/exit low power callback.
 *
 */
CHIP_ERROR Init();

#if CONFIG_APP_FREERTOS_OS && CONFIG_CHIP_NXP_PLATFORM_RW61X && CONFIG_NXP_USE_LOW_POWER
/**
 * Apply a timed low-power constraint.
 *
 * Init() must be called before using the runtime control interfaces.
 * Accepts Sleep through PowerDown and a duration of at least one OS tick.
 * Returns CHIP_ERROR_INVALID_ARGUMENT for invalid parameters and
 * CHIP_ERROR_INCORRECT_STATE while a timed mode is already active.
 * The timer restores the WFI constraint when the duration expires.
 */
CHIP_ERROR StartTimedMode(uint8_t mode, uint32_t durationMs);

void EnableDeepSleep();
void DisableDeepSleep();
#endif

} // namespace chip::NXP::App::LowPower
