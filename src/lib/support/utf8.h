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
namespace Utf8 {

/**
 * Validate that the given span looks like a valid UTF-8 string
 *
 * UTF-8 encoding described at
 * https://www.unicode.org/versions/Unicode12.0.0/UnicodeStandard-12.0.pdf
 * but TLDR is:
 *
 * | Scalar Value               | First B  | Second B | Third B  | Fourth B |
 * +----------------------------+----------+----------+----------+----------+
 * | 00000000 0xxxxxxx          | 0xxxxxxx |          |          |          |
 * | 00000yyy yyxxxxxx          | 110yyyyy | 10xxxxxx |          |          |
 * | zzzzyyyy yyxxxxxx          | 1110zzzz | 10yyyyyy | 10xxxxxx |          |
 * | 000uuuuu zzzzyyyy yyxxxxxx | 11110uuu | 10uuzzzz | 10yyyyyy | 10xxxxxx |
 * +----------------------------+----------+----------+----------+----------+
 *
 * Further more, individual encodings must be minimally encoded, so
 * 0xC0 is not a valid first byte( 1100_0000 10xxxxxx could have been encoded
 * as one-byte)
 */
bool IsValid(CharSpan span);

} // namespace Utf8
} // namespace chip
