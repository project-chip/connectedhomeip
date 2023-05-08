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

#include <app/SimpleSubscriptionResumptionStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kPeerNodeIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kFabricIndexTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kSubscriptionIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kMinIntervalTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kMaxIntervalTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kFabricFilteredTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kAttributePathsListTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEventPathsListTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kAttributePathTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEventPathTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEndpointIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kClusterIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kAttributeIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEventIdTag;
constexpr TLV::Tag SimpleSubscriptionResumptionStorage::kEventPathTypeTag;

SimpleSubscriptionResumptionStorage::SimpleSubscriptionInfoIterator::SimpleSubscriptionInfoIterator(
    SimpleSubscriptionResumptionStorage & storage) :
    mStorage(storage)
{
    mNextIndex = 0;
}

size_t SimpleSubscriptionResumptionStorage::SimpleSubscriptionInfoIterator::Count()
{
    return static_cast<size_t>(mStorage.Count());
}

bool SimpleSubscriptionResumptionStorage::SimpleSubscriptionInfoIterator::Next(SubscriptionInfo & output)
{
    for (; mNextIndex < CHIP_IM_MAX_NUM_SUBSCRIPTIONS; mNextIndex++)
    {
        CHIP_ERROR err = mStorage.Load(mNextIndex, output);
        if (err == CHIP_NO_ERROR)
        {
            // increment index for the next call
            mNextIndex++;
            return true;
        }

        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(DataManagement, "Failed to load subscription at index %u error %" CHIP_ERROR_FORMAT,
                         static_cast<unsigned>(mNextIndex), err.Format());
            mStorage.Delete(mNextIndex);
        }
    }

    return false;
}

void SimpleSubscriptionResumptionStorage::SimpleSubscriptionInfoIterator::Release()
{
    mStorage.mSubscriptionInfoIterators.ReleaseObject(this);
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Init(PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;

    uint16_t countMax;
    uint16_t len = sizeof(countMax);
    CHIP_ERROR err =
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(), &countMax, len);
    // If there's a previous countMax and it's larger than CHIP_IM_MAX_NUM_SUBSCRIPTIONS,
    // clean up subscriptions beyond the limit
    if ((err == CHIP_NO_ERROR) && (countMax != CHIP_IM_MAX_NUM_SUBSCRIPTIONS))
    {
        for (uint16_t subscriptionIndex = CHIP_IM_MAX_NUM_SUBSCRIPTIONS; subscriptionIndex < countMax; subscriptionIndex++)
        {
            Delete(subscriptionIndex);
        }
    }

    // Always save the current CHIP_IM_MAX_NUM_SUBSCRIPTIONS
    uint16_t countMaxToSave = CHIP_IM_MAX_NUM_SUBSCRIPTIONS;
    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName(),
                                                   &countMaxToSave, sizeof(uint16_t)));

    return CHIP_NO_ERROR;
}

SubscriptionResumptionStorage::SubscriptionInfoIterator * SimpleSubscriptionResumptionStorage::IterateSubscriptions()
{
    return mSubscriptionInfoIterators.CreateObject(*this);
}

