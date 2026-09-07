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
 *      Seeded FuzzTest harness for DeconstructAttestationElements and
 *      DecodeCertificationElements. Seeds are real attestation/CD blobs
 *      built at startup via Construct/Encode helpers — random fuzz bytes
 *      almost never satisfy the deeply nested TLV schema, so a seeded
 *      mutator has dramatically better coverage on the typed-field paths.
 */

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::Credentials;
using namespace fuzztest;

std::string BuildAttestationElementsSeed()
{
    Platform::MemoryInit();

    // Trivial CD payload (the deconstructor only re-emits this span).
    static const uint8_t certDecl[] = { 0x30, 0x82, 0x01, 0x00, 0x06, 0x09 };
    static uint8_t nonce[32]        = { 0x00 };
    for (uint8_t i = 0; i < 32; ++i)
    {
        nonce[i] = static_cast<uint8_t>(i + 1);
    }

    static const uint8_t firmwareInfo[] = { 'f', 'w', '-', 'i', 'n', 'f', 'o' };

    DeviceAttestationVendorReservedConstructor vendorReservedCtor(nullptr, 0);

    uint8_t outBuf[1024];
    MutableByteSpan out(outBuf);
    if (ConstructAttestationElements(ByteSpan(certDecl), ByteSpan(nonce), 0xDEADBEEF, ByteSpan(firmwareInfo), vendorReservedCtor,
                                     out) != CHIP_NO_ERROR)
    {
        Platform::MemoryShutdown();
        return {};
    }

    std::string s(reinterpret_cast<const char *>(out.data()), out.size());
    Platform::MemoryShutdown();
    return s;
}

std::string BuildCertificationDeclarationSeed(uint8_t pidCount = 1, bool dacOrigin = false, uint8_t paaCount = 0,
                                              uint8_t certType = 0, uint16_t versionNumber = 0x2694)
{
    Platform::MemoryInit();

    CertificationElements elements;
    elements.FormatVersion   = 1;
    elements.VendorId        = 0xFFF1;
    elements.ProductIdsCount = pidCount;
    for (uint8_t i = 0; i < pidCount && i < kMaxProductIdsCount; ++i)
    {
        elements.ProductIds[i] = static_cast<uint16_t>(0x8001 + i);
    }
    elements.DeviceTypeId      = 0x010C;
    static const char certId[] = "CSA00000SWC00000-00";
    memcpy(elements.CertificateId, certId, sizeof(certId));
    elements.SecurityLevel             = 0;
    elements.SecurityInformation       = 0;
    elements.VersionNumber             = versionNumber;
    elements.CertificationType         = certType;
    elements.DACOriginVIDandPIDPresent = dacOrigin;
    if (dacOrigin)
    {
        elements.DACOriginVendorId  = 0xFFF2;
        elements.DACOriginProductId = 0x8002;
    }
    elements.AuthorizedPAAListCount = paaCount;
    for (uint8_t i = 0; i < paaCount && i < kMaxAuthorizedPAAListCount; ++i)
    {
        for (uint8_t j = 0; j < kKeyIdentifierLength; ++j)
        {
            elements.AuthorizedPAAList[i][j] = static_cast<uint8_t>(0x10 + i + j);
        }
    }

    uint8_t outBuf[1024];
    MutableByteSpan out(outBuf);
    if (EncodeCertificationElements(elements, out) != CHIP_NO_ERROR)
    {
        Platform::MemoryShutdown();
        return {};
    }

    std::string s(reinterpret_cast<const char *>(out.data()), out.size());
    Platform::MemoryShutdown();
    return s;
}

std::string BuildNOCSRElementsSeed()
{
    Platform::MemoryInit();

    // A plausible CSR-shaped payload and a 32-byte nonce; the deconstructor only re-emits
    // these spans, so the exact contents do not matter for reaching the TLV-walk code.
    static const uint8_t csr[]  = { 0x30, 0x81, 0x00, 0x02, 0x01, 0x00 };
    static uint8_t csrNonce[32] = { 0x00 };
    for (uint8_t i = 0; i < 32; ++i)
    {
        csrNonce[i] = static_cast<uint8_t>(i + 1);
    }

    uint8_t outBuf[1024];
    MutableByteSpan out(outBuf);
    // Empty vendor-reserved spans: ConstructNOCSRElements emits only csr + nonce, which is the
    // common commissioning shape (the three vendor-reserved slots are optional).
    if (ConstructNOCSRElements(ByteSpan(csr), ByteSpan(csrNonce), ByteSpan(), ByteSpan(), ByteSpan(), out) != CHIP_NO_ERROR)
    {
        Platform::MemoryShutdown();
        return {};
    }

    std::string s(reinterpret_cast<const char *>(out.data()), out.size());
    Platform::MemoryShutdown();
    return s;
}

