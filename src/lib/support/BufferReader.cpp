/* See Project CHIP LICENSE file for licensing information. */


#include "BufferReader.h"

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

// Explicit Read instantiations for the data types we want to support.
template void Reader::RawRead(uint8_t *);
template void Reader::RawRead(uint16_t *);
template void Reader::RawRead(uint32_t *);
template void Reader::RawRead(uint64_t *);

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
