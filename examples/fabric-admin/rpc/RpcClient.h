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

#include <platform/CHIPDeviceLayer.h>

constexpr uint16_t kFabricBridgeServerPort = 33002;

/**
 * @brief Initializes the RPC client with the specified server port.
 *
 * This function sets the RPC server port and starts packet processing for the RPC client.
 *
 * @param rpcServerPort The port number on which the RPC server is running.
 * @return CHIP_NO_ERROR on successful initialization, or an appropriate CHIP_ERROR on failure.
 */
CHIP_ERROR InitRpcClient(uint16_t rpcServerPort);

/**
 * @brief Adds a synchronized device to the RPC client.
 *
 * This function attempts to add a device identified by its `nodeId` to the synchronized device list.
 * It logs the progress and checks if an `OpenCommissioningWindow` operation is already in progress.
 * If an operation is in progress, it returns `CHIP_ERROR_BUSY`.
 *
 * @param nodeId The Node ID of the device to be added.
 * @return CHIP_NO_ERROR on success, `CHIP_ERROR_BUSY` if an operation is already in progress,
 *         or `CHIP_ERROR_INTERNAL` if there is an internal error while activating the RPC call.
 */
CHIP_ERROR AddSynchronizedDevice(chip::NodeId nodeId);
