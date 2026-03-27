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

#include "FactoryDataProvider.h"
#include "CHIPDevicePlatformConfig.h"
#include "FactoryDataDecoder.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#if CONFIG_FACTORY_DATA
#define FACTORY_DATA_BUFFER_LEN (2560)
#endif

#define FACTORY_TEST 0

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

#if FACTORY_TEST
void buf_dump(const char * title, uint8_t * buf, uint32_t data_len)
{
    const uint32_t bat_num = 8;
    uint32_t times         = data_len / bat_num;
    uint32_t residue       = data_len % bat_num;
    uint8_t * residue_buf  = buf + times * bat_num;

    ChipLogProgress(DeviceLayer, "buf_dump: data_len %d, times %d, residue %d", data_len, times, residue);
    ChipLogProgress(DeviceLayer, "buf_dump: buf is 0x%08x, residue_buf is 0x%08x\r\n", (uint32_t) buf, (uint32_t) residue_buf);

    ChipLogProgress(DeviceLayer, "@@@@@@@@@@@@@@@@@@@@@%s@@@@@@@@@@@@@@@@@@@@@@@@@@@", title);

    for (int32_t i = 0; i < times; i++)
    {
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                        buf[i * bat_num], buf[i * bat_num + 1], buf[i * bat_num + 2], buf[i * bat_num + 3], buf[i * bat_num + 4],
                        buf[i * bat_num + 5], buf[i * bat_num + 6], buf[i * bat_num + 7]);
    }

    switch (residue)
    {
    case 1:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x\r\n", residue_buf[0]);
        break;
    case 2:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1]);
        break;
    case 3:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1], residue_buf[2]);
        break;
    case 4:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1], residue_buf[2],
                        residue_buf[3]);
        break;
    case 5:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1],
                        residue_buf[2], residue_buf[3], residue_buf[4]);
        break;
    case 6:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1],
                        residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5]);
        break;
    case 7:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0],
                        residue_buf[1], residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5], residue_buf[6]);
        break;

    default:
        break;
    }

    ChipLogProgress(DeviceLayer, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}
#endif

namespace {
static FactoryData sFactoryData = { 0 };
} // namespace

const FactoryData * FactoryDataProvider::GetFactoryData()
{
    return &sFactoryData;
}

CHIP_ERROR FactoryDataProvider::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_FACTORY_DATA
    uint8_t * buffer         = (uint8_t *) malloc(FACTORY_DATA_BUFFER_LEN);
    uint16_t factorydata_len = 0x5A5A;

    if (buffer)
    {
        FactoryDataDecoder decoder = FactoryDataDecoder::GetInstance();
        err                        = decoder.ReadFactoryData(buffer, FACTORY_DATA_BUFFER_LEN, &factorydata_len);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData factorydata_len %d!", factorydata_len);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ReadFactoryData failed!");
            free(buffer);
            return err;
        }
        err = decoder.DecodeFactoryData(buffer, &sFactoryData, factorydata_len);
#if FACTORY_TEST
        ChipLogDetail(DeviceLayer, "DecodeFactoryData passcode %d!", sFactoryData.cdata.passcode);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData discriminator %d!", sFactoryData.cdata.discriminator);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData vendor_id %d!", sFactoryData.dii.vendor_id);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData product_id %d!", sFactoryData.dii.product_id);
        buf_dump("DecodeFactoryData cd", sFactoryData.dac.cd.value, sFactoryData.dac.cd.len);
        buf_dump("DecodeFactoryData dac_cert", sFactoryData.dac.dac_cert.value, sFactoryData.dac.dac_cert.len);
        buf_dump("DecodeFactoryData dac_key", sFactoryData.dac.dac_key.value, sFactoryData.dac.dac_key.len);
        buf_dump("DecodeFactoryData pai_cert", sFactoryData.dac.pai_cert.value, sFactoryData.dac.pai_cert.len);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData spake2_it %d", sFactoryData.cdata.spake2_it);
        buf_dump("DecodeFactoryData rd_id_uid", sFactoryData.dii.rd_id_uid.value, sFactoryData.dii.rd_id_uid.len);
        buf_dump("DecodeFactoryData vendor_name", sFactoryData.dii.vendor_name.value, sFactoryData.dii.vendor_name.len);
        buf_dump("DecodeFactoryData product_name", sFactoryData.dii.product_name.value, sFactoryData.dii.product_name.len);
        buf_dump("DecodeFactoryData serial_num", sFactoryData.dii.serial_num.value, sFactoryData.dii.serial_num.len);
#endif
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "DecodeFactoryData failed!");
            free(buffer);
            return err;
        }

        ChipLogProgress(DeviceLayer, "FactoryData of length: %d retrieved successfully", factorydata_len);

        free(buffer);
    }

