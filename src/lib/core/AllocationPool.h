/*
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
 * @file Defines an allocation pool for objects.
 */

#ifndef ALLOCATION_POOL_H_
#define ALLOCATION_POOL_H_

#include <cstdint>
#include <new>
#include <utility>

#include <string.h>

#include <core/CHIPError.h>
#include <support/CodeUtils.h>

namespace chip {

/**
 * Defines an allocation pool for objects.
 *
 * Allows for static allocation of resources. As opposed to System::ObjectPool,
 * this class does not require reference counting or the use of a System Layer.
 */
template <class T, size_t kMaxItems>
class AllocationPool
{
public:
    AllocationPool()
    {
        memset(mInUse, 0, sizeof(mInUse)); // nothing is used
    }

    // Destructor will attempt to free any items that are still in use.
    ~AllocationPool()
    {
        for (size_t i = 0; i < kMaxItems; i++)
        {
            if (IsInUseIndex(i))
            {
                FreeIndex(i);
            }
        }
    }

    /**
     * Attempts to allocate a new object from the allocation pool.
     *
     * Returns NULL if no free instances are available in the pool.
     */
    template <typename... Args>
    T * Allocate(Args &&... value)
    {
        T * result = nullptr;

        size_t i = 0;
        while (result == nullptr && i < kMaxItems)
        {
            if (IsInUseIndex(i))
            {
                i++;
            }
            else
            {
                // memory available
                result = new ((void *) MemoryForIndex(i)) T(std::forward<Args &&>(value)...);
                MarkInUseIndex(i);
            }
        }

        return result;
    }

    /**
     * Frees a previously allocated object from the pool
     */
    CHIP_ERROR Free(T * ptr)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        uint8_t * rawPointer;
        size_t offset;
        size_t index;

        // Allow null free any time
        VerifyOrExit(ptr != NULL, err = CHIP_NO_ERROR);

        rawPointer = reinterpret_cast<uint8_t *>(ptr);

        // in range
        VerifyOrExit(rawPointer >= mMemory, err = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(rawPointer < mMemory + sizeof(mMemory), err = CHIP_ERROR_INVALID_ARGUMENT);

        // correct alignment
        offset = rawPointer - mMemory;
        VerifyOrExit(offset % sizeof(T) == 0, err = CHIP_ERROR_DATA_NOT_ALIGNED);
        index = offset / sizeof(T);

        VerifyOrDie(ptr == MemoryForIndex(index));

        err = FreeIndex(index);
    exit:
        return err;
    }

private:
    CHIP_ERROR FreeIndex(size_t index)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        // item should be in use (no double frees)
        VerifyOrExit(IsInUseIndex(index), err = CHIP_ERROR_INCORRECT_STATE);

        MemoryForIndex(index)->~T();
        MarkNotInUseIndex(index);
    exit:
        return err;
    }

    /// Checks if the specified index is in use
    bool IsInUseIndex(size_t index)
    {
        VerifyOrDie(index < kMaxItems);

        size_t offs = index / 32;
        size_t bit  = index % 32;

        return (mInUse[offs] & (1 << bit)) != 0;
    }

    /// Marks the specified indes in the in use bitset as currently in use
    void MarkInUseIndex(size_t index)
    {
        VerifyOrDie(index < kMaxItems);

        size_t offs = index / 32;
        size_t bit  = index % 32;

        mInUse[offs] |= (1 << bit);
    }

    /// Marks the specified indes in the in use bitset as not in use
    void MarkNotInUseIndex(size_t index)
    {
        VerifyOrDie(index < kMaxItems);

        size_t offs = index / 32;
        size_t bit  = index % 32;

        mInUse[offs] &= ~(1 << bit);
    }

    // Get the raw memory pointer to the specified index.
    T * MemoryForIndex(size_t index)
    {
        VerifyOrDie(index < kMaxItems);
        return reinterpret_cast<T *>(mMemory + index * sizeof(T));
    }

    uint32_t mInUse[(kMaxItems + 31) / 32];                     // in use bitset
    alignas(alignof(T)) uint8_t mMemory[sizeof(T) * kMaxItems]; // storage for data
};

} // namespace chip

#endif // ALLOCATION_POOL_H_
