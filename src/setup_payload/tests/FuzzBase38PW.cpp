#include <cstddef>
#include <cstdint>
#include <iostream>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "setup_payload/QRCodeSetupPayloadParser.h"
#include <setup_payload/Base38Decode.h>
#include <setup_payload/Base38Encode.h>

namespace {

using namespace fuzztest;
using namespace chip;

// The property Function
void Base38DecodeFuzz(const std::vector<uint8_t> & bytes)
{
    std::string base38EncodedString(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    std::vector<uint8_t> decodedData;

    // Ignoring return value, because in general the data is garbage and won't decode properly.
    // We're just testing that the decoder does not crash on the fuzzer-generated inputs.
    chip::base38Decode(base38EncodedString, decodedData);
}

// The invocation of the FuzzTest
FUZZ_TEST(Base38Decoder, Base38DecodeFuzz).WithDomains(Arbitrary<std::vector<uint8_t>>());

/* The property function for a base38 roundtrip Fuzzer.
 * It starts by encoding the fuzzing value passed
 * into Base38. The encoded value will then be decoded.
 *
 * The fuzzer verifies that the decoded value is the same
 * as the one in input.*/
void Base38RoundTripFuzz(const std::vector<uint8_t> & bytes)
{

    size_t outputSizeNeeded     = base38EncodedLength(bytes.size());
    const size_t kMaxOutputSize = 512;

    ASSERT_LT(outputSizeNeeded, kMaxOutputSize);

    ByteSpan span(bytes.data(), bytes.size());

    char encodedBuf[kMaxOutputSize];
    MutableCharSpan encodedSpan(encodedBuf);
    CHIP_ERROR encodingError = base38Encode(span, encodedSpan);
    ASSERT_EQ(encodingError, CHIP_NO_ERROR);

    std::string base38EncodedString(encodedSpan.data(), encodedSpan.size());

    std::vector<uint8_t> decodedData;
    CHIP_ERROR decodingError = base38Decode(base38EncodedString, decodedData);

    ASSERT_EQ(decodingError, CHIP_NO_ERROR);

    // Make sure that decoded data is equal to the original fuzzed input; the bytes vector
    ASSERT_EQ(decodedData, bytes);
}

// Max size of the vector is defined as 306 since that will give an outputSizeNeeded of 511 which is less than the required
// kMaxOutputSize
FUZZ_TEST(Base38Decoder, Base38RoundTripFuzz).WithDomains(Arbitrary<std::vector<uint8_t>>().WithMaxSize(306));

void FuzzQRCodeSetupPayloadParser(const std::string & s)
{
    chip::Platform::MemoryInit();

    SetupPayload payload;
    QRCodeSetupPayloadParser(s).populatePayload(payload);
}

FUZZ_TEST(Base38Decoder, FuzzQRCodeSetupPayloadParser).WithDomains(Arbitrary<std::string>());

} // namespace
