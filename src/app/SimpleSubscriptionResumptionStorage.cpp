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

#include <app/SimpleSubscriptionResumptionStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kFabricIndexTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kPeerNodeIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kSubscriptionIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kMinIntervalTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kMaxIntervalTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kFabricFilteredTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kPathCountTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kPathTypeTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEndpointIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kClusterIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kAttributeIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEventIdTag;

CHIP_ERROR SimpleSubscriptionResumptionStorage::SaveIndex(const SubscriptionIndex & index)
{
    std::array<uint8_t, MaxIndexSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < index.mSize; ++i)
    {
        TLV::TLVType innerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, innerType));
        ReturnErrorOnFailure(writer.Put(kFabricIndexTag, index.mNodes[i].GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, index.mNodes[i].GetNodeId()));
        ReturnErrorOnFailure(writer.EndContainer(innerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName(), buf.data(),
                                                   static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::LoadIndex(SubscriptionIndex & index)
{
    std::array<uint8_t, MaxIndexSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    if (mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName(), buf.data(), len) !=
        CHIP_NO_ERROR)
    {
        index.mSize = 0;
        return CHIP_NO_ERROR;
    }

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    size_t count = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        if (count >= ArraySize(index.mNodes))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        FabricIndex fabricIndex;
        ReturnErrorOnFailure(reader.Next(kFabricIndexTag));
        ReturnErrorOnFailure(reader.Get(fabricIndex));

        NodeId peerNodeId;
        ReturnErrorOnFailure(reader.Next(kPeerNodeIdTag));
        ReturnErrorOnFailure(reader.Get(peerNodeId));

        index.mNodes[count++] = ScopedNodeId(peerNodeId, fabricIndex);

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    index.mSize = count;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::FindByScopedNodeId(ScopedNodeId node, std::vector<SubscriptionInfo> & subscriptions)
{
    std::array<uint8_t, MaxStateSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    if (mStorage->SyncGetKeyValue(GetStorageKey(node).KeyName(), buf.data(), len) != CHIP_NO_ERROR)
    {
        return CHIP_NO_ERROR;
    }

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    size_t count = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        // Not using CHIP_IM_MAX_NUM_SUBSCRIPTIONS_PER_FABRIC per explanation
        // in MaxStateSize() header comment block
        if (count >= CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        TLV::TLVType subscriptionContainerType;
        ReturnErrorOnFailure(reader.EnterContainer(subscriptionContainerType));

        SubscriptionInfo subscriptionInfo = { .mNode = node };

        // Subscription ID
        ReturnErrorOnFailure(reader.Next(kSubscriptionIdTag));
        ReturnErrorOnFailure(reader.Get(subscriptionInfo.mSubscriptionId));

        // Min interval
        ReturnErrorOnFailure(reader.Next(kMinIntervalTag));
        ReturnErrorOnFailure(reader.Get(subscriptionInfo.mMinInterval));

        // Max interval
        ReturnErrorOnFailure(reader.Next(kMaxIntervalTag));
        ReturnErrorOnFailure(reader.Get(subscriptionInfo.mMaxInterval));

        // Fabric filtered boolean
        ReturnErrorOnFailure(reader.Next(kFabricFilteredTag));
        ReturnErrorOnFailure(reader.Get(subscriptionInfo.mFabricFiltered));

        // Paths
        uint8_t pathCount = 0;
        ReturnErrorOnFailure(reader.Next(kPathCountTag));
        ReturnErrorOnFailure(reader.Get(pathCount));

        for (uint8_t i = 0; i < pathCount; i++)
        {
            SubscriptionPathType pathType;
            ReturnErrorOnFailure(reader.Next(kPathTypeTag));
            ReturnErrorOnFailure(reader.Get(pathType));

            EndpointId endpointId;
            ReturnErrorOnFailure(reader.Next(kEndpointIdTag));
            ReturnErrorOnFailure(reader.Get(endpointId));

            ClusterId clusterId;
            ReturnErrorOnFailure(reader.Next(kClusterIdTag));
            ReturnErrorOnFailure(reader.Get(clusterId));

            switch (pathType)
            {
            case SubscriptionPathType::kAttributePath: {
                AttributeId attributeId;
                ReturnErrorOnFailure(reader.Next(kAttributeIdTag));
                ReturnErrorOnFailure(reader.Get(attributeId));

                subscriptionInfo.mAttributePaths.push_back(AttributePathParams(endpointId, clusterId, attributeId));
                break;
            }
            case SubscriptionPathType::kUrgentEventPath:
            case SubscriptionPathType::kNonUrgentEventPath: {
                bool isUrgent = (pathType == SubscriptionPathType::kUrgentEventPath);
                EventId eventId;
                ReturnErrorOnFailure(reader.Next(kEventIdTag));
                ReturnErrorOnFailure(reader.Get(eventId));

                subscriptionInfo.mEventPaths.push_back(EventPathParams(endpointId, clusterId, eventId, isUrgent));
                break;
            }
            }
        }
        subscriptions.push_back(subscriptionInfo);
        count++;
        ReturnErrorOnFailure(reader.ExitContainer(subscriptionContainerType));
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::SaveSubscriptions(const ScopedNodeId & node,
                                                                  const std::vector<SubscriptionInfo> & subscriptions)
{
    // Generate new state
    std::array<uint8_t, MaxSubscriptionSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (auto & subscriptionInfo : subscriptions)
    {
        TLV::TLVType subscriptionContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, subscriptionContainerType));
        ReturnErrorOnFailure(writer.Put(kSubscriptionIdTag, subscriptionInfo.mSubscriptionId));
        ReturnErrorOnFailure(writer.Put(kMinIntervalTag, subscriptionInfo.mMinInterval));
        ReturnErrorOnFailure(writer.Put(kMaxIntervalTag, subscriptionInfo.mMaxInterval));
        ReturnErrorOnFailure(writer.Put(kFabricFilteredTag, subscriptionInfo.mFabricFiltered));

        uint8_t pathCount = static_cast<uint8_t>(subscriptionInfo.mAttributePaths.size() + subscriptionInfo.mEventPaths.size());
        ReturnErrorOnFailure(writer.Put(kPathCountTag, pathCount));

        for (auto & attributePathParams : subscriptionInfo.mAttributePaths)
        {
            ReturnErrorOnFailure(writer.Put(kPathTypeTag, SubscriptionPathType::kAttributePath));
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, attributePathParams.mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, attributePathParams.mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, attributePathParams.mAttributeId));
        }

        for (auto & eventPathParams : subscriptionInfo.mEventPaths)
        {
            if (eventPathParams.mIsUrgentEvent)
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, SubscriptionPathType::kUrgentEventPath));
            }
            else
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, SubscriptionPathType::kNonUrgentEventPath));
            }
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, eventPathParams.mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, eventPathParams.mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, eventPathParams.mEventId));
        }

        ReturnErrorOnFailure(writer.EndContainer(subscriptionContainerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(GetStorageKey(node).KeyName(), buf.data(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Save(const SubscriptionInfo & subscriptionInfo)
{
    // Load index and update if fabric/node is new
    SubscriptionIndex subscriptionIndex;
    LoadIndex(subscriptionIndex);
    bool nodeIsNew = true;
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        if (subscriptionInfo.mNode == subscriptionIndex.mNodes[i])
        {
            nodeIsNew = false;
            break;
        }
    }
    if (nodeIsNew)
    {
        if (subscriptionIndex.mSize == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        subscriptionIndex.mNodes[subscriptionIndex.mSize++] = subscriptionInfo.mNode;
        SaveIndex(subscriptionIndex);
    }

    // Load existing subscriptions for node, then combine and save state
    std::vector<SubscriptionInfo> subscriptions;
    CHIP_ERROR err = FindByScopedNodeId(subscriptionInfo.mNode, subscriptions);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // Sanity check for duplicate subscription and remove
    for (auto iter = subscriptions.begin(); iter != subscriptions.end();)
    {
        if ((*iter).mSubscriptionId == subscriptionInfo.mSubscriptionId)
        {
            iter = subscriptions.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // Sanity check this will not go over fabric limit - count
    size_t totalSubscriptions = subscriptions.size();
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        // This node has already been loaded and counted
        if (subscriptionIndex.mNodes[i] == subscriptionInfo.mNode)
        {
            continue;
        }

        std::vector<SubscriptionInfo> otherSubscriptions;
        err = FindByScopedNodeId(subscriptionIndex.mNodes[i], otherSubscriptions);
        if (err == CHIP_NO_ERROR)
        {
            totalSubscriptions += otherSubscriptions.size();
        }
    }
    // Not using CHIP_IM_MAX_NUM_SUBSCRIPTIONS_PER_FABRIC per explanation
    // in MaxStateSize() header comment block
    if (totalSubscriptions == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Merge new subscription in and save
    subscriptions.push_back(subscriptionInfo);
    return SaveSubscriptions(subscriptionInfo.mNode, subscriptions);
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Delete(const SubscriptionInfo & subscriptionInfo)
{
    // load existing subscriptions, then search for subscription
    std::vector<SubscriptionInfo> subscriptions;
    CHIP_ERROR err = FindByScopedNodeId(subscriptionInfo.mNode, subscriptions);

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    bool subscriptionsChanged = false;
    for (auto iter = subscriptions.begin(); iter != subscriptions.end();)
    {
        if ((*iter).mSubscriptionId == subscriptionInfo.mSubscriptionId)
        {
            iter                 = subscriptions.erase(iter);
            subscriptionsChanged = true;
        }
        else
        {
            ++iter;
        }
    }

    if (subscriptionsChanged)
    {
        if (subscriptions.size())
        {
            return SaveSubscriptions(subscriptionInfo.mNode, subscriptions);
        }
        else
        {
            return mStorage->SyncDeleteKeyValue(GetStorageKey(subscriptionInfo.mNode).KeyName());
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::DeleteAll(FabricIndex fabricIndex)
{
    SubscriptionIndex subscriptionIndex;
    LoadIndex(subscriptionIndex);

    size_t i          = 0;
    bool indexChanged = false;
    while (i < subscriptionIndex.mSize)
    {
        if (subscriptionIndex.mNodes[i].GetFabricIndex() == fabricIndex)
        {
            mStorage->SyncDeleteKeyValue(GetStorageKey(subscriptionIndex.mNodes[i]).KeyName());

            // Update count and exit if exhausted all nodes
            --subscriptionIndex.mSize;
            if (i == subscriptionIndex.mSize)
            {
                break;
            }

            // Move the last element into this hole and keep looping
            subscriptionIndex.mNodes[i] = subscriptionIndex.mNodes[subscriptionIndex.mSize];
            indexChanged                = true;
        }
        else
        {
            i++;
        }
    }

    if (indexChanged)
    {
        SaveIndex(subscriptionIndex);
    }

    return CHIP_NO_ERROR;
}

StorageKeyName SimpleSubscriptionResumptionStorage::GetStorageKey(const ScopedNodeId & node)
{
    return DefaultStorageKeyAllocator::FabricSubscription(node.GetFabricIndex(), node.GetNodeId());
}

} // namespace app
} // namespace chip
