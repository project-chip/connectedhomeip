#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "credentials/CHIPCert.h"

namespace {

using namespace chip;
using namespace chip::Credentials;

using namespace fuzztest;

void ChipCertFuzzer(const std::vector<std::uint8_t> & bytes)
{
    ByteSpan span(bytes.data(), bytes.size());

    {
        NodeId nodeId;
        FabricId fabricId;
        (void) ExtractFabricIdFromCert(span, &fabricId);
        (void) ExtractNodeIdFabricIdFromOpCert(span, &nodeId, &fabricId);
    }

    {
        CATValues cats;
        (void) ExtractCATsFromOpCert(span, cats);
    }

    {
        Credentials::P256PublicKeySpan key;
        (void) ExtractPublicKeyFromChipCert(span, key);
    }

    {
        chip::System::Clock::Seconds32 rcacNotBefore;
        (void) ExtractNotBeforeFromChipCert(span, rcacNotBefore);
    }

    {
        Credentials::CertificateKeyId skid;
        (void) ExtractSKIDFromChipCert(span, skid);
    }

    {
        ChipDN subjectDN;
        (void) ExtractSubjectDNFromChipCert(span, subjectDN);
    }

    {
        uint8_t outCertBuf[kMaxDERCertLength];
        MutableByteSpan outCert(outCertBuf);
        (void) ConvertChipCertToX509Cert(span, outCert);
    }

    {
        // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ValidateChipRCAC(span);
        chip::Platform::MemoryShutdown();
    }
}

FUZZ_TEST(FuzzChipCert, ChipCertFuzzer).WithDomains(Arbitrary<std::vector<std::uint8_t>>());

// The Property function for DecodeChipCertFuzzer, The FUZZ_TEST Macro will call this function.
void DecodeChipCertFuzzer(const std::vector<std::uint8_t> & bytes, BitFlags<CertDecodeFlags> aDecodeFlag)
{
    ByteSpan span(bytes.data(), bytes.size());

    // TODO: #34352 To Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

    ChipCertificateData certData;
    (void) DecodeChipCert(span, certData, aDecodeFlag);

    chip::Platform::MemoryShutdown();
}

// This function allows us to fuzz using one of three CertDecodeFlags flags; by using FuzzTests's `ElementOf` API, we define an
// input domain by explicitly enumerating the set of values in it More Info:
// https://github.com/google/fuzztest/blob/main/doc/domains-reference.md#elementof-domains-element-of
auto AnyCertDecodeFlag()
{
    constexpr BitFlags<CertDecodeFlags> NullDecodeFlag;
    constexpr BitFlags<CertDecodeFlags> GenTBSHashFlag(CertDecodeFlags::kGenerateTBSHash);
    constexpr BitFlags<CertDecodeFlags> TrustAnchorFlag(CertDecodeFlags::kIsTrustAnchor);

    return ElementOf<CertDecodeFlags>({ NullDecodeFlag, GenTBSHashFlag, TrustAnchorFlag });
}

FUZZ_TEST(FuzzChipCert, DecodeChipCertFuzzer).WithDomains(Arbitrary<std::vector<std::uint8_t>>(), AnyCertDecodeFlag());
} // namespace
