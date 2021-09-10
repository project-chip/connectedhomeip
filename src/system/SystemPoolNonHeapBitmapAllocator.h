/*
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

#pragma once

#include <lib/core/CHIPConfig.h>

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

#include <atomic>
#include <limits>
#include <stddef.h>

#include <system/SystemPoolStatistics.h>

namespace chip {
namespace System {

class StaticAllocatorBitmap : public ObjectPoolStatistics
{
protected:
    /**
     * Use the largest data type supported by `std::atomic`. Putting multiple atomic inside a single cache line won't improve
     * concurrency, while the use of larger data type can improve the performance by reducing the number of outer loop iterations.
     */
    using tBitChunkType                         = unsigned long;
    static constexpr const tBitChunkType kBit1  = 1; // make sure bitshifts produce the right type
    static constexpr const size_t kBitChunkSize = std::numeric_limits<tBitChunkType>::digits;
    static_assert(ATOMIC_LONG_LOCK_FREE, "StaticAllocatorBitmap is not lock free");

public:
    StaticAllocatorBitmap(void * storage, std::atomic<tBitChunkType> * usage, size_t capacity, size_t elementSize);
    void * Allocate();
    void Deallocate(void * element);

protected:
    void * At(size_t index) { return static_cast<uint8_t *>(mElements) + mElementSize * index; }
    size_t IndexOf(void * element);

    using Lambda = bool (*)(void *, void *);
    bool ForEachActiveObjectInner(void * context, Lambda lambda);

private:
    const size_t mCapacity;
    void * mElements;
    const size_t mElementSize;
    std::atomic<tBitChunkType> * mUsage;
};

} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
