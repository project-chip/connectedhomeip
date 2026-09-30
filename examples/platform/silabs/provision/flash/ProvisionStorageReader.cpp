/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/TestEventTriggerDelegate.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <headers/ProvisionStorage.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#include <provision/ProvisionCrypto.h>
#include <provision/ProvisionStorageReader.h>
#include <provision/flash/ProvisionStorageFlash.h>
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
#endif
#include <cstring>
using namespace chip::Credentials;
namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
ProvisionStorageReader & ProvisionStorageReader::GetInstance()
{
    static ProvisionStorageReader instance;
    return instance;
}

CHIP_ERROR ProvisionStorageReader::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kSerialNumber, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetVendorId(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kVendorId, value);
#if defined(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID) && CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetVendorName(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kVendorName, value, max, size);
#if defined(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProductId(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kProductId, value);
#if defined(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID) && CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProductName(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kProductName, value, max, size);
#if defined(CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_TEST_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductLabel, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductUrl, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kPartNumber, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersion(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kHwVersion, value);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersionString(char * value, size_t max)
{
    size_t size    = 0;
    CHIP_ERROR err = Flash::Get(Parameters::ID::kHwVersionStr, value, max, size);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        err = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kManufacturingDate, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kPersistentUniqueId, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupDiscriminator(uint16_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kDiscriminator, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    ReturnErrorOnFailure(err);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pIterationCount(uint32_t & value)
{
    CHIP_ERROR err = Flash::Get(Parameters::ID::kSpake2pIterations, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err   = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSpake2pSalt, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSpake2pVerifier, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetFirmwareInformation(MutableByteSpan & value)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetCertificationDeclaration(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kCertification, value.data(), value.size(), size));
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example CD
        return Examples::GetExampleDACProvider()->GetCertificationDeclaration(value);
    }
#endif // SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kPaiCert, value.data(), value.size(), size));
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example PAI
        return Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(value);
    }
#endif // SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetDeviceAttestationCert(MutableByteSpan & value)
{
    size_t size    = 0;
    CHIP_ERROR err = (Flash::Get(Parameters::ID::kDacCert, value.data(), value.size(), size));
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        // Example DAC
        return Examples::GetExampleDACProvider()->GetDeviceAttestationCert(value);
    }
#endif // SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    CHIP_ERROR err = ProvisionCrypto::GetInstance().SignWithDeviceAttestationKey(message, signature);
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (err == CHIP_ERROR_NOT_FOUND || err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        return Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message, signature);
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kVersion, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    return Flash::Get(Parameters::ID::kSetupPayload, value, max, size);
}

#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION
CHIP_ERROR ProvisionStorageReader::GetOtaTlvEncryptionKeyId(uint32_t & keyId)
{
#if defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
    (void) keyId;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    return Flash::Get(Parameters::ID::kOtaTlvEncryptionKey, keyId);
#endif
}
CHIP_ERROR ProvisionStorageReader::DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset)
{
#if defined(SL_MBEDTLS_USE_TINYCRYPT) && SL_MBEDTLS_USE_TINYCRYPT
    uint8_t keyBuffer[Silabs::OtaTlvEncryptionKey::kOTAEncryptionKeyLength] = { 0 };
    size_t keyLength                                                        = 0;
    ReturnErrorOnFailure(Flash::Get(Parameters::ID::kOtaTlvEncryptionKey, keyBuffer, sizeof(keyBuffer), keyLength));
    VerifyOrReturnError(keyLength == sizeof(keyBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    return Silabs::OtaTlvEncryptionKey::Decrypt(ByteSpan(keyBuffer), block, ivOffset);
#else
    (void) block;
    (void) ivOffset;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}
#else
CHIP_ERROR ProvisionStorageReader::GetOtaTlvEncryptionKeyId(uint32_t & keyId)
{
    (void) keyId;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
CHIP_ERROR ProvisionStorageReader::DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset)
{
    (void) block;
    (void) ivOffset;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
#endif
CHIP_ERROR ProvisionStorageReader::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
#if defined(SL_MATTER_TEST_EVENT_TRIGGER_ENABLED) && SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    constexpr size_t kEnableKeyLength = TestEventTriggerDelegate::kEnableKeyLength;
    size_t keyLength                  = 0;
    VerifyOrReturnError(keySpan.size() >= kEnableKeyLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    CHIP_ERROR err = Flash::Get(Parameters::ID::kTestEventTriggerKey, keySpan.data(), kEnableKeyLength, keyLength);
#ifndef NDEBUG
#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLE_KEY
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        constexpr char enableKey[] = SL_MATTER_TEST_EVENT_TRIGGER_ENABLE_KEY;
        VerifyOrReturnError(Encoding::HexToBytes(enableKey, strlen(enableKey), keySpan.data(), kEnableKeyLength) ==
                                kEnableKeyLength,
                            CHIP_ERROR_INTERNAL);
        keyLength = kEnableKeyLength;
        err       = CHIP_NO_ERROR;
    }
#endif
#endif
    ReturnErrorOnFailure(err);
    VerifyOrReturnError(keyLength == kEnableKeyLength, CHIP_ERROR_INVALID_ARGUMENT);
    keySpan.reduce_size(keyLength);
    return CHIP_NO_ERROR;
#else
    (void) keySpan;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
