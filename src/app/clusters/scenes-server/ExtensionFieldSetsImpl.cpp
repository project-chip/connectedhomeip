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

CHIP_ERROR ExtensionFieldSetsImpl::Serialize(TLV::TLVWriter & writer) const
{
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(
        writer.StartContainer(TLV::ContextTag(TagEFS::kFieldSetArrayContainer), TLV::kTLVType_Array, arrayContainer));
    for (uint8_t i = 0; i < mFieldSetsCount; i++)
    {
        ReturnErrorOnFailure(mFieldSets[i].Serialize(writer));
    }

    return writer.EndContainer(arrayContainer);
}

CHIP_ERROR ExtensionFieldSetsImpl::Deserialize(TLV::TLVReader & reader)
{
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagEFS::kFieldSetArrayContainer)));
    ReturnErrorOnFailure(reader.EnterContainer(arrayContainer));

    uint8_t i = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR && i < kMaxClustersPerScene)
    {
        ReturnErrorOnFailure(mFieldSets[i].Deserialize(reader));
        i++;
    }
    mFieldSetsCount = i;

    // In the event of an OTA where the maximum number of clusters per scene has been reduced, the extension field set will be
    // considered "corrupted" if we don't manage to load it all (if err == CHIP_NO_ERROR after the loop). We therefore return an
    // error and this scene will have to be deleted. This is done because truncating an EFS doesn't guarantee the order of the
    // clusters loaded, which might lead to loading clusters that are no longer supported and losing supported ones.
    if (err != CHIP_END_OF_TLV)
    {
        if (err == CHIP_NO_ERROR)
            return CHIP_ERROR_BUFFER_TOO_SMALL;

        return err;
    }

    return reader.ExitContainer(arrayContainer);
}

void ExtensionFieldSetsImpl::Clear()
{
    for (uint8_t i = 0; i < mFieldSetsCount; i++)
    {
        mFieldSets[i].Clear();
    }

    mFieldSetsCount = 0;
}

/// @brief Inserts a field Set set into the array of extension field Set sets for a scene entry.
///        If the same ID is present in the EFS array, it will overwrite it.
/// @param fieldSet field set to be inserted
/// @return CHIP_NO_ERROR if insertion worked, CHIP_ERROR_NO_MEMORY if the array is already full
CHIP_ERROR ExtensionFieldSetsImpl::InsertFieldSet(const ExtensionFieldSet & fieldSet)
{
    uint8_t firstEmptyPosition = kInvalidPosition;

    VerifyOrReturnError(fieldSet.mID != kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!fieldSet.IsEmpty(), CHIP_ERROR_INVALID_ARGUMENT);

    for (uint8_t i = 0; i < kMaxClustersPerScene; i++)
    {
        if (mFieldSets[i].mID == fieldSet.mID)
        {
            mFieldSets[i] = fieldSet;
            return CHIP_NO_ERROR;
        }

        if (mFieldSets[i].IsEmpty() && firstEmptyPosition == kInvalidPosition)
        {
            firstEmptyPosition = i;
        }
    }

    // if found, replace at found position, otherwise insert at first free position, otherwise return error
    if (firstEmptyPosition < kMaxClustersPerScene)
    {
        mFieldSets[firstEmptyPosition] = fieldSet;
        mFieldSetsCount++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR ExtensionFieldSetsImpl::GetFieldSetAtPosition(ExtensionFieldSet & fieldSet, uint8_t position) const
{
    VerifyOrReturnError(position < mFieldSetsCount, CHIP_ERROR_INVALID_ARGUMENT);

    fieldSet = mFieldSets[position];

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtensionFieldSetsImpl::RemoveFieldAtPosition(uint8_t position)
{
    VerifyOrReturnValue(position < mFieldSetsCount, CHIP_NO_ERROR);

    uint8_t nextPos = static_cast<uint8_t>(position + 1);
    uint8_t moveNum = static_cast<uint8_t>(kMaxClustersPerScene - nextPos);

    // TODO: Implement general array management methods
    // Compress array after removal, if the removed position is not the last
    if (moveNum)
    {
        memmove(&mFieldSets[position], &mFieldSets[nextPos], sizeof(ExtensionFieldSet) * moveNum);
    }

    mFieldSetsCount--;
    // Clear last occupied position
    mFieldSets[mFieldSetsCount].Clear();

    return CHIP_NO_ERROR;
}

} // namespace scenes

} // namespace chip
