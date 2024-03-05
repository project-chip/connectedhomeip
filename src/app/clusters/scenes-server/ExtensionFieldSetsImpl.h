/**
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

#pragma once

#include <app/clusters/scenes-server/ExtensionFieldSets.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace scenes {

/// @brief Tags Used to serialize Extension Field Sets struct as well as individual field sets.
/// kFieldSetArrayContainer: Tag for the container of the EFS array
/// kClusterID: Tag for the ClusterID of a field set
/// kClusterFieldSetData: Tag for the serialized field set data
enum class TagEFS : uint8_t
{
    kFieldSetArrayContainer = 1,
    kClusterID,
    kClusterFieldSetData,
};

/// @brief Struct to serialize and de serialize a cluster extension field set
/// mID: Cluster ID, allows to identify which cluster is serialized
/// mBytesBuffer: Field ID serialized into a byte array
/// mUsedBytes: Number of bytes in the Buffer containing data, used for serializing only those bytes.
struct ExtensionFieldSet
{
    ClusterId mID                                  = kInvalidClusterId;
    uint8_t mBytesBuffer[kMaxFieldBytesPerCluster] = { 0 };
    uint8_t mUsedBytes                             = 0;

    ExtensionFieldSet() = default;
    ExtensionFieldSet(ClusterId cmID, const uint8_t * data, uint8_t dataSize) : mID(cmID), mUsedBytes(dataSize)
    {
        if (dataSize <= sizeof(mBytesBuffer))
        {
            memcpy(mBytesBuffer, data, mUsedBytes);
        }
        else
        {
            mUsedBytes = 0;
        }
    }

    ExtensionFieldSet(ClusterId cmID, ByteSpan bytes) : mID(cmID), mUsedBytes(static_cast<uint8_t>(bytes.size()))
    {
        if (bytes.size() <= sizeof(mBytesBuffer))
        {
            memcpy(mBytesBuffer, bytes.data(), bytes.size());
        }
        else
        {
            mUsedBytes = 0;
        }
    }

    ~ExtensionFieldSet() = default;

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagEFS::kClusterID), mID));
        ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(TagEFS::kClusterFieldSetData), mBytesBuffer, mUsedBytes));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader)
    {
        ByteSpan buffer;
        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEFS::kClusterID)));
        ReturnErrorOnFailure(reader.Get(mID));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEFS::kClusterFieldSetData)));
        ReturnErrorOnFailure(reader.Get(buffer));
        VerifyOrReturnError(buffer.size() <= sizeof(mBytesBuffer), CHIP_ERROR_BUFFER_TOO_SMALL);
        mUsedBytes = static_cast<decltype(mUsedBytes)>(buffer.size());
        memcpy(mBytesBuffer, buffer.data(), mUsedBytes);

        return reader.ExitContainer(container);
    }

    void Clear()
    {
        mID = kInvalidClusterId;
        memset(mBytesBuffer, 0, kMaxFieldBytesPerCluster);
        mUsedBytes = 0;
    }

    bool IsEmpty() const { return (mUsedBytes == 0); }

    bool operator==(const ExtensionFieldSet & other) const
    {
        return (mID == other.mID && mUsedBytes == other.mUsedBytes && !memcmp(mBytesBuffer, other.mBytesBuffer, mUsedBytes));
    }
};

class ExtensionFieldSetsImpl : public ExtensionFieldSets
{
public:
    ExtensionFieldSetsImpl(){};
    ~ExtensionFieldSetsImpl() override{};

    // overrides
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override;
    void Clear() override;
    bool IsEmpty() const override { return (mFieldSetsCount == 0); }
    uint8_t GetFieldSetCount() const override { return mFieldSetsCount; };

    CHIP_ERROR InsertFieldSet(const ExtensionFieldSet & field);
    CHIP_ERROR GetFieldSetAtPosition(ExtensionFieldSet & field, uint8_t position) const;
    CHIP_ERROR RemoveFieldAtPosition(uint8_t position);

    // implementation
    bool operator==(const ExtensionFieldSetsImpl & other) const
    {
        if (this->mFieldSetsCount != other.mFieldSetsCount)
        {
            return false;
        }

        for (uint8_t i = 0; i < mFieldSetsCount; i++)
        {
            if (!(this->mFieldSets[i] == other.mFieldSets[i]))
            {
                return false;
            }
        }
        return true;
    }

    ExtensionFieldSetsImpl & operator=(const ExtensionFieldSetsImpl & other)
    {
        for (uint8_t i = 0; i < other.mFieldSetsCount; i++)
        {
            this->mFieldSets[i] = other.mFieldSets[i];
        }
        mFieldSetsCount = other.mFieldSetsCount;

        for (uint8_t i = mFieldSetsCount; i < kMaxClustersPerScene; i++)
        {
            this->mFieldSets[i].Clear();
        }

        return *this;
    }

protected:
    ExtensionFieldSet mFieldSets[kMaxClustersPerScene];
    uint8_t mFieldSetsCount = 0;
};
} // namespace scenes
} // namespace chip
