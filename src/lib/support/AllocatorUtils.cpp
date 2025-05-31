/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include "AllocatorUtils.h"

namespace chip {
namespace Platform {

CHIP_ERROR AllocateMemoryAndCopySpan(Platform::ScopedMemoryBufferWithSize<uint8_t> & scopedBuffer, ByteSpan span)
{
    if (!span.size())
    {
        return CHIP_ERROR_INVALID_MESSAGE_LENGTH;
    }

    if (!scopedBuffer.Alloc(span.size()))
    {
        ChipLogError(Support, "AllocateMemoryAndCopySpan failed");
        return CHIP_ERROR_NO_MEMORY;
    }
    memcpy(scopedBuffer.Get(), span.data(), scopedBuffer.AllocatedSize());
    return CHIP_NO_ERROR;
}

} // namespace Platform
} // namespace chip
