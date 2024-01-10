#pragma once

#include "AttestationKey.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {


class AttestationKeyMbed: public AttestationKey
{
public:
    AttestationKeyMbed() = default;
    CHIP_ERROR Import(const uint8_t * asn1, size_t size) override;
    CHIP_ERROR Export(uint8_t * asn1, size_t max, size_t &size);
    CHIP_ERROR GenerateCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr) override;
    CHIP_ERROR SignMessage(const ByteSpan & message, MutableByteSpan & out_span) override;

protected:
    uint8_t mValue[128] = { 0 };
    size_t mSize = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
