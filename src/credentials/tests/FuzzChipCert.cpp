#include <cstddef>
#include <cstdint>

#include "credentials/CHIPCert.h"

using namespace chip;
using namespace chip::Credentials;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{

    NodeId nodeId;
    FabricId fabricId;

    ByteSpan span(data, len);

    (void) ExtractFabricIdFromCert(span, &fabricId);
    (void) ExtractNodeIdFabricIdFromOpCert(span, &nodeId, &fabricId);

    {
        ChipDN dn;
        (void) ExtractSubjectDNFromX509Cert(span, dn);
    }

    {
        Credentials::P256PublicKeySpan key;
        (void) ExtractPublicKeyFromChipCert(span, key);
    }

    {
        ChipCertificateData certData;
        (void) DecodeChipCert(span, certData);
    }

    return 0;
}
