#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "credentials/CHIPCert.h"
#include "credentials/CHIPCertificateSet.h"
#include "credentials/attestation_verifier/TestPAAStore.h"
#include "credentials/tests/CHIPAttCert_test_vectors.h"
#include "crypto/CHIPCryptoPAL.h"

namespace {

using namespace chip;
using namespace chip::Credentials;
using namespace fuzztest;

// Small C++17 helpers — abseil is not propagated through pw_fuzzer's public include path,
// so we rely on standard library primitives instead of absl::EndsWith / absl::StrContains.
constexpr bool EndsWithSv(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}
constexpr bool ContainsSv(std::string_view text, std::string_view needle)
{
    return text.find(needle) != std::string_view::npos;
}

/*-----------------------------------  Helper Functions: Seed Providers   -----------------------------------*/
/******************************************************************************************************************* */

const std::string OpCertsDir = "credentials/test/operational-certificates-error-cases/";

// Lower bounds for the in-tree seed corpus (currently 172 .chip / 100 .der error-case vectors).
// Set well below the real counts so ordinary corpus churn does not fail the build, but a broken
// path or filter (0 / a handful of files) does. Enforced by SeedCorpusIsPresent below.
constexpr size_t kMinChipSeeds = 50;
constexpr size_t kMinDerSeeds  = 20;

auto isChipFile     = [](std::string_view name) { return EndsWithSv(name, ".chip"); };
auto isDerFile      = [](std::string_view name) { return EndsWithSv(name, ".der"); };
auto isChipRCACFile = [](std::string_view name) { return ContainsSv(name, "RCAC") && EndsWithSv(name, ".chip"); };
auto isChipNOCFile  = [](std::string_view name) {
    return ContainsSv(name, "NOC") && ContainsSv(name, "Cert") && EndsWithSv(name, ".chip");
};

// Lambda that reads certificates from a directory and returns them as a vector of strings, to be used as seeds
auto seedProvider = [](auto filterFunction) -> std::vector<std::string> {
    std::vector<std::string> seeds;

    // The seed directory ships in the source tree but is absent in the OSS-Fuzz runner, which
    // runs fuzzers from a temporary directory without the build tree (check_build does so
    // deliberately, to reject $OUT-relative dependencies). Skip file-based seeding instead of
    // aborting when the directory is missing; on OSS-Fuzz the seeds are supplied via the
    // libFuzzer seed corpus (<target>_seed_corpus.zip) instead.
    // Non-throwing overload: a permission/I/O error must not abort the harness (the build
    // disables exceptions), which would defeat the purpose of tolerating a missing directory.
    std::error_code ec;
    if (!std::filesystem::is_directory(OpCertsDir, ec))
    {
        std::cout << "Seed directory '" << OpCertsDir << "' not found or inaccessible; continuing without file seeds" << std::endl;
        return seeds;
    }

    // fuzztest::ReadFilesFromDirectory returns a vector of tuples, each tuple contains a file
    // We need to unpack the tuples and then extract file content into a vector of strings.
    std::vector<std::tuple<std::string>> tupleVector = ReadFilesFromDirectory(OpCertsDir, filterFunction);

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
        RETURN_SAFELY_IGNORED ExtractFabricIdFromCert(span, &fabricId);
        RETURN_SAFELY_IGNORED ExtractNodeIdFabricIdFromOpCert(span, &nodeId, &fabricId);
    }

    {
        CATValues cats;
        RETURN_SAFELY_IGNORED ExtractCATsFromOpCert(span, cats);
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
        RETURN_SAFELY_IGNORED DecodeChipCert(span, certData, aDecodeFlag);
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
    RETURN_SAFELY_IGNORED ConvertChipCertToX509Cert(span, outCert);
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
        RETURN_SAFELY_IGNORED ValidateChipRCAC(span);
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

    RETURN_SAFELY_IGNORED ConvertX509CertToChipCert(span, outCert);
}
FUZZ_TEST(FuzzChipCert, ConvertX509CertToChipCertFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isDerFile)));

/******************************************************************************** */
/******************************************************************************** */

