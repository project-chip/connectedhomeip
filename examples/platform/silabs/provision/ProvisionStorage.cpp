#include "ProvisionStorage.h"
#include <setup_payload/SetupPayload.h>
#include <setup_payload/Base38Encode.h>
#include <lib/support/Base64.h>
#include <platform/CHIPDeviceError.h>

// using namespace chip::Credentials;
// using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;


CHIP_ERROR Storage::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err;
    constexpr uint8_t kDateStringLength = 10; // YYYY-MM-DD
    char date[kDateStringLength + 1];
    size_t date_len;
    char * parse_end;

    err = GetManufacturingDate(date, sizeof(date));
    SuccessOrExit(err);

    VerifyOrExit(date_len == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul(date, &parse_end, 10));
    VerifyOrExit(parse_end == date + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul(date + 5, &parse_end, 10));
    VerifyOrExit(parse_end == date + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul(date + 8, &parse_end, 10));
    VerifyOrExit(parse_end == date + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", date);
    }
    return err;
}

CHIP_ERROR Storage::GetSpake2pSalt(MutableByteSpan & value)
{
    // Base64
    char salt_b64[kSpake2pSaltB64LengthMax + 1] = { 0 };
    size_t size_b64                       = 0;
    ReturnErrorOnFailure(this->GetSpake2pSalt(salt_b64, sizeof(salt_b64), size_b64));

    // Decode
    uint8_t salt[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t size = chip::Base64Decode32(salt_b64, size_b64, salt);
    ReturnErrorCodeIf(size > value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Copy
    memcpy(value.data(), salt, size);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetSpake2pVerifier(MutableByteSpan & out_value, size_t & out_size)
{
    // Base64
    char verifier_b64[kSpake2pVerifierB64LengthMax + 1] = { 0 };
    size_t size_b64                                     = 0;
    ReturnErrorOnFailure(this->GetSpake2pVerifier(verifier_b64, sizeof(verifier_b64), size_b64));

    // Decode
    out_size = chip::Base64Decode32(verifier_b64, size_b64, reinterpret_cast<uint8_t *>(verifier_b64));
    ReturnErrorCodeIf(out_size > out_value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Copy
    memcpy(out_value.data(), verifier_b64, out_size);
    out_value.reduce_size(out_size);
    return CHIP_NO_ERROR;
}


CHIP_ERROR Storage::GetSetupPayload(chip::MutableCharSpan &value)
{
    uint8_t payload[kSetupPayloadSizeMax];
    size_t size = 0;

    // Setup bits
    size_t prefix_len = strlen(kQRCodePrefix);
    VerifyOrReturnError(value.size() > prefix_len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(this->GetSetupPayload(payload, sizeof(payload), size));
    VerifyOrReturnError(size > 0, CHIP_ERROR_NOT_FOUND);
    char *data = value.data();
    // Prefix
    memcpy(data, kQRCodePrefix, prefix_len);
    // Base38
    MutableCharSpan qr_code(data + prefix_len, value.size() - prefix_len);
    ReturnErrorOnFailure(base38Encode(ByteSpan(payload, size), qr_code));
    value.reduce_size(prefix_len + qr_code.size());
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
