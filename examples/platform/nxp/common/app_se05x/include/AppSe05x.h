/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

namespace chip {
namespace NXP {
namespace App {
namespace Se05x {

/**
 * @brief Initializes the NXP SE05x Secure Element.
 *
 * This function performs all required low‑level setup to make the SE05x
 * available for use by the application. Typical operations may include:
 * - Establishing communication with the SE05x (I2C/SPI)
 * - Verifying the presence and readiness of the secure element
 * - Configuring internal SE05x contexts or session handles
 *
 * This routine should be called once during system startup, before any
 * SE05x‑dependent functionality is used.
 *
 * @retval CHIP_NO_ERROR       Initialization completed successfully.
 * @retval CHIP_ERROR_xxx      A specific error describing why initialization
 *                              failed (communication failure, device not found,
 *                              unsupported configuration, etc.).
 */
CHIP_ERROR Init();

/**
 * @brief Performs post‑initialization tasks for the SE05x Secure Element.
 *
 * This function finalizes configuration after the SE05x has been initialized.
 * It may include steps such as:
 * - Creating or opening secure sessions
 * - Loading keys, certificates, or policies
 * - Running device integrity checks
 * - Preparing features required by higher‑level services
 *
 * This routine must be invoked only after a successful call to @ref Init().
 *
 * @retval CHIP_NO_ERROR       Post‑initialization completed successfully.
 * @retval CHIP_ERROR_xxx      A specific error indicating which post‑init
 *                              operation failed.
 */
CHIP_ERROR PostInit();

} // namespace Se05x
} // namespace App
} // namespace NXP
} // namespace chip
