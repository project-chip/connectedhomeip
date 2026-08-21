/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
 *   This file defines a C++ referenced counted object that auto deletes when
 *   all references to it have been removed.
 */

#pragma once

#include <atomic>
#include <limits>
#include <type_traits>

#include <stdlib.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {

template <class T>
class DeleteDeletor
{
public:
    static void Release(T * obj) { chip::Platform::Delete(obj); }
};

template <class T>
class NoopDeletor
{
public:
    static void Release(T * obj) {}
};

template <typename T>
class ReferenceCountedPtr;

template <typename DesiredCounterType>
struct FinalCounterType
{
    using Type = DesiredCounterType;
};

#ifdef __riscv_atomic // some riscv toolchain doesn't support atomic less than 32-bit.
template <>
struct FinalCounterType<uint8_t>
{
    using Type = uint32_t;
};
template <>
struct FinalCounterType<uint16_t>
{
    using Type = uint32_t;
};
#endif

/**
 * Like ReferenceCounted but with protected visibility of reference count management.
 * Use ReferenceCountedPtr<T> to automatically increment/decrement the ref count.
 *
 * NOTE: The default kInitRefCount here is 0, unlike ReferenceCounted where it's 1
 * The reason for this is that the expectation is that the owner of this object will
 * immediately wrap it in a `ReferenceCountedPtr` before exposing it to callers, which
 * will increment the count to 1.
 *
 * NOTE:  CounterType may differ from DesiredCounterType on some platforms (e.g. RISC-V),
 * due to FinalCounterType specializations that widen small unsigned types to 32-bit
 * to satisfy atomic operation requirements.
 */
template <class Subclass, class Deletor, int kInitRefCount = 0, typename DesiredCounterType = uint32_t>
class ReferenceCountedProtected
{
    static_assert(std::is_unsigned<DesiredCounterType>::value, "CounterType should be unsigned");
    using CounterType = typename FinalCounterType<DesiredCounterType>::Type;

public:
    /** Get the current reference counter value */
    CounterType GetReferenceCount() const { return mRefCount.load(std::memory_order_acquire); }

protected:
    friend class ReferenceCountedPtr<Subclass>;

    /** Adds one to the usage count of this class */
    Subclass * Ref()
    {
        CounterType oldRefCount = mRefCount.fetch_add(1, std::memory_order_relaxed);

        if constexpr (kInitRefCount)
        {
            VerifyOrDie(oldRefCount > 0);
        }
        VerifyOrDie(oldRefCount < std::numeric_limits<CounterType>::max());

        return static_cast<Subclass *>(this);
    }

    /**
     * Decrements the reference count by 1;
     * Not called "Release" to avoid confusion with Pool::Release & Deletor::Release
     */
    void Unref()
    {
        CounterType oldRefCount = mRefCount.fetch_sub(1, std::memory_order_acq_rel);

        VerifyOrDie(oldRefCount > 0);

        if (oldRefCount == 1)
        {
            Deletor::Release(static_cast<Subclass *>(this));
        }
    }

private:
    std::atomic<CounterType> mRefCount = kInitRefCount;
};

/**
 * A reference counted object maintains a count of usages and when the usage
 * count drops to 0, it deletes itself.
 */
template <class Subclass, class Deletor = DeleteDeletor<Subclass>, int kInitRefCount = 1, typename CounterType = uint32_t>
class ReferenceCounted : public ReferenceCountedProtected<Subclass, Deletor, kInitRefCount, CounterType>
{
public:
    /** Backwards compatibility; consider using ReferenceCountedProtected instead. */
    Subclass * Retain() { return this->Ref(); }

    /** Backwards compatibility; consider using ReferenceCountedProtected instead. */
    void Release() { this->Unref(); }
};

} // namespace chip
