/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/Span.h>

namespace chip {

/// Provides the ability to split a given string by a character.
///
/// Converts things like:
///   "a,b,c" split by ',': "a", "b", "c"
///   ",b,c" split by ',': "", "b", "c"
///   "a,,c" split by ',': "a", "", "c"
///   "a," split by ',': "a", ""
///   ",a" split by ',': "", "a"
///
///
/// WARNING: WILL DESTRUCTIVELY MODIFY THE STRING IN PLACE
///
class StringSplitter
{
public:
    StringSplitter(const char * s, char separator) : mNext(s), mSeparator(separator)
    {
        if ((mNext != nullptr) && (*mNext == '\0'))
        {
            mNext = nullptr; // end of string right away
        }
    }

    /// Returns the next character span
    ///
    /// out - contains the next element or a nullptr/0 sized span if
    ///       no elements available
    ///
    /// Returns true if an element is available, false otherwise.
    bool Next(CharSpan & out)
    {
        if (mNext == nullptr)
        {
            out = CharSpan();
            return false; // nothing left
        }

        const char * end = mNext;
        while ((*end != '\0') && (*end != mSeparator))
        {
            end++;
        }

        if (*end != '\0')
        {
            // intermediate element
            out   = CharSpan(mNext, static_cast<size_t>(end - mNext));
            mNext = end + 1;
        }
        else
        {
            // last element
            out   = CharSpan::fromCharString(mNext);
            mNext = nullptr;
        }

        return true;
    }

protected:
    const char * mNext; // start of next element to return by Next()
    const char mSeparator;
};

} // namespace chip
