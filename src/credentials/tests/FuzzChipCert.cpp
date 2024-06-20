#include <cstddef>
#include <cstdint>

#include "credentials/CHIPCert.h"

using namespace chip;
using namespace chip::Credentials;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    ByteSpan span(data, len);

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

    return 0;
}
