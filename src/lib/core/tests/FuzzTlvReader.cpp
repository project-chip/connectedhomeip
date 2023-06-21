#include <cstddef>
#include <cstdint>

#include "lib/core/TLV.h"
#include "lib/core/TLVUtilities.h"

using namespace chip;
using namespace chip::TLV;

using chip::TLV::TLVReader;

static CHIP_ERROR FuzzIterator(TLVReader & aReader, size_t aDepth, void * aContext)
{
    {
        aReader.GetLength();
    }
    {
        aReader.GetTag();
    }
    {
        aReader.GetType();
    }
    {
        size_t size;
        aReader.CountRemainingInContainer(&size);
    }
    {
        bool v;
        aReader.Get(v);
    }
    {
        int8_t v;
        aReader.Get(v);
    }
    {
        int16_t v;
        aReader.Get(v);
    }
    {
        int32_t v;
        aReader.Get(v);
    }
    {
        int64_t v;
        aReader.Get(v);
    }
    {
        uint8_t v;
        aReader.Get(v);
    }
    {
        uint16_t v;
        aReader.Get(v);
    }
    {
        uint32_t v;
        aReader.Get(v);
    }
    {
        uint64_t v;
        aReader.Get(v);
    }
    {
        double v;
        aReader.Get(v);
    }
    {
        float v;
        aReader.Get(v);
    }
    {
        chip::ByteSpan readerSpan;
        aReader.Get(readerSpan);
    }
    {
        chip::CharSpan readerSpan;
        aReader.Get(readerSpan);
    }
    {
        uint8_t bBuf[16];
        aReader.GetBytes(bBuf, sizeof(bBuf));
    }
    {
        char sBuf[16];
        aReader.GetString(sBuf, sizeof(sBuf));
    }

    return CHIP_NO_ERROR;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    TLVReader reader;
    reader.Init(data, len);
    chip::TLV::Utilities::Iterate(reader, FuzzIterator, nullptr);

    return 0;
}
