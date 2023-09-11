/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
