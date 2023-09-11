/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

#include "FreeRTOS.h"
#include <lwip/tcpip.h>

extern "C" {
#include <bl_sys.h>
}

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    BL_RST_REASON_E bootCause = bl_sys_rstinfo_get();

    if (BL_RST_POR == bootCause)
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (BL_RST_BOR == bootCause)
    {
        bootReason = BootReasonType::kBrownOutReset;
    }
    else if (BL_RST_WDT == bootCause)
    {
        bootReason = BootReasonType::kHardwareWatchdogReset;
    }
    else if (BL_RST_SOFTWARE == bootCause)
    {
        bootReason = BootReasonType::kSoftwareReset;
    }
    else
    {
        bootReason = BootReasonType::kUnspecified;
    }
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