void ExtractSubjectDNFromX509CertFuzz(const std::string & fuzzDerCerts)
{
    ByteSpan span(reinterpret_cast<const uint8_t *>(fuzzDerCerts.data()), fuzzDerCerts.size());
    ChipDN subjectDN;
    RETURN_SAFELY_IGNORED ExtractSubjectDNFromX509Cert(span, subjectDN);
}
FUZZ_TEST(FuzzChipCert, ExtractSubjectDNFromX509CertFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isDerFile)));

/*-----------------------------------  Frankencert-style Signature FuzzTest  -----------------------------------*/
/******************************************************************************************************************* */
//
// The Frankencert idea (Brubaker et al., S&P 2014) is to compose well-formed cert pieces from real corpora
// in recombined ways. We approximate it by giving the fuzzer independent string inputs, each seeded
// from a role-specific corpus, so mismatched issuer/subject and wrong-role pairings are on the table without a
// structure-aware mutator.
//
// NOTE: the operational-cert *chain*-validation harness that used to live here (LoadAndValidateChainFuzz,
// driving ChipCertificateSet::LoadCert/ValidateCert) was removed as redundant: FuzzCASE.HandleSigma3b already
// fuzzes that exact path via FabricTable::VerifyCredentials, seeded with a coherent Root01:ICA01:Node01_01
// identity, and reaches GetCertType/FindValidCert/ValidateCert far deeper than the byte-blob harness (which
// bounced off GetCertType). The (subject, signer) signature harness below is kept because it is the ONLY
// harness that reaches ChipCertificateSet::VerifyCertSignature's ECDSA path — HandleSigma3b never does.

/******************************************************************************** */

// Pair an independently-chosen (subject, signer) and run VerifyCertSignature. The seeds are role-agnostic
// so that every cross-pairing is exercised — a NOC signed by another NOC, an RCAC subject with an
// ICAC signer, etc. Bugs here would manifest as wrong-cert-accepted or as a crash inside the ECDSA path.
void VerifyCertSignatureFuzz(const std::string & subjectBytes, const std::string & signerBytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        BitFlags<CertDecodeFlags> generateHash(CertDecodeFlags::kGenerateTBSHash);

        ByteSpan subjectSpan(reinterpret_cast<const uint8_t *>(subjectBytes.data()), subjectBytes.size());
        ByteSpan signerSpan(reinterpret_cast<const uint8_t *>(signerBytes.data()), signerBytes.size());

        ChipCertificateData subject;
        ChipCertificateData signer;

        if (DecodeChipCert(subjectSpan, subject, generateHash) == CHIP_NO_ERROR &&
            DecodeChipCert(signerSpan, signer, generateHash) == CHIP_NO_ERROR)
        {
            RETURN_SAFELY_IGNORED VerifyCertSignature(subject, signer);
        }
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, VerifyCertSignatureFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)),
                 Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/*-----------------------------------  Gap-Filling: paths previously libFuzzer-only  ----------------------------------*/
/******************************************************************************************************************* */
//
// The harnesses below port API surfaces that existed as standalone libFuzzer harnesses
// (FuzzChipCertValidation.cpp, FuzzCertificateSet.cpp, FuzzOpCertExtractors.cpp) into pw_fuzzer
// FUZZ_TESTs so they are visible to the FuzzTest coverage workflow
// (scripts/tests/run_fuzztest_coverage.py) and benefit from seeded mutation.

void ValidateChipNetworkIdentityFuzz(const std::string & bytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());

        // 1-arg form: validate only.
        RETURN_SAFELY_IGNORED ValidateChipNetworkIdentity(span);

        // 2-arg form: also returns the network identifier (SKID-shaped output).
        uint8_t keyIdBuf[kKeyIdentifierLength];
        MutableCertificateKeyId keyId(keyIdBuf);
        RETURN_SAFELY_IGNORED ValidateChipNetworkIdentity(span, keyId);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, ValidateChipNetworkIdentityFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/******************************************************************************** */
