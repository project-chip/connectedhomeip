/*
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

#include <app/clusters/scenes/ExtensionFieldsSetsImpl.h>

namespace chip {
namespace scenes {

ExtensionFieldsSetsImpl::ExtensionFieldsSetsImpl() : ExtensionFieldsSets() {}

CHIP_ERROR ExtensionFieldsSetsImpl::Serialize(TLV::TLVWriter & writer) const
{
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TagFieldNum(), static_cast<uint8_t>(this->mFieldNum)));
    if (!this->is_empty())
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            if (!this->mEFS[i].is_empty())
            {
                LogErrorOnFailure(this->mEFS[i].Serialize(writer));
            }
        }
    }

    return writer.EndContainer(container);
}

CHIP_ERROR ExtensionFieldsSetsImpl::Deserialize(TLV::TLVReader & reader)
{
    TLV::TLVType container;
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
    ReturnErrorOnFailure(reader.EnterContainer(container));

    ReturnErrorOnFailure(reader.Next(TagFieldNum()));
    ReturnErrorOnFailure(reader.Get(this->mFieldNum));

    if (!this->is_empty())
    {
        for (uint8_t i = 0; i < this->mFieldNum; i++)
        {
            ReturnErrorOnFailure(this->mEFS[i].Deserialize(reader));
        }
    }

    return reader.ExitContainer(container);
}
void ExtensionFieldsSetsImpl::Clear()
{
    if (!this->is_empty())
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            this->mEFS[i].Clear();
        }
    }
    this->mFieldNum = 0;
}

bool ExtensionFieldsSetsImpl::is_empty() const
{
    return (this->mFieldNum <= 0);
}

/// @brief Inserts a field set into the array of extension field sets for a scene entry if the same ID is present in the EFS array,
/// it will overwrite it
/// @param field field set to be inserted
/// @return CHIP_NO_ERROR if insertion worked, CHIP_ERROR_BUFFER_TOO_SMALL if the array is already full
CHIP_ERROR ExtensionFieldsSetsImpl::insertField(ExtensionFieldsSet & field)
{
    CHIP_ERROR err     = CHIP_ERROR_INVALID_LIST_LENGTH;
    uint8_t idPosition = 0xff, fisrtEmptyPosition = 0xff;
    for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
    {
        if (this->mEFS[i].mID == field.mID)
        {
            idPosition = i;
            break;
        }

        if (this->mEFS[i].is_empty() && fisrtEmptyPosition == 0xFF)
        {
            fisrtEmptyPosition = i;
        }
    }

    // if found, insert at found position, otherwise at first free possition, otherwise return error
    if (idPosition < kMaxClusterPerScenes)
    {
        ReturnErrorOnFailure(this->mEFS[idPosition] = field);
        err = CHIP_NO_ERROR;
    }
    else if (fisrtEmptyPosition < kMaxClusterPerScenes)
    {
        ReturnErrorOnFailure(this->mEFS[fisrtEmptyPosition] = field);
        this->mFieldNum++;
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR ExtensionFieldsSetsImpl::getFieldAtPosition(ExtensionFieldsSet & field, uint8_t position)
{
    if (position < kMaxClusterPerScenes)
    {
        ReturnErrorOnFailure(field = this->mEFS[position]);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtensionFieldsSetsImpl::removeFieldAtPosition(uint8_t position)
{
    if (!this->is_empty())
    {
        if (position < kMaxClusterPerScenes)
        {
            if (!this->mEFS[position].is_empty())
            {
                this->mEFS[position].Clear();
                this->mFieldNum--;
            }
        }
        else
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace scenes

} // namespace chip
