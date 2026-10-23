/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    Copyright 2026 NXP
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
 *          Declares the platform-neutral API for reading the hardware boot/reset
 *          cause and mapping it to a Matter BootReasonType.
 *
 *          Each supported NXP platform must provide a
 *          translation unit that implements ReadAndDetermineBootReason().
 *
 *          Typical usage inside ConfigurationManagerImpl::Init():
 *
 *          @code
 *          #include <platform/nxp/common/BootReason.h>
 *
 *          BootReasonType bootReason = BootReasonType::kUnspecified;
 *          CHIP_ERROR err = chip::DeviceLayer::NXP::ReadAndDetermineBootReason(bootReason);
 *          SuccessOrExit(err);
 *          SuccessOrExit(err = StoreBootReason(to_underlying(bootReason)));
 *          @endcode
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {
namespace NXP {

/**
 * @brief Read the hardware reset-cause register(s) and map the result to a
 *        Matter BootReasonType.
 *
 * This function is implemented once per NXP platform target.  It reads the
 * platform-specific reset-status register, translates the raw cause bits into
 * the closest Matter BootReasonType, and returns the result through
 * @p bootReason.
 *
 * @param[out] bootReason  On success, set to the Matter BootReasonType that
 *                         best describes why the device was last reset.
 *                         Initialised to BootReasonType::kUnspecified before
 *                         the platform-specific mapping is applied; the callee
 *                         may leave it at kUnspecified when the raw cause cannot
 *                         be matched to any known reason.
 *
 * @return CHIP_NO_ERROR on success, or a platform-specific CHIP_ERROR on
 *         failure (e.g. if the reset-cause register cannot be read).
 */
CHIP_ERROR ReadAndDetermineBootReason(app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason);

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
