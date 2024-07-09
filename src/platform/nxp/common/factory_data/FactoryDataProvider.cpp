/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright 2023 NXP
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
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/Span.h>

#include "FactoryDataProvider.h"

#include <cctype>

#ifndef FACTORY_DATA_PROVIDER_LOG
#define FACTORY_DATA_PROVIDER_LOG 0
#endif

#if FACTORY_DATA_PROVIDER_LOG
#include "fsl_debug_console.h"
#define FACTORY_DATA_PROVIDER_PRINTF(...)                                                                                          \
    PRINTF("[%s] ", __FUNCTION__);                                                                                                 \
    PRINTF(__VA_ARGS__);                                                                                                           \
    PRINTF("\n\r");
#else
#define FACTORY_DATA_PROVIDER_PRINTF(...)
#endif

using namespace ::chip::Credentials;
using namespace ::chip::Crypto;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryDataProvider::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                            uint32_t * contentAddr)
{
    return static_cast<ImplClass *>(this)->SearchForId(searchedType, pBuf, bufLength, length, contentAddr);
}

CHIP_ERROR FactoryDataProvider::Init(void)
{
    return static_cast<ImplClass *>(this)->Init();
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    uint16_t declarationSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kCertDeclarationId, outBuffer.data(), outBuffer.size(), declarationSize));
    outBuffer.reduce_size(declarationSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPaiCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer)
{
    return static_cast<ImplClass *>(this)->SignWithDacKey(digestToSign, outSignBuffer);
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint16_t discriminatorLen = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDiscriminatorId, (uint8_t *) &setupDiscriminator, sizeof(setupDiscriminator),
                                     discriminatorLen));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    uint16_t temp = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kIcId, (uint8_t *) &iterationCount, sizeof(iterationCount), temp));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
    uint8_t saltB64[kSpake2pSalt_MaxBase64Len];
    uint16_t saltB64Len = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSaltId, &saltB64[0], sizeof(saltB64), saltB64Len));

    size_t saltLen = chip::Base64Decode32((char *) saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
    uint8_t verifierB64[kSpake2pSerializedVerifier_MaxBase64Len];
    uint16_t verifierB64Len = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVerifierId, &verifierB64[0], sizeof(verifierB64), verifierB64Len));

    verifierLen = chip::Base64Decode32((char *) verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSetupPasscodeId, (uint8_t *) &setupPasscode, sizeof(setupPasscode), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVendorNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVidId, (uint8_t *) &vendorId, sizeof(vendorId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPidId, (uint8_t *) &productId, sizeof(productId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPartNumber, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductURL, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductLabel, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSerialNumberId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    uint16_t length = 0;
    uint8_t date[ConfigurationManager::kMaxManufacturingDateLength];

    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kManufacturingDateId, date, ConfigurationManager::kMaxManufacturingDateLength, length));
    date[length] = '\0';

    if (length == 10 && isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) && date[4] == '-' &&
        isdigit(date[5]) && isdigit(date[6]) && date[7] == '-' && isdigit(date[8]) && isdigit(date[9]))
    {
        year  = (uint16_t) (1000 * (date[0] - '0') + 100 * (date[1] - '0') + 10 * (date[2] - '0') + date[3] - '0');
        month = (uint8_t) (10 * (date[5] - '0') + date[6] - '0');
        day   = (uint8_t) (10 * (date[8] - '0') + date[9] - '0');
    }
    else
    {
        ChipLogError(DeviceLayer, "Manufacturing date is not formatted correctly: YYYY-MM-DD.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kHardwareVersionId, (uint8_t *) &hardwareVersion, sizeof(hardwareVersion), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kHardwareVersionStrId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");
    uint16_t uniqueIdLen = 0;
    err                  = SearchForId(FactoryDataId::kUniqueId, (uint8_t *) uniqueIdSpan.data(), uniqueIdSpan.size(), uniqueIdLen);
#if defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (err != CHIP_NO_ERROR)
    {
        constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

        ReturnErrorCodeIf(sizeof(uniqueId) > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
        uniqueIdLen = sizeof(uniqueId);
        err         = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);
#endif

    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
{
    uint8_t productFinish;
    uint16_t length = 0;
    auto err        = SearchForId(FactoryDataId::kProductFinish, &productFinish, sizeof(productFinish), length);
    ReturnErrorCodeIf(err != CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(productFinish);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    uint8_t color;
    uint16_t length = 0;
    auto err        = SearchForId(FactoryDataId::kProductPrimaryColor, &color, sizeof(color), length);
    ReturnErrorCodeIf(err != CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *primaryColor = static_cast<app::Clusters::BasicInformation::ColorEnum>(color);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
