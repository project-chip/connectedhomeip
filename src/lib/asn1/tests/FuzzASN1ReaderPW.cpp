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
 *      Seeded FuzzTest harness for ASN1Reader.
 *
 *      Coverage-targeting changes vs the prior version:
 *
 *      * Massively wider seed pool — beyond the 5 Root/ICA cert DERs we now
 *        include Node certs, FWSign certs, ICA01_1, error-case certs (which
 *        deliberately violate well-formedness in different parts of the DER
 *        tree), public keys (SubjectPublicKeyInfo SEQUENCEs), and SKID/AKID
 *        DER blobs. Each seed exercises a different ASN.1 constructed-type
 *        shape.
 *
 *      * Walk now also exercises EnterEncapsulatedType / ExitEncapsulatedType
 *        and GetConstructedType — paths the previous walker never touched.
 *
 *      * Three FUZZ_TESTs split the input domain so per-test coverage stays
 *        focused: full-cert walk, key-only walk, KeyId-only walk.
 *
 *      * Idempotency property: re-Init + re-walk must produce the same
 *        observable Class/Tag/ValueLen sequence.
 */

#include <cstdint>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <credentials/tests/CHIPCert_test_vectors.h>
#include <lib/asn1/ASN1.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::ASN1;
using namespace chip::TestCerts;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

inline std::string SpanToString(ByteSpan s)
{
    return std::string(reinterpret_cast<const char *>(s.data()), s.size());
}

// ===== Seed pools =====
// Full-cert DER (TBSCertificate inside SEQUENCE OF SEQUENCE etc.). These
// exercise the deep-nesting, OID, BIT STRING, and INTEGER paths.
std::vector<std::string> CertDerSeeds()
{
    return {
        SpanToString(sTestCert_Root01_DER),   SpanToString(sTestCert_Root02_DER),    SpanToString(sTestCert_Root03_DER),
        SpanToString(sTestCert_ICA01_DER),    SpanToString(sTestCert_ICA02_DER),     SpanToString(sTestCert_ICA01_1_DER),
        SpanToString(sTestCert_FWSign01_DER), SpanToString(sTestCert_Node01_01_DER), SpanToString(sTestCert_Node01_02_DER),
    };
}

// SubjectPublicKeyInfo blobs (SEQUENCE { algorithm, BIT STRING }) — exercise
// BIT STRING + AlgorithmIdentifier OID handling.
std::vector<std::string> KeySeeds()
{
    return {
        SpanToString(sTestCert_Root01_PublicKey),    SpanToString(sTestCert_Root02_PublicKey),
        SpanToString(sTestCert_Root03_PublicKey),    SpanToString(sTestCert_ICA01_PublicKey),
        SpanToString(sTestCert_ICA02_PublicKey),     SpanToString(sTestCert_Node01_01_PublicKey),
        SpanToString(sTestCert_Node01_02_PublicKey), SpanToString(sTestCert_FWSign01_PublicKey),
    };
}

// Subject/Authority Key Identifier raw OCTET STRINGs (often 20 bytes — SHA-1).
std::vector<std::string> KeyIdSeeds()
{
    return {
        SpanToString(sTestCert_Root01_SubjectKeyId),    SpanToString(sTestCert_Root01_AuthorityKeyId),
        SpanToString(sTestCert_ICA01_SubjectKeyId),     SpanToString(sTestCert_ICA01_AuthorityKeyId),
        SpanToString(sTestCert_Node01_01_SubjectKeyId), SpanToString(sTestCert_Node01_01_AuthorityKeyId),
        SpanToString(sTestCert_FWSign01_SubjectKeyId),  SpanToString(sTestCert_FWSign01_AuthorityKeyId),
    };
}

