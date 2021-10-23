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

#include <limits>
#include <stdlib.h>

#include <lib/support/CHIPMem.h>

namespace chip {

template <class T>
class DeleteDeletor
{
public:
    static void Release(T * obj) { chip::Platform::Delete(obj); }
};

/**
 * A reference counted object maintains a count of usages and when the usage
 * count drops to 0, it deletes itself.
 */
template <class Subclass, class Deletor = DeleteDeletor<Subclass>, int kInitRefCount = 1>
class ReferenceCounted
{
public:
    typedef uint32_t count_type;

    /** Adds one to the usage count of this class */
    Subclass * Retain()
    {
        if (mRefCount == std::numeric_limits<count_type>::max())
        {
            abort();
        }
        ++mRefCount;

        return static_cast<Subclass *>(this);
    }

    /** Release usage of this class */
    void Release()
    {
        if (mRefCount == 0)
        {
            abort();
        }

        if (--mRefCount == 0)
        {
            Deletor::Release(static_cast<Subclass *>(this));
        }
    }

    /** Get the current reference counter value */
    count_type GetReferenceCount() const { return mRefCount; }

private:
    count_type mRefCount = kInitRefCount;
};

} // namespace chip
