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

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxIndexSize());

    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), MaxIndexSize());

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < index.mSize; ++i)
    {
        TLV::TLVType innerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, innerType));
        ReturnErrorOnFailure(writer.Put(kFabricIndexTag, index[i].GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, index[i].GetNodeId()));
        ReturnErrorOnFailure(writer.EndContainer(innerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName(),
                                                   backingBuffer.Get(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

#define DeleteIndexAndReturnLogErrorOnFailure(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName()); \
ChipLogError(DataManagement, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

CHIP_ERROR SimpleSubscriptionResumptionStorage::LoadIndex(SubscriptionIndex & index)
{
    return LoadIndex(index, 0);
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::LoadIndex(SubscriptionIndex & index, size_t allocateExtraSpace)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxIndexSize());
    if (backingBuffer.Get() == nullptr)
    {
        mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    uint16_t len = static_cast<uint16_t>(MaxIndexSize());

    index.mSize = 0;
    if (mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName(), backingBuffer.Get(), len) !=
        CHIP_NO_ERROR)
    {
        mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
        return CHIP_NO_ERROR;
    }

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);

    DeleteIndexAndReturnLogErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    DeleteIndexAndReturnLogErrorOnFailure(reader.EnterContainer(arrayType));

    size_t nodeCount;
    DeleteIndexAndReturnLogErrorOnFailure(reader.CountRemainingInContainer(&nodeCount));
    if (nodeCount >= CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    index.mNodes = std::unique_ptr<ScopedNodeId[]>(new(std::nothrow) ScopedNodeId[nodeCount + allocateExtraSpace]);
    if (!index.mNodes)
    {
        mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    index.mSize = nodeCount;

    for (size_t currentNodeIndex = 0; currentNodeIndex < nodeCount; currentNodeIndex++)
    {
        DeleteIndexAndReturnLogErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        TLV::TLVType containerType;
        DeleteIndexAndReturnLogErrorOnFailure(reader.EnterContainer(containerType));

        FabricIndex fabricIndex;
        DeleteIndexAndReturnLogErrorOnFailure(reader.Next(kFabricIndexTag));
        DeleteIndexAndReturnLogErrorOnFailure(reader.Get(fabricIndex));

        NodeId peerNodeId;
        DeleteIndexAndReturnLogErrorOnFailure(reader.Next(kPeerNodeIdTag));
        DeleteIndexAndReturnLogErrorOnFailure(reader.Get(peerNodeId));

        index[currentNodeIndex] = ScopedNodeId(peerNodeId, fabricIndex);

        DeleteIndexAndReturnLogErrorOnFailure(reader.ExitContainer(containerType));
    }

    DeleteIndexAndReturnLogErrorOnFailure(reader.ExitContainer(arrayType));
    DeleteIndexAndReturnLogErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

#define DeleteSubscriptionsAndReturnLogErrorOnFailure(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
mStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName()); \
ChipLogError(DataManagement, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

CHIP_ERROR SimpleSubscriptionResumptionStorage::FindByScopedNodeId(ScopedNodeId node, SubscriptionList & subscriptions)
{
    return FindByScopedNodeId(node, subscriptions, 0);
}
CHIP_ERROR SimpleSubscriptionResumptionStorage::FindByScopedNodeId(ScopedNodeId node, SubscriptionList & subscriptions, size_t allocateExtraSpace)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxStateSize());
    if (backingBuffer.Get() == nullptr)
    {
        mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionIndex().KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    uint16_t len = static_cast<uint16_t>(MaxStateSize());

    if (mStorage->SyncGetKeyValue(GetStorageKey(node).KeyName(), backingBuffer.Get(), len) != CHIP_NO_ERROR)
    {
        subscriptions.mSize = 0;
        mStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName());
        return CHIP_NO_ERROR;
    }

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);

    DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.EnterContainer(arrayType));

    size_t subscriptionCount;
    DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.CountRemainingInContainer(&subscriptionCount));
    if (subscriptionCount >= CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        mStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    subscriptions.mSubscriptions = std::unique_ptr<SubscriptionInfo[]>(new(std::nothrow) SubscriptionInfo[subscriptionCount + allocateExtraSpace]);
    if (!subscriptions.mSubscriptions)
    {
        mStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName());
        return CHIP_ERROR_NO_MEMORY;
    }

    subscriptions.mSize = subscriptionCount;

    for (size_t currentSubscriptionIndex = 0; currentSubscriptionIndex < subscriptionCount; currentSubscriptionIndex++)
    {
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType subscriptionContainerType;
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.EnterContainer(subscriptionContainerType));

        subscriptions[currentSubscriptionIndex] = { .mNodeId = node.GetNodeId(), .mFabricIndex = node.GetFabricIndex() };

        // Subscription ID
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kSubscriptionIdTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mSubscriptionId));

        // Min interval
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kMinIntervalTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mMinInterval));

        // Max interval
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kMaxIntervalTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mMaxInterval));

        // Fabric filtered boolean
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kFabricFilteredTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mFabricFiltered));

        // Attribute Paths
        uint16_t pathCount = 0;
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kPathCountTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(pathCount));

        subscriptions[currentSubscriptionIndex].mAttributePaths = Platform::ScopedMemoryBufferWithSize<AttributePathParamsValues>();
        if (pathCount)
        {
            subscriptions[currentSubscriptionIndex].mAttributePaths.Calloc(pathCount);
            for (uint16_t currentPathIndex = 0; currentPathIndex < pathCount; currentPathIndex++)
            {
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kEndpointIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mEndpointId));

                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kClusterIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mClusterId));

                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kAttributeIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mAttributeId));
            }
        }

        // Event Paths
        pathCount = 0;
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kPathCountTag));
        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(pathCount));

        subscriptions[currentSubscriptionIndex].mEventPaths = Platform::ScopedMemoryBufferWithSize<EventPathParamsValues>();
        if (pathCount)
        {
            subscriptions[currentSubscriptionIndex].mEventPaths.Calloc(pathCount);
            for (uint16_t currentPathIndex = 0; currentPathIndex < pathCount; currentPathIndex++)
            {
                EventPathType eventPathType;
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kPathTypeTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(eventPathType));

                subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mIsUrgentEvent = (eventPathType == EventPathType::kUrgent);

                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kEndpointIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mEndpointId));

                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kClusterIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mClusterId));

                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Next(kEventIdTag));
                DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.Get(subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mEventId));
            }
        }

        DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.ExitContainer(subscriptionContainerType));
    }

    DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.ExitContainer(arrayType));
    DeleteSubscriptionsAndReturnLogErrorOnFailure(reader.VerifyEndOfContainer());

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

    for (size_t currentSubscriptionIndex = 0; currentSubscriptionIndex < subscriptions.mSize; currentSubscriptionIndex++)
    {
        TLV::TLVType subscriptionContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, subscriptionContainerType));
        ReturnErrorOnFailure(writer.Put(kSubscriptionIdTag, subscriptions[currentSubscriptionIndex].mSubscriptionId));
        ReturnErrorOnFailure(writer.Put(kMinIntervalTag, subscriptions[currentSubscriptionIndex].mMinInterval));
        ReturnErrorOnFailure(writer.Put(kMaxIntervalTag, subscriptions[currentSubscriptionIndex].mMaxInterval));
        ReturnErrorOnFailure(writer.Put(kFabricFilteredTag, subscriptions[currentSubscriptionIndex].mFabricFiltered));

        ReturnErrorOnFailure(
            writer.Put(kPathCountTag, static_cast<uint16_t>(subscriptions[currentSubscriptionIndex].mAttributePaths.AllocatedCount())));
        for (size_t currentPathIndex = 0; currentPathIndex < subscriptions[currentSubscriptionIndex].mAttributePaths.AllocatedCount(); currentPathIndex++)
        {
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, subscriptions[currentSubscriptionIndex].mAttributePaths[currentPathIndex].mAttributeId));
        }

        ReturnErrorOnFailure(
            writer.Put(kPathCountTag, static_cast<uint16_t>(subscriptions[currentSubscriptionIndex].mEventPaths.AllocatedCount())));
        for (size_t currentPathIndex = 0; currentPathIndex < subscriptions[currentSubscriptionIndex].mEventPaths.AllocatedCount(); currentPathIndex++)
        {
            if (subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mIsUrgentEvent)
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, EventPathType::kUrgent));
            }
            else
            {
                ReturnErrorOnFailure(writer.Put(kPathTypeTag, EventPathType::kNonUrgent));
            }
            ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mEndpointId));
            ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mClusterId));
            ReturnErrorOnFailure(writer.Put(kAttributeIdTag, subscriptions[currentSubscriptionIndex].mEventPaths[currentPathIndex].mEventId));
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
    LoadIndex(subscriptionIndex, 1);
    bool nodeIsNew = true;
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        if (subscriptionNode == subscriptionIndex[i])
        {
            nodeIsNew = false;
            break;
        }
    }
    if (nodeIsNew)
    {
        if (!subscriptionIndex.mSize)
        {
            subscriptionIndex.mNodes = std::unique_ptr<ScopedNodeId[]>(new(std::nothrow) ScopedNodeId[1]);
            if (!subscriptionIndex.mNodes)
            {
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        if (subscriptionIndex.mSize == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        subscriptionIndex[subscriptionIndex.mSize++] = subscriptionNode;
        SaveIndex(subscriptionIndex);
    }

    // Load existing subscriptions for node, then combine and save state
    SubscriptionList subscriptions;
    CHIP_ERROR err = FindByScopedNodeId(subscriptionNode, subscriptions, 1);  // ask to allocate 1 extra space for new subscription
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // if this is the first subscription, allocate space for 1
    if (!subscriptions.mSize)
    {
        subscriptions.mSubscriptions = std::unique_ptr<SubscriptionInfo[]>(new(std::nothrow) SubscriptionInfo[1]);
        if (!subscriptions.mSubscriptions)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    // Sanity check for duplicate subscription and remove
    for (size_t i = 0; i < subscriptions.mSize; i++)
    {
        if (subscriptionInfo.mSubscriptionId == subscriptions[i].mSubscriptionId)
        {
            subscriptions.mSize--;
            // if not last element, move last element here, essentially deleting this
            if (i < subscriptions.mSize)
            {
                subscriptions[i] = std::move(subscriptions[subscriptions.mSize]);
            }
            break;
        }
    }

    // Sanity check this will not go over fabric limit - count
    size_t totalSubscriptions = subscriptions.mSize;
    for (size_t i = 0; i < subscriptionIndex.mSize; i++)
    {
        // This node has already been loaded and counted
        if (subscriptionNode == subscriptionIndex[i])
        {
            continue;
        }

        SubscriptionList otherSubscriptions;
        err = FindByScopedNodeId(subscriptionIndex[i], otherSubscriptions);
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
    subscriptions[subscriptions.mSize++] = std::move(subscriptionInfo);
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
        if (subscriptionInfo.mSubscriptionId == subscriptions[i].mSubscriptionId)
        {
            subscriptions.mSize--;
            // if not last element, move last element here, essentially deleting this
            if (i < subscriptions.mSize)
            {
                subscriptions[i] = std::move(subscriptions[subscriptions.mSize]);
            }

            subscriptionsChanged = true;
            break;
        }
    }

    if (subscriptionsChanged)
    {
        if (subscriptions.mSize)
        {
            return SaveSubscriptions(subscriptionNode, subscriptions);
        }

        // Remove node from index
        SubscriptionIndex subscriptionIndex;
        LoadIndex(subscriptionIndex);
        for (size_t i = 0; i < subscriptionIndex.mSize; i++)
        {
            if (subscriptionNode == subscriptionIndex[i])
            {
                subscriptionIndex.mSize--;
                // if not last element, move last element here, essentially deleting this
                if (i < subscriptionIndex.mSize)
                {
                    subscriptionIndex[i] = std::move(subscriptionIndex[subscriptionIndex.mSize]);
                }
                break;
            }
        }
        SaveIndex(subscriptionIndex);

        return mStorage->SyncDeleteKeyValue(GetStorageKey(subscriptionNode).KeyName());
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
        if (subscriptionIndex[i].GetFabricIndex() == fabricIndex)
        {
            mStorage->SyncDeleteKeyValue(GetStorageKey(subscriptionIndex[i]).KeyName());

            // Update count and exit if exhausted all nodes
            --subscriptionIndex.mSize;
            if (i == subscriptionIndex.mSize)
            {
                break;
            }

            // Move the last element into this hole and keep looping
            subscriptionIndex[i] = subscriptionIndex[subscriptionIndex.mSize];
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
