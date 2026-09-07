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

    RETURN_SAFELY_IGNORED ExtractFabricIdFromCert(span, &fabricId);
    RETURN_SAFELY_IGNORED ExtractNodeIdFabricIdFromOpCert(span, &nodeId, &fabricId);

    {
        ChipDN dn;
        RETURN_SAFELY_IGNORED ExtractSubjectDNFromX509Cert(span, dn);
    }

    {
        Credentials::P256PublicKeySpan key;
        RETURN_SAFELY_IGNORED ExtractPublicKeyFromChipCert(span, key);
    }

    {
        ChipCertificateData certData;
        RETURN_SAFELY_IGNORED DecodeChipCert(span, certData);
    }

    return 0;
}
