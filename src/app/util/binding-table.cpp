/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file Basic implementation of a binding table.
 */

#include <app/util/binding-table.h>
#include <app/util/config.h>

namespace chip {

BindingTable BindingTable::sInstance;

BindingTable::BindingTable()
{
    memset(mNextIndex, kNextNullIndex, sizeof(mNextIndex));
}

CHIP_ERROR BindingTable::Add(const EmberBindingTableEntry & entry)
{
    if (entry.type == MATTER_UNUSED_BINDING)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t newIndex = GetNextAvaiableIndex();
    if (newIndex >= MATTER_BINDING_TABLE_SIZE)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    mBindingTable[newIndex] = entry;
    CHIP_ERROR error        = SaveEntryToStorage(newIndex, kNextNullIndex);
    if (error == CHIP_NO_ERROR)
    {
        if (mTail == kNextNullIndex)
        {
            error = SaveListInfo(newIndex);
        }
        else
        {
            error = SaveEntryToStorage(mTail, newIndex);
        }
        if (error != CHIP_NO_ERROR)
        {
            mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(newIndex).KeyName());
        }
    }
    if (error != CHIP_NO_ERROR)
    {
        // Roll back
        mBindingTable[newIndex].type = MATTER_UNUSED_BINDING;
        return error;
    }

    if (mTail == kNextNullIndex)
    {
        mTail = newIndex;
        mHead = newIndex;
    }
    else
    {
        mNextIndex[mTail]    = newIndex;
        mNextIndex[newIndex] = kNextNullIndex;
        mTail                = newIndex;
    }

    mSize++;
    return CHIP_NO_ERROR;
}

const EmberBindingTableEntry & BindingTable::GetAt(uint8_t index)
{
    return mBindingTable[index];
}

CHIP_ERROR BindingTable::SaveEntryToStorage(uint8_t index, uint8_t nextIndex)
{
    EmberBindingTableEntry & entry    = mBindingTable[index];
    uint8_t buffer[kEntryStorageSize] = { 0 };
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::TLVType::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagFabricIndex), entry.fabricIndex));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagLocalEndpoint), entry.local));
    if (entry.clusterId.has_value())
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagCluster), *entry.clusterId));
    }
    if (entry.type == MATTER_UNICAST_BINDING)
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagRemoteEndpoint), entry.remote));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagNodeId), entry.nodeId));
    }
    else
    {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagGroupId), entry.groupId));
    }
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagNextEntry), nextIndex));
    ReturnErrorOnFailure(writer.EndContainer(container));
    ReturnErrorOnFailure(writer.Finalize());
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(index).KeyName(), buffer,
                                     static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR BindingTable::SaveListInfo(uint8_t head)
{
    uint8_t buffer[kListInfoStorageSize] = { 0 };
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::TLVType::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagStorageVersion), kStorageVersion));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagHead), head));
    ReturnErrorOnFailure(writer.EndContainer(container));
    ReturnErrorOnFailure(writer.Finalize());
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::BindingTable().KeyName(), buffer,
                                     static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR BindingTable::LoadFromStorage()
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint8_t buffer[kListInfoStorageSize] = { 0 };
    uint16_t size                        = sizeof(buffer);
    CHIP_ERROR error;

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::BindingTable().KeyName(), buffer, size));
    TLV::TLVReader reader;
    reader.Init(buffer, size);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType container;
    ReturnErrorOnFailure(reader.EnterContainer(container));

    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagStorageVersion)));
    uint32_t version;
    ReturnErrorOnFailure(reader.Get(version));
    VerifyOrReturnError(version == kStorageVersion, CHIP_ERROR_VERSION_MISMATCH);
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagHead)));
    uint8_t index;
    ReturnErrorOnFailure(reader.Get(index));
    mHead = index;
    while (index != kNextNullIndex)
    {
        uint8_t nextIndex;
        error = LoadEntryFromStorage(index, nextIndex);
        if (error != CHIP_NO_ERROR)
        {
            mHead = kNextNullIndex;
            mTail = kNextNullIndex;
            return error;
        }
        mTail = index;
        index = nextIndex;
        mSize++;
    }
    error = reader.ExitContainer(container);
    if (error != CHIP_NO_ERROR)
    {
        mHead = kNextNullIndex;
        mTail = kNextNullIndex;
    }
    return error;
}

