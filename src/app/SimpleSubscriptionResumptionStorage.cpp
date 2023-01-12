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
    if (index.mSize == 0)
    {
        return mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
    }

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

CHIP_ERROR SimpleSubscriptionResumptionStorage::FindByScopedNodeId(ScopedNodeId node, SubscriptionList & subscriptions)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxIndexSize());
    VerifyOrReturnError(backingBuffer.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t len = static_cast<uint16_t>(MaxIndexSize());

    if (mStorage->SyncGetKeyValue(GetStorageKey(node).KeyName(), backingBuffer.Get(), len) != CHIP_NO_ERROR)
    {
        subscriptions.mSize = 0;
        return CHIP_NO_ERROR;
    }

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    size_t count;
    ReturnErrorOnFailure(reader.CountRemainingInContainer(&count));
    if (count >= CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    subscriptions.mSize = count;

    CHIP_ERROR err;
    for (size_t i = 0; i < count; i++)
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType subscriptionContainerType;
        ReturnErrorOnFailure(reader.EnterContainer(subscriptionContainerType));

        subscriptions.mSubscriptions[i] = { .mNodeId = node.GetNodeId(), .mFabricIndex = node.GetFabricIndex() };

        // Subscription ID
        ReturnErrorOnFailure(reader.Next(kSubscriptionIdTag));
        ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mSubscriptionId));

        // Min interval
        ReturnErrorOnFailure(reader.Next(kMinIntervalTag));
        ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mMinInterval));

        // Max interval
        ReturnErrorOnFailure(reader.Next(kMaxIntervalTag));
        ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mMaxInterval));

        // Fabric filtered boolean
        ReturnErrorOnFailure(reader.Next(kFabricFilteredTag));
        ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mFabricFiltered));

        // Attribute Paths
        uint16_t pathCount = 0;
        ReturnErrorOnFailure(reader.Next(kPathCountTag));
        ReturnErrorOnFailure(reader.Get(pathCount));

        subscriptions.mSubscriptions[i].mAttributePaths = Platform::ScopedMemoryBufferWithSize<AttributePathParamsValues>();
        if (pathCount)
        {
            subscriptions.mSubscriptions[i].mAttributePaths.Calloc(pathCount);
            for (uint8_t j = 0; j < pathCount; j++)
            {
                ReturnErrorOnFailure(reader.Next(kEndpointIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mAttributePaths[j].mEndpointId));

                ReturnErrorOnFailure(reader.Next(kClusterIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mAttributePaths[j].mClusterId));

                ReturnErrorOnFailure(reader.Next(kAttributeIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mAttributePaths[j].mAttributeId));
            }
        }

        // Event Paths
        pathCount = 0;
        ReturnErrorOnFailure(reader.Next(kPathCountTag));
        ReturnErrorOnFailure(reader.Get(pathCount));

        subscriptions.mSubscriptions[i].mEventPaths = Platform::ScopedMemoryBufferWithSize<EventPathParamsValues>();
        if (pathCount)
        {
            subscriptions.mSubscriptions[i].mEventPaths.Calloc(pathCount);
            for (uint8_t j = 0; j < pathCount; j++)
            {
                EventPathType eventPathType;
                ReturnErrorOnFailure(reader.Next(kPathTypeTag));
                ReturnErrorOnFailure(reader.Get(eventPathType));

                subscriptions.mSubscriptions[i].mEventPaths[j].mIsUrgentEvent = (eventPathType == EventPathType::kUrgent);

                ReturnErrorOnFailure(reader.Next(kEndpointIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mEventPaths[j].mEndpointId));

                ReturnErrorOnFailure(reader.Next(kClusterIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mEventPaths[j].mClusterId));

                ReturnErrorOnFailure(reader.Next(kEventIdTag));
                ReturnErrorOnFailure(reader.Get(subscriptions.mSubscriptions[i].mEventPaths[j].mEventId));
            }
        }

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

