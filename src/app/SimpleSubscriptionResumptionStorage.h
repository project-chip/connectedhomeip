/*
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#pragma once

#include <app/SubscriptionResumptionStorage.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {
namespace app {

/**
 * An example SubscriptionResumptionStorage using PersistentStorageDelegate as it backend.
 */
class SimpleSubscriptionResumptionStorage : public SubscriptionResumptionStorage
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    static StorageKeyName GetStorageKey(const ScopedNodeId & node);

    CHIP_ERROR LoadIndex(SubscriptionIndex & index) override;

    CHIP_ERROR FindByScopedNodeId(ScopedNodeId node, std::vector<SubscriptionInfo> & subscriptions) override;

    CHIP_ERROR Save(const SubscriptionInfo & subscriptionInfo) override;

    CHIP_ERROR Delete(const SubscriptionInfo & subscriptionInfo) override;

    CHIP_ERROR DeleteAll(FabricIndex fabricIndex) override;

private:
    CHIP_ERROR SaveIndex(const SubscriptionIndex & index);
    CHIP_ERROR SaveSubscriptions(const ScopedNodeId & node, const std::vector<SubscriptionInfo> & subscriptions);

    static constexpr size_t MaxScopedNodeIdSize() { return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex)); }

    static constexpr size_t MaxIndexSize()
    {
        // The max size of the list is (1 byte control + bytes for actual value) times max number of list items
        return TLV::EstimateStructOverhead((1 + MaxScopedNodeIdSize()) * CHIP_IM_MAX_NUM_SUBSCRIPTIONS);
    }

    static constexpr size_t MaxSubscriptionPathsSize()
    {
        // Not using CHIP_IM_MAX_NUM_PATH_PER_SUBSCRIPTION for the same
        // reason MaxStateSize() uses CHIP_IM_MAX_NUM_SUBSCRIPTIONS instead
        // of CHIP_IM_MAX_NUM_SUBSCRIPTIONS_PER_FABRIC
        return TLV::EstimateStructOverhead(
            TLV::EstimateStructOverhead(sizeof(uint8_t), sizeof(EndpointId), sizeof(ClusterId), sizeof(AttributeId)) *
            CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS);
    }

    static constexpr size_t MaxSubscriptionSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(MaxScopedNodeIdSize(), sizeof(SubscriptionId), sizeof(uint16_t), sizeof(uint16_t),
                                           sizeof(bool), MaxSubscriptionPathsSize());
    }

    static constexpr size_t MaxStateSize()
    {
        // Due to IM engine subscription eviction logic, effective allowed maximum
        // subscriptions per fabric is higher than CHIP_IM_MAX_NUM_SUBSCRIPTIONS_PER_FABRIC
        // when number of fabrics is fewer than maximum. And so max state size should use
        // CHIP_IM_MAX_NUM_SUBSCRIPTIONS to better estimate, and allow IM engine eviction
        // logic to trigger the right clean up when needed.

        // The max size of the list is (1 byte control + bytes for actual value) times max number of list items
        return TLV::EstimateStructOverhead(1 + MaxSubscriptionSize() * CHIP_IM_MAX_NUM_SUBSCRIPTIONS);
    }

    enum class SubscriptionPathType : uint8_t
    {
        kAttributePath      = 0x1,
        kUrgentEventPath    = 0x2,
        kNonUrgentEventPath = 0x3,
    };

    // TODO: consider alternate storage scheme to optimize space requirement

    // Nodes TLV structure:
    //   Array of:
    //     Scoped Node ID struct of:
    //       Node ID
    //       Fabric index

    // Subscription TLV structure:
    //   Array of:
    //     Struct of: (Subscription info)
    //       Node ID
    //       Subscription ID
    //       Min interval
    //       Max interval
    //       Fabric filtered boolean
    //       Path Count x, with these fields repeating x times
    //         Type (attribute, urgent event, non-urgent event)
    //         Endpoint ID
    //         Cluster ID
    //         Attribute/event ID
    static constexpr TLV::Tag kFabricIndexTag    = TLV::ContextTag(1);
    static constexpr TLV::Tag kPeerNodeIdTag     = TLV::ContextTag(2);
    static constexpr TLV::Tag kSubscriptionIdTag = TLV::ContextTag(3);
    static constexpr TLV::Tag kMinIntervalTag    = TLV::ContextTag(4);
    static constexpr TLV::Tag kMaxIntervalTag    = TLV::ContextTag(5);
    static constexpr TLV::Tag kFabricFilteredTag = TLV::ContextTag(6);
    static constexpr TLV::Tag kPathCountTag      = TLV::ContextTag(7);
    static constexpr TLV::Tag kPathTypeTag       = TLV::ContextTag(8);
    static constexpr TLV::Tag kEndpointIdTag     = TLV::ContextTag(9);
    static constexpr TLV::Tag kClusterIdTag      = TLV::ContextTag(10);
    static constexpr TLV::Tag kAttributeIdTag    = TLV::ContextTag(11);
    static constexpr TLV::Tag kEventIdTag        = TLV::ContextTag(12);

    PersistentStorageDelegate * mStorage;
};
} // namespace app
} // namespace chip
