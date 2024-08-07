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
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>

#include <platform/CHIPDeviceConfig.h>

#include "FactoryDataProvider.h"
#include <bl_mfd.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(serializedKeypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serializedKeypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serializedKeypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serializedKeypair);
}

CHIP_ERROR FactoryDataProvider::Init()
{
    if (false == mfd_init())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    int len = 0;

    len = mfd_getCd(outBuffer.data(), outBuffer.size());
    if (len > 0)
    {
        outBuffer.reduce_size(len);
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    int len = 0;

    len = mfd_getDacCert(outBuffer.data(), outBuffer.size());
    if (len > 0)
    {
        outBuffer.reduce_size(len);
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    int len = 0;

    len = mfd_getPaiCert(outBuffer.data(), outBuffer.size());
    if (len > 0)
    {
        outBuffer.reduce_size(len);
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    chip::Crypto::P256PublicKey dacPublicKey;

    if (outSignBuffer.size() < signature.Capacity())
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    uint32_t dacCertSize = 0, dacPrivateKeySize = 0;
    uint8_t * pDacCertPtr       = mfd_getDacCertPtr(&dacCertSize);
    uint8_t * pDacPrivateKeyPtr = mfd_getDacPrivateKeyPtr(&dacPrivateKeySize);

    if (NULL == pDacCertPtr || 0 == dacCertSize || NULL == pDacPrivateKeyPtr || 0 == dacPrivateKeySize)
    {
        outSignBuffer.reduce_size(0);
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    ByteSpan dacCert(pDacCertPtr, dacCertSize);
    ByteSpan dacPrivateKey(pDacPrivateKeyPtr, dacPrivateKeySize);

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCert, dacPublicKey));

    ReturnErrorOnFailure(LoadKeypairFromRaw(dacPrivateKey, ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(signature.Bytes(), signature.Length()), outSignBuffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    int len = 0;

    len = mfd_getDiscriminator((uint8_t *) &setupDiscriminator, sizeof(setupDiscriminator));
    if (len > 0)
    {
        setupDiscriminator = 0xfff & setupDiscriminator;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    int len = 0;

    len = mfd_getSapke2It((uint8_t *) &iterationCount, sizeof(iterationCount));
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    int len = 0;

    len = mfd_getSapke2Salt(saltBuf.data(), saltBuf.size());
    if (len > 0)
    {
        saltBuf.reduce_size(len);
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    int len = 0;

    len = mfd_getSapke2Verifier(verifierBuf.data(), verifierBuf.size());
    if (len > 0)
    {
        verifierLen = len;
        verifierBuf.reduce_size(len);
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    int len = 0;

    len = mfd_getPasscode((uint8_t *) &setupPasscode, sizeof(setupPasscode));
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getVendorName(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    int len = 0;

    len = mfd_getVendorId((uint8_t *) &vendorId, sizeof(vendorId));
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getProductName(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    int len = 0;

    len = mfd_getProductId((uint8_t *) &productId, sizeof(productId));
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getPartNumber(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getProductUrl(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getProductLabel(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    int len = 0;

    len = mfd_getSerialNumber(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
#define OS_YEAR                                                                                                                    \
    ((((int) (__DATE__[7] - '0') * 10 + (int) (__DATE__[8] - '0')) * 10 + (int) (__DATE__[9] - '0')) * 10 +                        \
     (int) (__DATE__[10] - '0'))

#define OS_MONTH                                                                                                                   \
    (__DATE__[2] == 'n'       ? (__DATE__[1] == 'a' ? 1 : 6)                                                                       \
         : __DATE__[2] == 'b' ? 2                                                                                                  \
         : __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 3 : 4)                                                                       \
         : __DATE__[2] == 'y' ? 5                                                                                                  \
         : __DATE__[2] == 'l' ? 7                                                                                                  \
         : __DATE__[2] == 'g' ? 8                                                                                                  \
         : __DATE__[2] == 'p' ? 9                                                                                                  \
         : __DATE__[2] == 't' ? 10                                                                                                 \
         : __DATE__[2] == 'v' ? 11                                                                                                 \
                              : 12)

#define OS_DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

    if (mfd_getManufacturingDate(&year, &month, &day))
    {
        return CHIP_NO_ERROR;
    }

    year  = (uint16_t) OS_YEAR;
    month = (uint8_t) OS_MONTH;
    day   = (uint8_t) OS_DAY;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    int len = 0;

    len = mfd_getHardwareVersion((uint8_t *) &hardwareVersion, sizeof(hardwareVersion));
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    int len = 0;
    len     = mfd_getHardwareVersionString(buf, bufSize);
    if (len > 0)
    {
        buf[len] = 0;
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    int len = 0;

    len = mfd_getRotatingDeviceIdUniqueId(uniqueIdSpan.data(), uniqueIdSpan.size());
    if (len > 0)
    {
        return CHIP_NO_ERROR;
    }
    else if (0 == len)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return CHIP_ERROR_BUFFER_TOO_SMALL;
}

} // namespace DeviceLayer
} // namespace chip
