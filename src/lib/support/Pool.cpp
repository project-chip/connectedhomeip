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

#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>

namespace chip {

namespace internal {

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
                    IncreaseUsage();
                    return At(word * kBitChunkSize + offset);
                }

                value = usage.load(std::memory_order_relaxed); // if there is a race, update new usage
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
    VerifyOrDie(index < Capacity());

    auto value = mUsage[word].fetch_and(~(kBit1 << offset));
    VerifyOrDie((value & (kBit1 << offset)) != 0); // assert fail when free an unused slot
    DecreaseUsage();
}

size_t StaticAllocatorBitmap::IndexOf(void * element)
{
    std::ptrdiff_t diff = static_cast<uint8_t *>(element) - static_cast<uint8_t *>(mElements);
    VerifyOrDie(diff >= 0);
    VerifyOrDie(static_cast<size_t>(diff) % mElementSize == 0);
    auto index = static_cast<size_t>(diff) / mElementSize;
    VerifyOrDie(index < Capacity());
    return index;
}

Loop StaticAllocatorBitmap::ForEachActiveObjectInner(void * context, Lambda lambda)
{
    for (size_t word = 0; word * kBitChunkSize < Capacity(); ++word)
    {
        auto & usage = mUsage[word];
        auto value   = usage.load(std::memory_order_relaxed);
        for (size_t offset = 0; offset < kBitChunkSize && offset + word * kBitChunkSize < Capacity(); ++offset)
        {
            if ((value & (kBit1 << offset)) != 0)
            {
                if (lambda(context, At(word * kBitChunkSize + offset)) == Loop::Break)
                    return Loop::Break;
            }
        }
    }
    return Loop::Finish;
}

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

HeapObjectListNode * HeapObjectList::FindNode(void * object) const
{
    for (HeapObjectListNode * p = mNext; p != this; p = p->mNext)
    {
        if (p->mObject == object)
        {
            return p;
        }
    }
    return nullptr;
}

Loop HeapObjectList::ForEachNode(void * context, Lambda lambda)
{
    ++mIterationDepth;
    Loop result            = Loop::Finish;
    HeapObjectListNode * p = mNext;
    while (p != this)
    {
        if (p->mObject != nullptr)
        {
            if (lambda(context, p->mObject) == Loop::Break)
            {
                result = Loop::Break;
                break;
            }
        }
        p = p->mNext;
    }
    --mIterationDepth;
    if (mIterationDepth == 0 && mHaveDeferredNodeRemovals)
    {
        // Remove nodes for released objects.
        p = mNext;
        while (p != this)
        {
            HeapObjectListNode * next = p->mNext;
            if (p->mObject == nullptr)
            {
                p->Remove();
                Platform::Delete(p);
            }
            p = next;
        }

        mHaveDeferredNodeRemovals = false;
    }
    return result;
}

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace internal
} // namespace chip
