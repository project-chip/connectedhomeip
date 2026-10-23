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
 *          RW61x implementation of ReadAndDetermineBootReason().
 *
 *          Reset-cause bits are provided by the RW61x Power Management driver
 *          (fsl_power.h).  The mapping to Matter BootReasonType is:
 *
 *          | Hardware cause            | Matter BootReasonType           |
 *          |---------------------------|---------------------------------|
 *          | rebootCause == 0          | kPowerOnReboot                  |
 *          | kPOWER_ResetCauseWdt      | kHardwareWatchdogReset          |
 *          | kPOWER_ResetCauseSysResetReq (no OTA pending) | kSoftwareReset |
 *          | kPOWER_ResetCauseSysResetReq (OTA candidate)  | kSoftwareUpdateCompleted |
 *          | anything else             | kUnspecified                    |
 */

#include <platform/nxp/common/BootReason.h>

#include "fsl_power.h"
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OtaSupport.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace NXP {

using BootReasonEnum = app::Clusters::GeneralDiagnostics::BootReasonEnum;

CHIP_ERROR ReadAndDetermineBootReason(BootReasonEnum & bootReason)
{
    bootReason = BootReasonEnum::kUnspecified;

    uint32_t rebootCause = POWER_GetResetCause();
    POWER_ClearResetCause(rebootCause);

    /*
     * With the current RW61x implementation kBrownOutReset cannot be caught
     * because the PMU does not expose a dedicated brown-out flag via fsl_power.
     */
    if (rebootCause == 0)
    {
        bootReason = BootReasonEnum::kPowerOnReboot;
    }
    else if (rebootCause == kPOWER_ResetCauseWdt)
    {
        /* Reboot can be due to hardware or software watchdog */
        bootReason = BootReasonEnum::kHardwareWatchdogReset;
    }
    else if (rebootCause == kPOWER_ResetCauseSysResetReq)
    {
        bootReason = BootReasonEnum::kSoftwareReset;
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        OtaImgState_t imgState = OTA_GetImgState();
        if (imgState == OtaImgState_RunCandidate)
        {
            bootReason = BootReasonEnum::kSoftwareUpdateCompleted;
        }
#endif
    }

    return CHIP_NO_ERROR;
}

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
