/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "SysHeapMalloc.h"

#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

extern "C" {
#include <zephyr/init.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/sys/sys_heap.h>
}

#include <cstdint>
#include <cstring>

// Construct name of the given function wrapped with the `--wrap=symbol` GCC option.
#define WRAP(f) __wrap_##f

using namespace chip;

namespace {

// Alignment of memory blocks returned by malloc.
// Choose the value that guarantees that the returned memory blocks are castable to all built-in types.
constexpr size_t kMallocAlignment = alignof(long long);

uint8_t sHeapMemory[CONFIG_CHIP_MALLOC_SYS_HEAP_SIZE] alignas(kMallocAlignment);
sys_heap sHeap;
SYS_MUTEX_DEFINE(sLock);

// RAII helper for synchronizing access to the common heap.
class LockGuard
{
public:
    LockGuard() : mStatus(sys_mutex_lock(&sLock, K_FOREVER)) {}
    ~LockGuard();

    bool Locked() const { return mStatus == 0; }
    CHIP_ERROR Error() const { return System::MapErrorZephyr(mStatus); }

private:
    const int mStatus;
};

LockGuard::~LockGuard()
{
    if (mStatus == 0)
    {
        sys_mutex_unlock(&sLock);
    }
}

int initHeap()
{
    sys_heap_init(&sHeap, sHeapMemory, sizeof(sHeapMemory));
    return 0;
}

} // namespace

// Initialize the heap in the POST_KERNEL phase to make sure that it is ready even before
// C++ static constructors are called (which happens prior to the APPLICATION initialization phase).
SYS_INIT(initHeap, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

namespace chip {
namespace DeviceLayer {
namespace Malloc {

void * Malloc(size_t size)
{
    LockGuard lockGuard;

    return lockGuard.Locked() ? sys_heap_aligned_alloc(&sHeap, kMallocAlignment, size) : nullptr;
}

void * Calloc(size_t num, size_t size)
{
    size_t totalSize;

    if (size_mul_overflow(num, size, &totalSize))
    {
        return nullptr;
    }

    void * mem = malloc(totalSize);

    if (mem)
    {
        memset(mem, 0, totalSize);
    }

    return mem;
}

void * Realloc(void * mem, size_t size)
{
    LockGuard lockGuard;

    return lockGuard.Locked() ? sys_heap_aligned_realloc(&sHeap, mem, kMallocAlignment, size) : nullptr;
}

void Free(void * mem)
{
    LockGuard lockGuard;

    VerifyOrReturn(lockGuard.Locked());
    sys_heap_free(&sHeap, mem);
}

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS

CHIP_ERROR GetStats(Stats & stats)
{
    LockGuard lockGuard;
    ReturnErrorOnFailure(lockGuard.Error());

    sys_memory_stats sysHeapStats;
    ReturnErrorOnFailure(System::MapErrorZephyr(sys_heap_runtime_stats_get(&sHeap, &sysHeapStats)));

    stats.free    = sysHeapStats.free_bytes;
    stats.used    = sysHeapStats.allocated_bytes;
    stats.maxUsed = sysHeapStats.max_allocated_bytes;

    return CHIP_NO_ERROR;
}

void ResetMaxStats()
{
    (void) sys_heap_runtime_stats_reset_max(&sHeap);
}

#endif // CONFIG_SYS_HEAP_RUNTIME_STATS

} // namespace Malloc
} // namespace DeviceLayer
} // namespace chip

#ifdef CONFIG_CHIP_MALLOC_SYS_HEAP_OVERRIDE

extern "C" {

void * WRAP(malloc)(size_t size) __attribute((alias("_ZN4chip11DeviceLayer6Malloc6MallocEj")));
void * WRAP(calloc)(size_t num, size_t size) __attribute((alias("_ZN4chip11DeviceLayer6Malloc6CallocEjj")));
void * WRAP(realloc)(void * mem, size_t size) __attribute((alias("_ZN4chip11DeviceLayer6Malloc7ReallocEPvj")));
void WRAP(free)(void * mem) __attribute((alias("_ZN4chip11DeviceLayer6Malloc4FreeEPv")));

void * WRAP(_malloc_r)(_reent *, size_t size)
{
    return WRAP(malloc)(size);
}

void * WRAP(_calloc_r)(_reent *, size_t num, size_t size)
{
    return WRAP(calloc)(num, size);
}

void * WRAP(_realloc_r)(_reent *, void * mem, size_t size)
{
    return WRAP(realloc)(mem, size);
}

void WRAP(_free_r)(_reent *, void * mem)
{
    WRAP(free)(mem);
}

} // extern "C"

#endif // CONFIG_CHIP_MALLOC_SYS_HEAP_OVERRIDE
