/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013 Nest Labs, Inc.
 *    All rights reserved.
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

#include <support/Pool.h>

#include <nlassert.h>

namespace chip {

StaticAllocatorBitmap::StaticAllocatorBitmap(void * storage, std::atomic<tBitChunkType> * usage, size_t capacity,
                                             size_t elementSize) :
    StaticAllocatorBase(capacity),
    mElements(storage), mElementSize(elementSize), mUsage(usage)
{
    for (size_t word = 0; word * kBitChunkSize < Capacity(); ++word)
    {
        mUsage[word].store(0);
    }
}

void * StaticAllocatorBitmap::Allocate()
{
    for (size_t word = 0; word * kBitChunkSize < Capacity(); ++word)
    {
        auto & usage = mUsage[word];
        auto value   = usage.load(std::memory_order_relaxed);
        for (size_t offset = 0; offset < kBitChunkSize && offset + word * kBitChunkSize < Capacity(); ++offset)
        {
            if ((value & (kBit1 << offset)) == 0)
            {
                if (usage.compare_exchange_strong(value, value | (kBit1 << offset)))
                {
                    mAllocated++;
                    return At(word * kBitChunkSize + offset);
                }
                else
                {
                    value = usage.load(std::memory_order_relaxed); // if there is a race, update new usage
                }
            }
        }
    }
    return nullptr;
}

void StaticAllocatorBitmap::Deallocate(void * element)
{
    size_t index  = IndexOf(element);
    size_t word   = index / kBitChunkSize;
    size_t offset = index - (word * kBitChunkSize);

    // ensure the element is in the pool
    assert(index < Capacity());

    auto value = mUsage[word].fetch_and(~(kBit1 << offset));
    nlASSERT((value & (kBit1 << offset)) != 0); // assert fail when free an unused slot
    mAllocated--;
}

size_t StaticAllocatorBitmap::IndexOf(void * element)
{
    std::ptrdiff_t diff = static_cast<uint8_t *>(element) - static_cast<uint8_t *>(mElements);
    assert(diff >= 0);
    assert(static_cast<size_t>(diff) % mElementSize == 0);
    auto index = static_cast<size_t>(diff) / mElementSize;
    assert(index < Capacity());
    return index;
}

bool StaticAllocatorBitmap::ForEachActiveObjectInner(void * context, Lambda lambda)
{
    for (size_t word = 0; word * kBitChunkSize < Capacity(); ++word)
    {
        auto & usage = mUsage[word];
        auto value   = usage.load(std::memory_order_relaxed);
        for (size_t offset = 0; offset < kBitChunkSize && offset + word * kBitChunkSize < Capacity(); ++offset)
        {
            if ((value & (kBit1 << offset)) != 0)
            {
                if (!lambda(context, At(word * kBitChunkSize + offset)))
                    return false;
            }
        }
    }
    return true;
}

} // namespace chip
