/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        https://urldefense.com/v3/__http://www.apache.org/licenses/LICENSE-2.0__;!!N30Cs7Jr!UgbMbEQ59BIK-1Xslc7QXYm0lQBh92qA3ElecRe1CF_9YhXxbwPOZa6j4plru7B7kCJ7bKQgHxgQrket3-Dnk268sIdA7Qb8$
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
    clusterId ID                                                                    = kInvalidClusterId;
    uint8_t bytesBuffer[CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER] = { 0 };
    uint8_t usedBytes                                                               = 0;

    ExtensionFieldsSet() = default;
    ExtensionFieldsSet(clusterId cID, uint8_t * data, uint8_t dataSize) : ID(cID), usedBytes(dataSize)
    {
        if (dataSize <= CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER)
        {
            memcpy(bytesBuffer, data, usedBytes);
        }
    }
    ~ExtensionFieldsSet() = default;

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagClusterId(), static_cast<uint16_t>(this->ID)));
        ReturnErrorOnFailure(writer.PutBytes(TagEFS(), bytesBuffer, usedBytes));

        return writer.EndContainer(container);
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader)
    {
        ByteSpan buffer;
        TLV::TLVType container;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TagClusterId()));
        ReturnErrorOnFailure(reader.Get(this->ID));

        ReturnErrorOnFailure(reader.Next(TagEFS()));
        ReturnErrorOnFailure(reader.Get(buffer));
        if (buffer.size() > CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER)
        {
            this->usedBytes = CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER;
        }
        else
        {
            this->usedBytes = static_cast<uint8_t>(buffer.size());
        }
        memcpy(this->bytesBuffer, buffer.data(), this->usedBytes);
        // ReturnErrorOnFailure(reader.GetBytes(bytesBuffer, CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER));

        return reader.ExitContainer(container);
    }

    void Clear()
    {
        this->ID = kInvalidClusterId;
        memset(this->bytesBuffer, 0, CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER);
        this->usedBytes = 0;
    }

    bool is_empty() const { return (this->usedBytes == 0); }

    bool operator==(const ExtensionFieldsSet & other)
    {
        return (this->ID == other.ID && !memcmp(this->bytesBuffer, other.bytesBuffer, this->usedBytes) &&
                this->usedBytes == other.usedBytes);
    }

    CHIP_ERROR operator=(const ExtensionFieldsSet & other)
    {
        if (other.usedBytes <= CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER)
        {
            memcpy(this->bytesBuffer, other.bytesBuffer, other.usedBytes);
        }
        else
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        this->ID        = other.ID;
        this->usedBytes = other.usedBytes;

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
        for (uint8_t i = 0; i < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; i++)
        {
            if (!(this->EFS[i] == other.EFS[i]))
            {
                return false;
            }
        }
        return true;
    }

    CHIP_ERROR operator=(const ExtensionFieldsSetsImpl & other)
    {
        for (uint8_t i = 0; i < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; i++)
        {
            ReturnErrorOnFailure(this->EFS[i] = other.EFS[i]);
        }
        fieldNum = other.fieldNum;

        return CHIP_NO_ERROR;
    }

protected:
    static constexpr TLV::Tag TagFieldNum() { return TLV::ContextTag(1); }
    ExtensionFieldsSet EFS[CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES];
    uint8_t fieldNum = 0;
};
} // namespace scenes
} // namespace chip
