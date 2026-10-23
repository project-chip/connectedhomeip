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

#pragma once

#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

#include "fsl_common.h"
#include "fsl_device_registers.h"

namespace chip {
namespace DeviceLayer {
namespace NXP {

/**
 * @brief Verifies that hardware fuses required for encrypted OTA are programmed.
 *
 * The bootloader requires the following fuse to decrypt and authenticate OTA images:
 * - CUST_PROD_OEMFW_AUTH_PUK authentication key (NBOOT_FUSEID_CUST_PROD_OEMFW_AUTH_PUK)
 */
class OTAFuseVerifier
{
public:
    // Fuse IDs from fsl_nboot.h
    static constexpr uint32_t kFuseId_OEMFW_AUTH_PUK = 0x1F; // NBOOT_FUSEID_CUST_PROD_OEMFW_AUTH_PUK

    /**
     * @brief Verifies that the OTA encryption key is programmed and valid.
     *
     * The OTA encryption keys are a requirement for secure OTA updates. This function verifies that
     * the CUST_PROD_OEMFW_AUTH_PUK fuse (the authentication key) has been programmed with valid data.
     *
     * @returns CHIP_NO_ERROR if the OTA fuses are properly programmed, CHIP_ERROR_NOT_IMPLEMENTED otherwise.
     *
     * @warning This function should only be called after the device has completed secure subsystem initialization.
     *
     */
    static CHIP_ERROR VerifyOTAFusesReady();

private:
    /**
     * @brief Reads OTA encryption key fuse from secure subsystem.
     *
     * Reading the OTA encryption key fuse requires establishing an SSCP session with the secure
     * subsystem and issuing a fuse read command. The fuse data is returned in the provided buffer.
     *
     * @param[in] mu Pointer to the MU (Message Unit) peripheral for SSCP communication.
     * @param[in] fuse The fuse ID to read (kFuseId_OEMFW_AUTH_PUK).
     * @param[out] fuseData Pointer to buffer where fuse data will be stored.
     * @param[in] fuseLen Length of the fuse data buffer in bytes.
     *
     * @returns kStatus_Success if the fuse was read successfully, kStatus_Fail otherwise.
     *
     */
    static status_t read_ota_encryption_key_fuse(ELEMU_Type * mu, uint32_t fuse, uint32_t * fuseData, size_t fuseLen);
};

} // namespace NXP
} // namespace DeviceLayer
} // namespace chip
