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
 *          MCXW72 implementation of ReadAndDetermineBootReason().
 *
 *          Reset-cause bits are provided by the MCXW72 CMC (Core Mode Controller)
 *          driver (fsl_cmc.h).  The mapping to Matter BootReasonType is:
 *
 *          | Hardware cause            | Matter BootReasonType            |
 *          |---------------------------|----------------------------------|
 *          | CMC_SRS_POR_MASK or       |                                  |
 *          |   CMC_SRS_PIN_MASK        | kPowerOnReboot                   |
 *          | CMC_SRS_SW_MASK (no OTA)  | kSoftwareReset                   |
 *          | CMC_SRS_SW_MASK (OTA done)| kSoftwareUpdateCompleted         |
 *          | CMC_SRS_WDOG0_MASK or     |                                  |
 *          |   CMC_SRS_WDOG1_MASK      | kSoftwareWatchdogReset           |
 *          | anything else             | kUnspecified                     |
 *
 *          OTA completion is tracked via the kConfigKey_AppOTADone NVM flag
 *          written by ConfigurationManagerImpl::StoreSoftwareUpdateCompleted().
 *          When the flag is found set, it is immediately cleared so that a
 *          subsequent software reset is reported as kSoftwareReset again.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/common/BootReason.h>

#include "NXPConfig.h"
#include "fsl_cmc.h"

namespace chip {
namespace DeviceLayer {
namespace NXP {

using BootReasonEnum = app::Clusters::GeneralDiagnostics::BootReasonEnum;
using namespace ::chip::DeviceLayer::Internal;

CHIP_ERROR ReadAndDetermineBootReason(BootReasonEnum & bootReason)
{
    bootReason      = BootReasonEnum::kUnspecified;
    uint32_t reason = CMC_GetSystemResetStatus(CMC0);

    if ((reason & CMC_SRS_POR_MASK) || (reason & CMC_SRS_PIN_MASK))
    {
        bootReason = BootReasonEnum::kPowerOnReboot;
    }
    else if (reason & CMC_SRS_SW_MASK)
    {
        bootReason = BootReasonEnum::kSoftwareReset;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        bool otaDone   = false;
        CHIP_ERROR err = NXPConfig::ReadConfigValue(NXPConfig::kConfigKey_AppOTADone, otaDone);

        if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        {
            ReturnErrorAndLogOnFailure(err, DeviceLayer, "Failed to read OTA completion flag");
        }

        if (otaDone)
        {
            bootReason = BootReasonEnum::kSoftwareUpdateCompleted;
            ReturnErrorAndLogOnFailure(NXPConfig::WriteConfigValue(NXPConfig::kConfigKey_AppOTADone, false), DeviceLayer,
                                       "Failed to clear OTA completion flag");
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    }
    else if ((reason & CMC_SRS_WDOG0_MASK) || (reason & CMC_SRS_WDOG1_MASK))
    {
        bootReason = BootReasonEnum::kSoftwareWatchdogReset;
    }

    return CHIP_NO_ERROR;
}

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
