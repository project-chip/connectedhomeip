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

#if (!CONFIG_CHIP_K32W0_REAL_FACTORY_DATA || !(defined CONFIG_CHIP_K32W0_REAL_FACTORY_DATA))
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <credentials/examples/ExampleDACs.h>
#include <credentials/examples/ExamplePAI.h>
#endif

#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Base64.h>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>

#include "K32W0FactoryDataProvider.h"
extern "C" {
#include "Flash_Adapter.h"
}

#include <cctype>

namespace chip {
namespace DeviceLayer {

static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
    BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
static constexpr size_t kMaxKeyLen                = 32;

OtaUtils_EEPROM_ReadData pFunctionEepromRead = (OtaUtils_EEPROM_ReadData) K32W0FactoryDataProvider::ReadDataMemcpy;

uint32_t K32W0FactoryDataProvider::kFactoryDataStart        = (uint32_t) __FACTORY_DATA_START;
uint32_t K32W0FactoryDataProvider::kFactoryDataSize         = (uint32_t) __FACTORY_DATA_SIZE;
uint32_t K32W0FactoryDataProvider::kFactoryDataPayloadStart = kFactoryDataStart + sizeof(K32W0FactoryDataProvider::Header);

uint8_t K32W0FactoryDataProvider::ReadDataMemcpy(uint16_t num, uint32_t src, uint8_t * dst)
{
    memcpy(dst, (void *) (src), num);
    return 0;
}

K32W0FactoryDataProvider & K32W0FactoryDataProvider::GetDefaultInstance()
{
    static K32W0FactoryDataProvider sInstance;
    return sInstance;
}

K32W0FactoryDataProvider::K32W0FactoryDataProvider()
{
    maxLengths[FactoryDataId::kVerifierId]           = kSpake2pSerializedVerifier_MaxBase64Len;
    maxLengths[FactoryDataId::kSaltId]               = kSpake2pSalt_MaxBase64Len;
    maxLengths[FactoryDataId::kIcId]                 = sizeof(uint32_t);
    maxLengths[FactoryDataId::kDacPrivateKeyId]      = kMaxKeyLen;
    maxLengths[FactoryDataId::kDacCertificateId]     = Credentials::kMaxDERCertLength;
    maxLengths[FactoryDataId::kPaiCertificateId]     = Credentials::kMaxDERCertLength;
    maxLengths[FactoryDataId::kDiscriminatorId]      = sizeof(uint32_t);
    maxLengths[FactoryDataId::kSetupPasscodeId]      = sizeof(uint32_t);
    maxLengths[FactoryDataId::kVidId]                = sizeof(uint16_t);
    maxLengths[FactoryDataId::kPidId]                = sizeof(uint16_t);
    maxLengths[FactoryDataId::kCertDeclarationId]    = Credentials::kMaxCMSSignedCDMessage;
    maxLengths[FactoryDataId::kVendorNameId]         = ConfigurationManager::kMaxVendorNameLength;
    maxLengths[FactoryDataId::kProductNameId]        = ConfigurationManager::kMaxProductNameLength;
    maxLengths[FactoryDataId::kSerialNumberId]       = ConfigurationManager::kMaxSerialNumberLength;
    maxLengths[FactoryDataId::kManufacturingDateId]  = ConfigurationManager::kMaxManufacturingDateLength;
    maxLengths[FactoryDataId::kHardwareVersionId]    = sizeof(uint16_t);
    maxLengths[FactoryDataId::kHardwareVersionStrId] = ConfigurationManager::kMaxHardwareVersionStringLength;
    maxLengths[FactoryDataId::kUniqueId]             = ConfigurationManager::kMaxUniqueIDLength;
    maxLengths[FactoryDataId::kPartNumber]           = ConfigurationManager::kMaxPartNumberLength;
    maxLengths[FactoryDataId::kProductURL]           = ConfigurationManager::kMaxProductURLLength;
    maxLengths[FactoryDataId::kProductLabel]         = ConfigurationManager::kMaxProductLabelLength;
}

CHIP_ERROR K32W0FactoryDataProvider::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint32_t sum     = 0;

    ReturnErrorOnFailure(SetCustomIds());

