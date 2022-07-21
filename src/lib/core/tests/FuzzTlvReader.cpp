#include <cstddef>
#include <cstdint>

#include "lib/core/CHIPTLV.h"
#include "lib/core/CHIPTLVUtilities.hpp"

using chip::TLV::TLVReader;

static CHIP_ERROR FuzzIterator(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    return CHIP_NO_ERROR;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    TLVReader reader;
    reader.Init(data, len);
    chip::TLV::Utilities::Iterate(reader, FuzzIterator, nullptr);

    return 0;
}
