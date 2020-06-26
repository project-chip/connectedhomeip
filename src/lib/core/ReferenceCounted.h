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

#ifndef REFERENCE_COUNTED_H_
#define REFERENCE_COUNTED_H_

#include <stdlib.h>

namespace chip {

/**
 * A reference counted object maintains a count of usages and when the usage
 * count drops to 0, it deletes itself.
 */
template <class SUBCLASS>
class ReferenceCounted
{
public:
    virtual ~ReferenceCounted() {}

    /** Adds one to the usage count of this class */
    SUBCLASS * Retain(void)
    {
        if (mRefCount == UINT8_MAX)
        {
            abort();
        }
        ++mRefCount;

        return reinterpret_cast<SUBCLASS *>(this);
    }

    /** Release usage of this class */
    void Release(void)
    {
        if (mRefCount == 0)
        {
            abort();
        }

        if (--mRefCount == 0)
        {
            delete this;
        }
    }

    /** Get the current reference counter value */
    uint8_t GetReferenceCount() const { return mRefCount; }

private:
    uint8_t mRefCount = 1;
};

} // namespace chip

#endif // REFERENCE_COUNTED_H_
