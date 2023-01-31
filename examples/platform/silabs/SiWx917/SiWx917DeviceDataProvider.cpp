/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "SiWx917DeviceDataProvider.h"
#include <platform/silabs/SilabsConfig.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <setup_payload/Base38Encode.h>
#include <setup_payload/SetupPayload.h>
#include "DeviceConfig.h"
#include "siwx917_utils.h"

namespace chip {
namespace DeviceLayer {
namespace SIWx917 {
#include <string>
using namespace std;

// using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

#ifdef SIWX917_USE_COMISSIONABLE_DATA 
uint8_t SIWx917DeviceDataProvider::WriteBits(uint8_t * bits, uint8_t offset, uint64_t input, uint8_t numberOfBits, uint8_t totalPayloadInBits){
    if((offset + numberOfBits) > totalPayloadInBits){
        return -1;
    }
    uint8_t index = offset;
    offset += numberOfBits;
    while(input != 0){
        if(input & 1){
            bits[int(index/8)] |= (1 << (index % 8));
        }
        index++;
        input >>= 1;
    }
    return offset;
}

void SIWx917DeviceDataProvider::generateQrCodeBitSet(uint8_t * payload){
    uint8_t kVersionFieldLengthInBits =3;
    uint8_t kVendorIDFieldLengthInBits = 16;
    uint8_t kProductIDFieldLengthInBits = 16;
    uint8_t kCommissioningFlowFieldLengthInBits = 2;
    uint8_t kRendezvousInfoFieldLengthInBits = 8;
    uint8_t kPayloadDiscriminatorFieldLengthInBits = 12;
    uint8_t kSetupPINCodeFieldLengthInBits = 27;
    uint8_t kPaddingFieldLengthInBits = 4;

    uint8_t kTotalPayloadDataSizeInBits = (kVersionFieldLengthInBits + kVendorIDFieldLengthInBits + kProductIDFieldLengthInBits +
                                       kCommissioningFlowFieldLengthInBits + kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
                                       kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits);
    uint8_t offset = 0;
    uint8_t * fillBits;
    fillBits = (uint8_t *)malloc(sizeof(uint8_t)*11);
    memset(fillBits,0,sizeof(fillBits)*11);

    offset = WriteBits(fillBits, offset, 0, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)vendorId, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)productId, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)commissionableFlow, kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)rendezvousFlag, kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)discriminatorValue, kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, (uint64_t)passcode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteBits(fillBits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits);

    for(uint8_t i = 0;i<kTotalPayloadDataSizeInBits/8;i++){
        payload[i] = fillBits[i];
    }
}

CHIP_ERROR SIWx917DeviceDataProvider::FlashFactoryData(){
    // flashing the value to the nvm3 section of the flash
    // TODO: remove this once it is removed SiWx917 have the nvm3 simiplicity commander support
    CHIP_ERROR err;
    // Checking for the value of CM and flag
    if((commissionableFlow > 3) || (rendezvousFlag > 7)){
        return CHIP_ERROR_INTERNAL;
    }
    // writing to the flash based on the value given in the DeviceConfig.h
    if(discriminatorValue != 0){    
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_SetupDiscriminator, discriminatorValue);
        if(err != CHIP_NO_ERROR){
            return err;
        }
    }
    uint8_t payload[11];			
    generateQrCodeBitSet(payload);
    err = SILABSConfig::WriteConfigValueBin(SILABSConfig::kConfigKey_SetupPayloadBitSet, payload, 11);
    if(err != CHIP_NO_ERROR){
        return err;
    }
    if(spake2Interation != 0){
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_Spake2pIterationCount, spake2Interation);
        if(err != CHIP_NO_ERROR){
            return err;
        }
    }
    if(spake2Salt != NULL){
        err = SILABSConfig::WriteConfigValueStr(SILABSConfig::kConfigKey_Spake2pSalt, spake2Salt);
        if(err != CHIP_NO_ERROR){
            return err;
        }
    }
    if(spake2Verifier != NULL){
        err = SILABSConfig::WriteConfigValueStr(SILABSConfig::kConfigKey_Spake2pVerifier, spake2Verifier);
        if(err != CHIP_NO_ERROR){
            return err;
        }
    }
    if(productId != 0){
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_ProductId, productId);
        if(err != CHIP_NO_ERROR){
            return err;
        }
    }
    if(vendorId != 0){
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_VendorId, vendorId);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
    if(strlen(productName) != 0){
        err = SILABSConfig::WriteConfigValueStr(SILABSConfig::kConfigKey_ProductName, productName);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
    if(strlen(vendorName) != 0){
        err = SILABSConfig::WriteConfigValueStr(SILABSConfig::kConfigKey_VendorName, vendorName);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
    if(strlen(hwVersionString) != 0){
        err = SILABSConfig::WriteConfigValueStr(SILABSConfig::kConfigKey_HardwareVersionString, hwVersionString);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
    if(rotatingId != 0){
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_UniqueId, rotatingId);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
    if(commissionableFlow != 0){
        err = SILABSConfig::WriteConfigValue(SILABSConfig::kConfigKey_ProductName, commissionableFlow);
        if(err != CHIP_NO_ERROR){
            return err;
        }    
    }
}
#endif

CHIP_ERROR SIWx917DeviceDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err;
    uint32_t setupDiscriminator32;

    err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_SetupDiscriminator, setupDiscriminator32);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setupDiscriminator32 = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err                  = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR

    VerifyOrReturnLogError(setupDiscriminator32 <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(setupDiscriminator32);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    CHIP_ERROR err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Spake2pIterationCount, iterationCount);

#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        iterationCount = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err            = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);

#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        saltB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
        ReturnErrorCodeIf(saltB64Len > sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(saltB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, saltB64Len);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)

    ReturnErrorOnFailure(err);

    uint8_t saltByteArray[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltLen                                   = chip::Base64Decode32(saltB64, saltB64Len, saltByteArray);
    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltBuf.data(), saltByteArray, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

    err = SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64),
                                           verifierB64Len);

