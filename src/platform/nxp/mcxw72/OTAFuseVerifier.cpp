/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OTAFuseVerifier.h"
#include <fsl_common.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

extern "C" {
#include "fsl_device_registers.h"
#include "fsl_sscp_mu.h"
#include "fsl_sss_api.h"
#include "fsl_sss_mgmt.h"
#include "fsl_sss_sscp.h"
}

#define FUSE_LEN (32u) // 32 bytes - 256 bits for fuse data
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)

namespace chip {
namespace DeviceLayer {
namespace NXP {

status_t OTAFuseVerifier::read_ota_encryption_key_fuse(ELEMU_Type * mu, uint32_t fuse, uint32_t * fuseData, size_t fuseLen)
{
    sscp_context_t sscpContext    = { 0u };
    sss_mgmt_t mgmtContext        = { 0u };
    sss_sscp_session_t sssSession = { 0u };
    status_t status               = kStatus_Fail;

    /* Core Freq in MHz */
    mgmtContext.clockFrequency = CORE_CLK_FREQ / 1000000U;
    size_t optLen              = 4u;

    do
    {
        if (sscp_mu_init(&sscpContext, mu) != kStatus_SSCP_Success)
        {
            break;
        }
        /* OPEN SESSION */
        if (sss_sscp_open_session(&sssSession, 1u, kType_SSS_Ele200, &sscpContext) != kStatus_SSS_Success)
        {
            break;
        }
        if (sss_mgmt_context_init(&mgmtContext, &sssSession) != kStatus_SSS_Success)
        {
            break;
        }

        /* READ FUSE */
        sss_mgmt_fuse_read(&mgmtContext, fuse, fuseData, &fuseLen, (uintptr_t) &mgmtContext.clockFrequency, &optLen);

        if (sss_mgmt_context_free(&mgmtContext) != kStatus_SSS_Success)
        {
            break;
        }
        /* If all steps before passes without break, then consider it as success*/
        status = kStatus_Success;

    } while (false);

    return status;
}

CHIP_ERROR OTAFuseVerifier::VerifyOTAFusesReady()
{
    alignas(uint32_t) uint8_t fuseData[FUSE_LEN];

    status_t status = read_ota_encryption_key_fuse(ELEMUA, kFuseId_OEMFW_AUTH_PUK, (uint32_t *) fuseData, FUSE_LEN);

    if (status != kStatus_Success)
    {
        ChipLogError(DeviceLayer, "Failed to read OTA fuses, status: 0x%08X", static_cast<unsigned>(status));
        return CHIP_ERROR_INTERNAL;
    }
    ChipLogProgress(DeviceLayer, "OTA fuse read successfully, verifying value...");

    // Verify the fuse value is not ` zeros (unprogrammed)
    bool fusedKeyEmpty = true;
    for (size_t i = 0; i < FUSE_LEN; i++)
    {
        if (fuseData[i] != 0)
        {
            fusedKeyEmpty = false;
            break;
        }
    }

    if (fusedKeyEmpty)
    {
        ChipLogError(DeviceLayer, "OTA encryption key fuse is not programmed (all zeros)");
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }

    ChipLogProgress(DeviceLayer, "OTA fuse value verified successfully");
    return CHIP_NO_ERROR;
}

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
