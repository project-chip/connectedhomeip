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
/// WARNING: WILL DESTRUCTIVELY MODIFY THE STRING IN PLACE
///
class StringSplitter
{
public:
    StringSplitter(char * s, char separator) : mNext(s), mSeparator(separator) {}

    // returns null when no more items available
    const char * Next()
    {
        char * current = mNext;
        if ((current == nullptr) || (*current == '\0'))
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
            mNext = current + strlen(current);
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
        if (s != nullptr)
        {
            mData = strdup(s);
            VerifyOrDie(mData != nullptr);
            mNext = mData;
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
    FixedStringSplitter(const char * s, char separator) : StringSplitter(mData, separator)
    {
        if (s != nullptr)
        {
            chip::Platform::CopyString(mData, s);
        }
        else
        {
            mData[0] = '\0';
        }
    }

private:
    char mData[N];
};

} // namespace chip
