#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string.h>

#include <setup_payload/Base38Decode.h>
#include <setup_payload/Base38Encode.h>

using namespace chip;

/**
 *    @file
 *      This file describes a base38 roundtrip Fuzzer.
 *      It starts by encoding the fuzzing value passed
 *      in Base38. The value encoded will then be decoded.
 *      The fuzzer verify that the decoded value is the same
 *      as the one in input.
 */

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    size_t outputSizeNeeded     = base38EncodedLength(len);
    const size_t kMaxOutputSize = 512;

    if (outputSizeNeeded > kMaxOutputSize)
    {
        return 0;
    }

    ByteSpan span(data, len);
    char encodedBuf[kMaxOutputSize];
    MutableCharSpan encodedSpan(encodedBuf);
    CHIP_ERROR encodingError = base38Encode(span, encodedSpan);

    if (encodingError != CHIP_NO_ERROR)
    {
        __builtin_trap();
    }

    std::string base38EncodedString(encodedSpan.data(), encodedSpan.size());

    std::vector<uint8_t> decodedData;
    CHIP_ERROR decodingError = base38Decode(base38EncodedString, decodedData);

    if (decodingError == CHIP_NO_ERROR)
    {
        if (decodedData.size() != len)
        {
            __builtin_trap();
        }

        if (memcmp(data, decodedData.data(), len) != 0)
        {
            __builtin_trap();
        }
    }
    else
    {
        __builtin_trap();
    }
    return 0;
}
