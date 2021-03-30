#include "platform/mbed_toolchain.h"
#include "platform/mbed_atomic.h"

// TODO: Remove!
// This file is a temporary workaround until atomic integration has been solved

void __sync_synchronize() { 
    MBED_BARRIER();
}

unsigned int __atomic_fetch_add_4(volatile uint32_t *ptr, uint32_t val, int memorder) { 
    return core_util_atomic_fetch_add_explicit_u32(ptr, val, memorder);
}