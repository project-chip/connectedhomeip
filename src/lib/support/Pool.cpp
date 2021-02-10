/* See Project CHIP LICENSE file for licensing information. */

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

} // namespace chip
