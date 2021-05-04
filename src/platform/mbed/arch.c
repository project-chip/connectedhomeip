#include "platform/mbed_atomic.h"
#include "platform/mbed_toolchain.h"

// TODO: Remove!
// This file is a temporary workaround until atomic integration has been solved

static mbed_memory_order mem_order(int order)
{
    switch (order)
    {
    case __ATOMIC_RELAXED:
        return mbed_memory_order_relaxed;
    case __ATOMIC_CONSUME:
        return mbed_memory_order_consume;
    case __ATOMIC_ACQUIRE:
        return mbed_memory_order_acquire;
    case __ATOMIC_RELEASE:
        return mbed_memory_order_release;
    case __ATOMIC_ACQ_REL:
        return mbed_memory_order_acq_rel;
    case __ATOMIC_SEQ_CST:
        return mbed_memory_order_seq_cst;
    default:
        // Should not happen! return sequential consistency in such case
        MBED_ASSERT(false);
        return mbed_memory_order_seq_cst;
    }
}

void __sync_synchronize()
{
    MBED_BARRIER();
}

unsigned int __atomic_fetch_add_4(volatile void * ptr, unsigned int val, int memorder)
{
    return core_util_atomic_fetch_add_explicit_u32((volatile uint32_t *) ptr, val, mem_order(memorder));
}

uint64_t __atomic_load_8(const volatile void * ptr, int memorder)
{
    return core_util_atomic_load_explicit_u64((const volatile uint64_t *) ptr, mem_order(memorder));
}

uint64_t __atomic_exchange_8(volatile void * ptr, uint64_t val, int memorder)
{
    return core_util_atomic_exchange_explicit_u64((volatile uint64_t *) ptr, val, mem_order(memorder));
}

// Note: Weak version not supported in library, the weak parameter is simply dropped.
// see https://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
bool __atomic_compare_exchange_8(volatile void * ptr, void * expected, unsigned long long desired, int success_memorder,
                                 int failure_memorder)
{
    return core_util_atomic_cas_explicit_u64((volatile uint64_t *) ptr, (uint64_t *) expected, desired, mem_order(success_memorder),
                                             mem_order(failure_memorder));
}
