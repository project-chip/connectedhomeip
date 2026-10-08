/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#include <platform/silabs/SilabsConfig.h>
#include <platform/silabs/multi-ota/OtaTlvEncryptionKey.h>
#include <provision/ProvisionCrypto.h>
#include <provision/ProvisionStorageReader.h>
#include <silabs_creds.h>

#ifndef NDEBUG
#if defined(SL_MATTER_TEST_EVENT_TRIGGER_ENABLED) && (SL_MATTER_GN_BUILD == 0)
#include <sl_matter_test_event_trigger_config.h>
#endif
#endif

#include <cstring>

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

ProvisionStorageReader & ProvisionStorageReader::GetInstance()
{
    static ProvisionStorageReader instance;
    return instance;
}

namespace {

CHIP_ERROR ReadFileByOffset(const char * description, uint32_t offset, uint32_t size, MutableByteSpan & value)
{
    uint32_t base_addr = 0;
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, base_addr));

    uint8_t * address = reinterpret_cast<uint8_t *>(static_cast<uintptr_t>(base_addr + offset));
    ByteSpan span(address, size);
    ChipLogProgress(DeviceLayer, "%s, addr:0x%06x+%03u, size:%u", description, (unsigned) base_addr, (unsigned) offset,
                    (unsigned) size);
    return CopySpanToMutableSpan(span, value);
}

CHIP_ERROR ReadFileByKey(const char * description, uint32_t offset_key, uint32_t size_key, MutableByteSpan & value)
{
    uint32_t offset = 0;
    uint32_t size   = 0;

    // Offset
    VerifyOrReturnError(SilabsConfig::ConfigValueExists(offset_key), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(offset_key, offset));

    // Size
    VerifyOrReturnError(SilabsConfig::ConfigValueExists(size_key), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(size_key, size));

    return ReadFileByOffset(description, offset, size, value);
}

} // namespace

//
// DeviceInstanceInfoProvider
//

CHIP_ERROR ProvisionStorageReader::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_SerialNum, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetVendorId(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_VendorId, value);
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
    size_t name_len = 0; // Without counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_VendorName, value, max, name_len);
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
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_ProductId, value);

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
    size_t name_len = 0; // Without counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductName, value, max, name_len);
#if defined(CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_TEST_PRODUCT_NAME);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductLabel, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductURL, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_PartNumber, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersion(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_HardwareVersion, value);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetHardwareVersionString(char * value, size_t max)
{
    size_t hw_version_len = 0; // Without counting null-terminator

    CHIP_ERROR err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, value, max, hw_version_len);