void DeconstructAttestationFuzz(const std::string & bytes)
{
    Platform::MemoryInit();

    ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
    ByteSpan certDecl;
    ByteSpan attestationNonce;
    uint32_t timestamp = 0;
    ByteSpan firmwareInfo;
    DeviceAttestationVendorReservedDeconstructor vendorReserved;

    RETURN_SAFELY_IGNORED DeconstructAttestationElements(span, certDecl, attestationNonce, timestamp, firmwareInfo, vendorReserved);

    size_t count = 0;
    RETURN_SAFELY_IGNORED CountVendorReservedElementsInDA(span, count);

    Platform::MemoryShutdown();
}

void DecodeCertificationDeclarationFuzz(const std::string & bytes)
{
    Platform::MemoryInit();

    ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
    {
        CertificationElements out{};
        RETURN_SAFELY_IGNORED DecodeCertificationElements(span, out);
    }
    {
        CertificationElementsWithoutPIDs out{};
        RETURN_SAFELY_IGNORED DecodeCertificationElements(span, out);
    }
    {
        CertificationElementsDecoder dec;
        (void) dec.IsProductIdIn(span, 0x8001);
        // HasAuthorizedPAA had no PW coverage; probe it on the same fuzzed CD blob.
        ByteSpan paa(reinterpret_cast<const uint8_t *>("AUTHORIZED-PAA-PROBE"), 20);
        (void) dec.HasAuthorizedPAA(span, paa);
    }

    Platform::MemoryShutdown();
}

void DeconstructNOCSRFuzz(const std::string & bytes)
{
    Platform::MemoryInit();

    ByteSpan span(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
    ByteSpan csr;
    ByteSpan csrNonce;
    ByteSpan vendorReserved1;
    ByteSpan vendorReserved2;
    ByteSpan vendorReserved3;

    RETURN_SAFELY_IGNORED DeconstructNOCSRElements(span, csr, csrNonce, vendorReserved1, vendorReserved2, vendorReserved3);

    Platform::MemoryShutdown();
}

auto SeededAttestation()
{
    auto s = BuildAttestationElementsSeed();
    std::vector<std::string> seeds;
    if (!s.empty())
        seeds.push_back(std::move(s));
    return Arbitrary<std::string>().WithSeeds(seeds);
}

auto SeededCD()
{
    std::vector<std::string> seeds;
    auto add = [&](std::string s) {
        if (!s.empty())
            seeds.push_back(std::move(s));
    };
    add(BuildCertificationDeclarationSeed(1, false, 0, 0, 0x2694)); // baseline
    add(BuildCertificationDeclarationSeed(4, false, 0, 0, 0x2694)); // multiple PIDs
    add(BuildCertificationDeclarationSeed(1, true, 0, 0, 0x2694));  // with DACOrigin
    add(BuildCertificationDeclarationSeed(1, false, 2, 1, 0x2694)); // with PAA list, type 1
    add(BuildCertificationDeclarationSeed(1, true, 3, 2, 0x2694));  // DACOrigin + PAA list + type 2
    add(BuildCertificationDeclarationSeed(8, true, 5, 3, 0xFFFF));  // many PIDs + many PAAs + max type
    return Arbitrary<std::string>().WithSeeds(seeds);
}

auto SeededNOCSR()
{
    auto s = BuildNOCSRElementsSeed();
    std::vector<std::string> seeds;
    if (!s.empty())
        seeds.push_back(std::move(s));
    return Arbitrary<std::string>().WithSeeds(seeds);
}

FUZZ_TEST(AttestationElements, DeconstructAttestationFuzz).WithDomains(SeededAttestation());
FUZZ_TEST(AttestationElements, DecodeCertificationDeclarationFuzz).WithDomains(SeededCD());
FUZZ_TEST(AttestationElements, DeconstructNOCSRFuzz).WithDomains(SeededNOCSR());

} // namespace
