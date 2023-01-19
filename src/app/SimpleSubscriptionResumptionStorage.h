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
 *      This file defines a basic implementation of SubscriptionResumptionStorage that
 *      persists subscriptions in a flat list in TLV.
 */

#pragma once

#include <app/SubscriptionResumptionStorage.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Pool.h>

namespace chip {
namespace app {

/**
 * An example SubscriptionResumptionStorage using PersistentStorageDelegate as it backend.
 */
class SimpleSubscriptionResumptionStorage : public SubscriptionResumptionStorage
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    SubscriptionInfoIterator * IterateSubscriptions() override;

    CHIP_ERROR Save(SubscriptionInfo & subscriptionInfo) override;

    CHIP_ERROR Delete(NodeId nodeId, FabricIndex fabricIndex, SubscriptionId subscriptionId) override;

    CHIP_ERROR DeleteAll(FabricIndex fabricIndex) override;

protected:
    CHIP_ERROR Save(TLV::TLVWriter & writer, SubscriptionInfo & subscriptionInfo);
    CHIP_ERROR Load(size_t subscriptionIndex, SubscriptionInfo & subscriptionInfo);
    CHIP_ERROR Delete(size_t subscriptionIndex);
    size_t Count();
    size_t MaxCount();
    CHIP_ERROR DeleteMaxCount();

    class SimpleSubscriptionInfoIterator : public SubscriptionInfoIterator
    {
    public:
        SimpleSubscriptionInfoIterator(SimpleSubscriptionResumptionStorage & storage);
        size_t Count() override;
        bool Next(SubscriptionInfo & output) override;
        void Release() override;

    private:
        SimpleSubscriptionResumptionStorage & mStorage;
        uint16_t mNextIndex;
        size_t mMaxCount;
    };

    static constexpr size_t MaxScopedNodeIdSize() { return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex)); }

    static constexpr size_t MaxSubscriptionPathsSize()
    {
        // IM engine declares an attribute path pool and an event path pool, and each pool
        // includes CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS for subscriptions
        return 2 *
            TLV::EstimateStructOverhead(
                   TLV::EstimateStructOverhead(sizeof(uint8_t), sizeof(EndpointId), sizeof(ClusterId), sizeof(AttributeId)) *
                   CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS);
    }

    static constexpr size_t MaxSubscriptionSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(MaxScopedNodeIdSize(), sizeof(SubscriptionId), sizeof(uint16_t), sizeof(uint16_t),
                                           sizeof(bool), MaxSubscriptionPathsSize());
    }

    enum class EventPathType : uint8_t
    {
        kUrgent    = 0x1,
        kNonUrgent = 0x2,
    };

    // TODO: consider alternate storage scheme to optimize space requirement

    // Flat list of subscriptions indexed from from 0 to CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS-1
    //
    // Each entry in list is a Subscription TLV structure:
    //   Struct of:
    //     Node ID
    //     Fabric Index
    //     Subscription ID
    //     Min interval
    //     Max interval
    //     Fabric filtered boolean
    //     Attribute Path Count x, with these fields repeating x times
    //       Endpoint ID
    //       Cluster ID
    //       Attribute/event ID
    //     Event Path Count x, with these fields repeating x times
    //       Event subscription type (urgent / non-urgent)
    //       Endpoint ID
    //       Cluster ID
    //       Event ID

    static constexpr TLV::Tag kPeerNodeIdTag     = TLV::ContextTag(2);
    static constexpr TLV::Tag kFabricIndexTag    = TLV::ContextTag(1);
    static constexpr TLV::Tag kSubscriptionIdTag = TLV::ContextTag(3);
    static constexpr TLV::Tag kMinIntervalTag    = TLV::ContextTag(4);
    static constexpr TLV::Tag kMaxIntervalTag    = TLV::ContextTag(5);
    static constexpr TLV::Tag kFabricFilteredTag = TLV::ContextTag(6);
    static constexpr TLV::Tag kPathCountTag      = TLV::ContextTag(7);
    static constexpr TLV::Tag kEventPathTypeTag  = TLV::ContextTag(8);
    static constexpr TLV::Tag kEndpointIdTag     = TLV::ContextTag(9);
    static constexpr TLV::Tag kClusterIdTag      = TLV::ContextTag(10);
    static constexpr TLV::Tag kAttributeIdTag    = TLV::ContextTag(11);
    static constexpr TLV::Tag kEventIdTag        = TLV::ContextTag(12);

    PersistentStorageDelegate * mStorage;
    ObjectPool<SimpleSubscriptionInfoIterator, kIteratorsMax> mSubscriptionInfoIterators;
};
} // namespace app
} // namespace chip
