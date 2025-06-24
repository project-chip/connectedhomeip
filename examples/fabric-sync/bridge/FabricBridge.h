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

#include "FabricAdminDelegate.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <optional>
#include <string>

namespace bridge {

#define ScopedNodeId_init_default chip::ScopedNodeId()
#define SynchronizedDevice_init_default                                                                                            \
    {                                                                                                                              \
        ScopedNodeId_init_default, /* id */                                                                                        \
            std::nullopt,          /* uniqueId */                                                                                  \
            std::nullopt,          /* vendorName */                                                                                \
            std::nullopt,          /* vendorId */                                                                                  \
            std::nullopt,          /* productName */                                                                               \
            std::nullopt,          /* productId */                                                                                 \
            std::nullopt,          /* nodeLabel */                                                                                 \
            std::nullopt,          /* hardwareVersion */                                                                           \
            std::nullopt,          /* hardwareVersionString */                                                                     \
            std::nullopt,          /* softwareVersion */                                                                           \
            std::nullopt,          /* softwareVersionString */                                                                     \
            std::nullopt           /* isIcd */                                                                                     \
    }

#define AdministratorCommissioningChanged_init_default                                                                             \
    {                                                                                                                              \
        ScopedNodeId_init_default,                                                                          /* id */               \
            chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen, /* windowStatus */     \
            std::nullopt, /* openerFabricIndex */                                                                                  \
            std::nullopt  /* openerVendorId */                                                                                     \
    }

struct SynchronizedDevice
{
    chip::ScopedNodeId id;

    std::optional<std::string> uniqueId;
    std::optional<std::string> vendorName;
    std::optional<chip::VendorId> vendorId;
    std::optional<std::string> productName;
    std::optional<uint16_t> productId;
    std::optional<std::string> nodeLabel;
    std::optional<uint16_t> hardwareVersion;
    std::optional<std::string> hardwareVersionString;
    std::optional<uint32_t> softwareVersion;
    std::optional<std::string> softwareVersionString;
    std::optional<bool> isIcd;
};

struct AdministratorCommissioningChanged
{
    chip::ScopedNodeId id;
    chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum windowStatus;
    std::optional<chip::FabricIndex> openerFabricIndex;
    std::optional<chip::VendorId> openerVendorId;
};

class FabricBridge final
{
public:
    static FabricBridge & Instance();

    void SetDelegate(FabricAdminDelegate * delegate) { mFabricAdmin = delegate; }
    FabricAdminDelegate * GetDelegate() { return mFabricAdmin; }

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
    CHIP_ERROR AddSynchronizedDevice(const SynchronizedDevice & data);

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
    CHIP_ERROR AdminCommissioningAttributeChanged(const AdministratorCommissioningChanged & data);

    /**
     * @brief Notify the system that the reachability status of a bridged device has changed.
     *
     * @param scopedNodeId Identifier of the bridged device whose reachability has changed.
     * @param reachability Boolean indicating the new reachability status of the device.
     *                      - `true`: Device is reachable.
     *                      - `false`: Device is not reachable.
     *
     * @return CHIP_ERROR Error code representing the outcome of the operation.
     * - CHIP_NO_ERROR: The operation was successful.
     * - CHIP_ERROR_BUSY: Another operation is currently in progress, preventing this action.
     * - CHIP_ERROR_INTERNAL: An internal error occurred while processing the reachability change.
     */
    CHIP_ERROR DeviceReachableChanged(chip::ScopedNodeId scopedNodeId, bool reachability);

private:
    static FabricBridge sInstance;

    FabricAdminDelegate * mFabricAdmin;
};

} // namespace bridge