#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        verifierB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
        ReturnErrorCodeIf(verifierB64Len > sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(verifierB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, verifierB64Len);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)

    ReturnErrorOnFailure(err);

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetSetupPayload(MutableCharSpan & payloadBuf)
{
    CHIP_ERROR err                                      = CHIP_NO_ERROR;
    uint8_t payloadBitSet[kTotalPayloadDataSizeInBytes] = { 0 };
    size_t bitSetLen                                    = 0;

    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, payloadBitSet, kTotalPayloadDataSizeInBytes,
                                           bitSetLen);

#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        static constexpr uint8_t kTestSetupPayloadBitset[] = { 0x88, 0xFF, 0x2F, 0x00, 0x44, 0x00, 0xE0, 0x4B, 0x84, 0x68, 0x02 };
        bitSetLen                                          = sizeof(kTestSetupPayloadBitset);
        ReturnErrorCodeIf(bitSetLen > kTotalPayloadDataSizeInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(payloadBitSet, kTestSetupPayloadBitset, bitSetLen);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)

    ReturnErrorOnFailure(err);

    size_t prefixLen = strlen(kQRCodePrefix);

    if (payloadBuf.size() < prefixLen + 1)
    {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else
    {
        MutableCharSpan subSpan = payloadBuf.SubSpan(prefixLen, payloadBuf.size() - prefixLen);
        memcpy(payloadBuf.data(), kQRCodePrefix, prefixLen);
        err = base38Encode(MutableByteSpan(payloadBitSet), subSpan);
        // Reduce output span size to be the size of written data
        payloadBuf.reduce_size(subSpan.size() + prefixLen);
    }

    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    size_t vendorNameLen = 0; // without counting null-terminator
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_VendorName, buf, bufSize, vendorNameLen);
}

CHIP_ERROR SIWx917DeviceDataProvider::GetVendorId(uint16_t & vendorId)
{
    ChipError err       = CHIP_NO_ERROR;
    uint32_t vendorId32 = 0;

    err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_VendorId, vendorId32);

#if defined(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID) && CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        vendorId32 = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        err        = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID) && CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID

    ReturnErrorOnFailure(err);
    vendorId = static_cast<uint16_t>(vendorId32);
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetProductName(char * buf, size_t bufSize)
{
    size_t productNameLen = 0; // without counting null-terminator
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_ProductName, buf, bufSize, productNameLen);
}

CHIP_ERROR SIWx917DeviceDataProvider::GetProductId(uint16_t & productId)
{
    ChipError err        = CHIP_NO_ERROR;
    uint32_t productId32 = 0;

    err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_ProductId, productId32);

#if defined(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID) && CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        productId32 = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        err         = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID) && CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
    ReturnErrorOnFailure(err);

    productId = static_cast<uint16_t>(productId32);
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    size_t hardwareVersionStringLen = 0; // without counting null-terminator
    CHIP_ERROR err =
        SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_HardwareVersionString, buf, bufSize, hardwareVersionStringLen);
#if defined(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        memcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING, sizeof(bufSize));
        err = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    CHIP_ERROR err;
    uint32_t hardwareVersion32;

    err = SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_HardwareVersion, hardwareVersion32);
#if defined(CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        hardwareVersion32 = CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION;
        err               = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION)

    hardwareVersion = static_cast<uint16_t>(hardwareVersion32);
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    size_t uniqueIdLen = 0;
    err =
        SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_UniqueId, uniqueIdSpan.data(), uniqueIdSpan.size(), uniqueIdLen);
#ifdef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

        ReturnErrorCodeIf(sizeof(uniqueId) > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
        uniqueIdLen = sizeof(uniqueId);
    }
#endif // CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID

    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);

#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
    return err;
}

CHIP_ERROR SIWx917DeviceDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    size_t serialNumberLen = 0; // without counting null-terminator
    return SilabsConfig::ReadConfigValueStr(SilabsConfig::kConfigKey_SerialNum, buf, bufSize, serialNumberLen);
}

CHIP_ERROR SIWx917DeviceDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err;
    constexpr uint8_t kDateStringLength = 10; // YYYY-MM-DD
    char dateStr[kDateStringLength + 1];
    size_t dateLen;
    char * parseEnd;

    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_ManufacturingDate, reinterpret_cast<uint8_t *>(dateStr),
                                           sizeof(dateStr), dateLen);
    SuccessOrExit(err);

    VerifyOrExit(dateLen == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul(dateStr, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul(dateStr + 5, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul(dateStr + 8, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", dateStr);
    }
    return err;
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

SIWx917DeviceDataProvider & SIWx917DeviceDataProvider::GetDeviceDataProvider()
{
    static SIWx917DeviceDataProvider sDataProvider;
    return sDataProvider;
}

} // namespace SIWx917
} // namespace DeviceLayer
} // namespace chip
