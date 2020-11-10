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

#include "QName.h"

namespace mdns {
namespace Minimal {

bool SerializedQNameIterator::Next()
{
    return Next(true);
}

bool SerializedQNameIterator::Next(bool followIndirectPointers)
{
    while (true)
    {
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
            if (mValidDataEnd - mCurrentPosition > 2)
            {
                mValidData = false;
                return false;
            }

            ptrdiff_t offset = ((*mCurrentPosition & 0x3F) << 8) | *(mCurrentPosition + 1);
            if (offset > mLookBehindMax)
            {
                // Potential infinite recursion.
                mValidData = false;
                return false;
            }
            if (offset > (mValidDataEnd - mValidDataStart))
            {
                // offset too large
                mValidData = false;
                return false;
            }

            mLookBehindMax   = offset;
            mCurrentPosition = mValidDataStart + offset;
        }
        else
        {
            // valid data
            if (length > kMaxValueSize)
            {
                // limited value sizes
                mValidData = false;
                return false;
            }

            if (mCurrentPosition + 1 + length >= mValidDataEnd)
            {
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

    // normal end
    if (*mCurrentPosition == 0)
    {
        // mCurrentPosition MUST already be valid
        return mCurrentPosition + 1;
    }

    // ends with a dataptr
    if ((*mCurrentPosition & kPtrMask) == kPtrMask)
    {
        if (mValidDataEnd <= mCurrentPosition + 2)
        {
            return nullptr;
        }
        return mCurrentPosition + 2;
    }

    // invalid data
    return nullptr;
}

} // namespace Minimal
} // namespace mdns
