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

/**
 *    @file
 *      This file defines the Peer TCP Parameter Storage object that provides
 *      APIs for storage and retireval of the TCP parameters for a given peer
 *      node scoped to a fabric.
 */

#include <transport/raw/PeerTCPParamsStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Transport {

constexpr TLV::Tag PeerTCPParamsStorage::kSupportedTransportsTag;
constexpr TLV::Tag PeerTCPParamsStorage::kMaxTCPMessageSizeTag;

StorageKeyName PeerTCPParamsStorage::GetStorageKey(const ScopedNodeId & node)
{
    return DefaultStorageKeyAllocator::TCPPeerParams(node.GetFabricIndex(), node.GetNodeId());
}

CHIP_ERROR PeerTCPParamsStorage::FindByScopedNodeId(const ScopedNodeId & node, uint16_t & supportedTransports,
                                                    uint32_t & maxTCPMessageSize)
{
    return LoadTCPParamsFromStorage(node, supportedTransports, maxTCPMessageSize);
}

CHIP_ERROR PeerTCPParamsStorage::SaveTrackedNodesList(const TrackedNodesList & list)
{
    std::array<uint8_t, MaxNodesListSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < list.mSize; ++i)
    {
        TLV::TLVType innerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, innerType));
        ReturnErrorOnFailure(writer.Put(kFabricIndexTag, list.mNodes[i].GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, list.mNodes[i].GetNodeId()));
        ReturnErrorOnFailure(writer.EndContainer(innerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    return mTCPParamsStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TCPPeerList().KeyName(), buf.data(),
                                              static_cast<uint16_t>(len));
}

CHIP_ERROR PeerTCPParamsStorage::LoadTrackedNodesList(TrackedNodesList & list)
{
    std::array<uint8_t, MaxNodesListSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    if (mTCPParamsStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::TCPPeerList().KeyName(), buf.data(), len) != CHIP_NO_ERROR)
    {
        list.mSize = 0;
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
        if (count >= ArraySize(list.mNodes))
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

        list.mNodes[count++] = ScopedNodeId(peerNodeId, fabricIndex);

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    list.mSize = count;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PeerTCPParamsStorage::SaveTCPParams(const ScopedNodeId & node, const uint16_t & supportedTransports,
                                               const uint32_t & maxTCPMessageSize)
{
    TrackedNodesList list;
    ReturnErrorOnFailure(LoadTrackedNodesList(list));

    for (size_t i = 0; i < list.mSize; ++i)
    {
        if (list.mNodes[i] == node)
        {
            // Node already exists in the list. Over-write new values.
            ReturnErrorOnFailure(SaveTCPParamsToStorage(node, supportedTransports, maxTCPMessageSize));
            return CHIP_NO_ERROR;
        }
    }

    // Not found in current list. If list is full, evict first one.
    if (list.mSize == kMaxTrackedNodes)
    {
        ReturnErrorOnFailure(DeleteTCPParamsFromStorage(list.mNodes[0]));
        ReturnErrorOnFailure(LoadTrackedNodesList(list));
    }

    ReturnErrorOnFailure(SaveTCPParamsToStorage(node, supportedTransports, maxTCPMessageSize));

    list.mNodes[list.mSize++] = node;

    // Save back the tracked nodes list
    return SaveTrackedNodesList(list);
}

CHIP_ERROR PeerTCPParamsStorage::DeleteTCPParams(const ScopedNodeId & node)
{
    TrackedNodesList list;
    ReturnErrorOnFailure(LoadTrackedNodesList(list));

    CHIP_ERROR err = DeleteTCPParamsFromStorage(node);
    if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // If unable to delete storage state, log error and move forward with attempt
        // to remove node from tracked nodes list.
        ChipLogError(SecureChannel,
                     "Unable to delete TCP params info from storage for node " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(node.GetNodeId()), err.Format());
    }

    size_t indexToRemove;
    for (indexToRemove = 0; indexToRemove < list.mSize; indexToRemove++)
    {
        if (list.mNodes[indexToRemove] == node)
        {
            break;
        }
    }

    if (indexToRemove >= list.mSize)
    {
        ChipLogError(SecureChannel,
                     "Unable to find TCP params info for node in tracked nodes list" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(node.GetNodeId()), err.Format());
        return CHIP_NO_ERROR;
    }

    // Shift elements to the left
    for (size_t j = indexToRemove; j < list.mSize - 1; j++)
    {
        list.mNodes[j] = list.mNodes[j + 1];
    }

    list.mSize -= 1;

    // Save back the tracked nodes list
    return SaveTrackedNodesList(list);
}

