#pragma once

#include <lib/support/Span.h>
#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class AttestationKey
{
public:
    virtual ~AttestationKey() = default;
    virtual CHIP_ERROR Import(const uint8_t * asn1, size_t size) = 0;
    virtual CHIP_ERROR GenerateCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr) = 0;
    virtual CHIP_ERROR SignMessage(const ByteSpan & message, MutableByteSpan & out_span) = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
