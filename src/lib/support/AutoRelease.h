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

/**
 *    @file
 *          Contains a standard RAII class which calls Release on dtor.
 */

#pragma once

#include <lib/support/CodeUtils.h>
#include <stddef.h>

namespace chip {

/// RAII class for iterators that guarantees that Release() will be called
/// on the underlying type.  This is effectively a simple unique_ptr, except
/// calling Release instead of delete
/// See also ReferenceCountedPtr<T>, which automatically retains a ReferenceCounted
template <typename Releasable>
class AutoRelease
{
public:
    AutoRelease(Releasable * releasable) : mReleasable(releasable) {}
    __attribute__((always_inline)) inline ~AutoRelease() { Release(); }

    // Not copyable
    AutoRelease(const AutoRelease &)             = delete;
    AutoRelease & operator=(const AutoRelease &) = delete;

    AutoRelease(AutoRelease && other) : mReleasable(other.mReleasable) { other.mReleasable = nullptr; }
    AutoRelease & operator=(AutoRelease && other)
    {
        if (this != &other)
        {
            Release();
            mReleasable       = other.mReleasable;
            other.mReleasable = nullptr;
        }
        return *this;
    }

    inline Releasable * operator->() { return mReleasable; }
    inline const Releasable * operator->() const { return mReleasable; }
    inline const Releasable & operator*() const { return *mReleasable; }
    inline Releasable & operator*() { return *mReleasable; }

    inline operator bool() { return mReleasable != nullptr; }
    inline bool IsNull() const { return mReleasable == nullptr; }

    __attribute__((always_inline)) inline void Release()
    {
        VerifyOrReturn(mReleasable != nullptr);
        mReleasable->Release();
        mReleasable = nullptr;
    }

    void Set(Releasable * releasable)
    {
        if (mReleasable != releasable)
        {
            Release();
            mReleasable = releasable;
        }
    }

protected:
    Releasable * mReleasable = nullptr;
};

// Template deduction guides to allow auto-release creation from pointers
template <class T>
AutoRelease(T * releasable) -> AutoRelease<T>;

} // namespace chip
