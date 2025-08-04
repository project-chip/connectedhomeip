#include <cstddef>
#include <cstdint>
#include <iostream>

#include <setup_payload/Base38Decode.h>

using namespace chip;

/**
 *    @file
 *      This file describes a Fuzzer for decoding base38 encoded strings.
 */

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    std::string base38EncodedString(reinterpret_cast<const char *>(data), len);
    std::vector<uint8_t> decodedData;

    // Ignoring return value, because in general the data is garbage and won't decode properly.
    // We're just testing that the decoder does not crash on the fuzzer-generated inputs.
    chip::base38Decode(base38EncodedString, decodedData);

    return 0;
}
