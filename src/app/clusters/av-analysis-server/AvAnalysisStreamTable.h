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
    uint16_t videoStreamID = 0;

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
     * Adds an entry in PendingInitiation state for a camera stream, generating a new AnalysisStreamID
     * Returns the new entry, or nullptr if the table is full or not initialized.
     */
    AnalysisStreamEntry * Add(uint16_t aVideoStreamId, const ScopedNodeId & aCameraNode)
    {
        VerifyOrReturnValue(mEntries.Get() != nullptr && !IsFull(), nullptr);

        AnalysisStreamEntry & entry = mEntries[mCount];
        entry                       = AnalysisStreamEntry{};
        entry.analysisStreamID      = GenerateAnalysisStreamId();
        entry.videoStreamID         = aVideoStreamId;
        entry.cameraNode            = aCameraNode;
        mCount++;
        return &entry;
    }

    /**
     * Finds the entry backed by the given camera stream, if any.
     */
    AnalysisStreamEntry * FindByCameraStream(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId)
    {
        for (uint8_t i = 0; i < mCount; i++)
        {
            if (mEntries[i].cameraNode == aCameraNode && mEntries[i].videoStreamID == aVideoStreamId)
            {
                return &mEntries[i];
            }
        }
        return nullptr;
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

    // Worst-case TLV size of one persisted entry (analysis id + camera node id + fabric + video id).
    static constexpr size_t kEntrySerializedSize =
        TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(NodeId), sizeof(FabricIndex), sizeof(uint16_t));

    // TLV overhead of the enclosing structure written by Encode(): the outer container, the
    // next-AnalysisStreamID field and the entry array container.
    static constexpr size_t kArraySerializedOverhead = 16;

    /**
     * Writes the next-AnalysisStreamID counter and the in-use entries as one anonymous TLV structure.
     */
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter) const
    {
        TLV::TLVType outerType;
        ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagNextStreamId), mNextAnalysisStreamId));

        TLV::TLVType arrayType;
        ReturnErrorOnFailure(aWriter.StartContainer(TLV::ContextTag(kPersistedTagEntries), TLV::kTLVType_Array, arrayType));
        for (const auto & entry : *this)
        {
            TLV::TLVType entryType;
            ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryType));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagStreamId), entry.analysisStreamID));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagCameraNodeId), entry.cameraNode.GetNodeId()));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagCameraFabric), entry.cameraNode.GetFabricIndex()));
            ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(kPersistedTagVideoStreamId), entry.videoStreamID));
            ReturnErrorOnFailure(aWriter.EndContainer(entryType));
        }
        ReturnErrorOnFailure(aWriter.EndContainer(arrayType));
        return aWriter.EndContainer(outerType);
    }

    /**
     * Replaces the table contents and the next-AnalysisStreamID counter from TLV written by Encode().
     * Restored entries restart in PendingInitiation state. On failure the table is left empty, never
     * half-loaded.
     */
    CHIP_ERROR Decode(TLV::TLVReader & aReader)
    {
        CHIP_ERROR err = DecodeContents(aReader);
        if (err != CHIP_NO_ERROR)
        {
            mCount                = 0;
            mNextAnalysisStreamId = 0;
        }
        return err;
    }

private:
    /**
     * Parses the next-AnalysisStreamID counter and the entry array, populating the table as it goes.
     * The caller rolls back on failure.
     */
    CHIP_ERROR DecodeContents(TLV::TLVReader & aReader)
    {
        ReturnErrorOnFailure(aReader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(aReader.EnterContainer(outerType));

        ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagNextStreamId)));
        ReturnErrorOnFailure(aReader.Get(mNextAnalysisStreamId));

        ReturnErrorOnFailure(aReader.Next(TLV::kTLVType_Array, TLV::ContextTag(kPersistedTagEntries)));
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
            uint16_t videoStreamId   = 0;
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagStreamId)));
            ReturnErrorOnFailure(aReader.Get(streamId));
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagCameraNodeId)));
            ReturnErrorOnFailure(aReader.Get(cameraNodeId));
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagCameraFabric)));
            ReturnErrorOnFailure(aReader.Get(cameraFabric));
            ReturnErrorOnFailure(aReader.Next(TLV::ContextTag(kPersistedTagVideoStreamId)));
            ReturnErrorOnFailure(aReader.Get(videoStreamId));
            ReturnErrorOnFailure(aReader.ExitContainer(entryType));

            VerifyOrReturnError(mEntries.Get() != nullptr && !IsFull(), CHIP_ERROR_NO_MEMORY);
            VerifyOrReturnError(Find(streamId) == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            AnalysisStreamEntry & entry = mEntries[mCount];
            entry                       = AnalysisStreamEntry{};
            entry.analysisStreamID      = streamId;
            entry.videoStreamID         = videoStreamId;
            entry.cameraNode            = ScopedNodeId(cameraNodeId, cameraFabric);
            mCount++;
        }
        VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

        ReturnErrorOnFailure(aReader.ExitContainer(arrayType));
        ReturnErrorOnFailure(aReader.ExitContainer(outerType));
        return CHIP_NO_ERROR;
    }

    // Context tags of the persisted blob written by Encode()
    static constexpr uint8_t kPersistedTagNextStreamId = 0;
    static constexpr uint8_t kPersistedTagEntries      = 1;
    // Context tags of one persisted entry
    static constexpr uint8_t kPersistedTagStreamId      = 0;
    static constexpr uint8_t kPersistedTagCameraNodeId  = 1;
    static constexpr uint8_t kPersistedTagCameraFabric  = 2;
    static constexpr uint8_t kPersistedTagVideoStreamId = 3;

    uint16_t GenerateAnalysisStreamId()
    {
        uint16_t id;
        do
        {
            id                    = mNextAnalysisStreamId;
            mNextAnalysisStreamId = (id >= kMaxAnalysisStreamId) ? 0 : static_cast<uint16_t>(id + 1);
        } while (Find(id) != nullptr);
        return id;
    }

    static constexpr uint16_t kMaxAnalysisStreamId = 65534;

    Platform::ScopedMemoryBuffer<AnalysisStreamEntry> mEntries;
    uint8_t mCapacity              = 0;
    uint8_t mCount                 = 0;
    uint16_t mNextAnalysisStreamId = 0;
};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
