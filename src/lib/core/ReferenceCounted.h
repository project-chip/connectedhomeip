/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * A reference counted object maintains a count of usages and when the usage
 * count drops to 0, it deletes itself.
 */
template <class Subclass, class Deletor = DeleteDeletor<Subclass>, int kInitRefCount = 1, typename CounterType = uint32_t>
class ReferenceCounted
{
public:
    /** Adds one to the usage count of this class */
    Subclass * Retain()
    {
        VerifyOrDie(!kInitRefCount || mRefCount > 0);
        VerifyOrDie(mRefCount < std::numeric_limits<CounterType>::max());
        ++mRefCount;

        return static_cast<Subclass *>(this);
    }

    /** Release usage of this class */
    void Release()
    {
        VerifyOrDie(mRefCount != 0);

        if (--mRefCount == 0)
        {
            Deletor::Release(static_cast<Subclass *>(this));
        }
    }

    /** Get the current reference counter value */
    CounterType GetReferenceCount() const { return mRefCount; }

private:
    CounterType mRefCount = kInitRefCount;
};

} // namespace chip