// DecodeChipDN takes a TLVReader, not a ByteSpan. Construct the reader inline.
void DecodeChipDNFuzz(const std::string & bytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        TLV::TLVReader reader;
        reader.Init(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
        if (reader.Next() == CHIP_NO_ERROR)
        {
            ChipDN dn;
            RETURN_SAFELY_IGNORED dn.DecodeFromTLV(reader);
        }
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, DecodeChipDNFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/******************************************************************************** */
// Cross-cert helper that derives compressed fabric id from (rcac, noc). Two independent inputs
// so the fuzzer can pair mismatched roles — the function is one of the few that reads from two
// certificates simultaneously, and the compressed-fabric derivation involves an HKDF over the
// rcac public key plus the noc fabric id.
void OpCertCrossExtractorFuzz(const std::string & rcacBytes, const std::string & nocBytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan rcac(reinterpret_cast<const uint8_t *>(rcacBytes.data()), rcacBytes.size());
        ByteSpan noc(reinterpret_cast<const uint8_t *>(nocBytes.data()), nocBytes.size());

        // NB: current header param order is (rcac, noc, compressedFabricId, fabricId, nodeId).
        CompressedFabricId compressedFabricId = 0;
        NodeId nodeId                         = 0;
        FabricId fabricId                     = 0;
        RETURN_SAFELY_IGNORED
        ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(rcac, noc, compressedFabricId, fabricId, nodeId);

        // Also exercise the rcac-only public-key extraction.
        P256PublicKeySpan rcacPublicKey;
        RETURN_SAFELY_IGNORED ExtractPublicKeyFromChipCert(rcac, rcacPublicKey);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, OpCertCrossExtractorFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipRCACFile)),
                 Arbitrary<std::string>().WithSeeds(seedProvider(isChipNOCFile)));

/******************************************************************************** */
// Consolidated chip-cert extractors that the previous PW file did not exercise. These are all
// reachable from FabricTable's AddNewFabric path on every NOC the fabric admin writes.
void OpCertChipExtractorsFuzz(const std::string & bytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());

        {
            ChipDN dn;
            RETURN_SAFELY_IGNORED ExtractSubjectDNFromChipCert(span, dn);
        }
        {
            P256PublicKeySpan key;
            RETURN_SAFELY_IGNORED ExtractPublicKeyFromChipCert(span, key);
        }
        {
            CertificateKeyId skid;
            RETURN_SAFELY_IGNORED ExtractSKIDFromChipCert(span, skid);
        }
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, OpCertChipExtractorsFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)));

/******************************************************************************** */
// LoadCert via the TLVReader overload — distinct code path from the ByteSpan overload in
// LoadAndValidateChainFuzz. Reader-driven loading is what FabricTable + VendorIdVerificationClient
// use in the live commissioning path.
void LoadCertViaReaderFuzz(const std::string & bytes, uint8_t rawFlagByte)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        BitFlags<CertDecodeFlags> flags;
        flags.SetRaw(rawFlagByte);

        ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());

        ChipCertificateSet certSet;
        if (certSet.Init(/* maxCertsArraySize */ 4) == CHIP_NO_ERROR)
        {
            TLV::TLVReader reader;
            reader.Init(span.data(), span.size());
            if (reader.Next() == CHIP_NO_ERROR)
            {
                RETURN_SAFELY_IGNORED certSet.LoadCert(reader, flags, span);
            }
            certSet.Release();
        }
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, LoadCertViaReaderFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(seedProvider(isChipFile)), Arbitrary<uint8_t>());

/*-----------------------------------  Attestation chain (PAA -> PAI -> DAC)  ----------------------------------*/
/******************************************************************************************************************* */
//
// Attestation is the commissioner's first cryptographic gate. Every commissioned device hands the
// commissioner a DAC, a PAI, and (via DCL or hardcoded trust store) a PAA root. The commissioner
// must walk PAA -> PAI -> DAC and reject anything that doesn't validate. That walk is implemented
// in Crypto::ValidateCertificateChain — a totally distinct code path from ChipCertificateSet's
// TLV-chip-cert chain (which is for *operational* certs, not attestation).
//
// We seed each role with real DER-encoded test vectors from CHIPAttCert_test_vectors.h, so the
// fuzzer starts from inputs that decode cleanly and spends its budget exploring mutation rather
// than guessing valid DER. We also seed with PAA variants (ValInFuture, ValInPast, NoVID) to
// exercise the validity-window and VID-bound paths.

