#include <cstddef>
#include <cstdint>

#include "credentials/CHIPCert.h"

using namespace chip;
using namespace chip::Credentials;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    ByteSpan span(data, len);

    {
        ChipDN dn;
        (void) ExtractSubjectDNFromX509Cert(span, dn);
    }

    {
        uint8_t outCertBuf[kMaxCHIPCertLength];
        MutableByteSpan outCert(outCertBuf);
        (void) ConvertX509CertToChipCert(span, outCert);
    }

    return 0;
}