CHIP_ERROR PeerTCPParamsStorage::DeleteAllTCPParams(FabricIndex fabricIndex)
{
    CHIP_ERROR firstErr = CHIP_NO_ERROR;
    size_t deletedCount = 0;
    TrackedNodesList list;
    // Load the current list of nodes with stored TCP params
    ReturnErrorOnFailure(LoadTrackedNodesList(list));
    size_t initialSize = list.mSize;

    for (size_t i = 0; i < initialSize; ++i)
    {
        CHIP_ERROR err   = CHIP_NO_ERROR;
        size_t cur       = i - deletedCount;
        size_t remaining = initialSize - i;
        uint16_t supportedTransports;
        uint32_t maxTCPMessageSize;
        // Skip the nodes outside the given fabric
        if (list.mNodes[cur].GetFabricIndex() != fabricIndex)
        {
            continue;
        }
        err      = LoadTCPParamsFromStorage(list.mNodes[cur], supportedTransports, maxTCPMessageSize);
        firstErr = (firstErr == CHIP_NO_ERROR) ? err : firstErr;
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel,
                         "TCP params cache deletion partially failed for fabric index %u, "
                         "unable to load node state: %" CHIP_ERROR_FORMAT,
                         fabricIndex, err.Format());
            continue;
        }
        err      = DeleteTCPParamsFromStorage(list.mNodes[cur]);
        firstErr = firstErr == CHIP_NO_ERROR ? err : firstErr;
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel,
                         "TCP params cache is in an inconsistent state!  "
                         "Unable to delete node state during attempted deletion of fabric index %u: %" CHIP_ERROR_FORMAT,
                         fabricIndex, err.Format());
            continue;
        }
        ++deletedCount;
        --remaining;
        if (remaining)
        {
            // Shift nodes in the tracked list after the deletion of one.
            memmove(&list.mNodes[cur], &list.mNodes[cur + 1], remaining * sizeof(list.mNodes[0]));
        }
    }
    if (deletedCount)
    {
        // Adjust the tracked nodes list by the number of nodes deleted.
        list.mSize -= deletedCount;
        CHIP_ERROR err = SaveTrackedNodesList(list);
        firstErr       = (firstErr == CHIP_NO_ERROR) ? err : firstErr;
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel,
                         "TCP params cache is in an inconsistent state!  "
                         "Unable to save tracked nodes list during attempted deletion of fabric index %u: %" CHIP_ERROR_FORMAT,
                         fabricIndex, err.Format());
        }
    }

    return firstErr;
}

CHIP_ERROR PeerTCPParamsStorage::SaveTCPParamsToStorage(const ScopedNodeId & node, const uint16_t & supportedTransports,
                                                        const uint32_t & maxTCPMessageSize)
{
    // Save TCP params into key: /f/<fabricIndex>/tcp/<nodeId>
    std::array<uint8_t, MaxTCPParamsInfoSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(kSupportedTransportsTag, supportedTransports));
    ReturnErrorOnFailure(writer.Put(kMaxTCPMessageSizeTag, maxTCPMessageSize));

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto len = writer.GetLengthWritten();
    VerifyOrDie(CanCastTo<uint16_t>(len));

    return mTCPParamsStorage->SyncSetKeyValue(GetStorageKey(node).KeyName(), buf.data(), static_cast<uint16_t>(len));
}

CHIP_ERROR PeerTCPParamsStorage::LoadTCPParamsFromStorage(const ScopedNodeId & node, uint16_t & supportedTransports,
                                                          uint32_t & maxTCPMessageSize)
{
    std::array<uint8_t, MaxTCPParamsInfoSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    ReturnErrorOnFailure(mTCPParamsStorage->SyncGetKeyValue(GetStorageKey(node).KeyName(), buf.data(), len));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ReturnErrorOnFailure(reader.Next(kSupportedTransportsTag));
    ReturnErrorOnFailure(reader.Get(supportedTransports));

    ReturnErrorOnFailure(reader.Next(kMaxTCPMessageSizeTag));
    ReturnErrorOnFailure(reader.Get(maxTCPMessageSize));

    ReturnErrorOnFailure(reader.ExitContainer(containerType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR PeerTCPParamsStorage::DeleteTCPParamsFromStorage(const ScopedNodeId & node)
{
    return mTCPParamsStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName());
}

} // namespace Transport
} // namespace chip
