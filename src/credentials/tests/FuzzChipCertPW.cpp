#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "credentials/CHIPCert.h"

namespace {

using namespace chip;
using namespace chip::Credentials;
using namespace fuzztest;

/*-----------------------------------  Helper Functions: Seed Providers   -----------------------------------*/
/******************************************************************************************************************* */

const std::string OpCertsDir = "credentials/test/operational-certificates-error-cases/";
auto isChipFile              = [](std::string_view name) { return absl::EndsWith(name, ".chip"); };
auto isDerFile               = [](std::string_view name) { return absl::EndsWith(name, ".der"); };
auto isChipRCACFile = [](std::string_view name) { return absl::StrContains(name, "RCAC") && absl::EndsWith(name, ".chip"); };

// Lambda that reads certificates from a directory and returns them as a vector of strings, to be used as seeds
auto seedProvider = [](auto filterFunction) -> std::vector<std::string> {
    // fuzztest::ReadFilesFromDirectory returns a vector of tuples, each tuple contains a file
    // We need to unpack the tuples and then extract file content into a vector of strings.
    std::vector<std::tuple<std::string>> tupleVector = ReadFilesFromDirectory(OpCertsDir, filterFunction);
    std::vector<std::string> seeds;

    if (tupleVector.size() == 0)
    {
        std::cout << "No Matching Seed files found in the chosen directory" << std::endl;
    }
    // DEBUG TIP: print tupleVector.size() here to check that we have the correct number of files as seeds.
    for (auto & [fileContents] : tupleVector)
    {
        seeds.push_back(fileContents);
    }
    return seeds;
};

void ChipCertFuzzer(const std::string & fuzzChipCerts)
{
    ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzChipCerts.data()), fuzzChipCerts.size());

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
}
FUZZ_TEST(FuzzChipCert, ChipCertFuzzer).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/*-----------------------------------  Chip Cert FuzzTests  -----------------------------------*/
/******************************************************************************************************************* */

// The Property function for DecodeChipCertFuzzer, The FUZZ_TEST Macro will call this function.
void DecodeChipCertFuzzer(const std::string & fuzzChipCerts, BitFlags<CertDecodeFlags> aDecodeFlag)
{
    // TODO: #34352 To Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzChipCerts.data()), fuzzChipCerts.size());

        ChipCertificateData certData;
        (void) DecodeChipCert(span, certData, aDecodeFlag);
    }
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

FUZZ_TEST(FuzzChipCert, DecodeChipCertFuzzer)
    .WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)), AnyCertDecodeFlag());

/*************************************** */

void ConvertChipCertToX509CertFuzz(const std::string & fuzzChipCerts)
{
    ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzChipCerts.data()), fuzzChipCerts.size());

    uint8_t outCertBuf[kMaxDERCertLength];
    MutableByteSpan outCert(outCertBuf);
    (void) ConvertChipCertToX509Cert(span, outCert);
}
FUZZ_TEST(FuzzChipCert, ConvertChipCertToX509CertFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/******************************************************************************** */
/******************************************************************************** */

void ValidateChipRCACFuzz(const std::string & fuzzRcacCerts)
{
    // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzRcacCerts.data()), fuzzRcacCerts.size());
        ValidateChipRCAC(span);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, ValidateChipRCACFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipRCACFile)));

/*-----------------------------------  DER Cert FuzzTests  -----------------------------------*/
/******************************************************************************** */

void ConvertX509CertToChipCertFuzz(const std::string & fuzzDerCerts)
{
    ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzDerCerts.data()), fuzzDerCerts.size());

    uint8_t outCertBuf[kMaxDERCertLength];
    MutableByteSpan outCert(outCertBuf);

    ConvertX509CertToChipCert(span, outCert);
}
FUZZ_TEST(FuzzChipCert, ConvertX509CertToChipCertFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isDerFile)));

/******************************************************************************** */
/******************************************************************************** */

void ExtractSubjectDNFromX509CertFuzz(const std::string & fuzzDerCerts)
{
    ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzDerCerts.data()), fuzzDerCerts.size());
    ChipDN subjectDN;
    ExtractSubjectDNFromX509Cert(span, subjectDN);
}
FUZZ_TEST(FuzzChipCert, ExtractSubjectDNFromX509CertFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isDerFile)));

} // namespace