namespace AttestationSeeds {

inline std::vector<std::string> SeedSpansToVectorOfStrings(std::initializer_list<chip::ByteSpan> spans)
{
    std::vector<std::string> out;
    out.reserve(spans.size());
    for (const chip::ByteSpan & span : spans)
    {
        out.emplace_back(reinterpret_cast<const char *>(span.data()), span.size());
    }
    return out;
}

inline std::vector<std::string> PAASeeds()
{
    // The FIRST seed of each role domain (PAA/PAI/DAC) must form a coherent, currently-valid
    // chain so the pristine seed tuple passes Crypto::ValidateCertificateChain and exercises the
    // deep validation logic; without this the fuzzer bounces off the DER/chain gate.
    // Canonical valid chain: PAA_FFF1 -> PAI_FFF1_8000 -> DAC_FFF1_8000_0000
    // (see TestChipCryptoPAL.cpp ValidateCertificateChain success vectors).
    // sTestCert_PAA_FFF1_Cert lives in TestPAAStore.h (still chip::TestCerts), not
    // CHIPAttCert_test_vectors.h; it is linked via cert_test_vectors -> :test_paa_store.
    return SeedSpansToVectorOfStrings({
        chip::TestCerts::sTestCert_PAA_FFF1_Cert, // valid root, signs PAI_FFF1_8000
        chip::TestCerts::sTestCert_PAA_FFF2_ValInFuture_Cert,
        chip::TestCerts::sTestCert_PAA_FFF2_ValInPast_Cert,
        chip::TestCerts::sTestCert_PAA_NoVID_ToResignPAIs_Cert,
    });
}

inline std::vector<std::string> PAISeeds()
{
    return SeedSpansToVectorOfStrings({
        chip::TestCerts::sTestCert_PAI_FFF1_8000_Cert, // valid, issued by PAA_FFF1, signs DAC_FFF1_8000_0000
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAI_FFF2_8004_Cert,
    });
}

inline std::vector<std::string> DACSeeds()
{
    return SeedSpansToVectorOfStrings({
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0001_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0002_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_2CDPs_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_CDP_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_CDP_Long_Cert,
        chip::TestCerts::sTestCert_DAC_FFF1_8000_0000_CDP_Wrong_Prefix_Cert,
    });
}

inline std::vector<std::string> AttestationDERSeeds()
{
    // Mixed DER corpus across the three roles, for seeding harnesses that don't need role distinction.
    std::vector<std::string> all = PAASeeds();
    auto pai                     = PAISeeds();
    auto dac                     = DACSeeds();
    all.insert(all.end(), pai.begin(), pai.end());
    all.insert(all.end(), dac.begin(), dac.end());
    return all;
}

} // namespace AttestationSeeds

/******************************************************************************** */
// Real attestation chain walker. Calls into Crypto::ValidateCertificateChain which is what the
// DefaultDACVerifier::VerifyAttestationInformation pipeline uses.
void AttestationChainValidationFuzz(const std::string & rootDer, const std::string & caDer, const std::string & leafDer)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        Crypto::CertificateChainValidationResult result = Crypto::CertificateChainValidationResult::kSuccess;
        RETURN_SAFELY_IGNORED Crypto::ValidateCertificateChain(
            reinterpret_cast<const uint8_t *>(rootDer.data()), rootDer.size(), reinterpret_cast<const uint8_t *>(caDer.data()),
            caDer.size(), reinterpret_cast<const uint8_t *>(leafDer.data()), leafDer.size(), result);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, AttestationChainValidationFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(AttestationSeeds::PAASeeds()),
                 Arbitrary<std::string>().WithSeeds(AttestationSeeds::PAISeeds()),
                 Arbitrary<std::string>().WithSeeds(AttestationSeeds::DACSeeds()));

/******************************************************************************** */
// Format-only validator: enforces basicConstraints/keyUsage/EKU constraints per role.
// Each role has different rules — PAA must be self-signed CA, PAI must be CA with pathLen=0,
// DAC must not be CA. Wrong-role inputs and malformed extensions are the primary find class here.
void VerifyAttestationCertFormatFuzz(const std::string & derBytes, uint8_t certTypeRaw)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        chip::ByteSpan span(reinterpret_cast<const uint8_t *>(derBytes.data()), derBytes.size());

        // Map uint8_t -> AttestationCertType, cycling through the 3 valid values.
        Crypto::AttestationCertType type;
        switch (certTypeRaw % 3)
        {
        case 0:
            type = Crypto::AttestationCertType::kPAA;
            break;
        case 1:
            type = Crypto::AttestationCertType::kPAI;
            break;
        default:
            type = Crypto::AttestationCertType::kDAC;
            break;
        }
        RETURN_SAFELY_IGNORED Crypto::VerifyAttestationCertificateFormat(span, type);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, VerifyAttestationCertFormatFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(AttestationSeeds::AttestationDERSeeds()), Arbitrary<uint8_t>());

