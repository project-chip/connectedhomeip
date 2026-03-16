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

#include <controller/CommissioningWindowParams.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <optional>
#include <string>

namespace bridge {

class FabricAdminDelegate
{
public:
    virtual ~FabricAdminDelegate() = default;

    /**
     * Opens a commissioning window for a specified node using pre-computed PAKE passcode verifier.
     *
     * @param params    Params for opening the commissioning window using verifier.
     * @return CHIP_ERROR An error code indicating the success or failure of the operation.
     * - CHIP_NO_ERROR: The RPC command was successfully sent.
     * - CHIP_ERROR_BUSY: Another commissioning window is currently in progress.
     * - CHIP_ERROR_INTERNAL: An internal error occurred.
     */
    virtual CHIP_ERROR OpenCommissioningWindow(chip::Controller::CommissioningWindowVerifierParams params,
                                               chip::FabricIndex fabricIndex) = 0;

    /**
     * Reverse commission a bridge using the specified parameters.
     *
     * This function initiates the commissioning process for a bridge node, utilizing
     * the provided passcode parameters, vendor ID, and product ID.
     *
     * @param params    Parameters required for commissioning the device using passcode.
     * @param vendorId  The Vendor ID (VID) of the device being commissioned. This identifies
     *                  the manufacturer of the device.
     * @param productId The Product ID (PID) of the device being commissioned. This identifies
     *                  the specific product within the vendor's lineup.
     *
     * @return CHIP_ERROR An error code indicating the success or failure of the operation.
     * - CHIP_NO_ERROR: The RPC command was successfully sent and the commissioning process was initiated.
     * - CHIP_ERROR_INTERNAL: An internal error occurred during the preparation or sending of the command.
     */
    virtual CHIP_ERROR CommissionRemoteBridge(chip::Controller::CommissioningWindowPasscodeParams params, chip::VendorId vendorId,
                                              uint16_t productId) = 0;

    /**
     * Sends a request to keep a device active for a specified duration.
     *
     * @param ScopedNodeId          Identifies the target node within the fabric.
     * @param StayActiveDurationMs  The duration for which the device should stay active.
     * @param TimeoutMs             Timeout for the operation.
     *
     * @return CHIP_ERROR An error code indicating the success or failure of the
     * operation.
     * - CHIP_NO_ERROR: Request processed successfully.
     * - CHIP_ERROR_INTERNAL: An internal error occurred during the preparation or sending
     * of the command.
     */
    virtual CHIP_ERROR KeepActive(chip::ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs) = 0;
};

} // namespace bridge
