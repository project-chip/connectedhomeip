/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstddef>
#include <cstdint>

#include <app/data-model/Decode.h>
#include <clusters/AvAnalysis/Enums.h>
#include <clusters/AvAnalysis/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

/**
 * Backing storage for one entry of the AnalysisStreams attribute (AnalysisStreamStruct).
 */
struct AnalysisStreamEntry
{
    uint16_t analysisStreamID = 0;
    DataModel::Nullable<EndpointId> webRTCEndpointID;
    DataModel::Nullable<EndpointId> pushAVEndpointID;
    AnalysisStreamStateEnum state = AnalysisStreamStateEnum::kPendingInitiation;

    ScopedNodeId cameraNode;

    Structs::AnalysisStreamStruct::Type ToEncodableStruct() const
    {
        Structs::AnalysisStreamStruct::Type encodable;
        encodable.analysisStreamID    = analysisStreamID;
        encodable.webRTCEndpointID    = MakeOptional(webRTCEndpointID);
        encodable.pushAVEndpointID    = MakeOptional(pushAVEndpointID);
        encodable.analysisStreamState = state;
        return encodable;
    }
};

/**
 * Fixed-capacity table backing the AnalysisStreams attribute. Capacity is allocated once at Init()
 * and bounded by MaxAnalysisStreamCount; no reallocation happens afterwards.
 */
class AnalysisStreamTable
{
public:
    AnalysisStreamTable() = default;

    /**
     * Allocates storage for at most aCapacity entries. Must be called exactly once before any other use.
     */
    CHIP_ERROR Init(uint8_t aCapacity)
    {
        VerifyOrReturnError(mEntries.Get() == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(aCapacity > 0, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(mEntries.Calloc(aCapacity), CHIP_ERROR_NO_MEMORY);
        mCapacity = aCapacity;
        return CHIP_NO_ERROR;
    }

    uint8_t Capacity() const { return mCapacity; }
    uint8_t Count() const { return mCount; }
    bool IsFull() const { return mCount == mCapacity; }

    /**
     * Adds an entry in PendingInitiation state for the given AnalysisStreamID. The id is the VideoStreamID
     * returned by the camera's VideoStreamAllocate command (spec: AnalysisStreamID field), not locally minted.
     * Returns the new entry, or nullptr if the table is full, not initialized, or the id is already present.
     */
    AnalysisStreamEntry * Add(uint16_t aAnalysisStreamId)
    {
        VerifyOrReturnValue(mEntries.Get() != nullptr && !IsFull(), nullptr);
        VerifyOrReturnValue(Find(aAnalysisStreamId) == nullptr, nullptr);

        AnalysisStreamEntry & entry = mEntries[mCount];
        entry                       = AnalysisStreamEntry{};
        entry.analysisStreamID      = aAnalysisStreamId;
        mCount++;
        return &entry;
    }

    AnalysisStreamEntry * Find(uint16_t aAnalysisStreamId)
    {
        for (uint8_t i = 0; i < mCount; i++)
        {
            if (mEntries[i].analysisStreamID == aAnalysisStreamId)
            {
                return &mEntries[i];
            }
        }
        return nullptr;
    }

    /**
     * Removes the entry with the given id, if present. Returns true if an entry was removed.
     */
    bool Remove(uint16_t aAnalysisStreamId)
    {
        for (uint8_t i = 0; i < mCount; i++)
        {
            if (mEntries[i].analysisStreamID == aAnalysisStreamId)
            {
                // Keep entries dense; ordering of the AnalysisStreams list is not specified.
                mEntries[i] = mEntries[mCount - 1];
                mCount--;
                return true;
            }
        }
        return false;
    }

    const AnalysisStreamEntry & operator[](uint8_t aIndex) const { return mEntries[aIndex]; }
    AnalysisStreamEntry & operator[](uint8_t aIndex) { return mEntries[aIndex]; }

    // Iteration over the in-use entries
    AnalysisStreamEntry * begin() { return mEntries.Get(); }
    AnalysisStreamEntry * end() { return mEntries.Get() + mCount; }
    const AnalysisStreamEntry * begin() const { return mEntries.Get(); }
    const AnalysisStreamEntry * end() const { return mEntries.Get() + mCount; }

    // Worst-case TLV size of one persisted entry (id + camera node id + fabric index).
    static constexpr size_t kEntrySerializedSize =
        TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(NodeId), sizeof(FabricIndex));

    // TLV overhead of the enclosing anonymous array container written by Encode().
    static constexpr size_t kArraySerializedOverhead = 4;

    /**
     * Writes the in-use entries as an anonymous TLV array of {AnalysisStreamID, camera NodeId, fabric}.
     */
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter) const
    {
        TLV::TLVType arrayType;
        ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
        for (const auto & entry : *this)
        {
            TLV::TLVType entryType;
            ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryType));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagStreamId), entry.analysisStreamID));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagCameraNodeId), entry.cameraNode.GetNodeId()));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagCameraFabric), entry.cameraNode.GetFabricIndex()));
            ReturnErrorOnFailure(aWriter.EndContainer(entryType));
        }
        return aWriter.EndContainer(arrayType);
    }

    /**
     * Replaces the table contents from a TLV array written by Encode(). Restored entries restart in
     * PendingInitiation state.
     */
    CHIP_ERROR Decode(TLV::TLVReader & aReader)
    {
        ReturnErrorOnFailure(aReader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
        TLV::TLVType arrayType;
        ReturnErrorOnFailure(aReader.EnterContainer(arrayType));

        mCount = 0;
        CHIP_ERROR err;
        while ((err = aReader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
        {
            TLV::TLVType entryType;
            ReturnErrorOnFailure(aReader.EnterContainer(entryType));

            uint16_t streamId        = 0;
            NodeId cameraNodeId      = kUndefinedNodeId;
            FabricIndex cameraFabric = kUndefinedFabricIndex;
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagStreamId)));
            ReturnErrorOnFailure(aReader.Get(streamId));
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagCameraNodeId)));
            ReturnErrorOnFailure(aReader.Get(cameraNodeId));
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagCameraFabric)));
            ReturnErrorOnFailure(aReader.Get(cameraFabric));
            ReturnErrorOnFailure(aReader.ExitContainer(entryType));

            AnalysisStreamEntry * entry = Add(streamId);
            VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NO_MEMORY);
            entry->cameraNode = ScopedNodeId(cameraNodeId, cameraFabric);
        }
        VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

        ReturnErrorOnFailure(aReader.ExitContainer(arrayType));
        return CHIP_NO_ERROR;
    }

private:
    // Context tags of one persisted entry written by Encode()
    static constexpr uint8_t kPersistedTagStreamId     = 0;
    static constexpr uint8_t kPersistedTagCameraNodeId = 1;
    static constexpr uint8_t kPersistedTagCameraFabric = 2;

    Platform::ScopedMemoryBuffer<AnalysisStreamEntry> mEntries;
    uint8_t mCapacity = 0;
    uint8_t mCount    = 0;
};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
