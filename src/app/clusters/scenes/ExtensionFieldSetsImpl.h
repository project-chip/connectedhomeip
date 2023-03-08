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

#include <app/clusters/scenes/ExtensionFieldSets.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace scenes {

enum EFSTLVTag
{
    kTagEFSArrayContainer = 1,
    kTagEFSFieldNum       = 1,
    kTagEFSContainer,
    kTagEFSClusterID,
    kTagEFS,
};

using clusterId = chip::ClusterId;

struct ExtensionFieldsSet
{
    clusterId mID                              = kInvalidClusterId;
    uint8_t mBytesBuffer[kMaxFieldsPerCluster] = { 0 };
    uint8_t mUsedBytes                         = 0;

    ExtensionFieldsSet() = default;
    ExtensionFieldsSet(clusterId cmID, const uint8_t * data, uint8_t dataSize) : mID(cmID), mUsedBytes(dataSize)
    {
        if (dataSize <= kMaxFieldsPerCluster)
        {
            memcpy(mBytesBuffer, data, mUsedBytes);
        }
    }

    ExtensionFieldsSet(clusterId cmID, ByteSpan bytes) : mID(cmID), mUsedBytes(static_cast<uint8_t>(bytes.size()))
    {
        if (bytes.size() <= kMaxFieldsPerCluster)
        {
            memcpy(mBytesBuffer, bytes.data(), bytes.size());
        }
    }

    ~ExtensionFieldsSet() = default;

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kTagEFSContainer), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagEFSClusterID), static_cast<uint16_t>(this->mID)));
        ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kTagEFS), mBytesBuffer, mUsedBytes));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader)
    {
        ByteSpan buffer;
        TLV::TLVType container;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(kTagEFSContainer)));
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagEFSClusterID)));
        ReturnErrorOnFailure(reader.Get(this->mID));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagEFS)));
        ReturnErrorOnFailure(reader.Get(buffer));
        VerifyOrReturnError(buffer.size() <= kMaxFieldsPerCluster, CHIP_ERROR_BUFFER_TOO_SMALL);
        this->mUsedBytes = static_cast<uint8_t>(buffer.size());
        memcpy(this->mBytesBuffer, buffer.data(), this->mUsedBytes);

        return reader.ExitContainer(container);
    }

    void Clear()
    {
        this->mID = kInvalidClusterId;
        memset(this->mBytesBuffer, 0, kMaxFieldsPerCluster);
        this->mUsedBytes = 0;
    }

    bool IsEmpty() const { return (this->mUsedBytes == 0); }

    bool operator==(const ExtensionFieldsSet & other)
    {
        return (this->mID == other.mID && !memcmp(this->mBytesBuffer, other.mBytesBuffer, this->mUsedBytes) &&
                this->mUsedBytes == other.mUsedBytes);
    }
};

class ExtensionFieldSetsImpl : public ExtensionFieldSets
{
public:
    ExtensionFieldSetsImpl();
    ~ExtensionFieldSetsImpl() override{};

    // overrides
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override;
    void Clear() override;
    bool IsEmpty() const override { return (this->mFieldNum == 0); }
    uint8_t GetFieldNum() const override { return this->mFieldNum; };

    // implementation
    CHIP_ERROR InsertFieldSet(const ExtensionFieldsSet & field);
    CHIP_ERROR GetFieldSetAtPosition(ExtensionFieldsSet & field, uint8_t position);
    CHIP_ERROR RemoveFieldAtPosition(uint8_t position);

    bool operator==(const ExtensionFieldSetsImpl & other)
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            if (!(this->mEFS[i] == other.mEFS[i]))
            {
                return false;
            }
        }
        return true;
    }

    ExtensionFieldSetsImpl & operator=(const ExtensionFieldSetsImpl & other)
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            this->mEFS[i] = other.mEFS[i];
        }
        mFieldNum = other.mFieldNum;

        return *this;
    }

protected:
    ExtensionFieldsSet mEFS[kMaxClusterPerScenes];
    uint8_t mFieldNum = 0;
};
} // namespace scenes
} // namespace chip
