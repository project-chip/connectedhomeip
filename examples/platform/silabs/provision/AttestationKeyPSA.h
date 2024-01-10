#pragma once

#include "AttestationKey.h"
#include <psa/crypto.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static constexpr uint32_t kCreds_KeyId_Default = (PSA_KEY_ID_USER_MIN + 1);

class AttestationKeyPSA: public AttestationKey
{
public:
    AttestationKeyPSA(uint32_t id) { mId = (id > 0)? id : kCreds_KeyId_Default; }
    CHIP_ERROR Import(const uint8_t * asn1, size_t size) override;
    CHIP_ERROR GenerateCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr) override;
    CHIP_ERROR SignMessage(const ByteSpan & message, MutableByteSpan & out_span) override;
    uint32_t GetId() { return mId; }

protected:
    uint32_t mId = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
