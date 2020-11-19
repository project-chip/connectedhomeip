/*
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *
 * @brief
 *   Defines a header only BitMapObjectPool.
 */

#pragma once

#include <array>
#include <assert.h>
#include <atomic>
#include <limits>
#include <new>
#include <stddef.h>

namespace chip {

/**
 *  @brief
 *   A class template used for allocating Objects.
 *
 *  @tparam     T   a subclass of Object to be allocated.
 *  @tparam     N   a positive integer max number of objects the pool provides.
 */
template <class T, size_t N>
class BitMapObjectPool
{
public:
    BitMapObjectPool()
    {
        for (size_t word = 0; word * kBitChunkSize < N; ++word)
        {
            mUsage[word].store(0);
        }
    }

    static size_t Size() { return N; }

    template <typename... Args>
    T * New(Args &&... args)
    {
        for (size_t word = 0; word * kBitChunkSize < N; ++word)
        {
            auto & usage = mUsage[word];
            auto value   = usage.load(std::memory_order_relaxed);
            for (size_t offset = 0; offset < kBitChunkSize && offset + word * kBitChunkSize < N; ++offset)
            {
                if ((value & (kBit1 << offset)) == 0)
                {
                    if (usage.compare_exchange_strong(value, value | (kBit1 << offset)))
                    {
                        return new (GetPoolHead() + (word * kBitChunkSize + offset)) T(std::forward<Args>(args)...);
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

    void Delete(T * obj)
    {
        if (obj == nullptr)
            return;

        size_t at     = static_cast<size_t>(obj - GetPoolHead());
        size_t word   = at / kBitChunkSize;
        size_t offset = at - (word * kBitChunkSize);

        // ensure the obj is in the pool
        assert(at >= 0);
        assert(at < N);

        obj->~T();

        auto value = mUsage[word].fetch_and(~(kBit1 << offset));
        assert((value & (kBit1 << offset)) != 0); // assert fail when free an unused slot
    }

    // Test-only function declaration
    template <typename F>
    void ForEachActiveObject(F f);

private:
    T * GetPoolHead() { return reinterpret_cast<T *>(mMemory); }
    const T * GetPoolHead() const { return reinterpret_cast<const T *>(mMemory); }

    /**
     * Use the largest data type supported by `std::atomic`. Putting multiple atomic inside a single cache line won't improve
     * concurrency, while the use of larger data type can improve the performance by reducing the number of outer loop iterations.
     * C++11 guarantees that `std::atomic` supports `uintmax_t`.
     */
    using tBitChunkType                         = uintmax_t;
    static constexpr const tBitChunkType kBit1  = 1; // make sure bitshifts produce the right type
    static constexpr const size_t kBitChunkSize = std::numeric_limits<tBitChunkType>::digits;

    std::array<std::atomic<tBitChunkType>, (N + kBitChunkSize - 1) / kBitChunkSize> mUsage;
    alignas(alignof(T)) uint8_t mMemory[N * sizeof(T)];
};

} // namespace chip
