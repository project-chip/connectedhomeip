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
 *          RT1060 implementation of ReadAndDetermineBootReason().
 *
 *          Reset-cause flags are provided by the i.MX RT1060 SRC (System Reset
 *          Controller) driver (fsl_src.h).  The mapping to Matter BootReasonType
 *          is:
 *
 *          | Hardware cause               | Matter BootReasonType            |
 *          |------------------------------|----------------------------------|
 *          | kSRC_IppUserResetFlag        | kPowerOnReboot                   |
 *          | kSRC_IppResetPinFlag         | kPowerOnReboot                   |
 *          | kSRC_WatchdogResetFlag       | kHardwareWatchdogReset           |
 *          | kSRC_Wdog3ResetFlag          | kHardwareWatchdogReset           |
 *          | OTA candidate detected       | kSoftwareUpdateCompleted         |
 *          | anything else                | kUnspecified                     |
 *
 *          Note: kBrownOutReset is not exposed by the RT1060 SRC and therefore
 *          cannot be detected.
 */

#include <platform/nxp/common/BootReason.h>

#include "fsl_src.h"

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

    uint32_t resetFlags = SRC_GetResetStatusFlags(SRC);
    SRC_ClearResetStatusFlags(SRC, resetFlags);

    if ((resetFlags & kSRC_IppUserResetFlag) || (resetFlags & kSRC_IppResetPinFlag))
    {
        bootReason = BootReasonEnum::kPowerOnReboot;
    }
    else if ((resetFlags & kSRC_LockupSysResetFlag) || (resetFlags & kSRC_JTAGSoftwareResetFlag))
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
    else if ((resetFlags & kSRC_WatchdogResetFlag) || (resetFlags & kSRC_Wdog3ResetFlag))
    {
        bootReason = BootReasonEnum::kHardwareWatchdogReset;
    }

    return CHIP_NO_ERROR;
}

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
