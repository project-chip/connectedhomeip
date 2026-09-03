/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      FuzzTest harness for the X.509 helper surface of the crypto PAL
 *      (`CHIPCryptoPAL.h`): key-identifier, distinguished-name, VID/PID and
 *      CRL-distribution-point extraction, issuance-time comparison, resigned
 *      certificate matching, and CSR verification.
 *
 *      These helpers each parse a DER certificate independently of the chain
 *      validators, so they are covered here rather than in
 *      `FuzzChipCertPW.cpp`, which already drives
 *      `Crypto::ValidateCertificateChain` and
 *      `Crypto::VerifyAttestationCertificateFormat`.
 *
 *      Seeds come from the compiled attestation-certificate test vectors, not
 *      from a directory read, so they travel with the binary. They are attached
 *      to the individual domains, because only domain-level seeds are injected
 *      when the suite runs under the libFuzzer compatibility engine. Each pool
 *      leads with the coherent FFF1 chain (PAA_FFF1 -> PAI_FFF1_8000 ->
 *      DAC_FFF1_8000_0000): FuzzTest builds its pristine seed tuple from the
 *      first seed of each domain, so a mismatched lead would fail the parse and
 *      leave the helpers unreached.
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <credentials/attestation_verifier/TestPAAStore.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::Crypto;
using namespace chip::TestCerts;
using namespace fuzztest;

using Bytes = std::vector<uint8_t>;

Bytes ToBytes(const ByteSpan & span)
{
    return Bytes(span.begin(), span.end());
}

std::vector<Bytes> ToSeeds(const std::vector<ByteSpan> & spans)
{
    std::vector<Bytes> seeds;
    seeds.reserve(spans.size());
    for (const ByteSpan & span : spans)
    {
        seeds.push_back(ToBytes(span));
    }
    return seeds;
}

// Device attestation certificates, leading with the FFF1 chain leaf and
// followed by variants covering other vendor/product ids and every validity
// edge case the test corpus provides.
auto AnyDacCert()
{
    return Arbitrary<Bytes>().WithSeeds(ToSeeds({
        sTestCert_DAC_FFF1_8000_0000_Cert,
        sTestCert_DAC_FFF1_8000_0004_Cert,
        sTestCert_DAC_FFF2_8001_0008_Cert,
        sTestCert_DAC_FFF2_8002_0017_Cert,
        sTestCert_DAC_FFF2_8003_0018_FB_Cert,
        sTestCert_DAC_FFF2_8004_001C_FB_Cert,
        sTestCert_DAC_FFF2_8004_0020_ValInPast_Cert,
        sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert,
        sTestCert_DAC_FFF2_8004_0030_Val1SecBefore_Cert,
        sTestCert_DAC_FFF2_8006_0034_ValInFuture_Cert,
    }));
}

// Product attestation intermediates, leading with the FFF1 chain issuer.
auto AnyPaiCert()
{
    return Arbitrary<Bytes>().WithSeeds(ToSeeds({
        sTestCert_PAI_FFF1_8000_Cert,
        sTestCert_PAI_FFF2_8001_Cert,
        sTestCert_PAI_FFF2_8001_Resigned_Cert,
        sTestCert_PAI_FFF2_8001_ResignedSKIDDiff_Cert,
        sTestCert_PAI_FFF2_8001_ResignedSubjectDiff_Cert,
        sTestCert_PAI_FFF2_8004_FB_Cert,
        sTestCert_PAI_FFF2_8005_ValInPast_Cert,
        sTestCert_PAI_FFF2_8006_ValInFuture_Cert,
        sTestCert_PAI_FFF2_NoPID_Cert,
    }));
}

// Certificates that actually carry a CRL distribution point, in every shape the
// corpus provides: single/multiple DPs, multiple URIs, an HTTPS scheme, an
// over-long URI, a wrong URI prefix, and one/two CRL issuers.
auto AnyCdpCert()
{
    return Arbitrary<Bytes>().WithSeeds(ToSeeds({
        sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_FFF1_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_2DPs_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_2URIs_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_HTTPS_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Long_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Wrong_Prefix_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_2CRLIssuers_PAA_FFF1_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_CRL_Issuer_PAA_FFF1_2DPs_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_NoVID_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Cert,
        sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Long_Cert,
        sTestCert_DAC_FFF1_8000_0000_2CDPs_Cert,
        sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAA_FFF1_Cert,
        sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAI_FFF2_8004_Cert,
    }));
}