#if defined(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        VerifyOrReturnError(value != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(max > strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(value, max, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        err = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetManufacturingDate(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ManufacturingDate, (char *) value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetPersistentUniqueId(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_PersistentUniqueId, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupDiscriminator(uint16_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err   = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    VerifyOrReturnLogError(value <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pIterationCount(uint32_t & value)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, value);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        value = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err   = CHIP_NO_ERROR;
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pSalt(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSpake2pVerifier(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetFirmwareInformation(MutableByteSpan & value)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionStorageReader::GetCertificationDeclaration(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey("GetCertificationDeclaration", SilabsConfig::kConfigKey_Creds_CD_Offset,
                                   SilabsConfig::kConfigKey_Creds_CD_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset("GetCertificationDeclaration", SL_CREDENTIALS_CD_OFFSET, SL_CREDENTIALS_CD_SIZE, value);
    }
#endif
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        err = chip::Credentials::Examples::GetExampleDACProvider()->GetCertificationDeclaration(value);
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey("GetProductAttestationIntermediateCert", SilabsConfig::kConfigKey_Creds_PAI_Offset,
                                   SilabsConfig::kConfigKey_Creds_PAI_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset("GetProductAttestationIntermediateCert", SL_CREDENTIALS_PAI_OFFSET, SL_CREDENTIALS_PAI_SIZE, value);
    }
#endif
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        err = chip::Credentials::Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(value);
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetDeviceAttestationCert(MutableByteSpan & value)
{
    CHIP_ERROR err = ReadFileByKey("GetDeviceAttestationCert", SilabsConfig::kConfigKey_Creds_DAC_Offset,
                                   SilabsConfig::kConfigKey_Creds_DAC_Size, value);
#if defined(SL_PROVISION_VERSION_1_0) && SL_PROVISION_VERSION_1_0
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // Reading from the old script's location.
        err = ReadFileByOffset("GetDeviceAttestationCert", SL_CREDENTIALS_DAC_OFFSET, SL_CREDENTIALS_DAC_SIZE, value);
    }
#endif
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        return chip::Credentials::Examples::GetExampleDACProvider()->GetDeviceAttestationCert(value);
    }
#endif
    return err;
}

CHIP_ERROR ProvisionStorageReader::GetProvisionVersion(char * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Provision_Version, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::GetSetupPayload(uint8_t * value, size_t max, size_t & size)
{
    return SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, value, max, size);
}

CHIP_ERROR ProvisionStorageReader::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    CHIP_ERROR error = ProvisionCrypto::GetInstance().SignWithDeviceAttestationKey(message, signature);
#if SL_MATTER_ENABLE_EXAMPLE_CREDENTIALS
    if (error == CHIP_ERROR_NOT_FOUND || error == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        return chip::Credentials::Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message, signature);
    }
#endif
    return error;
}

CHIP_ERROR ProvisionStorageReader::GetTestEventTriggerKey(MutableByteSpan & keySpan)
{
#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    constexpr size_t kEnableKeyLength = 16;
    size_t keyLength                  = 0;
    VerifyOrReturnError(keySpan.size() >= kEnableKeyLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    CHIP_ERROR err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_Test_Event_Trigger_Key, keySpan.data(),
                                                      kEnableKeyLength, keyLength);
#ifndef NDEBUG
#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLE_KEY
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        constexpr char enableKey[] = SL_MATTER_TEST_EVENT_TRIGGER_ENABLE_KEY;
        VerifyOrReturnError(Encoding::HexToBytes(enableKey, strlen(enableKey), keySpan.data(), kEnableKeyLength) ==
                                kEnableKeyLength,
                            CHIP_ERROR_INTERNAL);
        err = CHIP_NO_ERROR;
    }
#endif
#endif
    ReturnErrorOnFailure(err);
    keySpan.reduce_size(kEnableKeyLength);
    return CHIP_NO_ERROR;
#else
    (void) keySpan;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR ProvisionStorageReader::GetOtaTlvEncryptionKeyId(uint32_t & value)
{
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION && !defined(SL_MBEDTLS_USE_TINYCRYPT)
    return SilabsConfig::ReadConfigValue(SilabsConfig::kOtaTlvEncryption_KeyId, value);
#else
    (void) value;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR ProvisionStorageReader::DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset)
{
#if defined(SL_MATTER_ENABLE_OTA_ENCRYPTION) && SL_MATTER_ENABLE_OTA_ENCRYPTION && defined(SL_MBEDTLS_USE_TINYCRYPT) &&            \
    SL_MBEDTLS_USE_TINYCRYPT
    uint8_t keyBuffer[Silabs::OtaTlvEncryptionKey::kOTAEncryptionKeyLength] = { 0 };
    size_t keyLength                                                        = 0;
    ReturnErrorOnFailure(
        SilabsConfig::ReadConfigValueBin(SilabsConfig::kOtaTlvEncryption_KeyId, keyBuffer, sizeof(keyBuffer), keyLength));
    VerifyOrReturnError(keyLength == sizeof(keyBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    return Silabs::OtaTlvEncryptionKey::Decrypt(ByteSpan(keyBuffer), block, ivOffset);
#else
    (void) block;
    (void) ivOffset;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
