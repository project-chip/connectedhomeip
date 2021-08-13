/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <support/Span.h>

#pragma once

namespace chip {

/**
 * @brief Create ZCL string from char string.  If the char string is longer than 254 chars, or too long to fit into the provided
 * buffer, this function will place as many chars as it can, up to 254, into the buffer and then return an error.
 *
 * Even if an error is returned, the buffer still holds a valid (though truncated) ZCL string, _unless_ the buffer is 0-sized, in
 * which case it can't hold anything.
 */
extern CHIP_ERROR MakeZclCharString(MutableByteSpan & buffer, const char * cString);

} // namespace chip
