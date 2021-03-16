#include "platform/mbed_toolchain.h"
#include "platform/mbed_atomic.h"

// TODO: Remove!
// This file is a temporary workaround until atomic integration has been solved

void __sync_synchronize() { 
    MBED_BARRIER();
}

unsigned int __atomic_fetch_add_4(volatile void *ptr, unsigned int val, int memorder) { 
    return core_util_atomic_fetch_add_explicit_u32((volatile uint32_t*)ptr, val, memorder);
}

uint64_t __atomic_load_8(const volatile void* ptr, int memorder) {
    return core_util_atomic_load_explicit_u64((const volatile uint64_t*)ptr, memorder);
}

uint64_t __atomic_exchange_8(volatile void* ptr, uint64_t val, int memorder) {
    return core_util_atomic_exchange_explicit_u64((volatile uint64_t*)ptr, val, memorder);
}


bool __atomic_compare_exchange_8(volatile void *ptr, void*expected, uint64_t desired, bool weak, int success_memorder, int failure_memorder)
{
    if (weak) {
        return core_util_atomic_compare_exchange_weak_explicit_u64((volatile uint64_t*)ptr, (uint64_t*)expected, desired, success_memorder, failure_memorder);
    } else {
        return core_util_atomic_cas_explicit_u64((volatile uint64_t*)ptr, (uint64_t*)expected, desired, success_memorder, failure_memorder);
    }
}