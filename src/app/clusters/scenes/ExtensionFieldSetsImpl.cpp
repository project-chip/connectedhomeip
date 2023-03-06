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

#include "ExtensionFieldSetsImpl.h"

namespace chip {
namespace scenes {

ExtensionFieldSetsImpl::ExtensionFieldSetsImpl() : ExtensionFieldsSets() {}

CHIP_ERROR ExtensionFieldSetsImpl::Serialize(TLV::TLVWriter & writer) const
{
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kTagEFSArrayContainer), TLV::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagEFSFieldNum), static_cast<uint8_t>(this->mFieldNum)));
    if (!this->IsEmpty())
    {
        for (uint8_t i = 0; i < this->mFieldNum; i++)
        {
            if (!this->mEFS[i].IsEmpty())
            {
                ReturnErrorOnFailure(this->mEFS[i].Serialize(writer));
            }
        }
    }

    return writer.EndContainer(container);
}

CHIP_ERROR ExtensionFieldSetsImpl::Deserialize(TLV::TLVReader & reader)
{
    TLV::TLVType container;
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(kTagEFSArrayContainer)));
    ReturnErrorOnFailure(reader.EnterContainer(container));

    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagEFSFieldNum)));
    ReturnErrorOnFailure(reader.Get(this->mFieldNum));

    if (!this->IsEmpty())
    {
        for (uint8_t i = 0; i < this->mFieldNum; i++)
        {
            ReturnErrorOnFailure(this->mEFS[i].Deserialize(reader));
        }
    }

    return reader.ExitContainer(container);
}

void ExtensionFieldSetsImpl::Clear()
{
    if (!this->IsEmpty())
    {
        for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
        {
            this->mEFS[i].Clear();
        }
    }
    this->mFieldNum = 0;
}

/// @brief Inserts a field Set set into the array of extension field Set sets for a scene entry.
///        If the same ID is present in the EFS array, it will overwrite it.
/// @param fieldSet field set to be inserted
/// @return CHIP_NO_ERROR if insertion worked, CHIP_ERROR_NO_MEMORY if the array is already full
CHIP_ERROR ExtensionFieldSetsImpl::InsertFieldSet(ExtensionFieldsSet & fieldSet)
{
    CHIP_ERROR err             = CHIP_ERROR_NO_MEMORY;
    uint8_t idPosition         = kInvalidPosition;
    uint8_t firstEmptyPosition = kInvalidPosition;

    VerifyOrReturnError(fieldSet.mID != kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);

    for (uint8_t i = 0; i < kMaxClusterPerScenes; i++)
    {
        if (this->mEFS[i].mID == fieldSet.mID)
        {
            idPosition = i;
            break;
        }

        if (this->mEFS[i].IsEmpty() && firstEmptyPosition == 0xFF)
        {
            firstEmptyPosition = i;
        }
    }

    // if found, replace at found position, otherwise at insert first free position, otherwise return error
    if (idPosition < kMaxClusterPerScenes)
    {
        this->mEFS[idPosition] = fieldSet;
        err                    = CHIP_NO_ERROR;
    }
    else if (firstEmptyPosition < kMaxClusterPerScenes)
    {
        this->mEFS[firstEmptyPosition] = fieldSet;
        this->mFieldNum++;
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR ExtensionFieldSetsImpl::GetFieldSetAtPosition(ExtensionFieldsSet & fieldSet, uint8_t position)
{
    VerifyOrReturnError(position < this->mFieldNum, CHIP_ERROR_BUFFER_TOO_SMALL);

    fieldSet = this->mEFS[position];

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtensionFieldSetsImpl::RemoveFieldAtPosition(uint8_t position)
{
    VerifyOrReturnError(position < kMaxClusterPerScenes, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(!this->IsEmpty() && !this->mEFS[position].IsEmpty(), CHIP_NO_ERROR);

    uint8_t nextPos = static_cast<uint8_t>(position + 1);
    uint8_t moveNum = static_cast<uint8_t>(kMaxClusterPerScenes - nextPos);

    // TODO: Implement general array management methods
    // Compress array after removal
    memmove(&this->mEFS[position], &this->mEFS[nextPos], sizeof(ExtensionFieldsSet) * moveNum);

    this->mFieldNum--;
    // Clear last occupied position
    this->mEFS[mFieldNum].Clear(); //

    return CHIP_NO_ERROR;
}

} // namespace scenes

} // namespace chip