uint16_t SimpleSubscriptionResumptionStorage::Count()
{
    uint16_t subscriptionCount = 0;
    for (uint16_t subscriptionIndex = 0; subscriptionIndex < CHIP_IM_MAX_NUM_SUBSCRIPTIONS; subscriptionIndex++)
    {
        if (mStorage->SyncDoesKeyExist(DefaultStorageKeyAllocator::SubscriptionResumption(subscriptionIndex).KeyName()))
        {
            subscriptionCount++;
        }
    }

    return subscriptionCount;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Delete(uint16_t subscriptionIndex)
{
    return mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumption(subscriptionIndex).KeyName());
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Load(uint16_t subscriptionIndex, SubscriptionInfo & subscriptionInfo)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxSubscriptionSize());
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t len = static_cast<uint16_t>(MaxSubscriptionSize());
    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumption(subscriptionIndex).KeyName(),
                                                   backingBuffer.Get(), len));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType subscriptionContainerType;
    ReturnErrorOnFailure(reader.EnterContainer(subscriptionContainerType));

    // Node ID
    ReturnErrorOnFailure(reader.Next(kPeerNodeIdTag));
    ReturnErrorOnFailure(reader.Get(subscriptionInfo.mNodeId));

    // Fabric index
    ReturnErrorOnFailure(reader.Next(kFabricIndexTag));
    ReturnErrorOnFailure(reader.Get(subscriptionInfo.mFabricIndex));

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

    // Attribute Paths
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_List, kAttributePathsListTag));
    TLV::TLVType attributesListType;
    ReturnErrorOnFailure(reader.EnterContainer(attributesListType));

    size_t pathCount = 0;
    ReturnErrorOnFailure(reader.CountRemainingInContainer(&pathCount));

    // If a stack struct is being reused to iterate, free the previous paths ScopedMemoryBuffer
    subscriptionInfo.mAttributePaths.Free();
    if (pathCount)
    {
        subscriptionInfo.mAttributePaths.Calloc(pathCount);
        ReturnErrorCodeIf(subscriptionInfo.mAttributePaths.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
        for (size_t pathIndex = 0; pathIndex < pathCount; pathIndex++)
        {
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, kAttributePathTag));
            TLV::TLVType attributeContainerType;
            ReturnErrorOnFailure(reader.EnterContainer(attributeContainerType));

            ReturnErrorOnFailure(reader.Next(kEndpointIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mAttributePaths[pathIndex].mEndpointId));

            ReturnErrorOnFailure(reader.Next(kClusterIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mAttributePaths[pathIndex].mClusterId));

            ReturnErrorOnFailure(reader.Next(kAttributeIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mAttributePaths[pathIndex].mAttributeId));

            ReturnErrorOnFailure(reader.ExitContainer(attributeContainerType));
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(attributesListType));

    // Event Paths
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_List, kEventPathsListTag));
    TLV::TLVType eventsListType;
    ReturnErrorOnFailure(reader.EnterContainer(eventsListType));

    ReturnErrorOnFailure(reader.CountRemainingInContainer(&pathCount));

    // If a stack struct is being reused to iterate, free the previous paths ScopedMemoryBuffer
    subscriptionInfo.mEventPaths.Free();
    if (pathCount)
    {
        subscriptionInfo.mEventPaths.Calloc(pathCount);
        ReturnErrorCodeIf(subscriptionInfo.mEventPaths.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
        for (size_t pathIndex = 0; pathIndex < pathCount; pathIndex++)
        {
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, kEventPathTag));
            TLV::TLVType eventContainerType;
            ReturnErrorOnFailure(reader.EnterContainer(eventContainerType));

            EventPathType eventPathType;
            ReturnErrorOnFailure(reader.Next(kEventPathTypeTag));
            ReturnErrorOnFailure(reader.Get(eventPathType));

            subscriptionInfo.mEventPaths[pathIndex].mIsUrgentEvent = (eventPathType == EventPathType::kUrgent);

            ReturnErrorOnFailure(reader.Next(kEndpointIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mEventPaths[pathIndex].mEndpointId));

            ReturnErrorOnFailure(reader.Next(kClusterIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mEventPaths[pathIndex].mClusterId));

            ReturnErrorOnFailure(reader.Next(kEventIdTag));
            ReturnErrorOnFailure(reader.Get(subscriptionInfo.mEventPaths[pathIndex].mEventId));

            ReturnErrorOnFailure(reader.ExitContainer(eventContainerType));
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(eventsListType));

    ReturnErrorOnFailure(reader.ExitContainer(subscriptionContainerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Save(TLV::TLVWriter & writer, SubscriptionInfo & subscriptionInfo)
{
    TLV::TLVType subscriptionContainerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, subscriptionContainerType));
    ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, subscriptionInfo.mNodeId));
    ReturnErrorOnFailure(writer.Put(kFabricIndexTag, subscriptionInfo.mFabricIndex));
    ReturnErrorOnFailure(writer.Put(kSubscriptionIdTag, subscriptionInfo.mSubscriptionId));
    ReturnErrorOnFailure(writer.Put(kMinIntervalTag, subscriptionInfo.mMinInterval));
    ReturnErrorOnFailure(writer.Put(kMaxIntervalTag, subscriptionInfo.mMaxInterval));
    ReturnErrorOnFailure(writer.Put(kFabricFilteredTag, subscriptionInfo.mFabricFiltered));

    // Attribute paths
    TLV::TLVType attributesListType;
    ReturnErrorOnFailure(writer.StartContainer(kAttributePathsListTag, TLV::kTLVType_List, attributesListType));
    for (size_t pathIndex = 0; pathIndex < subscriptionInfo.mAttributePaths.AllocatedSize(); pathIndex++)
    {
        TLV::TLVType attributeContainerType = TLV::kTLVType_Structure;
        ReturnErrorOnFailure(writer.StartContainer(kAttributePathTag, TLV::kTLVType_Structure, attributeContainerType));

        ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptionInfo.mAttributePaths[pathIndex].mEndpointId));
        ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptionInfo.mAttributePaths[pathIndex].mClusterId));
        ReturnErrorOnFailure(writer.Put(kAttributeIdTag, subscriptionInfo.mAttributePaths[pathIndex].mAttributeId));

        ReturnErrorOnFailure(writer.EndContainer(attributeContainerType));
    }
    ReturnErrorOnFailure(writer.EndContainer(attributesListType));

    // Event paths
    TLV::TLVType eventsListType;
    ReturnErrorOnFailure(writer.StartContainer(kEventPathsListTag, TLV::kTLVType_List, eventsListType));
    for (size_t pathIndex = 0; pathIndex < subscriptionInfo.mEventPaths.AllocatedSize(); pathIndex++)
    {
        TLV::TLVType eventContainerType = TLV::kTLVType_Structure;
        ReturnErrorOnFailure(writer.StartContainer(kEventPathTag, TLV::kTLVType_Structure, eventContainerType));

        if (subscriptionInfo.mEventPaths[pathIndex].mIsUrgentEvent)
        {
            ReturnErrorOnFailure(writer.Put(kEventPathTypeTag, EventPathType::kUrgent));
        }
        else
        {
            ReturnErrorOnFailure(writer.Put(kEventPathTypeTag, EventPathType::kNonUrgent));
        }
        ReturnErrorOnFailure(writer.Put(kEndpointIdTag, subscriptionInfo.mEventPaths[pathIndex].mEndpointId));
        ReturnErrorOnFailure(writer.Put(kClusterIdTag, subscriptionInfo.mEventPaths[pathIndex].mClusterId));
        ReturnErrorOnFailure(writer.Put(kEventIdTag, subscriptionInfo.mEventPaths[pathIndex].mEventId));

        ReturnErrorOnFailure(writer.EndContainer(eventContainerType));
    }
    ReturnErrorOnFailure(writer.EndContainer(eventsListType));

    ReturnErrorOnFailure(writer.EndContainer(subscriptionContainerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Save(SubscriptionInfo & subscriptionInfo)
{
    // Find empty index or duplicate if exists
    uint16_t subscriptionIndex;
    uint16_t firstEmptySubscriptionIndex = CHIP_IM_MAX_NUM_SUBSCRIPTIONS; // initialize to out of bounds as "not set"
    for (subscriptionIndex = 0; subscriptionIndex < CHIP_IM_MAX_NUM_SUBSCRIPTIONS; subscriptionIndex++)
    {
        SubscriptionInfo currentSubscriptionInfo;
        CHIP_ERROR err = Load(subscriptionIndex, currentSubscriptionInfo);

        // if empty and firstEmptySubscriptionIndex isn't set yet, then mark empty spot
        if ((firstEmptySubscriptionIndex == CHIP_IM_MAX_NUM_SUBSCRIPTIONS) && (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
        {
            firstEmptySubscriptionIndex = subscriptionIndex;
        }

        // delete duplicate
        if (err == CHIP_NO_ERROR)
        {
            if ((subscriptionInfo.mNodeId == currentSubscriptionInfo.mNodeId) &&
                (subscriptionInfo.mFabricIndex == currentSubscriptionInfo.mFabricIndex) &&
                (subscriptionInfo.mSubscriptionId == currentSubscriptionInfo.mSubscriptionId))
            {
                Delete(subscriptionIndex);
                // if duplicate is the first empty spot, then also set it
                if (firstEmptySubscriptionIndex == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
                {
                    firstEmptySubscriptionIndex = subscriptionIndex;
                }
            }
        }
    }

    // Fail if no empty space
    if (firstEmptySubscriptionIndex == CHIP_IM_MAX_NUM_SUBSCRIPTIONS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Now construct subscription state and save
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxSubscriptionSize());
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), MaxSubscriptionSize());

    ReturnErrorOnFailure(Save(writer, subscriptionInfo));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);

    ReturnErrorOnFailure(
        mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::SubscriptionResumption(firstEmptySubscriptionIndex).KeyName(),
                                  backingBuffer.Get(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::Delete(NodeId nodeId, FabricIndex fabricIndex, SubscriptionId subscriptionId)
{
    bool subscriptionFound   = false;
    CHIP_ERROR lastDeleteErr = CHIP_NO_ERROR;

    uint16_t remainingSubscriptionsCount = 0;
    for (uint16_t subscriptionIndex = 0; subscriptionIndex < CHIP_IM_MAX_NUM_SUBSCRIPTIONS; subscriptionIndex++)
    {
        SubscriptionInfo subscriptionInfo;
        CHIP_ERROR err = Load(subscriptionIndex, subscriptionInfo);

        // delete match
        if (err == CHIP_NO_ERROR)
        {
            if ((nodeId == subscriptionInfo.mNodeId) && (fabricIndex == subscriptionInfo.mFabricIndex) &&
                (subscriptionId == subscriptionInfo.mSubscriptionId))
            {
                subscriptionFound    = true;
                CHIP_ERROR deleteErr = Delete(subscriptionIndex);
                if (deleteErr != CHIP_NO_ERROR)
                {
                    lastDeleteErr = deleteErr;
                }
            }
            else
            {
                remainingSubscriptionsCount++;
            }
        }
    }

    // if there are no persisted subscriptions, the MaxCount can also be deleted
    if (remainingSubscriptionsCount == 0)
    {
        DeleteMaxCount();
    }

    if (lastDeleteErr != CHIP_NO_ERROR)
    {
        return lastDeleteErr;
    }

    return subscriptionFound ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::DeleteMaxCount()
{
    return mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::SubscriptionResumptionMaxCount().KeyName());
}

CHIP_ERROR SimpleSubscriptionResumptionStorage::DeleteAll(FabricIndex fabricIndex)
{
    CHIP_ERROR deleteErr = CHIP_NO_ERROR;

    uint16_t count = 0;
    for (uint16_t subscriptionIndex = 0; subscriptionIndex < CHIP_IM_MAX_NUM_SUBSCRIPTIONS; subscriptionIndex++)
    {
        SubscriptionInfo subscriptionInfo;
        CHIP_ERROR err = Load(subscriptionIndex, subscriptionInfo);

        if (err == CHIP_NO_ERROR)
        {
            if (fabricIndex == subscriptionInfo.mFabricIndex)
            {
                err = Delete(subscriptionIndex);
                if ((err != CHIP_NO_ERROR) && (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
                {
                    deleteErr = err;
                }
            }
            else
            {
                count++;
            }
        }
    }

    // if there are no persisted subscriptions, the MaxCount can also be deleted
    if (count == 0)
    {
        CHIP_ERROR err = DeleteMaxCount();

        if ((err != CHIP_NO_ERROR) && (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
        {
            deleteErr = err;
        }
    }

    return deleteErr;
}

} // namespace app
} // namespace chip
