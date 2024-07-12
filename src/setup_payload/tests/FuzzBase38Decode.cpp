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
    CHIP_ERROR err = chip::base38Decode(base38EncodedString, decodedData);

    return 0;
}
