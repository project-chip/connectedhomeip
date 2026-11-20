/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "ProvisionStorageReader.h"

#include <crypto/CHIPCryptoPAL.h>
#include <headers/ProvisionStorage.h>
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#include <setup_payload/Base38Encode.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

CHIP_ERROR ProvisionStorageReader::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    constexpr size_t kDateLength            = 8;  // YYYYMMDD
    constexpr size_t kLegacyDateLength      = 10; // YYYY-MM-DD
    char date[kManufacturingDateBufferSize] = { 0 };
    char normalized[kDateLength + 1]        = { 0 };
    size_t dateLength                       = 0;
    char * parseEnd                         = nullptr;

    ReturnErrorOnFailure(GetManufacturingDate(reinterpret_cast<uint8_t *>(date), sizeof(date), dateLength));
    const char * parsedDate = date;
    if (dateLength == kLegacyDateLength && date[4] == '-' && date[7] == '-')
    {
        snprintf(normalized, sizeof(normalized), "%.4s%.2s%.2s", date, date + 5, date + 8);
        parsedDate = normalized;
        dateLength = kDateLength;
    }

    VerifyOrReturnError(dateLength >= kDateLength, CHIP_ERROR_INVALID_ARGUMENT);

    char field[5] = { 0 };
    memcpy(field, parsedDate, 4);
    year = static_cast<uint16_t>(strtoul(field, &parseEnd, 10));
    VerifyOrReturnError(parseEnd == field + 4, CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(field, parsedDate + 4, 2);
    field[2] = 0;
    month    = static_cast<uint8_t>(strtoul(field, &parseEnd, 10));
    VerifyOrReturnError(parseEnd == field + 2, CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(field, parsedDate + 6, 2);
    field[2] = 0;
    day      = static_cast<uint8_t>(strtoul(field, &parseEnd, 10));
    VerifyOrReturnError(parseEnd == field + 2, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
{
    char date[kManufacturingDateBufferSize] = { 0 };
    size_t dateLength                       = 0;
    ReturnErrorOnFailure(GetManufacturingDate(reinterpret_cast<uint8_t *>(date), sizeof(date), dateLength));

    if (dateLength == 10 && date[4] == '-' && date[7] == '-')
    {
        suffixBuffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    const size_t suffixLength = dateLength > 8 ? dateLength - 8 : 0;
    VerifyOrReturnError(suffixLength <= suffixBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    if (suffixLength > 0)
    {
        memcpy(suffixBuffer.data(), date + 8, suffixLength);
    }
    suffixBuffer.reduce_size(suffixLength);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = GetPersistentUniqueId(value.data(), value.size(), size);
#if defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;
        VerifyOrReturnError(sizeof(uniqueId) <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(value.data(), uniqueId, sizeof(uniqueId));
        size = sizeof(uniqueId);
        err  = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSetupPasscode(uint32_t & value)
{
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    value = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    return CHIP_NO_ERROR;
#else
    (void) value;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR ProvisionStorageReader::SetSetupDiscriminator(uint16_t value)
{
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ProvisionStorageReader::SetSetupPasscode(uint32_t value)
{
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pSalt(MutableByteSpan & value)
{
    char encoded[kSpake2pSaltB64BufferSize] = { 0 };
    size_t encodedSize                      = 0;
    CHIP_ERROR err                          = GetSpake2pSalt(encoded, sizeof(encoded), encodedSize);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        encodedSize = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
        VerifyOrReturnError(encodedSize <= sizeof(encoded), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(encoded, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, encodedSize);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    uint8_t decoded[kSpake2pSaltDecodedBufferSize] = { 0 };
    VerifyOrReturnError(BASE64_MAX_DECODED_LEN(encodedSize) <= kSpake2pSaltDecodedBufferSize, CHIP_ERROR_INVALID_ARGUMENT);
    const size_t decodedSize = chip::Base64Decode32(encoded, static_cast<uint32_t>(encodedSize), decoded);
    VerifyOrReturnError(decodedSize >= chip::Crypto::kSpake2p_Min_PBKDF_Salt_Length &&
                            decodedSize <= chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length,
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(decodedSize <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(value.data(), decoded, decodedSize);
    value.reduce_size(decodedSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pVerifier(MutableByteSpan & value, size_t & size)
{
    VerifyOrReturnError(value.size() >= chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    char encoded[kSpake2pVerifierB64BufferSize] = { 0 };
    size_t encodedSize                          = 0;
    CHIP_ERROR err                              = GetSpake2pVerifier(encoded, sizeof(encoded), encodedSize);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        encodedSize = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
        VerifyOrReturnError(encodedSize <= sizeof(encoded), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(encoded, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, encodedSize);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    uint8_t decoded[kSpake2pVerifierDecodedBufferSize] = { 0 };
    VerifyOrReturnError(BASE64_MAX_DECODED_LEN(encodedSize) <= kSpake2pVerifierDecodedBufferSize, CHIP_ERROR_INVALID_ARGUMENT);
    size = chip::Base64Decode32(encoded, static_cast<uint32_t>(encodedSize), decoded);
    VerifyOrReturnError(size == chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(size <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(value.data(), decoded, size);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSetupPayload(MutableCharSpan & value)
{
    uint8_t payload[Storage::kSetupPayloadSizeMax] = { 0 };
    size_t size                                    = 0;

    // Setup bits
    size_t prefix_len = strlen(kQRCodePrefix);
    VerifyOrReturnError(value.size() > prefix_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    CHIP_ERROR err = GetSetupPayload(payload, sizeof(payload), size);
#if SL_MATTER_QR_CODE_ENABLED || SILABS_LOG_ENABLED
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        PayloadContents payloadContents;

        // Configure PayloadContents
        payloadContents.version = 0;
        payloadContents.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kBLE);
        payloadContents.setUpPINCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;

        uint16_t discriminator = 0;
        ReturnErrorOnFailure(GetSetupDiscriminator(discriminator));
        payloadContents.discriminator.SetLongValue(discriminator);

        ReturnErrorOnFailure(GetVendorId(payloadContents.vendorID));
        ReturnErrorOnFailure(GetProductId(payloadContents.productID));

        // Generate Setup payload byte array
        ReturnErrorOnFailure(QRCodeBasicSetupPayloadGenerator(payloadContents).payloadBase38Representation(value));

        return CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#endif // SL_MATTER_QR_CODE_ENABLED || SILABS_LOG_ENABLED

    ReturnErrorOnFailure(err);
    VerifyOrReturnError(size > 0, CHIP_ERROR_NOT_FOUND);
    char * data = value.data();
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