CHIP_ERROR SimpleSubscriptionResumptionStorage::SaveSubscriptions(const ScopedNodeId & node, const SubscriptionList & subscriptions)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxIndexSize());
    VerifyOrReturnError(backingBuffer.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), MaxIndexSize());

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < subscriptions.mSize; i++)
    {
        TLV::TLVType subscriptionContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, subscriptionContainerType));
        ReturnErrorOnFailure(writer.Put(kSubscriptionIdTag, subscriptions.mSubscriptions[i].mSubscriptionId));
        ReturnErrorOnFailure(writer.Put(kMinIntervalTag, subscriptions.mSubscriptions[i].mMinInterval));
        ReturnErrorOnFailure(writer.Put(kMaxIntervalTag, subscriptions.mSubscriptions[i].mMaxInterval));
        ReturnErrorOnFailure(writer.Put(kFabricFilteredTag, subscriptions.mSubscriptions[i].mFabricFiltered));

        ReturnErrorOnFailure(
            writer.Put(kPathCountTag, static_cast<uint16_t>(subscriptions.mSubscriptions[i].mAttributePaths.AllocatedCount())));
        for (size_t j = 0; j < subscriptions.mSubscriptions[i].mAttributePaths.AllocatedCount(); j++)
        {
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptions.mSubscriptions[i].mAttributePaths[j].mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptions.mSubscriptions[i].mAttributePaths[j].mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, subscriptions.mSubscriptions[i].mAttributePaths[j].mAttributeId));
        }

        ReturnErrorOnFailure(
            writer.Put(kPathCountTag, static_cast<uint16_t>(subscriptions.mSubscriptions[i].mEventPaths.AllocatedCount())));
        for (size_t j = 0; j < subscriptions.mSubscriptions[i].mAttributePaths.AllocatedCount(); j++)
        {
            if (subscriptions.mSubscriptions[i].mEventPaths[j].mIsUrgentEvent)
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, EventPathType::kUrgent));
            }
            else
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, EventPathType::kNonUrgent));
            }
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptions.mSubscriptions[i].mEventPaths[j].mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptions.mSubscriptions[i].mEventPaths[j].mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, subscriptions.mSubscriptions[i].mEventPaths[j].mEventId));
        }

        ReturnErrorOnFailure(writer.EndContainer(subscriptionContainerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(GetStorageKey(node).KeyName(), backingBuffer.Get(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Save(SubscriptionInfo & subscriptionInfo)
{
    ScopedNodeId subscriptionNode = ScopedNodeId(subscriptionInfo.mNodeId, subscriptionInfo.mFabricIndex);
    // Load index and update if fabric/node is new
    SubscriptionIndex subscriptionIndex;
    LoadIndex(subscriptionIndex);
    bool nodeIsNew = true;
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        if (subscriptionNode == subscriptionIndex.mNodes[i])
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
        subscriptionIndex.mNodes[subscriptionIndex.mSize++] = subscriptionNode;
        SaveIndex(subscriptionIndex);
    }

    // Load existing subscriptions for node, then combine and save state
    SubscriptionList subscriptions;
    CHIP_ERROR err = FindByScopedNodeId(subscriptionNode, subscriptions);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // Sanity check for duplicate subscription and remove
    for (size_t i = 0; i < subscriptions.mSize; i++)
    {
        if (subscriptionInfo.mSubscriptionId == subscriptions.mSubscriptions[i].mSubscriptionId)
        {
            subscriptions.mSize--;
            // if not last element, move last element here, essentially deleting this
            if (i < subscriptions.mSize)
            {
                subscriptions.mSubscriptions[i] = std::move(subscriptions.mSubscriptions[subscriptions.mSize]);
            }
            break;
        }
    }

    // Sanity check this will not go over fabric limit - count
    size_t totalSubscriptions = subscriptions.mSize;
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        // This node has already been loaded and counted
        if (subscriptionNode == subscriptionIndex.mNodes[i])
        {
            continue;
        }

        SubscriptionList otherSubscriptions;
        err = FindByScopedNodeId(subscriptionIndex.mNodes[i], otherSubscriptions);
        if (err == CHIP_NO_ERROR)
        {
            totalSubscriptions += otherSubscriptions.mSize;
        }
    }
    // Not using CHIP_IM_MAX_NUM_SUBSCRIPTIONS_PER_FABRIC per explanation
    // in MaxStateSize() header comment block
    if (totalSubscriptions == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Merge new subscription in and save
    subscriptions.mSubscriptions[subscriptions.mSize++] = std::move(subscriptionInfo);
    return SaveSubscriptions(subscriptionNode, subscriptions);
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Delete(const SubscriptionInfo & subscriptionInfo)
{
    ScopedNodeId subscriptionNode = ScopedNodeId(subscriptionInfo.mNodeId, subscriptionInfo.mFabricIndex);
    // load existing subscriptions, then search for subscription
    SubscriptionList subscriptions;
    CHIP_ERROR err = FindByScopedNodeId(subscriptionNode, subscriptions);

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    bool subscriptionsChanged = false;
    for (size_t i = 0; i < subscriptions.mSize; i++)
    {
        if (subscriptions.mSubscriptions[i].mSubscriptionId == subscriptionInfo.mSubscriptionId)
        {
            subscriptions.mSize--;
            // if not last element, move last element here, essentially deleting this
            if (i < subscriptions.mSize)
            {
                subscriptions.mSubscriptions[i] = std::move(subscriptions.mSubscriptions[subscriptions.mSize]);
            }
            break;
        }
    }

    if (subscriptionsChanged)
    {
        if (subscriptions.mSize)
        {
            return SaveSubscriptions(subscriptionNode, subscriptions);
        }
        else
        {
            // Remove node from index
            SubscriptionIndex subscriptionIndex;
            LoadIndex(subscriptionIndex);
            for (size_t i = 0; i < subscriptionIndex.mSize; i++)
            {
                if (subscriptionNode == subscriptionIndex.mNodes[i])
                {
                    subscriptionIndex.mSize--;
                    // if not last element, move last element here, essentially deleting this
                    if (i < subscriptionIndex.mSize)
                    {
                        subscriptionIndex.mNodes[i] = std::move(subscriptionIndex.mNodes[subscriptionIndex.mSize]);
                    }
                    break;
                }
            }
            SaveIndex(subscriptionIndex);

            return mStorage->SyncDeleteKeyValue(GetStorageKey(subscriptionNode).KeyName());
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