// IsCertificateValidAtCurrentTime reads the wall clock, so its result is not
// reproducible across runs; only the absence of a crash is checked here.
void IssuanceTimeComparison(const Bytes & candidate, const Bytes & issuer)
{
    const ByteSpan candidateSpan(candidate.data(), candidate.size());

    RETURN_SAFELY_IGNORED IsCertificateValidAtIssuance(candidateSpan, ByteSpan(issuer.data(), issuer.size()));
    RETURN_SAFELY_IGNORED IsCertificateValidAtCurrentTime(candidateSpan);
}

FUZZ_TEST(ChipCryptoPal, IssuanceTimeComparison).WithDomains(AnyDacCert(), AnyPaiCert());

void SerialNumberExtraction(const Bytes & cert)
{
    uint8_t serialNumberBuf[kMaxCertificateSerialNumberLength] = { 0 };
    MutableByteSpan serialNumber(serialNumberBuf);
    RETURN_SAFELY_IGNORED ExtractSerialNumberFromX509Cert(ByteSpan(cert.data(), cert.size()), serialNumber);
}

FUZZ_TEST(ChipCryptoPal, SerialNumberExtraction).WithDomains(AnyDacCert());

void KeyIdentifierExtraction(const Bytes & cert)
{
    const ByteSpan certSpan(cert.data(), cert.size());

    uint8_t skidBuf[kSubjectKeyIdentifierLength];
    MutableByteSpan skid(skidBuf);
    RETURN_SAFELY_IGNORED ExtractSKIDFromX509Cert(certSpan, skid);

    uint8_t akidBuf[kAuthorityKeyIdentifierLength];
    MutableByteSpan akid(akidBuf);
    RETURN_SAFELY_IGNORED ExtractAKIDFromX509Cert(certSpan, akid);
}

FUZZ_TEST(ChipCryptoPal, KeyIdentifierExtraction).WithDomains(AnyDacCert());

void CrlDistributionPointUriExtraction(const Bytes & cert)
{
    char cdpBuf[kMaxCRLDistributionPointURLLength] = { '\0' };
    MutableCharSpan cdpUrl(cdpBuf);
    RETURN_SAFELY_IGNORED ExtractCRLDistributionPointURIFromX509Cert(ByteSpan(cert.data(), cert.size()), cdpUrl);
}

FUZZ_TEST(ChipCryptoPal, CrlDistributionPointUriExtraction).WithDomains(AnyCdpCert());

void CrlIssuerExtraction(const Bytes & cert)
{
    uint8_t crlIssuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan crlIssuer(crlIssuerBuf);
    RETURN_SAFELY_IGNORED ExtractCDPExtensionCRLIssuerFromX509Cert(ByteSpan(cert.data(), cert.size()), crlIssuer);
}

FUZZ_TEST(ChipCryptoPal, CrlIssuerExtraction).WithDomains(AnyCdpCert());

void VidPidExtractionFromCert(const Bytes & cert)
{
    AttestationCertVidPid vidPid;
    RETURN_SAFELY_IGNORED ExtractVIDPIDFromX509Cert(ByteSpan(cert.data(), cert.size()), vidPid);
}

FUZZ_TEST(ChipCryptoPal, VidPidExtractionFromCert).WithDomains(AnyDacCert());

auto AnyDNAttrType()
{
    return ElementOf({ DNAttrType::kUnspecified, DNAttrType::kCommonName, DNAttrType::kMatterVID, DNAttrType::kMatterPID });
}

// The attribute string is parsed as text (including the "Mvid:"/"Mpid:" prefixes
// embedded in a common name), so it is fuzzed as a free-form string rather than
// seeded from certificates.
void VidPidExtractionFromAttributeString(DNAttrType attrType, const std::string & attrString)
{
    AttestationCertVidPid vidPid;
    AttestationCertVidPid vidPidFromCN;

    const ByteSpan attrSpan(reinterpret_cast<const uint8_t *>(attrString.data()), attrString.size());
    RETURN_SAFELY_IGNORED ExtractVIDPIDFromAttributeString(attrType, attrSpan, vidPid, vidPidFromCN);
}

