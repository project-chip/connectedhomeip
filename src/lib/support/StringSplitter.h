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

#include <lib/support/CHIPMemString.h>

#include <stdlib.h>
#include <string.h>

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
    StringSplitter(char * s, char separator) : mNext(s), mSeparator(separator) {
        if ((mNext != nullptr) && (*mNext == '\0')) {
            mNext = nullptr; // end of string right away
        }
    }

    // returns null when no more items available
    const char * Next()
    {
        char * current = mNext;
        if (current == nullptr)
        {
            return nullptr; // nothing left
        }

        char * comma = strchr(current, mSeparator);

        if (comma != nullptr)
        {
            mNext  = comma + 1;
            *comma = '\0';
        }
        else
        {
            // last element, position on the final 0
            mNext = nullptr;
        }

        return current;
    }

protected:
    char * mNext; // next element to return by calling Next()
    char mSeparator;
};

/// A string splitter that works on a copy of the given string
/// using strdup()
class StrdupStringSplitter : public StringSplitter
{
public:
    StrdupStringSplitter(const char * s, char separator) : StringSplitter(nullptr, separator)
    {
        if ((s != nullptr) && (*s != '\0'))
        {
            mData = strdup(s);
            VerifyOrDie(mData != nullptr);
            mNext = mData;
        }
        else
        {
            mData = nullptr;
        }
    }

    ~StrdupStringSplitter()
    {
        if (mData != nullptr)
        {
            free(mData);
        }
    }

private:
    char * mData;
};

/// StringSplitter working on a locally allocated buffer copy
///
/// If input is larger than the internal size, it will be truncated.
template <size_t N>
class FixedStringSplitter : public StringSplitter
{
public:
    FixedStringSplitter(const char * s, char separator) : StringSplitter(nullptr, separator)
    {
        if (s != nullptr)
        {
            chip::Platform::CopyString(mData, s);
            mNext = mData;
        }
        else
        {
            mData[0] = '\0';
            mNext = nullptr;
        }
    }

private:
    char mData[N];
};

} // namespace chip