CHIP_ERROR BindingTable::LoadEntryFromStorage(uint8_t index, uint8_t & nextIndex)
{
    uint8_t buffer[kEntryStorageSize] = { 0 };
    uint16_t size                     = sizeof(buffer);
    EmberBindingTableEntry entry;

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(index).KeyName(), buffer, size));
    TLV::TLVReader reader;
    reader.Init(buffer, size);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType container;
    ReturnErrorOnFailure(reader.EnterContainer(container));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagFabricIndex)));
    ReturnErrorOnFailure(reader.Get(entry.fabricIndex));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagLocalEndpoint)));
    ReturnErrorOnFailure(reader.Get(entry.local));
    ReturnErrorOnFailure(reader.Next());
    if (reader.GetTag() == TLV::ContextTag(kTagCluster))
    {
        ClusterId clusterId;
        ReturnErrorOnFailure(reader.Get(clusterId));
        entry.clusterId.emplace(clusterId);
        ReturnErrorOnFailure(reader.Next());
    }
    else
    {
        entry.clusterId = std::nullopt;
    }
    if (reader.GetTag() == TLV::ContextTag(kTagRemoteEndpoint))
    {
        entry.type = MATTER_UNICAST_BINDING;
        ReturnErrorOnFailure(reader.Get(entry.remote));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagNodeId)));
        ReturnErrorOnFailure(reader.Get(entry.nodeId));
    }
    else
    {
        entry.type = MATTER_MULTICAST_BINDING;
        ReturnErrorCodeIf(reader.GetTag() != TLV::ContextTag(kTagGroupId), CHIP_ERROR_INVALID_TLV_TAG);
        ReturnErrorOnFailure(reader.Get(entry.groupId));
    }
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagNextEntry)));
    ReturnErrorOnFailure(reader.Get(nextIndex));
    ReturnErrorOnFailure(reader.ExitContainer(container));
    mBindingTable[index] = entry;
    mNextIndex[index]    = nextIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTable::RemoveAt(Iterator & iter)
{
    CHIP_ERROR error;
    if (iter.mTable != this || iter.mIndex == kNextNullIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (iter.mIndex == mTail)
    {
        mTail = iter.mPrevIndex;
    }
    uint8_t next = mNextIndex[iter.mIndex];
    if (iter.mIndex != mHead)
    {
        error = SaveEntryToStorage(iter.mPrevIndex, next);
        if (error == CHIP_NO_ERROR)
        {
            mNextIndex[iter.mPrevIndex] = next;
        }
    }
    else
    {
        error = SaveListInfo(next);
        if (error == CHIP_NO_ERROR)
        {
            mHead = next;
        }
    }
    if (error == CHIP_NO_ERROR)
    {
        // The remove is considered "submitted" once the change on prev node takes effect
        if (mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(iter.mIndex).KeyName()) != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to remove binding table entry %u from storage", iter.mIndex);
        }
        mBindingTable[iter.mIndex].type = MATTER_UNUSED_BINDING;
        mNextIndex[iter.mIndex]         = kNextNullIndex;
        mSize--;
    }
    iter.mIndex = next;
    return error;
}

BindingTable::Iterator BindingTable::begin()
{
    Iterator iter;
    iter.mTable     = this;
    iter.mPrevIndex = kNextNullIndex;
    iter.mIndex     = mHead;
    return iter;
}

BindingTable::Iterator BindingTable::end()
{
    Iterator iter;
    iter.mTable = this;
    iter.mIndex = kNextNullIndex;
    return iter;
}

uint8_t BindingTable::GetNextAvaiableIndex()
{
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++)
    {
        if (mBindingTable[i].type == MATTER_UNUSED_BINDING)
        {
            return i;
        }
    }
    return MATTER_BINDING_TABLE_SIZE;
}

BindingTable::Iterator BindingTable::Iterator::operator++()
{
    if (mIndex != kNextNullIndex)
    {
        mPrevIndex = mIndex;
        mIndex     = mTable->mNextIndex[mIndex];
    }
    return *this;
}

} // namespace chip
