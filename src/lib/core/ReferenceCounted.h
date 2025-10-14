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

/**
 * Like ReferenceCounted but with protected visibility of reference count management.
 * Use ReferenceCountedPtr<T> to automatically increment/decrement the ref count.
 *
 * NOTE: The default kInitRefCount here is 0, unlike ReferenceCounted where it's 1
 * The reason for this is that the expectation is that the owner of this object will
 * immediately wrap it in a `ReferenceCountedPtr` before exposing it to callers, which
 * will increment the count to 1.
 */
template <class Subclass, class Deletor, int kInitRefCount = 0, typename CounterType = uint32_t>
class ReferenceCountedProtected
{
public:
    /** Get the current reference counter value */
    CounterType GetReferenceCount() const { return mRefCount; }

protected:
    friend class ReferenceCountedPtr<Subclass>;

    /** Adds one to the usage count of this class */
    Subclass * Ref()
    {
        if constexpr (kInitRefCount)
        {
            VerifyOrDie(mRefCount > 0);
        }
        VerifyOrDie(mRefCount < std::numeric_limits<CounterType>::max());
        ++mRefCount;

        return static_cast<Subclass *>(this);
    }

    /**
     * Decrements the reference count by 1;
     * Not called "Release" to avoid confusion with Pool::Release & Deletor::Release
     */
    void Unref()
    {
        VerifyOrDie(mRefCount != 0);

        if (--mRefCount == 0)
        {
            Deletor::Release(static_cast<Subclass *>(this));
        }
    }

private:
    CounterType mRefCount = kInitRefCount;
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
