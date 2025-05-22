#include "OtaTlvEncryptionKey.h"
#include <platform/silabs/SilabsConfig.h>

#include "mbedtls/aes.h"

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace OtaTlvEncryptionKey {

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

CHIP_ERROR OtaTlvEncryptionKey::Import(ByteSpan keySpan)
{
    VerifyOrReturnError(keySpan.size() == kOTAEncryptionKeyLength, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(DeviceLayer, "Invalid key length: %lu", (unsigned long) keySpan.size()));

    // Store the key in a member variable for later use
    memcpy(mKey, keySpan.data(), keySpan.size());
    mKeyLen = keySpan.size();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(MutableByteSpan & block, uint32_t & mIVOffset)
{
    uint8_t iv[16];
    uint8_t stream_block[16] = { 0 };
    size_t nc_off            = 0;
    uint32_t offset          = 0;
    size_t remaining         = block.size();

    memcpy(iv, au8Iv, sizeof(au8Iv));

    // Set IV based on mIVOffset
    uint32_t counter = ((uint32_t) iv[12] << 24) | ((uint32_t) iv[13] << 16) | ((uint32_t) iv[14] << 8) | (uint32_t) iv[15];

    counter += (mIVOffset / 16);
    iv[12] = (counter >> 24) & 0xFF;
    iv[13] = (counter >> 16) & 0xFF;
    iv[14] = (counter >> 8) & 0xFF;
    iv[15] = counter & 0xFF;

    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);

    if (mbedtls_aes_setkey_enc(&aes_ctx, mKey, mKeyLen * 8) != 0)
    {
        ChipLogError(DeviceLayer, "Failed to set AES key");
        mbedtls_aes_free(&aes_ctx);
        return CHIP_ERROR_INTERNAL;
    }

    // Decrypt
    if (mbedtls_aes_crypt_ctr(&aes_ctx, block.size(), &nc_off, iv, stream_block, block.data(), block.data()) != 0)
    {
        ChipLogError(DeviceLayer, "AES-CTR decryption failed");
        mbedtls_aes_free(&aes_ctx);
        return CHIP_ERROR_INTERNAL;
    }

    mIVOffset += block.size();

    ChipLogProgress(DeviceLayer, "Decryption complete");
    mbedtls_aes_free(&aes_ctx);
    return CHIP_NO_ERROR;
}
} // namespace OtaTlvEncryptionKey
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
