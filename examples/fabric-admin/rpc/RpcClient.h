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

#include <lib/core/ScopedNodeId.h>
#include <platform/CHIPDeviceLayer.h>

#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"

/**
 * @brief Sets the RPC server port to which the RPC client will connect.
 *
 * @param port The port number.
 */
void SetRpcRemoteServerPort(uint16_t port);

/**
 * @brief Starts packet processing for the RPC client.
 *
 * @return CHIP_NO_ERROR on successful start, or an appropriate CHIP_ERROR on failure.
 */
CHIP_ERROR StartRpcClient();

/**
 * @brief Adds a synchronized device to the RPC client.
 *
 * This function attempts to add a device identified by its `nodeId` to the synchronized device list.
 * It logs the progress and checks if an `AddSynchronizedDevice` operation is already in progress.
 * If an operation is in progress, it returns `CHIP_ERROR_BUSY`.
 *
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully processed.
 * - CHIP_ERROR_BUSY: Another operation is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred while activating the RPC call.
 */
CHIP_ERROR AddSynchronizedDevice(const chip_rpc_SynchronizedDevice & data);

/**
 * @brief Removes a synchronized device from the RPC client.
 *
 * This function attempts to remove a device identified by its `nodeId` from the synchronized device list.
 * It logs the progress and checks if a `RemoveSynchronizedDevice` operation is already in progress.
 * If an operation is in progress, it returns `CHIP_ERROR_BUSY`.
 *
 * @param scopedNodeId The Scoped Node ID of the device to be removed.
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully processed.
 * - CHIP_ERROR_BUSY: Another operation is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred while activating the RPC call.
 */
CHIP_ERROR RemoveSynchronizedDevice(chip::ScopedNodeId scopedNodeId);

/**
 * @brief Received StayActiveResponse on behalf of client that previously called KeepActive
 *
 * @param scopedNodeId The Scoped Node ID of the device we recieved a StayActiveResponse.
 * @param promisedActiveDurationMs the computed duration (in milliseconds) that the ICD intends to stay active for.
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully processed.
 * - CHIP_ERROR_BUSY: Another operation is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred while activating the RPC call.
 */
CHIP_ERROR ActiveChanged(chip::ScopedNodeId scopedNodeId, uint32_t promisedActiveDurationMs);

/**
 * @brief CADMIN attribute has changed of one of the bridged devices that was previously added.
 *
 * @param data information regarding change in AdministratorCommissioning attributes
 * @return CHIP_ERROR An error code indicating the success or failure of the operation.
 * - CHIP_NO_ERROR: The RPC command was successfully processed.
 * - CHIP_ERROR_BUSY: Another operation is currently in progress.
 * - CHIP_ERROR_INTERNAL: An internal error occurred while activating the RPC call.
 */
CHIP_ERROR AdminCommissioningAttributeChanged(const chip_rpc_AdministratorCommissioningChanged & data);
