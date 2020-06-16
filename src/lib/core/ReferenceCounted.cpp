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

#include <type_traits>

#include <support/CodeUtils.h>

namespace chip {

/** Adds one to the usage count of this class */
template <class SUBCLASS>
SUBCLASS * ReferenceCounted<SUBCLASS>::Retain(void)
{
    VerifyOrDie(mRefCount < UINT8_MAX);
    ++mRefCount;

    return reinterpret_cast<SUBCLASS *>(this);
}

/** Release usage of this class */
template <class SUBCLASS>
void ReferenceCounted<SUBCLASS>::Release(void)
{
    VerifyOrDie(mRefCount != 0);

    if (--mRefCount == 0)
    {
        delete this;
    }
}

/** Get the current reference counter value */
template <class SUBCLASS>
uint8_t ReferenceCounted<SUBCLASS>::GetReferenceCount() const
{
    return mRefCount;
}

} // namespace chip
