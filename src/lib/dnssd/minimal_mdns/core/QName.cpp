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
#include <assert.h>
#include <strings.h>

#include "QName.h"

namespace mdns {
namespace Minimal {

bool SerializedQNameIterator::Next()
{
    return mIsValid && Next(true);
}

bool SerializedQNameIterator::Next(bool followIndirectPointers)
{
    if (!mIsValid)
    {
        return false;
    }

    while (true)
    {
        assert(mValidData.Contains(mCurrentPosition));

        const uint8_t length = *mCurrentPosition;
        if (*mCurrentPosition == 0)
        {
            // Done with all items
            return false;
        }

        if ((length & kPtrMask) == kPtrMask)
        {
            if (!followIndirectPointers)
            {
                // Stop at first indirect pointer
                return false;
            }

            // PTR contains 2 bytes
            if (!mValidData.Contains(mCurrentPosition + 1))
            {
                mIsValid = false;
                return false;
            }

            size_t offset = static_cast<size_t>(((*mCurrentPosition & 0x3F) << 8) | *(mCurrentPosition + 1));
            if (offset >= mLookBehindMax)
            {
                // Potential infinite recursion.
                mIsValid = false;
                return false;
            }
            if (offset > mValidData.Size())
            {
                // offset too large
                mIsValid = false;
                return false;
            }

            // Look behind has to keep going backwards, otherwise we may
            // get into an infinite list
            if (offset >= static_cast<size_t>(mCurrentPosition - mValidData.Start()))
            {
                mIsValid = false;
                return false;
            }

            mLookBehindMax   = offset;
            mCurrentPosition = mValidData.Start() + offset;
        }
        else
        {
            // This branch handles non-pointer data. This will be string of size [length].
            if (length > kMaxValueSize)
            {
                // value is too large (larger than RFC limit)
                mIsValid = false;
                return false;
            }

            if (!mValidData.Contains(mCurrentPosition + 1 + length))
            {
                // string outside valid data
                mIsValid = false;
                return false;
            }

            memcpy(mValue, mCurrentPosition + 1, length);
            mValue[length]   = '\0';
            mCurrentPosition = mCurrentPosition + length + 1;
            return true;
        }
    }
}

const uint8_t * SerializedQNameIterator::FindDataEnd()
{
    while (Next(false))
    {
        // nothing to do, just advance
    }

    if (!IsValid())
    {
        return nullptr;
    }

    // normal end
    if (*mCurrentPosition == 0)
    {
        // mCurrentPosition MUST already be valid
        return mCurrentPosition + 1;
    }

    // ends with a dataptr
    if ((*mCurrentPosition & kPtrMask) == kPtrMask)
    {
        if (!mValidData.Contains(mCurrentPosition + 1))
        {
            return nullptr;
        }
        return mCurrentPosition + 2;
    }

    // invalid data
    return nullptr;
}

bool SerializedQNameIterator::operator==(const FullQName & other) const
{
    SerializedQNameIterator self = *this; // allow iteration
    size_t idx                   = 0;

    while ((idx < other.nameCount) && self.Next())
    {
        if (strcasecmp(self.Value(), other.names[idx]) != 0)
        {
            return false;
        }
        idx++;
    }

    return ((idx == other.nameCount) && !self.Next());
}

bool SerializedQNameIterator::operator==(const SerializedQNameIterator & other) const
{
    SerializedQNameIterator a = *this; // allow iteration
    SerializedQNameIterator b = other;

    while (true)
    {
        bool hasA = a.Next();
        bool hasB = b.Next();

        if (hasA ^ hasB)
        {
            return false; /// one is longer than the other
        }

        if (!a.IsValid() || !b.IsValid())
        {
            return false; // invalid data
        }

        if (!hasA || !hasB)
        {
            break;
        }

        if (strcasecmp(a.Value(), b.Value()) != 0)
        {
            return false;
        }
    }

    return a.IsValid() && b.IsValid();
}

bool FullQName::operator==(const FullQName & other) const
{
    if (nameCount != other.nameCount)
    {
        return false;
    }
    for (size_t i = 0; i < nameCount; i++)
    {
        if (strcasecmp(names[i], other.names[i]) != 0)
        {
            return false;
        }
    }
    return true;
}

} // namespace Minimal
} // namespace mdns