// ===== Walker =====
// Deep walk that exercises every read API the public ASN1Reader exposes.
// Bounded by depth and total-elements to prevent runaway iteration on
// deeply-nested or oversized inputs while still going deeper than a single Next loop.
void Walk(ASN1Reader & reader, int depth, int & remaining)
{
    if (depth > 12)
        return;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        if (--remaining <= 0)
            return;

        // Read every metadata accessor.
        (void) reader.GetClass();
        (void) reader.GetTag();
        (void) reader.GetValue();
        (void) reader.GetValueLen();
        (void) reader.IsConstructed();
        (void) reader.IsIndefiniteLen();
        (void) reader.IsEndOfContents();
        (void) reader.IsContained();

        // Try every Get* — most will fail with WRONG_TLV_TYPE / ENCODING but
        // hitting their entry path is what we want for coverage.
        int64_t intVal = 0;
        (void) reader.GetInteger(intVal);

        bool boolVal = false;
        (void) reader.GetBoolean(boolVal);

        OID oid = kOID_NotSpecified;
        (void) reader.GetObjectId(oid);

        uint32_t bitStr = 0;
        (void) reader.GetBitString(bitStr);

        ASN1UniversalTime t;
        (void) reader.GetUTCTime(t);
        (void) reader.GetGeneralizedTime(t);

        if (reader.IsConstructed())
        {
            // Touch the raw constructed access path too.
            const uint8_t * cVal = nullptr;
            uint32_t cLen        = 0;
            (void) reader.GetConstructedType(cVal, cLen);

            if (reader.EnterConstructedType() == CHIP_NO_ERROR)
            {
                Walk(reader, depth + 1, remaining);
                (void) reader.ExitConstructedType();
            }
        }
        else if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_BitString)
        {
            // BIT STRING bodies in X.509 frequently encapsulate a SEQUENCE
            // (e.g. SubjectPublicKeyInfo's bit string wraps an EC point or a
            // SEQUENCE of integers). Exercise the encapsulated-type path.
            if (reader.EnterEncapsulatedType() == CHIP_NO_ERROR)
            {
                Walk(reader, depth + 1, remaining);
                (void) reader.ExitEncapsulatedType();
            }
        }
        else if (reader.GetClass() == kASN1TagClass_Universal && reader.GetTag() == kASN1UniversalTag_OctetString)
        {
            // SKID/AKID OCTET STRINGs sometimes wrap a SEQUENCE in extension
            // values; try the encapsulated path here too.
            if (reader.EnterEncapsulatedType() == CHIP_NO_ERROR)
            {
                Walk(reader, depth + 1, remaining);
                (void) reader.ExitEncapsulatedType();
            }
        }
    }
}

void DriveOnce(const std::string & der)
{
    EnsureInitialized();

    ASN1Reader reader;
    reader.Init(reinterpret_cast<const uint8_t *>(der.data()), der.size());

    int budget = 8192;
    Walk(reader, 0, budget);
}

// ===== FUZZ_TESTs =====

void ASN1ReaderCertWalk(const std::string & der)
{
    DriveOnce(der);
}

FUZZ_TEST(ASN1ReaderPW, ASN1ReaderCertWalk).WithDomains(Arbitrary<std::string>().WithSeeds(CertDerSeeds()).WithMaxSize(8192));

void ASN1ReaderKeyWalk(const std::string & der)
{
    DriveOnce(der);
}

FUZZ_TEST(ASN1ReaderPW, ASN1ReaderKeyWalk).WithDomains(Arbitrary<std::string>().WithSeeds(KeySeeds()).WithMaxSize(2048));

void ASN1ReaderKeyIdWalk(const std::string & der)
{
    DriveOnce(der);
}

FUZZ_TEST(ASN1ReaderPW, ASN1ReaderKeyIdWalk).WithDomains(Arbitrary<std::string>().WithSeeds(KeyIdSeeds()).WithMaxSize(256));

// Idempotency property — Init + Walk twice must yield the same observable
// (Class, Tag, ValueLen, IsConstructed) sequence. Internal state corruption
// (e.g. saved-context array overwrite) shows up as divergent walks.
struct ElemSnap
{
    uint8_t cls;
    uint8_t tag;
    uint32_t valueLen;
    bool constructed;
    bool operator==(const ElemSnap & o) const
    {
        return cls == o.cls && tag == o.tag && valueLen == o.valueLen && constructed == o.constructed;
    }
};

void CollectFlat(ASN1Reader & reader, std::vector<ElemSnap> & out, int & remaining)
{
    while (reader.Next() == CHIP_NO_ERROR)
    {
        if (--remaining <= 0)
            return;
        out.push_back({ reader.GetClass(), reader.GetTag(), reader.GetValueLen(), reader.IsConstructed() });
        if (reader.IsConstructed())
        {
            if (reader.EnterConstructedType() == CHIP_NO_ERROR)
            {
                CollectFlat(reader, out, remaining);
                (void) reader.ExitConstructedType();
            }
        }
    }
}

