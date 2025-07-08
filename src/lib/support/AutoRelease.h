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

#include <stddef.h>

namespace chip {

/// RAII class for iterators that guarantees that Release() will be called
/// on the underlying type.  This is effectively a simple unique_ptr, except
/// calling Release instead of delete
template <typename Releasable>
class AutoRelease
{
public:
    AutoRelease(Releasable * releasable) : mReleasable(releasable) {}
    ~AutoRelease() { Release(); }

    // Not copyable or movable
    AutoRelease(const AutoRelease &)             = delete;
    AutoRelease(const AutoRelease &&)            = delete;
    AutoRelease & operator=(const AutoRelease &) = delete;

    inline Releasable * operator->() { return mReleasable; }
    inline const Releasable * operator->() const { return mReleasable; }

    inline bool IsNull() const { return mReleasable == nullptr; }

    void Release()
    {
        VerifyOrReturn(mReleasable != nullptr);
        mReleasable->Release();
        mReleasable = nullptr;
    }

private:
    Releasable * mReleasable = nullptr;
};

} // namespace chip