#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    setupDiscriminator = sFactoryData.cdata.discriminator;
    err                = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    uint32_t val;
    val                = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
    setupDiscriminator = static_cast<uint16_t>(val);
    err                = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(sFactoryData.cdata.spake2_it != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    iterationCount = sFactoryData.cdata.spake2_it;
    err            = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    iterationCount     = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
    err                = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

#if CONFIG_FACTORY_DATA
    saltB64Len = sFactoryData.cdata.spake2_salt.len;
    VerifyOrReturnError(saltB64Len <= sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltB64, sFactoryData.cdata.spake2_salt.value, saltB64Len);
    err = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    saltB64Len         = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
    VerifyOrReturnError(saltB64Len <= sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, saltB64Len);
    err            = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
#endif // CONFIG_FACTORY_DATA

    ReturnErrorOnFailure(err);
    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    VerifyOrReturnError(saltLen <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

#if CONFIG_FACTORY_DATA
    verifierB64Len = sFactoryData.cdata.spake2_verifier.len;
    VerifyOrReturnError(verifierB64Len <= sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierB64, sFactoryData.cdata.spake2_verifier.value, verifierB64Len);
    err = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    verifierB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
    VerifyOrReturnError(verifierB64Len <= sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, verifierB64Len);
    err = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
#endif // CONFIG_FACTORY_DATA

    ReturnErrorOnFailure(err);
    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    VerifyOrReturnError(verifierLen <= verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return err;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(sFactoryData.cdata.passcode != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupPasscode = sFactoryData.cdata.passcode;
    err           = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    // setupPasscode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    // err           = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= sFactoryData.dii.vendor_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sFactoryData.dii.vendor_name.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, sFactoryData.dii.vendor_name.value, sFactoryData.dii.vendor_name.len);
    buf[sFactoryData.dii.vendor_name.len] = 0;
    err                                   = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    err      = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    vendorId = sFactoryData.dii.vendor_id;
    err      = CHIP_NO_ERROR;
#else
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    err      = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA
    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= sFactoryData.dii.product_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sFactoryData.dii.product_name.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, sFactoryData.dii.product_name.value, sFactoryData.dii.product_name.len);
    buf[sFactoryData.dii.product_name.len] = 0;
    err                                    = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    err                 = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    productId = sFactoryData.dii.product_id;
    err       = CHIP_NO_ERROR;
#else
    productId           = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    err                 = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= sFactoryData.dii.serial_num.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sFactoryData.dii.serial_num.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, sFactoryData.dii.serial_num.value, sFactoryData.dii.serial_num.len);
    buf[sFactoryData.dii.serial_num.len] = 0;
    err                                  = CHIP_NO_ERROR;
#else
    size_t serialNumLen = 0; // without counting null-terminator
#ifdef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER[0] != 0)
    {
        VerifyOrReturnError(sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) <= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) - 1;
        err          = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER

    VerifyOrReturnError(serialNumLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(buf[serialNumLen] == 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    err             = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
#if FACTORY_TEST
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    enum
    {
        kDateStringLength = 10 // YYYY-MM-DD
    };
    char * parseEnd;
    size_t dateLen;
    dateLen = sFactoryData.dii.mfg_date.len;
    VerifyOrExit(dateLen == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul((char *) sFactoryData.dii.mfg_date.value, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) sFactoryData.dii.mfg_date.value + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul((char *) sFactoryData.dii.mfg_date.value + 5, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) sFactoryData.dii.mfg_date.value + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul((char *) sFactoryData.dii.mfg_date.value + 8, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) sFactoryData.dii.mfg_date.value + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

    err             = CHIP_NO_ERROR;
#endif
#else
    err             = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_FACTORY_DATA

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND && err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", sFactoryData.dii.mfg_date.value);
    }
    return err;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
#if FACTORY_TEST
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    hardwareVersion = sFactoryData.dii.hw_ver;
    err             = CHIP_NO_ERROR;
#endif
#else
    hardwareVersion = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION);
    err             = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= sFactoryData.dii.hw_ver_string.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sFactoryData.dii.hw_ver_string.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, sFactoryData.dii.hw_ver_string.value, sFactoryData.dii.hw_ver_string.len);
    buf[sFactoryData.dii.hw_ver_string.len] = 0;
    err                                     = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    err = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(uniqueIdSpan.size() >= sFactoryData.dii.rd_id_uid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sFactoryData.dii.rd_id_uid.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(uniqueIdSpan.data(), sFactoryData.dii.rd_id_uid.value, sFactoryData.dii.rd_id_uid.len);
    err = CHIP_NO_ERROR;
#else
    err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");
    constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

    VerifyOrReturnError(sizeof(uniqueId) <= uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sizeof(uniqueId) == ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
    uniqueIdSpan.reduce_size(sizeof(uniqueId));
    err = CHIP_NO_ERROR;
#endif
#endif // CONFIG_FACTORY_DATA

    return err;
}

} // namespace DeviceLayer
} // namespace chip