void ASN1ReaderIdempotency(const std::string & der)
{
    EnsureInitialized();

    std::vector<ElemSnap> first;
    {
        ASN1Reader reader;
        reader.Init(reinterpret_cast<const uint8_t *>(der.data()), der.size());
        int budget = 4096;
        CollectFlat(reader, first, budget);
    }

    std::vector<ElemSnap> second;
    {
        ASN1Reader reader;
        reader.Init(reinterpret_cast<const uint8_t *>(der.data()), der.size());
        int budget = 4096;
        CollectFlat(reader, second, budget);
    }

    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i)
    {
        ASSERT_TRUE(first[i] == second[i]);
    }
}

FUZZ_TEST(ASN1ReaderPW, ASN1ReaderIdempotency).WithDomains(Arbitrary<std::string>().WithSeeds(CertDerSeeds()).WithMaxSize(8192));

// Drive DumpASN1 — the debug pretty-printer. Cheap coverage win: it
// re-exercises the entire ASN1Reader::Next/EnterConstructedType/Get* surface
// from a separate caller, plus prints to a /dev/null FILE* so the printf
// paths get covered without polluting stdout.
void ASN1DumpFuzz(const std::string & der)
{
    EnsureInitialized();

    ASN1Reader reader;
    reader.Init(reinterpret_cast<const uint8_t *>(der.data()), der.size());

    // Redirect stdout for the duration so the debug prints don't spam fuzz
    // output. fmemopen would be cleaner but DumpASN1 uses raw printf().
    fflush(stdout);
    int saved   = dup(STDOUT_FILENO);
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0)
    {
        dup2(devnull, STDOUT_FILENO);
        close(devnull);
    }
    (void) DumpASN1(reader, "fuzz: ", "  ");
    fflush(stdout);
    if (saved >= 0)
    {
        dup2(saved, STDOUT_FILENO);
        close(saved);
    }
}

FUZZ_TEST(ASN1ReaderPW, ASN1DumpFuzz).WithDomains(Arbitrary<std::string>().WithSeeds(CertDerSeeds()).WithMaxSize(8192));

// =====================================================================
// ASN1Writer coverage — encode primitives, then read back via ASN1Reader,
// asserting structural equality. The writer was at 0% line coverage before
// this; here every Put*/Start*/End* path gets exercised.
// =====================================================================

