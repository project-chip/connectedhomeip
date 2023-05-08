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
    // Structs to hold path param values as is_trivial struct
    struct AttributePathParamsValues
    {
        ClusterId mClusterId;
        AttributeId mAttributeId;
        EndpointId mEndpointId;
        void SetValues(const AttributePathParams & params)
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
        void SetValues(const EventPathParams & params)
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
        CHIP_ERROR SetAttributePaths(const ObjectList<AttributePathParams> * pAttributePathList)
        {
            mAttributePaths.Free();
            if (!pAttributePathList)
            {
                return CHIP_NO_ERROR;
            }
            const ObjectList<AttributePathParams> * attributePath = pAttributePathList;
            size_t attributePathCount                             = 0;
            while (attributePath)
            {
                attributePathCount++;
                attributePath = attributePath->mpNext;
            }
            ReturnErrorCodeIf((attributePathCount * sizeof(AttributePathParamsValues)) > UINT16_MAX, CHIP_ERROR_NO_MEMORY);
            mAttributePaths.Calloc(attributePathCount);
            ReturnErrorCodeIf(mAttributePaths.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
            attributePath = pAttributePathList;
            for (size_t i = 0; i < attributePathCount; i++)
            {
                mAttributePaths[i].SetValues(attributePath->mValue);
                attributePath = attributePath->mpNext;
            }
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR SetEventPaths(const ObjectList<EventPathParams> * pEventPathList)
        {
            mEventPaths.Free();
            if (!pEventPathList)
            {
                return CHIP_NO_ERROR;
            }
            const ObjectList<EventPathParams> * eventPath = pEventPathList;
            size_t eventPathCount                         = 0;
            while (eventPath)
            {
                eventPathCount++;
                eventPath = eventPath->mpNext;
            }
            ReturnErrorCodeIf((eventPathCount * sizeof(EventPathParamsValues)) > UINT16_MAX, CHIP_ERROR_NO_MEMORY);
            mEventPaths.Calloc(eventPathCount);
            ReturnErrorCodeIf(mEventPaths.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
            eventPath = pEventPathList;
            for (size_t i = 0; i < eventPathCount; i++)
            {
                mEventPaths[i].SetValues(eventPath->mValue);
                eventPath = eventPath->mpNext;
            }
            return CHIP_NO_ERROR;
        }
    };

    using SubscriptionInfoIterator = CommonIterator<SubscriptionInfo>;

    virtual ~SubscriptionResumptionStorage(){};

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
     */
    virtual CHIP_ERROR Save(SubscriptionInfo & subscriptionInfo) = 0;

    /**
     * Delete subscription resumption information by node ID, fabric index, and subscription ID.
     */
    virtual CHIP_ERROR Delete(NodeId nodeId, FabricIndex fabricIndex, SubscriptionId subscriptionId) = 0;

    /**
     * Remove all subscription resumption information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     *
     * @param fabricIndex the index of the fabric for which to remove subscription resumption information
     */
    virtual CHIP_ERROR DeleteAll(FabricIndex fabricIndex) = 0;
};
} // namespace app
} // namespace chip
