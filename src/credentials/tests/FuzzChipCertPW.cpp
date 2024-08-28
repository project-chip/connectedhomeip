#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "credentials/CHIPCert.h"

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
        ChipCertificateData certData;
        (void) DecodeChipCert(span, certData);
    }

    {
        uint8_t outCertBuf[kMaxDERCertLength];
        MutableByteSpan outCert(outCertBuf);
        (void) ConvertChipCertToX509Cert(span, outCert);
    }
}

FUZZ_TEST(ChipCert, ChipCertFuzzer).WithDomains(Arbitrary<std::vector<std::uint8_t>>());
