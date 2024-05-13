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

#include <lib/core/CHIPEncoding.h>

#include <string.h>
#include <type_traits>

namespace chip {
namespace Encoding {
namespace LittleEndian {

namespace {

// This helper methods return void and put the value being read into an
// outparam because that allows us to easily overload on the type of the
// thing being read.
void ReadHelper(const uint8_t * p, bool * dest)
{
    *dest = (*p != 0);
}

template <typename T>
void ReadHelper(const uint8_t * p, T * dest)
{
    std::make_unsigned_t<T> result;
    memcpy(&result, p, sizeof(result));
    result = chip::Encoding::LittleEndian::HostSwap(result);

    *dest = static_cast<T>(result);
}

} // anonymous namespace

template <typename T>
void Reader::RawReadLowLevelBeCareful(T * retval)
{
    static_assert(CHAR_BIT == 8, "Our various sizeof checks rely on bytes and octets being the same thing");
    static_assert((-1 & 3) == 3, "LittleEndian::BufferReader only works with 2's complement architectures.");

    VerifyOrReturn(IsSuccess());

    constexpr size_t data_size = sizeof(T);

    if (mAvailable < data_size)
    {
        mStatus = CHIP_ERROR_BUFFER_TOO_SMALL;
        // Ensure that future reads all fail.
        mAvailable = 0;
        return;
    }

    ReadHelper(mReadPtr, retval);
    mReadPtr += data_size;

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
template void Reader::RawReadLowLevelBeCareful(char *);
template void Reader::RawReadLowLevelBeCareful(bool *);
template void Reader::RawReadLowLevelBeCareful(int8_t *);
template void Reader::RawReadLowLevelBeCareful(int16_t *);
template void Reader::RawReadLowLevelBeCareful(int32_t *);
template void Reader::RawReadLowLevelBeCareful(int64_t *);
template void Reader::RawReadLowLevelBeCareful(uint8_t *);
template void Reader::RawReadLowLevelBeCareful(uint16_t *);
template void Reader::RawReadLowLevelBeCareful(uint32_t *);
template void Reader::RawReadLowLevelBeCareful(uint64_t *);

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
