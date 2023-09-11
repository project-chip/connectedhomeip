/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "StringBuilder.h"

namespace chip {

StringBuilderBase & StringBuilderBase::AddFormat(const char * format, ...)
{
    va_list args;
    va_start(args, format);

    char * output = nullptr;
    if (mWriter.Available() > 0)
    {
        output = reinterpret_cast<char *>(mWriter.Buffer() + mWriter.Needed());
    }
    else
    {
        output = reinterpret_cast<char *>(mWriter.Buffer() + mWriter.Size());
    }

    // the + 1 size here because StringBuilder reserves one byte for final null terminator
    int needed = vsnprintf(output, mWriter.Available() + 1, format, args);

    // on invalid formats, printf-style methods return negative numbers
    if (needed > 0)
    {
        mWriter.Skip(static_cast<size_t>(needed));
    }

    va_end(args);
    NullTerminate();
    return *this;
}

StringBuilderBase & StringBuilderBase::AddMarkerIfOverflow()
{
    if (mWriter.Fit())
    {
        return *this;
    }

    for (unsigned i = 0; i < 3; i++)
    {
        if (mWriter.Size() >= i + 1)
        {
            mWriter.Buffer()[mWriter.Size() - i - 1] = '.';
        }
    }
    return *this;
}
} // namespace chip
