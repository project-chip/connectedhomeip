/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/// Provides the ability to split a given span into spans of or below a given size
template <size_t kSize>
class ChunkSplitter
{
public:
    ChunkSplitter(CharSpan s) : span(s) {}

    /// Returns the next character span
    ///
    /// out - contains the next span of size at most kSize or an empty span if no more elements are available
    ///
    /// Returns true if an element is available, false otherwise.
    bool Next(CharSpan & out)
    {
        if (offset >= span.size())
        {
            out = CharSpan();
            return false; // nothing left
        }
        else
        {
            size_t nextSize = std::min(kSize, span.size() - offset);
            out             = span.SubSpan(offset, nextSize);
            offset += nextSize;
            return true;
        }
    }

protected:
    const CharSpan span; // the full span to split
    size_t offset = 0;   // the start of the next chunk to return
};

} // namespace chip