    for (uint8_t i = 1; i < K32W0FactoryDataProvider::kNumberOfIds; i++)
    {
        sum += maxLengths[i];
    }

    if (sum > kFactoryDataSize)
    {
        ChipLogError(DeviceLayer, "Max size of factory data: %" PRIu32 " is bigger than reserved factory data size: %" PRIu32, sum,
                     kFactoryDataSize);
    }

    error = Validate();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory data validation failed with error: %s", ErrorStr(error));
        return error;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::Validate()
{
    uint8_t sha256Output[SHA256_HASH_SIZE] = { 0 };

    ReturnErrorOnFailure(Restore());

    auto status = OtaUtils_ReadFromInternalFlash((uint16_t) sizeof(Header), kFactoryDataStart, (uint8_t *) &mHeader, NULL,
                                                 pFunctionEepromRead);
    ReturnErrorCodeIf(gOtaUtilsSuccess_c != status, CHIP_FACTORY_DATA_HEADER_READ);
    ReturnErrorCodeIf(mHeader.hashId != kHashId, CHIP_FACTORY_DATA_HASH_ID);

    SHA256_Hash((uint8_t *) kFactoryDataPayloadStart, mHeader.size, sha256Output);
    ReturnErrorCodeIf(memcmp(sha256Output, mHeader.hash, kHashLen) != 0, CHIP_FACTORY_DATA_SHA_CHECK);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::Restore()
{
    CHIP_ERROR error      = CHIP_NO_ERROR;
    uint16_t backupLength = 0;

    // Check if PDM id related to factory data backup exists.
    // If it does, it means an external event (such as a power loss)
    // interrupted the factory data update process and the section
    // from internal flash is most likely erased and should be restored.
    if (PDM_bDoesDataExist(kNvmId_FactoryDataBackup, &backupLength))
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;
        buffer.Calloc(kFactoryDataSize);
        ReturnErrorCodeIf(buffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

        auto status = PDM_eReadDataFromRecord(kNvmId_FactoryDataBackup, (void *) buffer.Get(), kFactoryDataSize, &backupLength);
        ReturnErrorCodeIf(PDM_E_STATUS_OK != status, CHIP_FACTORY_DATA_PDM_RESTORE);

        error = UpdateData(buffer.Get());
        if (CHIP_NO_ERROR == error)
        {
            PDM_vDeleteDataRecord(kNvmId_FactoryDataBackup);
        }
    }

    // TODO: add hook to enable restore customization

    return error;
}

CHIP_ERROR K32W0FactoryDataProvider::UpdateData(uint8_t * pBuf)
{
    NV_Init();

    auto status = NV_FlashEraseSector(kFactoryDataStart, kFactoryDataSize);
    ReturnErrorCodeIf(status != kStatus_FLASH_Success, CHIP_FACTORY_DATA_FLASH_ERASE);

    Header * header = (Header *) pBuf;
    status          = NV_FlashProgramUnaligned(kFactoryDataStart, sizeof(Header) + header->size, pBuf);
    ReturnErrorCodeIf(status != kStatus_FLASH_Success, CHIP_FACTORY_DATA_FLASH_PROGRAM);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                                 uint32_t * offset)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;
    uint32_t addr  = kFactoryDataPayloadStart;
    uint8_t type   = 0;

    while (addr < (kFactoryDataPayloadStart + mHeader.size))
    {
        if (gOtaUtilsSuccess_c != OtaUtils_ReadFromInternalFlash((uint16_t) sizeof(type), addr, &type, NULL, pFunctionEepromRead) ||
            gOtaUtilsSuccess_c !=
                OtaUtils_ReadFromInternalFlash((uint16_t) sizeof(length), addr + 1, (uint8_t *) &length, NULL, pFunctionEepromRead))
            break;

        if (searchedType == type)
        {
            if ((type >= K32W0FactoryDataProvider::kNumberOfIds) || (length > maxLengths[type]))
            {
                ChipLogError(DeviceLayer, "Failed validity check for factory data with: id=%d, length=%d", type, length);
                break;
            }

            if (bufLength < length)
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            else
            {
                if (gOtaUtilsSuccess_c !=
                    OtaUtils_ReadFromInternalFlash(length, addr + kValueOffset, pBuf, NULL, pFunctionEepromRead))
                    break;

                if (offset)
                    *offset = (addr - kFactoryDataPayloadStart);
                err = CHIP_NO_ERROR;
            }
            break;
        }
        else
        {
            /* Jump past 2 bytes of length and then use length to jump to next data */
            addr = addr + kValueOffset + length;
        }
    }

    return err;
}

CHIP_ERROR K32W0FactoryDataProvider::SetCustomIds()
{
    ChipLogError(DeviceLayer, "SetCustomIds() is not implemented for default FactoryDataProvider");
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    uint16_t declarationSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kCertDeclarationId, outBuffer.data(), outBuffer.size(), declarationSize));
    outBuffer.reduce_size(declarationSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPaiCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    Crypto::P256SerializedKeypair serializedKeypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    /* Get private key of DAC certificate from reserved section */
    uint8_t keyBuf[kMaxKeyLen];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize));
    dacPrivateKeySpan.reduce_size(keySize);

    /* Only the private key is used when signing */
    ReturnErrorOnFailure(serializedKeypair.SetLength(Crypto::kP256_PublicKey_Length + dacPrivateKeySpan.size()));
    memcpy(serializedKeypair.Bytes() + Crypto::kP256_PublicKey_Length, dacPrivateKeySpan.data(), dacPrivateKeySpan.size());

    ReturnErrorOnFailure(keypair.Deserialize(serializedKeypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR K32W0FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t discriminator = 0;
    uint16_t temp          = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDiscriminatorId, (uint8_t *) &discriminator, sizeof(discriminator), temp));
    setupDiscriminator = (uint16_t)(discriminator & 0x0000FFFF);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    uint16_t temp = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kIcId, (uint8_t *) &iterationCount, sizeof(iterationCount), temp));

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    uint16_t saltB64Len                     = 0;

    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSaltId, (uint8_t *) (&saltB64[0]), sizeof(saltB64), saltB64Len));
    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    uint16_t verifierB64Len                                   = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVerifierId, (uint8_t *) &verifierB64[0], sizeof(verifierB64), verifierB64Len));

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSetupPasscodeId, (uint8_t *) &setupPasscode, sizeof(setupPasscode), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVendorNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kVidId, (uint8_t *) &vendorId, sizeof(vendorId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductNameId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductId(uint16_t & productId)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPidId, (uint8_t *) &productId, sizeof(productId), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kPartNumber, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductURL, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kProductLabel, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kSerialNumberId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    uint16_t length = 0;
    uint8_t date[ConfigurationManager::kMaxManufacturingDateLength];

    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kManufacturingDateId, date, ConfigurationManager::kMaxManufacturingDateLength, length));
    date[length] = '\0';

    if (length == 10 && isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) && date[4] == '-' &&
        isdigit(date[5]) && isdigit(date[6]) && date[7] == '-' && isdigit(date[8]) && isdigit(date[9]))
    {
        year  = 1000 * (date[0] - '0') + 100 * (date[1] - '0') + 10 * (date[2] - '0') + date[3] - '0';
        month = 10 * (date[5] - '0') + date[6] - '0';
        day   = 10 * (date[8] - '0') + date[9] - '0';
    }
    else
    {
        ChipLogError(DeviceLayer, "Manufacturing date is not formatted correctly: YYYY-MM-DD.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(
        SearchForId(FactoryDataId::kHardwareVersionId, (uint8_t *) &hardwareVersion, sizeof(hardwareVersion), length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    uint16_t length = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kHardwareVersionStrId, (uint8_t *) buf, bufSize, length));
    buf[length] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    char uniqueId[ConfigurationManager::kMaxUniqueIDLength] = { 0 };
    uint16_t uniqueIdLen                                    = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kUniqueId, (uint8_t *) (&uniqueId[0]), sizeof(uniqueId), uniqueIdLen));
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    ReturnErrorCodeIf(uniqueIdLen > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(uniqueIdLen != ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(uniqueIdSpan.data(), uniqueId, uniqueIdLen);
    uniqueIdSpan.reduce_size(uniqueIdLen);
    return CHIP_NO_ERROR;
#endif

    return err;
}

} // namespace DeviceLayer
} // namespace chip
