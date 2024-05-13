/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <cstdint>

#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/ScopedBuffer.h>

namespace mdns {
namespace Minimal {

/// Contructs a FullQName from a SerializedNameIterator
///
/// Generally a conversion from an iterator to a `const char *[]`
/// using heap for underlying storage of the data.
class HeapQName
{
public:
    HeapQName() {}
    HeapQName(SerializedQNameIterator name)
    {
        // Storage is:
        //    - separate pointers into mElementPointers
        //    - allocated pointers inside that
        mElementCount = 0;

        SerializedQNameIterator it = name;
        while (it.Next())
        {
            // Count all elements
            mElementCount++;
        }

        if (!it.IsValid())
        {
            return;
        }

        mElementPointers.Alloc(mElementCount);
        if (!mElementPointers)
        {
            return;
        }
        // ensure all set to null since we may need to free
        for (size_t i = 0; i < mElementCount; i++)
        {
            mElementPointers[i] = nullptr;
        }

        it         = name;
        size_t idx = 0;
        while (it.Next())
        {
            mElementPointers[idx] = chip::Platform::MemoryAllocString(it.Value(), strlen(it.Value()));
            if (!mElementPointers[idx])
            {
                return;
            }
            idx++;
        }
        mIsOk = true;
    }

    HeapQName(const HeapQName & other) { *this = other; }

    HeapQName & operator=(const HeapQName & other)
    {
        Free();

        if (!other)
        {
            return *this; // No point in copying the other value
        }

        mElementCount = other.mElementCount;
        mElementPointers.Alloc(other.mElementCount);
        if (!mElementPointers)
        {
            return *this;
        }

        for (size_t i = 0; i < mElementCount; i++)
        {
            mElementPointers[i] = nullptr;
        }

        for (size_t i = 0; i < mElementCount; i++)
        {
            const char * other_data = other.mElementPointers[i];
            mElementPointers[i]     = chip::Platform::MemoryAllocString(other_data, strlen(other_data));
            if (!mElementPointers[i])
            {
                return *this;
            }
        }
        mIsOk = true;
        return *this;
    }

    ~HeapQName() { Free(); }

    bool IsOk() const { return mIsOk; }

    operator bool() const { return IsOk(); }
    bool operator!() const { return !IsOk(); }

    /// Returns the contained FullQName.
    ///
    /// VALIDITY: since this references data inside `this` it is only valid
    ///           as long as `this` is valid.
    FullQName Content() const
    {
        FullQName result;

        result.names     = mElementPointers.Get();
        result.nameCount = mElementCount;

        return result;
    }

private:
    void Free()
    {
        if (!mElementPointers)
        {
            return;
        }

        for (size_t i = 0; i < mElementCount; i++)
        {
            if (mElementPointers[i] != nullptr)
            {
                chip::Platform::MemoryFree(mElementPointers[i]);
                mElementPointers[i] = nullptr;
            }
        }
        mElementPointers.Free();
        mElementCount = 0;
        mIsOk         = false;
    }

    bool mIsOk           = false;
    size_t mElementCount = 0;
    chip::Platform::ScopedMemoryBuffer<char *> mElementPointers;
};

} // namespace Minimal
} // namespace mdns
