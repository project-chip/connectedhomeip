/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "BufferReader.h"

#include <string.h>

namespace chip {
namespace Encoding {
namespace LittleEndian {

namespace {
// These helper methods return void and put the value being read into an
// outparam because that allows us to easily overload on the type of the
// thing being read.
void ReadHelper(const uint8_t *& p, uint8_t * dest)
{
    *dest = Read8(p);
}
void ReadHelper(const uint8_t *& p, uint16_t * dest)
{
    *dest = Read16(p);
}
void ReadHelper(const uint8_t *& p, uint32_t * dest)
{
    *dest = Read32(p);
}
void ReadHelper(const uint8_t *& p, uint64_t * dest)
{
    *dest = Read64(p);
}
} // anonymous namespace

template <typename T>
void Reader::RawRead(T * retval)
{
    static_assert(CHAR_BIT == 8, "Our various sizeof checks rely on bytes and octets being the same thing");

    static constexpr size_t data_size = sizeof(T);

    if (mAvailable < data_size)
    {
        mStatus = CHIP_ERROR_BUFFER_TOO_SMALL;
        // Ensure that future reads all fail.
        mAvailable = 0;
        return;
    }

    ReadHelper(mReadPtr, retval);
    mAvailable = static_cast<uint16_t>(mAvailable - data_size);
}

Reader & Reader::ReadBytes(uint8_t * dest, size_t size)
{
    static_assert(CHAR_BIT == 8, "Our various sizeof checks rely on bytes and octets being the same thing");

    if ((size > UINT16_MAX) || (mAvailable < size))
    {
        mStatus = CHIP_ERROR_BUFFER_TOO_SMALL;
        // Ensure that future reads all fail.
        mAvailable = 0;
        return *this;
    }

    memcpy(dest, mReadPtr, size);

    mReadPtr += size;
    mAvailable = static_cast<uint16_t>(mAvailable - size);
    return *this;
}

// Explicit Read instantiations for the data types we want to support.
template void Reader::RawRead(uint8_t *);
template void Reader::RawRead(uint16_t *);
template void Reader::RawRead(uint32_t *);
template void Reader::RawRead(uint64_t *);

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
