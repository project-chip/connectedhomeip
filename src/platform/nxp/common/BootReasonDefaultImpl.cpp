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
 *          Default implementation of ReadAndDetermineBootReason().
 *
 *          This translation unit is compiled for every NXP platform and
 *          provides a fallback that simply reports BootReasonType::kUnspecified.
 *          Platforms that can determine the actual hardware reset cause
 *          (rw61x, rt1060, mcxw72, …) must supply their own implementation
 *          of ReadAndDetermineBootReason().
 */

#include <platform/nxp/common/BootReason.h>

namespace chip {
namespace DeviceLayer {
namespace NXP {

/**
 * Returns BootReasonEnum::kUnspecified.
 *
 * Platforms that cannot determine the actual reset cause at run-time
 * (or have not yet provided a platform-specific implementation) will
 * silently report kUnspecified rather than failing the boot sequence.
 */
CHIP_ERROR ReadAndDetermineBootReason(app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason)
{
    bootReason = app::Clusters::GeneralDiagnostics::BootReasonEnum::kUnspecified;
    return CHIP_NO_ERROR;
}

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
