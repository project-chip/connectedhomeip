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

#include <string>

#include <lib/support/Span.h>

namespace chip {

/**
 *  @brief Convert a CharSpan to an owning std::string.
 *
 *  Handles a default-constructed (`data()==nullptr`, `size()==0`) span without invoking
 *  `std::string(nullptr, 0)`, which is undefined behavior per the C++ standard. Both default
 *  spans and spans with `size()==0` but a non-null pointer produce an empty string; non-empty
 *  spans copy `size()` bytes verbatim.
 *
 *  Lives in its own header so `lib/support/Span.h` can stay free of `<string>` (it is included
 *  by core SDK headers compiled for embedded targets where `std::string` is undesirable).
 *  Callers that already pull `<string>` (e.g. controller / app / Darwin code) can include this
 *  header to deduplicate the otherwise-repeated
 *  `span.empty() ? std::string() : std::string(span.data(), span.size())` idiom.
 */
inline std::string CharSpanToStdString(CharSpan span)
{
    return span.empty() ? std::string() : std::string(span.data(), span.size());
}

} // namespace chip