/*-----------------------------------  Better-seeded existing DER fuzzers  ------------------------------------*/
/******************************************************************************************************************* */
// Re-seed the round-trip-from-DER fuzzers with real attestation certs. The originals only had
// error-case .der files, so the fuzzer mostly bounced off the ASN.1 layer. With valid PAA/PAI/DAC
// seeds we get past DER decode and into the Matter-specific cert-shape checks.

void SeededConvertX509CertToChipCertFuzz(const std::string & derBytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(derBytes.data()), derBytes.size());
        uint8_t outCertBuf[kMaxDERCertLength];
        MutableByteSpan outCert(outCertBuf);
        RETURN_SAFELY_IGNORED ConvertX509CertToChipCert(span, outCert);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, SeededConvertX509CertToChipCertFuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(AttestationSeeds::AttestationDERSeeds()));

void SeededExtractSubjectDNFromX509Fuzz(const std::string & derBytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        ByteSpan span(reinterpret_cast<const uint8_t *>(derBytes.data()), derBytes.size());
        ChipDN subjectDN;
        RETURN_SAFELY_IGNORED ExtractSubjectDNFromX509Cert(span, subjectDN);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, SeededExtractSubjectDNFromX509Fuzz)
    .WithDomains(Arbitrary<std::string>().WithSeeds(AttestationSeeds::AttestationDERSeeds()));

/*-----------------------------------  Re-signing Frankencert FuzzTest  -----------------------------------*/
/******************************************************************************************************************* */
//
// Coverage-driven harness for ChipCertificateSet::ValidateCert. Every other cert harness only *reaches* the
// operational-cert validation logic via a static seed — any mutation breaks the signature/DER, so the validator
// bails before the field checks (VerifyCertSignature stays on its failure arm). This closes that gap the way real
// Frankencert testing does: hold a fixed in-harness CA (built once), let the fuzzer mutate the certificate
// *fields* (node/fabric id, serial, validity window, and the effective time the chain is validated at), then
// RE-SIGN the resulting cert with the CA key so the signature is always valid. Validation therefore proceeds past
// VerifyCertSignature into the validity-window / DN-classification / key-usage / cert-type logic on
// mutated-but-validly-signed input.
//
// No production code is stubbed or weakened: NewNodeOperationalX509Cert, ConvertX509CertToChipCert and
// ChipCertificateSet::ValidateCert are the real project functions; the CA + trust anchor mirror a production trust
// store. The CA (two P256 keygens + a self-signed root) is built once behind a function-local static, so
// per-input cost is one ECDSA sign + one DER->CHIP conversion + one validation.

struct ResignCaFixture
{
    Crypto::P256Keypair rootKeypair;
    Crypto::P256Keypair leafKeypair;
    ChipDN rootDN; // RCAC subject, reused as the NOC issuer DN
    uint8_t rcacChipBuf[kMaxCHIPCertLength];
    ByteSpan rcacChip;
    CHIP_ERROR status = CHIP_ERROR_INTERNAL;

    ResignCaFixture()
    {
        if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
            return;
        if (rootKeypair.Initialize(Crypto::ECPKeyTarget::ECDSA) != CHIP_NO_ERROR)
            return;
        if (leafKeypair.Initialize(Crypto::ECPKeyTarget::ECDSA) != CHIP_NO_ERROR)
            return;
        if (rootDN.AddAttribute_MatterRCACId(0xCAFECAFECAFECAFE) != CHIP_NO_ERROR)
            return;

        // Wide validity window (the canonical one used across the cert tests): 1990-01-01 .. 2013-02-xx epoch.
        X509CertRequestParams rootParams = { 1, 631161876, 729942000, rootDN, rootDN };
        uint8_t der[kMaxDERCertLength];
        MutableByteSpan derSpan(der);
        if (NewRootX509Cert(rootParams, rootKeypair, derSpan) != CHIP_NO_ERROR)
            return;

        MutableByteSpan chipSpan(rcacChipBuf);
        if (ConvertX509CertToChipCert(derSpan, chipSpan) != CHIP_NO_ERROR)
            return;
        rcacChip = chipSpan;
        status   = CHIP_NO_ERROR;
    }
};