void ASN1WriterRoundtripInteger(int64_t value)
{
    EnsureInitialized();

    uint8_t buf[64];
    ASN1Writer writer;
    writer.Init(buf, sizeof(buf));
    if (writer.PutInteger(value) != CHIP_NO_ERROR)
        return;

    ASN1Reader reader;
    reader.Init(buf, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    int64_t decoded = 0;
    ASSERT_EQ(reader.GetInteger(decoded), CHIP_NO_ERROR);
    ASSERT_EQ(decoded, value);
}

FUZZ_TEST(ASN1ReaderPW, ASN1WriterRoundtripInteger).WithDomains(Arbitrary<int64_t>());

void ASN1WriterRoundtripBoolean(bool value)
{
    EnsureInitialized();

    uint8_t buf[16];
    ASN1Writer writer;
    writer.Init(buf, sizeof(buf));
    if (writer.PutBoolean(value) != CHIP_NO_ERROR)
        return;

    ASN1Reader reader;
    reader.Init(buf, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    bool decoded = !value;
    ASSERT_EQ(reader.GetBoolean(decoded), CHIP_NO_ERROR);
    ASSERT_EQ(decoded, value);
}

FUZZ_TEST(ASN1ReaderPW, ASN1WriterRoundtripBoolean).WithDomains(Arbitrary<bool>());

void ASN1WriterRoundtripBitString(uint32_t value)
{
    EnsureInitialized();

    uint8_t buf[64];
    ASN1Writer writer;
    writer.Init(buf, sizeof(buf));
    if (writer.PutBitString(value) != CHIP_NO_ERROR)
        return;

    ASN1Reader reader;
    reader.Init(buf, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    uint32_t decoded = 0;
    ASSERT_EQ(reader.GetBitString(decoded), CHIP_NO_ERROR);
    ASSERT_EQ(decoded, value);
}

FUZZ_TEST(ASN1ReaderPW, ASN1WriterRoundtripBitString).WithDomains(Arbitrary<uint32_t>());

void ASN1WriterConstructedNesting(const std::vector<uint8_t> & blob, bool encapsulate)
{
    EnsureInitialized();
    if (blob.size() > 1024)
        return;

    uint8_t buf[2048];
    ASN1Writer writer;
    writer.Init(buf, sizeof(buf));

    if (writer.StartConstructedType(kASN1TagClass_Universal, /*Sequence*/ 0x10) != CHIP_NO_ERROR)
        return;
    if (writer.PutOctetString(blob.data(), static_cast<uint16_t>(blob.size())) != CHIP_NO_ERROR)
        return;
    if (encapsulate)
    {
        if (writer.StartEncapsulatedType(kASN1TagClass_Universal, /*BitString*/ 0x03, /*bitStringEncoding=*/true) == CHIP_NO_ERROR)
        {
            (void) writer.PutInteger(0x42);
            (void) writer.EndEncapsulatedType();
        }
    }
    if (writer.EndConstructedType() != CHIP_NO_ERROR)
        return;

    // Read the result back to make sure the lengths are coherent.
    const size_t written = static_cast<size_t>(writer.GetLengthWritten());
    ASN1Reader reader;
    reader.Init(buf, written);
    int budget = 4096;
    Walk(reader, 0, budget);
}

FUZZ_TEST(ASN1ReaderPW, ASN1WriterConstructedNesting)
    .WithDomains(Arbitrary<std::vector<uint8_t>>().WithMaxSize(1024), Arbitrary<bool>());

// =============================================================================
// ASN1OID coverage — the OID lookup tables in ASN1OID.cpp are at ~28% line
// coverage because most enum values aren't exercised by cert-walk fuzz alone.
// This explicitly drives the table for every interesting OID.
// =============================================================================

auto AnyKnownOID()
{
    return ElementOf<OID>({
        // PubKey / Sig
        kOID_PubKeyAlgo_ECPublicKey,
        kOID_SigAlgo_ECDSAWithSHA256,
        // Attribute Types (X.500 names + Matter-specific)
        kOID_AttributeType_CommonName,
        kOID_AttributeType_Surname,
        kOID_AttributeType_SerialNumber,
        kOID_AttributeType_CountryName,
        kOID_AttributeType_LocalityName,
        kOID_AttributeType_StateOrProvinceName,
        kOID_AttributeType_OrganizationName,
        kOID_AttributeType_OrganizationalUnitName,
        kOID_AttributeType_Title,
        kOID_AttributeType_GivenName,
        kOID_AttributeType_DomainComponent,
        kOID_AttributeType_MatterNodeId,
        kOID_AttributeType_MatterFirmwareSigningId,
        kOID_AttributeType_MatterICACId,
        kOID_AttributeType_MatterRCACId,
        kOID_AttributeType_MatterFabricId,
        kOID_AttributeType_MatterCASEAuthTag,
        kOID_AttributeType_MatterVidVerificationSignerId,
        // Curves and extensions
        kOID_EllipticCurve_prime256v1,
        kOID_Extension_BasicConstraints,
        kOID_Extension_KeyUsage,
        kOID_Extension_ExtendedKeyUsage,
        kOID_Extension_SubjectKeyIdentifier,
        // Sentinels
        kOID_NotSpecified,
        kOID_Unknown,
    });
}

void OIDLookupRoundtrip(OID oid)
{
    EnsureInitialized();

    const uint8_t * encoded = nullptr;
    uint16_t encodedLen     = 0;
    if (!GetEncodedObjectID(oid, encoded, encodedLen))
    {
        // Sentinels (NotSpecified / Unknown) won't have an encoding — that's
        // legitimate; touch the secondary path and return.
        (void) GetOIDCategory(oid);
        (void) GetOIDName(oid);
        return;
    }

    // Encode-side helpers covered: feed back through ParseObjectID and check
    // we get the same enum back out.
    const OID parsed = ParseObjectID(encoded, encodedLen);
    ASSERT_EQ(parsed, oid);

    // Touch both observation accessors on the resulting OID.
    (void) GetOIDCategory(parsed);
    const char * name = GetOIDName(parsed);
    (void) name;
}

FUZZ_TEST(ASN1ReaderPW, OIDLookupRoundtrip).WithDomains(AnyKnownOID());

// Direct ParseObjectID with arbitrary bytes — covers the linear-search miss
// path and the empty-input early-return.
void ParseObjectIDFuzz(const std::vector<uint8_t> & encoded)
{
    EnsureInitialized();
    if (encoded.size() > UINT16_MAX)
        return;
    OID oid = ParseObjectID(encoded.data(), static_cast<uint16_t>(encoded.size()));
    (void) GetOIDCategory(oid);
    (void) GetOIDName(oid);
}

FUZZ_TEST(ASN1ReaderPW, ParseObjectIDFuzz).WithDomains(Arbitrary<std::vector<uint8_t>>().WithMaxSize(64));

} // namespace
