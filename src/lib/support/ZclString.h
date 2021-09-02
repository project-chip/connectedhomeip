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
#include <lib/support/Span.h>

#pragma once

namespace chip {

/**
 * @brief Create ZCL string (pascal string) from char string.
 * The maximum size of the ZCL string is defined by the kBufferMaximumSize.
 * In case char string exceeds the maximum of size ZCL string, the string will be 0-sized
 * and the method will return an error.
 */
extern CHIP_ERROR MakeZclCharString(MutableByteSpan & buffer, const char * cString);

} // namespace chip