const ResignCaFixture & ResignCa()
{
    static ResignCaFixture ca;
    return ca;
}

void ValidateSignedChipCertFuzz(uint64_t nodeId, uint64_t fabricId, int64_t serialNumber, uint32_t validityStart,
                                uint32_t validityEnd, uint32_t effectiveTimeSecs, const std::vector<uint32_t> & cats)
{
    const ResignCaFixture & ca = ResignCa();
    if (ca.status != CHIP_NO_ERROR)
        return;

    // NOC subject DN from the fuzzed identity, issued by our CA's root DN.
    ChipDN nocDN;
    if (nocDN.AddAttribute_MatterNodeId(nodeId) != CHIP_NO_ERROR || nocDN.AddAttribute_MatterFabricId(fabricId) != CHIP_NO_ERROR)
        return;

    // Count-amplify the CAT (CASE Authentication Tag) list — a variable-length subject-DN field that
    // fills the fixed std::array<CASEAuthTag, kMaxSubjectCATAttributeCount> in CATValues. Fuzz both the
    // count (0 .. just past the cap) and the values: over-cap counts exercise the generator's rejection
    // boundary, at-cap counts drive ExtractCATsFromOpCert's fixed-array fill on a valid multi-CAT cert.
    for (size_t i = 0; i < cats.size() && i <= kMaxSubjectCATAttributeCount + 1; ++i)
    {
        if (nocDN.AddAttribute_MatterCASEAuthTag(cats[i]) != CHIP_NO_ERROR)
            break;
    }

    X509CertRequestParams nocParams = { serialNumber, validityStart, validityEnd, nocDN, ca.rootDN };

    // Re-sign with the CA root key: the NOC signature is always valid no matter how the fields were mutated.
    // Field combinations the generator itself rejects (negative serial, malformed DN) return early as uninteresting.
    uint8_t der[kMaxDERCertLength];
    MutableByteSpan derSpan(der);
    if (NewNodeOperationalX509Cert(nocParams, ca.leafKeypair.Pubkey(), ca.rootKeypair, derSpan) != CHIP_NO_ERROR)
        return;

    uint8_t nocChipBuf[kMaxCHIPCertLength];
    MutableByteSpan nocChip(nocChipBuf);
    if (ConvertX509CertToChipCert(derSpan, nocChip) != CHIP_NO_ERROR)
        return;

    // Drive the CAT extraction fixed-array fill on the freshly-signed, CAT-bearing NOC.
    {
        CATValues extractedCats;
        RETURN_SAFELY_IGNORED ExtractCATsFromOpCert(nocChip, extractedCats);
    }

    ChipCertificateSet certSet;
    if (certSet.Init(2) != CHIP_NO_ERROR)
        return;

    BitFlags<CertDecodeFlags> anchor;
    anchor.Set(CertDecodeFlags::kIsTrustAnchor).Set(CertDecodeFlags::kGenerateTBSHash);
    BitFlags<CertDecodeFlags> genHash(CertDecodeFlags::kGenerateTBSHash);

    RETURN_SAFELY_IGNORED certSet.LoadCert(ca.rcacChip, anchor);
    if (certSet.LoadCert(nocChip, genHash) == CHIP_NO_ERROR)
    {
        ValidationContext context;
        context.Reset();
        context.SetEffectiveTime<CurrentChipEpochTime>(chip::System::Clock::Seconds32(effectiveTimeSecs));
        context.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);

        const ChipCertificateData * leaf = certSet.GetLastCert();
        RETURN_SAFELY_IGNORED certSet.ValidateCert(leaf, context);
    }

    certSet.Release();
    // Intentionally no MemoryShutdown: the CA fixture persists across iterations.
}

FUZZ_TEST(FuzzChipCert, ValidateSignedChipCertFuzz)
    .WithDomains(Arbitrary<uint64_t>().WithSeeds({ 0x1122334455667788ULL }), // nodeId
                 Arbitrary<uint64_t>().WithSeeds({ 0xFAB0000000000001ULL }), // fabricId
                 Arbitrary<int64_t>().WithSeeds({ 1 }),                      // serialNumber
                 Arbitrary<uint32_t>().WithSeeds({ 631161876U }),            // validityStart
                 Arbitrary<uint32_t>().WithSeeds({ 729942000U }),            // validityEnd
                 Arbitrary<uint32_t>().WithSeeds({ 680000000U }),            // effectiveTime (within validity)
                 // CATs: fuzz count + values; seed one valid CAT (id 0xABCD, version 1) to reach the CAT path
                 Arbitrary<std::vector<uint32_t>>().WithSeeds({ std::vector<uint32_t>{ 0xABCD0001U } }));

