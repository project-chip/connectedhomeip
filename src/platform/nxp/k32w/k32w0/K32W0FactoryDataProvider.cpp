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

#include "OtaUtils.h"
#include "fsl_flash.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/Base64.h>
#include <lib/support/Span.h>

#include "K32W0FactoryDataProvider.h"

/* Grab symbol for the base address from the linker file. */
extern uint32_t __FACTORY_DATA_START[];
extern uint32_t __FACTORY_DATA_SIZE[];

namespace chip {
namespace {

CHIP_ERROR LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serialized_keypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serialized_keypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serialized_keypair);
}
} // namespace

namespace DeviceLayer {

K32W0FactoryDataProvider & K32W0FactoryDataProvider::GetDefaultInstance()
{
    static K32W0FactoryDataProvider sInstance;
    return sInstance;
}

static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
    BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;
static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
static constexpr size_t kMaxCertLen               = 600;
static constexpr size_t kMaxKeyLen                = 32;

static constexpr size_t kVerifierId       = 1;
static constexpr size_t kSaltId           = 2;
static constexpr size_t kIcId             = 3;
static constexpr size_t kDacPrivateKeyId  = 4;
static constexpr size_t kDacCertificateId = 5;
static constexpr size_t kPaiCertificateId = 6;
static constexpr size_t kDiscriminatorId  = 7;

static constexpr size_t kMaxId = kDiscriminatorId;

static uint16_t maxLengths[kMaxId + 1];
static uint32_t factoryDataActualSize = 0;

typedef otaUtilsResult_t (*OtaUtils_EEPROM_ReadData)(uint16_t nbBytes, uint32_t address, uint8_t * pInbuf);

static uint8_t ReadDataMemCpy(uint16_t num, uint32_t src, uint8_t * dst)
{
    memcpy(dst, (void *) (src), num);
    return 0;
}

CHIP_ERROR SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length)
{
    CHIP_ERROR err                               = CHIP_ERROR_NOT_FOUND;
    uint32_t factoryDataStartAddress             = (uint32_t) __FACTORY_DATA_START;
    uint32_t addr                                = factoryDataStartAddress;
    OtaUtils_EEPROM_ReadData pFunctionEepromRead = (OtaUtils_EEPROM_ReadData) ReadDataMemCpy;
    uint8_t type                                 = 0;

    while (addr < (factoryDataStartAddress + (uint32_t) __FACTORY_DATA_SIZE))
    {
        if (gOtaUtilsSuccess_c != OtaUtils_ReadFromInternalFlash((uint16_t) sizeof(type), addr, &type, NULL, pFunctionEepromRead) ||
            gOtaUtilsSuccess_c !=
                OtaUtils_ReadFromInternalFlash((uint16_t) sizeof(length), addr + 1, (uint8_t *) &length, NULL, pFunctionEepromRead))
            break;

        if ((type > kMaxId) || (length > maxLengths[type]))
        {
            break;
        }

        if (searchedType == type)
        {
            if (bufLength < length)
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            else
            {
                if (gOtaUtilsSuccess_c != OtaUtils_ReadFromInternalFlash(length, addr + 3, pBuf, NULL, pFunctionEepromRead))
                    break;

                err = CHIP_NO_ERROR;
            }
            break;
        }
        else
        {
            /* Jump past 2 bytes of length and then use length to jump to next data */
            addr = addr + 3 + length;
        }
    }

    return err;
}

CHIP_ERROR K32W0FactoryDataProvider::Init()
{

    maxLengths[kVerifierId]       = kSpake2pSerializedVerifier_MaxBase64Len;
    maxLengths[kSaltId]           = kSpake2pSalt_MaxBase64Len;
    maxLengths[kIcId]             = 4;
    maxLengths[kDacPrivateKeyId]  = kMaxKeyLen;
    maxLengths[kDacCertificateId] = kMaxCertLen;
    maxLengths[kPaiCertificateId] = kMaxCertLen;
    maxLengths[kDiscriminatorId]  = 4;

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
}

CHIP_ERROR K32W0FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(kDacCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    uint16_t certificateSize = 0;
    ReturnErrorOnFailure(SearchForId(kPaiCertificateId, outBuffer.data(), outBuffer.size(), certificateSize));
    outBuffer.reduce_size(certificateSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    Crypto::P256PublicKey dacPublicKey;
    uint8_t certBuf[kMaxCertLen];
    MutableByteSpan dacCertSpan(certBuf);
    uint16_t certificateSize = 0;
    CHIP_ERROR err           = CHIP_NO_ERROR;

    err = SearchForId(kDacCertificateId, dacCertSpan.data(), dacCertSpan.size(), certificateSize);
    ReturnErrorOnFailure(err);
    dacCertSpan.reduce_size(certificateSize);
    /* Extract Public Key of DAC certificate from itself */
    err = Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey);
    ReturnErrorOnFailure(err);

    /* Get private key of DAC certificate from reserved section */
    uint8_t keyBuf[kMaxKeyLen];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    ReturnErrorOnFailure(SearchForId(kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize));
    dacPrivateKeySpan.reduce_size(keySize);

    ReturnErrorOnFailure(LoadKeypairFromRaw(ByteSpan(dacPrivateKeySpan.data(), dacPrivateKeySpan.size()),
                                            ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR K32W0FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t discriminator = 0;
    uint16_t temp          = 0;

    ReturnErrorOnFailure(SearchForId(kDiscriminatorId, (uint8_t *) &discriminator, sizeof(discriminator), temp));
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

    return SearchForId(kIcId, (uint8_t *) &iterationCount, sizeof(iterationCount), temp);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    uint16_t saltB64Len                     = 0;

    ReturnErrorOnFailure(SearchForId(kSaltId, (uint8_t *) (&saltB64[0]), sizeof(saltB64), saltB64Len));
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
    ReturnErrorOnFailure(SearchForId(kVerifierId, (uint8_t *) &verifierB64[0], sizeof(verifierB64), verifierB64Len));

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
CHIP_ERROR K32W0FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetProductId(uint16_t & productId)
{
    productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    return CHIP_NO_ERROR;
}

CHIP_ERROR K32W0FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR K32W0FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
#endif

    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER */
} // namespace DeviceLayer
} // namespace chip
