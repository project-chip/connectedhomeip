/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      This file defines the interface to store subscription information.
 */

#pragma once

#include <app/ReadClient.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {

/**
 * The SubscriptionResumptionStorage interface is used to persist subscriptions when they are established.
 */
class SubscriptionResumptionStorage
{
public:
    // Structs to hold
    struct AttributePathParamsValues
    {
        ClusterId mClusterId;
        AttributeId mAttributeId;
        EndpointId mEndpointId;
        void SetValues(AttributePathParams & params)
        {
            mEndpointId  = params.mEndpointId;
            mClusterId   = params.mClusterId;
            mAttributeId = params.mAttributeId;
        }
        AttributePathParams GetParams() { return AttributePathParams(mEndpointId, mClusterId, mAttributeId); }
    };
    struct EventPathParamsValues
    {
        ClusterId mClusterId;
        EventId mEventId;
        EndpointId mEndpointId;
        bool mIsUrgentEvent;
        void SetValues(EventPathParams & params)
        {
            mEndpointId    = params.mEndpointId;
            mClusterId     = params.mClusterId;
            mEventId       = params.mEventId;
            mIsUrgentEvent = params.mIsUrgentEvent;
        }
        EventPathParams GetParams() { return EventPathParams(mEndpointId, mClusterId, mEventId, mIsUrgentEvent); }
    };

    /**
     * Struct to hold information about subscriptions
     */
    struct SubscriptionInfo
    {
        NodeId mNodeId;
        FabricIndex mFabricIndex;
        SubscriptionId mSubscriptionId;
        uint16_t mMinInterval;
        uint16_t mMaxInterval;
        bool mFabricFiltered;
        Platform::ScopedMemoryBufferWithSize<AttributePathParamsValues> mAttributePaths;
        Platform::ScopedMemoryBufferWithSize<EventPathParamsValues> mEventPaths;
    };

    using SubscriptionInfoIterator = CommonIterator<SubscriptionInfo>;

    virtual ~SubscriptionResumptionStorage(){};

    /**
     * Recover fabric-scoped node identities of persisted subscribers.
     *
     * @param subscriberIndex the nodes for previously persisted subscribers
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_KEY_NOT_FOUND if no subscription resumption information can be found, else an
     * appropriate CHIP error on failure
     */
    //    virtual CHIP_ERROR LoadIndex(SubscriptionIndex & subscriberIndex) = 0;

    /**
     * Recover subscription resumption info for a given fabric-scoped node identity.
     *
     * @param node the node for which to recover subscription resumption information
     * @param subscriptions (out) recovered subscriptions info
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_KEY_NOT_FOUND if no subscription resumption information can be found, else an
     * appropriate CHIP error on failure
     */
    //    virtual CHIP_ERROR FindByScopedNodeId(ScopedNodeId node, SubscriptionList & subscriptions) = 0;

    /**
     * Iterate through persisted subscriptions
     *
     * @return A valid iterator on success. Use CommonIterator accessor to retrieve SubscriptionInfo
     */
    virtual SubscriptionInfoIterator * IterateSubscriptions() = 0;

    /**
     * Save subscription resumption information to storage.
     *
     * @param subscriptionInfo the subscription information to save - caller should expect the passed in value is consumed
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR Save(SubscriptionInfo & subscriptionInfo) = 0;

    /**
     * Save subscription resumption information to storage.
     *
     * @param subscriptionInfo the subscription information to delete - only node and subscriptionId will be used to find the
     * subscription
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    //    virtual CHIP_ERROR Delete(const SubscriptionInfo & subscriptionInfo) = 0;

    /**
     * Delete subscription resumption information by node ID, fabric index, and subscription ID.
     *
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR Delete(NodeId nodeId, FabricIndex fabricIndex, SubscriptionId subscriptionId) = 0;

    /**
     * Remove all subscription resumption information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     *
     * @param fabricIndex the index of the fabric for which to remove subscription resumption information
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR DeleteAll(FabricIndex fabricIndex) = 0;
};
} // namespace app
} // namespace chip
