
#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "lib/core/TLV.h"
#include "lib/core/TLVUtilities.h"

namespace {

using chip::TLV::TLVReader;

using namespace fuzztest;

static CHIP_ERROR FuzzIterator(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    aReader.GetLength();
    aReader.GetTag();
    aReader.GetType();
    return CHIP_NO_ERROR;
}

// The Property Function
void FuzzTlvReader(const std::vector<std::uint8_t> & bytes)
{
    TLVReader reader;
    reader.Init(bytes.data(), bytes.size());
    chip::TLV::Utilities::Iterate(reader, FuzzIterator, nullptr);
}
// Fuzz tests are instantiated with the FUZZ_TEST macro
FUZZ_TEST(TLVReader, FuzzTlvReader).WithDomains(Arbitrary<std::vector<std::uint8_t>>());

} // namespace
