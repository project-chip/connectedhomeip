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

namespace chip {

BindingTable BindingTable::sInstance;

BindingTable::BindingTable()
{
    memset(mNextIndex, EMBER_BINDING_TABLE_SIZE, sizeof(mNextIndex));
}

CHIP_ERROR BindingTable::Add(const EmberBindingTableEntry & entry)
{
    if (entry.type == EMBER_UNUSED_BINDING)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t newIndex = GetNextAvaiableIndex();
    if (newIndex >= EMBER_BINDING_TABLE_SIZE)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    mBindingTable[newIndex] = entry;
    if (mTail == EMBER_BINDING_TABLE_SIZE)
    {
        mTail = newIndex;
        mHead = newIndex;
    }
    else
    {
        mNextIndex[mTail] = newIndex;
        mTail             = newIndex;
    }
    mSize++;
    return CHIP_NO_ERROR;
}

const EmberBindingTableEntry & BindingTable::GetAt(uint8_t index)
{
    return mBindingTable[index];
}

BindingTable::Iterator BindingTable::RemoveAt(Iterator iter)
{
    if (iter.mTable != this || iter.mIndex == EMBER_BINDING_TABLE_SIZE)
    {
        return iter;
    }
    if (iter.mIndex == mTail)
    {
        mTail = iter.mPrevIndex;
    }
    uint8_t next = mNextIndex[iter.mIndex];
    if (iter.mIndex != mHead)
    {
        mNextIndex[iter.mPrevIndex] = next;
    }
    else
    {
        mHead = next;
    }
    mBindingTable[iter.mIndex].type = EMBER_UNUSED_BINDING;
    mNextIndex[iter.mIndex]         = EMBER_BINDING_TABLE_SIZE;
    mSize--;
    iter.mIndex = next;
    return iter;
}

BindingTable::Iterator BindingTable::begin()
{
    Iterator iter;
    iter.mTable     = this;
    iter.mPrevIndex = EMBER_BINDING_TABLE_SIZE;
    iter.mIndex     = mHead;
    return iter;
}

BindingTable::Iterator BindingTable::end()
{
    Iterator iter;
    iter.mTable = this;
    iter.mIndex = EMBER_BINDING_TABLE_SIZE;
    return iter;
}

uint8_t BindingTable::GetNextAvaiableIndex()
{
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        if (mBindingTable[i].type == EMBER_UNUSED_BINDING)
        {
            return i;
        }
    }
    return EMBER_BINDING_TABLE_SIZE;
}

BindingTable::Iterator BindingTable::Iterator::operator++()
{
    if (mIndex != EMBER_BINDING_TABLE_SIZE)
    {
        mPrevIndex = mIndex;
        mIndex     = mTable->mNextIndex[mIndex];
    }
    return *this;
}

} // namespace chip
