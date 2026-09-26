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

#include <crypto/CHIPCryptoPAL.h>
#include <headers/ProvisionStorage.h>
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#ifdef SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
#include <credentials/examples/DeviceAttestationCredsExample.h>
#endif // SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    constexpr uint8_t kDateLength              = 8;  // YYYYMMDD
    constexpr uint8_t kLegacyDateLength        = 10; // YYYY-MM-DD
    char date[kManufacturingDateLengthMax + 1] = { 0 };
    char temp[kManufacturingDateLengthMax + 1] = { 0 };
    size_t date_len                            = 0;
    char * parse_end                           = nullptr;
    CHIP_ERROR err                             = CHIP_NO_ERROR;

    ReturnErrorOnFailure(GetManufacturingDate((uint8_t *) date, sizeof(date), date_len));
    // Convert legacy date format to new date format
    if ((kLegacyDateLength == date_len) && ('-' == date[4]) && ('-' == date[7]))
    {
        date_len = kDateLength;
        snprintf(temp, sizeof(temp), "%.4s%.2s%.2s", date, date + 5, date + 8);
        memcpy(date, temp, date_len);
        ReturnErrorOnFailure(SetManufacturingDate(date, date_len));
    }
    VerifyOrExit(date_len >= kDateLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    // Year
    memcpy(temp, date, 4); // yyyy
    temp[4] = 0;
    year    = static_cast<uint16_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 4), err = CHIP_ERROR_INVALID_ARGUMENT);
    // Month
    memcpy(temp, &date[4], 2); // mm
    temp[2] = 0;
    month   = static_cast<uint8_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 2), err = CHIP_ERROR_INVALID_ARGUMENT);
    // Day
    memcpy(temp, &date[6], 2); // dd
    temp[2] = 0;
    day     = static_cast<uint8_t>(strtoul(temp, &parse_end, 10));
    VerifyOrExit(parse_end == (temp + 2), err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        printf("Invalid manufacturing date: %s", date);
    }
    return err;
}

CHIP_ERROR Storage::GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
{
    char date[kManufacturingDateLengthMax + 1] = { 0 };
    size_t date_len                            = 0;

    ReturnErrorOnFailure(GetManufacturingDate((uint8_t *) date, sizeof(date), date_len));
    if (date_len > 8)
    {
        const size_t suffixLen = date_len - 8;
        VerifyOrReturnError(suffixLen <= suffixBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(suffixBuffer.data(), date + 8, suffixLen);
        suffixBuffer.reduce_size(suffixLen);
        return CHIP_NO_ERROR;
    }
    suffixBuffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size = 0;

    CHIP_ERROR err = GetPersistentUniqueId(value.data(), value.size(), size);
#if defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        constexpr uint8_t unique_id[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;
        VerifyOrReturnError(sizeof(unique_id) <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(value.data(), unique_id, sizeof(unique_id));
        size = sizeof(unique_id);
        err  = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::GetSpake2pSalt(MutableByteSpan & value)
{
    // Base64
    char salt_b64[kSpake2pSaltB64LengthMax + 1] = { 0 };
    size_t size_b64                             = 0;

    CHIP_ERROR err = this->GetSpake2pSalt(salt_b64, sizeof(salt_b64), size_b64);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        size_b64 = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
        VerifyOrReturnError(size_b64 <= sizeof(salt_b64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(salt_b64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, size_b64);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    // Decode
    uint8_t salt[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length] = { 0 };
    size_t size                                                = chip::Base64Decode32(salt_b64, size_b64, salt);
    VerifyOrReturnError(size >= chip::Crypto::kSpake2p_Min_PBKDF_Salt_Length &&
                            size <= chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length,
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(size <= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Copy
    memcpy(value.data(), salt, size);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetDeviceAttestationCertForProfile(chip::Credentials::DeviceAttestationCertProfile profile,
                                                       MutableByteSpan & out_dac_buffer)
{
    // Silabs storage only tracks the legacy Matter chain today; any other profile is served
    // via the example provider so callers keep a defined error path.
    if (profile == chip::Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy)
    {
        CHIP_ERROR err = GetDeviceAttestationCert(out_dac_buffer);
        if (err != CHIP_ERROR_NOT_FOUND)
        {
            return err;
        }
    }
#ifdef SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    return Credentials::Examples::GetExampleDACProvider()->GetDeviceAttestationCertForProfile(profile, out_dac_buffer);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR Storage::GetProductAttestationIntermediateCertForProfile(chip::Credentials::DeviceAttestationCertProfile profile,
                                                                    MutableByteSpan & out_pai_buffer)
{
    if (profile == chip::Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy)
    {
        CHIP_ERROR err = GetProductAttestationIntermediateCert(out_pai_buffer);
        if (err != CHIP_ERROR_NOT_FOUND)
        {
            return err;
        }
    }
#ifdef SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    return Credentials::Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCertForProfile(profile, out_pai_buffer);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

chip::Credentials::DeviceAttestationProfileSupport Storage::GetDeviceAttestationProfileSupport() const
{
    const auto legacy = BitMask<chip::Credentials::DeviceAttestationCertProfileBitmap>(
        chip::Credentials::DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy);
    return { legacy, legacy, legacy };
}

chip::Credentials::DeviceAttestationCertProfile Storage::GetPreferredDeviceAttestationChainProfile() const
{
    // Silabs storage only tracks the legacy Matter chain.
    return chip::Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy;
}

CHIP_ERROR Storage::GetDeviceAttestationDocumentSegment(chip::Credentials::DeviceAttestationDocumentType documentType,
                                                        chip::Credentials::DeviceAttestationCertProfile profile, size_t offset,
                                                        MutableByteSpan & out_document_buffer, size_t & out_document_size)
{
    // Silabs storage returns whole documents from NVM3, so only offset 0 on the legacy chain
    // maps onto an actual read; every other request is delegated to the example provider.
    if (profile == chip::Credentials::DeviceAttestationCertProfile::kEcdsaMatterLegacy && offset == 0)
    {
        CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;
        switch (documentType)
        {
        case chip::Credentials::DeviceAttestationDocumentType::kDACCertificate:
            err = GetDeviceAttestationCert(out_document_buffer);
            break;
        case chip::Credentials::DeviceAttestationDocumentType::kPAICertificate:
            err = GetProductAttestationIntermediateCert(out_document_buffer);
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (err != CHIP_ERROR_NOT_FOUND)
        {
            ReturnErrorOnFailure(err);
            out_document_size = out_document_buffer.size();
            return CHIP_NO_ERROR;
        }
    }
#ifdef SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    return Credentials::Examples::GetExampleDACProvider()->GetDeviceAttestationDocumentSegment(
        documentType, profile, offset, out_document_buffer, out_document_size);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR Storage::GetSpake2pVerifier(MutableByteSpan & out_value, size_t & out_size)
{
    VerifyOrReturnError(out_value.size() >= chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Base64
    char verifier_b64[kSpake2pVerifierB64LengthMax + 1] = { 0 };
    size_t size_b64                                     = 0;

    CHIP_ERROR err = this->GetSpake2pVerifier(verifier_b64, sizeof(verifier_b64), size_b64);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        size_b64 = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
        VerifyOrReturnError(size_b64 <= sizeof(verifier_b64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(verifier_b64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, size_b64);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    // Decode
    out_size = chip::Base64Decode32(verifier_b64, size_b64, out_value.data());
    VerifyOrReturnError(out_size == chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_INVALID_ARGUMENT);
    out_value.reduce_size(out_size);
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
