#pragma once

#include <lib/support/Span.h>
#include <lib/core/CHIPError.h>
#include <psa/crypto.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static constexpr uint32_t kCreds_KeyId_Default = (PSA_KEY_ID_USER_MIN + 1);

class AttestationKey
{
public:
    AttestationKey(uint32_t id = 0) { mId = (id > 0)? id : kCreds_KeyId_Default; }
    ~AttestationKey() = default;

    uint32_t GetId() { return mId; }
    CHIP_ERROR Import(const uint8_t * asn1, size_t size);
    CHIP_ERROR Export(uint8_t * asn1, size_t max, size_t &size);
    CHIP_ERROR GenerateCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr);
    CHIP_ERROR SignMessage(const ByteSpan & message, MutableByteSpan & out_span);

protected:
    uint32_t mId = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
