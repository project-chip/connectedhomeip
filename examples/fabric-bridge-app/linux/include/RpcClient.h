/*
 *
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
#include <platform/CHIPDeviceLayer.h>

constexpr uint16_t kFabricAdminServerPort = 33001;

/**
 * Initializes the RPC client by setting the server port and starting packet processing.
 *
 * @param rpcServerPort The port number of the RPC server.
 * @return CHIP_ERROR An error code indicating the success or failure of the initialization process.
 * - CHIP_NO_ERROR: Initialization was successful.
 * - Other error codes indicating specific failure reasons.
 */
CHIP_ERROR InitRpcClient(uint16_t rpcServerPort);

/**
 * Opens a commissioning window for a specified node using setup PIN (passcode).
 *
 * @param params    Params for opening the commissioning window using passcode.
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully processed.
 * - CHIP_ERROR_BUSY: Another commissioning window is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred.
 */
CHIP_ERROR
OpenCommissioningWindow(chip::Controller::CommissioningWindowPasscodeParams params);

/**
 * Opens a commissioning window for a specified node using pre-computed PAKE passcode verifier.
 *
 * @param params    Params for opening the commissioning window using verifier.
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully sent.
 * - CHIP_ERROR_BUSY: Another commissioning window is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred.
 */
CHIP_ERROR
OpenCommissioningWindow(chip::Controller::CommissioningWindowVerifierParams params);

CHIP_ERROR KeepActive(chip::NodeId nodeId, uint32_t stayActiveDurationMs);