FUZZ_TEST(ChipCryptoPal, VidPidExtractionFromAttributeString)
    .WithDomains(AnyDNAttrType(),
                 Arbitrary<std::string>().WithSeeds({ "Mvid:FFF1", "Mpid:8000", "Mvid:FFF1 Mpid:8000", "Matter Test DAC 0000",
                                                      "Mvid:", "Mvid:FFFF Mpid:FFFF" }));

void SubjectAndIssuerExtraction(const Bytes & cert)
{
    const ByteSpan certSpan(cert.data(), cert.size());

    uint8_t subjectBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan subject(subjectBuf);
    RETURN_SAFELY_IGNORED ExtractSubjectFromX509Cert(certSpan, subject);

    uint8_t issuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
    MutableByteSpan issuer(issuerBuf);
    RETURN_SAFELY_IGNORED ExtractIssuerFromX509Cert(certSpan, issuer);
}

FUZZ_TEST(ChipCryptoPal, SubjectAndIssuerExtraction).WithDomains(AnyDacCert());

// The resigned-certificate lookup walks a caller-supplied candidate list, so the
// list length is fuzzed alongside the reference certificate. The candidates are
// the resigned PAI variants, which are what the helper exists to match.
const ByteSpan kResignedCandidates[] = {
    sTestCert_PAI_FFF2_8001_Resigned_Cert,
    sTestCert_PAI_FFF2_8001_ResignedSKIDDiff_Cert,
    sTestCert_PAI_FFF2_8001_ResignedSubjectDiff_Cert,
    sTestCert_PAI_FFF2_NoPID_Resigned_Cert,
};

void ResignedCertificateLookup(const Bytes & cert, size_t candidateCount)
{
    ByteSpan replacement;
    RETURN_SAFELY_IGNORED ReplaceCertIfResignedCertFound(ByteSpan(cert.data(), cert.size()), kResignedCandidates, candidateCount,
                                                         replacement);
}

FUZZ_TEST(ChipCryptoPal, ResignedCertificateLookup)
    .WithDomains(AnyPaiCert(), InRange<size_t>(0, MATTER_ARRAY_SIZE(kResignedCandidates)));

// A CSR whose signature verifies against this public key, used as the lead seed
// so that mutations start from a structurally valid request.
const uint8_t kGoodCsrSubjectPublicKey[] = {
    0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01, 0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8, 0xe2, 0x98, 0xac, 0x2f,
    0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04, 0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00,
    0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f, 0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15,
    0x7d, 0x93, 0xe6, 0x80, 0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d, 0x4a,
};

const uint8_t kGoodCsr[] = {
    0x30, 0x81, 0xca, 0x30, 0x70, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03,
    0x43, 0x53, 0x52, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01, 0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8,
    0xe2, 0x98, 0xac, 0x2f, 0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04, 0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00,
    0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f, 0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15, 0x7d, 0x93, 0xe6, 0x80,
    0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d, 0x4a, 0xa0, 0x00, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x04, 0x03, 0x02, 0x05, 0x00, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x1d, 0x86, 0x21, 0xb4, 0xc2, 0xe1, 0xa9, 0xf3, 0xbc,
    0xc8, 0x7c, 0xda, 0xb4, 0xb9, 0xc6, 0x8c, 0xd0, 0xe4, 0x9a, 0x9c, 0xef, 0x02, 0x93, 0x98, 0x27, 0x7e, 0x81, 0x21, 0x5d, 0x20,
    0x9d, 0x32, 0x02, 0x21, 0x00, 0x8b, 0x6b, 0x49, 0xb6, 0x7d, 0x3e, 0x67, 0x9e, 0xb1, 0x22, 0xd3, 0x63, 0x82, 0x40, 0x4f, 0x49,
    0xa4, 0xdc, 0x17, 0x35, 0xac, 0x4b, 0x7a, 0xbf, 0x52, 0x05, 0x58, 0x68, 0xe0, 0xaa, 0xd2, 0x8e,
};

void CertificateSigningRequestVerification(const Bytes & csr)
{
    P256PublicKey expectedPublicKey(kGoodCsrSubjectPublicKey);
    RETURN_SAFELY_IGNORED VerifyCertificateSigningRequest(csr.data(), csr.size(), expectedPublicKey);
}

FUZZ_TEST(ChipCryptoPal, CertificateSigningRequestVerification)
    .WithDomains(Arbitrary<Bytes>().WithSeeds({ Bytes(kGoodCsr, kGoodCsr + sizeof(kGoodCsr)) }));

} // namespace
