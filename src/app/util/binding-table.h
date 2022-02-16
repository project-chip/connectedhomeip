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
 * @file API declarations for a binding table.
 */

#pragma once

#include <app/util/af-types.h>

namespace chip {

class BindingTable
{
    friend class Iterator;

public:
    BindingTable();

    class Iterator
    {
        friend class BindingTable;

    public:
        EmberBindingTableEntry & operator*() { return mTable->mBindingTable[mIndex]; }

        const EmberBindingTableEntry & operator*() const { return mTable->mBindingTable[mIndex]; }

        EmberBindingTableEntry * operator->() { return &(mTable->mBindingTable[mIndex]); }

        const EmberBindingTableEntry * operator->() const { return &(mTable->mBindingTable[mIndex]); }

        Iterator operator++();

        bool operator==(const Iterator & rhs) { return mIndex == rhs.mIndex; }

        bool operator!=(const Iterator & rhs) { return mIndex != rhs.mIndex; }

        uint8_t GetIndex() { return mIndex; }

    private:
        BindingTable * mTable;
        uint8_t mPrevIndex;
        uint8_t mIndex;
    };

    CHIP_ERROR Add(const EmberBindingTableEntry & entry);

    const EmberBindingTableEntry & GetAt(uint8_t index);

    // The RemoveAt function shares the same sematics as the std::list::remove.
    // It returns the next iterator after removal and the old iterator is no loger valid.
    Iterator RemoveAt(Iterator iter);

    // Returns the number of active entries in the binding table.
    // *NOTE* The function does not return the capacity of the binding table.
    uint8_t Size() { return mSize; }

    Iterator begin();

    Iterator end();

    static BindingTable & GetInstance() { return sInstance; }

private:
    static BindingTable sInstance;

    uint8_t GetNextAvaiableIndex();

    EmberBindingTableEntry mBindingTable[EMBER_BINDING_TABLE_SIZE];
    uint8_t mNextIndex[EMBER_BINDING_TABLE_SIZE];

    uint8_t mHead = EMBER_BINDING_TABLE_SIZE;
    uint8_t mTail = EMBER_BINDING_TABLE_SIZE;
    uint8_t mSize = 0;
};

} // namespace chip
