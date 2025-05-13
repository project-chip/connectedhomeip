#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>
#include <psa/crypto.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace OtaTlvEncryptionKey {

static constexpr uint32_t kAES_KeyId_Default    = (PSA_KEY_ID_USER_MIN + 2);
static constexpr size_t kOTAEncryptionKeyLength = 128u / 8u; // 128 bits

static constexpr uint8_t au8Iv[] = {
    0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x00, 0x00, 0x00, 0x00
};

class OtaTlvEncryptionKey
{
public:
    OtaTlvEncryptionKey(uint32_t id = 0) { mId = (id > 0) ? id : kAES_KeyId_Default; }
    ~OtaTlvEncryptionKey() = default;

    uint32_t GetId() { return mId; }
    CHIP_ERROR Import(const uint8_t * key, size_t key_len);
    CHIP_ERROR Decrypt(MutableByteSpan & block, uint32_t & mIVOffset);

protected:
    uint32_t mId = 0;
};

} // namespace OtaTlvEncryptionKey
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
