/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include "ReferenceCountedHandle.h"
#include <lib/support/CodeUtils.h>

namespace chip {

/** ReferenceCountedPtr acts like a shared_ptr to an object derived from ReferenceCounted. */
template <typename T>
class ReferenceCountedPtr
{
public:
    ReferenceCountedPtr() : mRefCounted(nullptr) {}
    ReferenceCountedPtr(T * releasable) : mRefCounted(releasable ? releasable->Ref() : nullptr) {}
    ReferenceCountedPtr(const ReferenceCountedPtr & src) : ReferenceCountedPtr(src.mRefCounted) {}
    ReferenceCountedPtr(const ReferenceCountedHandle<T> & src) : ReferenceCountedPtr(&src.Get()) {}
    ReferenceCountedPtr(ReferenceCountedPtr && src) noexcept : mRefCounted(src.mRefCounted) { src.mRefCounted = nullptr; }

    ReferenceCountedPtr & operator=(const ReferenceCountedPtr & src)
    {
        Set(src.mRefCounted);
        return *this;
    }
    ReferenceCountedPtr & operator=(ReferenceCountedPtr && src) noexcept
    {
        if (this != &src)
        {
            Release();
            mRefCounted     = src.mRefCounted;
            src.mRefCounted = nullptr;
        }
        return *this;
    }

    ~ReferenceCountedPtr() { Release(); }

    inline T * operator->() const { return mRefCounted; }
    inline T & operator*() const { return *mRefCounted; }

    inline operator bool() { return mRefCounted != nullptr; }
    inline bool IsNull() const { return mRefCounted == nullptr; }

    inline bool operator==(const ReferenceCountedPtr & other) const { return this->mRefCounted == other.mRefCounted; }
    inline bool operator!=(const ReferenceCountedPtr & other) const { return this->mRefCounted != other.mRefCounted; }
    inline bool operator==(const ReferenceCountedHandle<T> & other) const { return this->mRefCounted == &other.Get(); }
    inline bool operator!=(const ReferenceCountedHandle<T> & other) const { return this->mRefCounted != &other.Get(); }
    inline bool operator==(const T & other) const { return this->mRefCounted == &other; }
    inline bool operator!=(const T & other) const { return this->mRefCounted != &other; }

    void Release()
    {
        VerifyOrReturn(mRefCounted != nullptr);
        mRefCounted->Unref();
        mRefCounted = nullptr;
    }

    void Set(T * releasable)
    {
        if (mRefCounted != releasable)
        {
            Release();
            mRefCounted = releasable ? releasable->Ref() : nullptr;
        }
    }

protected:
    T * mRefCounted = nullptr;
};

} // namespace chip
