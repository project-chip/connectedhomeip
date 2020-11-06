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
#include <new>
#include <stddef.h>

namespace chip {

template <class T, size_t N>
class BitMapObjectPool
{
public:
    BitMapObjectPool()
    {
        for (size_t word = 0; word * kAtomicSize < N; ++word)
        {
            mUsage[word].store(0);
        }
    }

    static size_t Size() { return N; }

    template <typename... Args>
    T * New(Args &&... args)
    {
        for (size_t word = 0; word * kAtomicSize < N; ++word)
        {
            auto & usage   = mUsage[word];
            uint32_t value = usage.load();
            for (size_t offset = 0; offset < kAtomicSize && offset + word * kAtomicSize < N; ++offset)
            {
                if ((value & (1 << offset)) == 0)
                {
                    if (usage.compare_exchange_strong(value, value | (1 << offset)))
                    {
                        return new (GetPoolHead() + (word * kAtomicSize + offset)) T(std::forward<Args>(args)...);
                    }
                    else
                    {
                        value = usage.load(); // if there is a race, update new usage
                    }
                }
            }
        }
        return nullptr;
    }

    void Delete(T * obj)
    {
        if (obj == nullptr) return;

        size_t at     = static_cast<size_t>(obj - GetPoolHead());
        size_t word   = at / kAtomicSize;
        size_t offset = at - (word * kAtomicSize);

        // ensure the obj is in the pool
        assert(at >= 0);
        assert(at < N);

        obj->~T();

        uint32_t value = mUsage[word].fetch_and(~(1 << offset));
        assert((value & (1 << offset)) != 0); // assert fail when free an unused slot
    }

#if !defined(NDEBUG)
    template <typename F>
    void ForEachActiveObject(F f)
    {
        for (size_t word = 0; word * kAtomicSize < N; ++word)
        {
            auto & usage   = mUsage[word];
            uint32_t value = usage.load();
            for (size_t offset = 0; offset < kAtomicSize && offset + word * kAtomicSize < N; ++offset)
            {
                if ((value & (1 << offset)) != 0)
                {
                    f(GetPoolHead() + (word * kAtomicSize + offset));
                }
            }
        }
    }

    size_t GetNumObjectsInUse()
    {
        size_t count = 0;
        ForEachActiveObject([&count](T *) { ++count; });
        return count;
    }
#endif

private:
    T * GetPoolHead() { return reinterpret_cast<T *>(mMemory); }

    static constexpr const size_t kAtomicSize = 32;

    std::array<std::atomic_uint32_t, (N + kAtomicSize - 1) / kAtomicSize> mUsage;
    alignas(alignof(T)) uint8_t mMemory[N * sizeof(T)];
};

} // namespace chip