/*-----------------------------------  PW parity for un-ported libFuzzer cert helpers  -----------------------------------*/
/******************************************************************************************************************* */
// FuzzEcdsaSignatureConvert.cpp and FuzzAsn1ChipEpochTime.cpp had no FuzzTest equivalent. Both run on
// fuzzed bytes: the ECDSA signature raw<->DER conversion sits on attestation-response and
// op-cert signatures; the ASN.1<->CHIP-epoch conversion parses X.509 NotBefore/NotAfter from peer op-certs
// and DACs. Ported here so the FuzzTest coverage workflow sees them.

void EcdsaSignatureConvertFuzz(const std::string & bytes)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        const uint8_t * data = reinterpret_cast<const uint8_t *>(bytes.data());
        const size_t len     = bytes.size();

        // Raw -> DER
        if (len >= Crypto::kP256_ECDSA_Signature_Length_Raw)
        {
            uint8_t rawBuf[Crypto::kP256_ECDSA_Signature_Length_Raw];
            memcpy(rawBuf, data, sizeof(rawBuf));
            P256ECDSASignatureSpan rawSig(rawBuf);
            uint8_t derBuf[256];
            MutableByteSpan derOut(derBuf);
            RETURN_SAFELY_IGNORED ConvertECDSASignatureRawToDER(rawSig, derOut);
        }

        // Integer DER -> raw
        {
            ByteSpan derInt(data, len);
            uint8_t rawBuf[64];
            RETURN_SAFELY_IGNORED ConvertIntegerDERToRaw(derInt, rawBuf, sizeof(rawBuf));
        }
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, EcdsaSignatureConvertFuzz).WithDomains(Arbitrary<std::string>());

void Asn1ChipEpochTimeFuzz(const std::string & bytes, uint32_t epochCandidate)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        // fuzzed bytes -> raw ASN1UniversalTime -> epoch
        chip::ASN1::ASN1UniversalTime t{};
        const size_t n = bytes.size() < sizeof(t) ? bytes.size() : sizeof(t);
        if (n > 0)
        {
            memcpy(&t, bytes.data(), n);
        }
        uint32_t epoch = 0;
        RETURN_SAFELY_IGNORED ASN1ToChipEpochTime(t, epoch);

        // fuzzed epoch -> ASN1 time (reverse direction)
        chip::ASN1::ASN1UniversalTime back{};
        RETURN_SAFELY_IGNORED ChipEpochToASN1Time(epochCandidate, back);
    }
    chip::Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzChipCert, Asn1ChipEpochTimeFuzz).WithDomains(Arbitrary<std::string>(), Arbitrary<uint32_t>());

/*-----------------------------------  Seed-corpus guard  -----------------------------------*/
/******************************************************************************************************************* */
// A seedless run of a DER/TLV certificate parser bounces off the ASN.1 front door and yields
// near-zero coverage, so silently fuzzing without seeds gives false confidence. This unit test
// (runs in the FuzzTest binary's unit-test mode, i.e. CI) fails loudly if the in-tree seed
// directory exists but yields an implausibly small corpus (wrong CWD, broken filter, or a
// moved/emptied seed dir). It intentionally passes when the directory is absent: on OSS-Fuzz the
// fuzzers run from a temp CWD and seeds arrive via <target>_seed_corpus.zip, so a missing dir is
// expected there and must not fail the build.
TEST(FuzzChipCertSeeds, SeedCorpusIsPresent)
{
    std::error_code ec;
    if (!std::filesystem::is_directory(OpCertsDir, ec))
    {
        // OSS-Fuzz path (seeds supplied out-of-tree); nothing to assert here.
        return;
    }

    EXPECT_GE(seedProvider(isChipFile).size(), kMinChipSeeds);
    EXPECT_GE(seedProvider(isDerFile).size(), kMinDerSeeds);
}

} // namespace
