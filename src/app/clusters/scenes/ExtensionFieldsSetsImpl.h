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

#include <app/clusters/scenes/ExtensionFieldsSets.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace scenes {

using clusterId = chip::ClusterId;

struct ExtensionFieldsSet
{
    static constexpr TLV::Tag TagClusterId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagEFS() { return TLV::ContextTag(2); }
    clusterId mID                              = kInvalidClusterId;
    uint8_t mBytesBuffer[kMaxFieldsPerCluster] = { 0 };
    uint8_t mUsedBytes                         = 0;

    ExtensionFieldsSet() = default;
    ExtensionFieldsSet(clusterId cmID, uint8_t * data, uint8_t dataSize) : mID(cmID), mUsedBytes(dataSize)
    {
        if (dataSize <= kMaxFieldsPerCluster)
        {
            memcpy(mBytesBuffer, data, mUsedBytes);
        }
    }
    ~ExtensionFieldsSet() = default;

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagClusterId(), static_cast<uint16_t>(this->mID)));
        ReturnErrorOnFailure(writer.PutBytes(TagEFS(), mBytesBuffer, mUsedBytes));

        return writer.EndContainer(container);
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader)
    {
        ByteSpan buffer;
        TLV::TLVType container;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TagClusterId()));
        ReturnErrorOnFailure(reader.Get(this->mID));

        ReturnErrorOnFailure(reader.Next(TagEFS()));
        ReturnErrorOnFailure(reader.Get(buffer));
        if (buffer.size() > kMaxFieldsPerCluster)
        {
            this->mUsedBytes = kMaxFieldsPerCluster;
        }
        else
        {
            this->mUsedBytes = static_cast<uint8_t>(buffer.size());
        }
        memcpy(this->mBytesBuffer, buffer.data(), this->mUsedBytes);

        return reader.ExitContainer(container);
    }

    void Clear()
    {
        this->mID = kInvalidClusterId;
        memset(this->mBytesBuffer, 0, kMaxFieldsPerCluster);
        this->mUsedBytes = 0;
    }

    bool is_empty() const { return (this->mUsedBytes == 0); }

    bool operator==(const ExtensionFieldsSet & other)
    {
        return (this->mID == other.mID && !memcmp(this->mBytesBuffer, other.mBytesBuffer, this->mUsedBytes) &&
                this->mUsedBytes == other.mUsedBytes);
    }

    CHIP_ERROR operator=(const ExtensionFieldsSet & other)
    {
        if (other.mUsedBytes <= kMaxFieldsPerCluster)
        {
            memcpy(this->mBytesBuffer, other.mBytesBuffer, other.mUsedBytes);
        }
        else
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        this->mID        = other.mID;
        this->mUsedBytes = other.mUsedBytes;

        return CHIP_NO_ERROR;
    }
};

class ExtensionFieldsSetsImpl : public ExtensionFieldsSets
{
public:
    ExtensionFieldsSetsImpl();
    ~ExtensionFieldsSetsImpl() override{};

    // overrides
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override;
    void Clear() override;
    bool is_empty() const override;

    // implementation
    CHIP_ERROR insertField(ExtensionFieldsSet & field);
    CHIP_ERROR getFieldAtPosition(ExtensionFieldsSet & field, uint8_t position);
    CHIP_ERROR removeFieldAtPosition(uint8_t position);

    bool operator==(const ExtensionFieldsSetsImpl & other)
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

    CHIP_ERROR operator=(const ExtensionFieldsSetsImpl & other)
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            ReturnErrorOnFailure(this->mEFS[i] = other.mEFS[i]);
        }
        mFieldNum = other.mFieldNum;

        return CHIP_NO_ERROR;
    }

protected:
    static constexpr TLV::Tag TagFieldNum() { return TLV::ContextTag(1); }
    ExtensionFieldsSet mEFS[kMaxClusterPerScenes];
    uint8_t mFieldNum = 0;
};
} // namespace scenes
} // namespace chip
