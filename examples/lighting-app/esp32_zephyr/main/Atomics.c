/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/*
 * GCC __atomic_fetch_* built-ins for SoCs without the RISC-V A extension
 * (ESP32-C3, ESP32-C2). The compiler turns std::atomic read-modify-write
 * operations into these library calls, and the Zephyr SDK ships no libatomic.
 * Zephyr's lib/cpp/abi/cpp_atomics.c only provides __atomic_compare_exchange_N,
 * so the fetch operations used by CHIP are added here with the same interrupt
 * lock. The memory order argument is ignored: irq_lock() is a full barrier.
 */

#include <stdint.h>
#include <zephyr/kernel.h>

#define DEFINE_ATOMIC_FETCH_OP(name, op)                                                                                           \
    uint32_t __atomic_fetch_##name##_4(volatile void * ptr, uint32_t value, int memorder)                                          \
    {                                                                                                                              \
        unsigned int key      = irq_lock();                                                                                        \
        volatile uint32_t * p = ptr;                                                                                               \
        uint32_t old          = *p;                                                                                                \
        *p                    = old op value;                                                                                      \
        irq_unlock(key);                                                                                                           \
        return old;                                                                                                                \
    }

DEFINE_ATOMIC_FETCH_OP(add, +)
DEFINE_ATOMIC_FETCH_OP(sub, -)
DEFINE_ATOMIC_FETCH_OP(and, &)
