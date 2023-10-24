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

#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
#include <matter_factory_data.h>
#else
extern "C" {
#include <utils_base64.h>
}
#endif

#include "FactoryDataProvider.h"

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
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
    if (false == mfd_init())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else

    static const unsigned char Chip_Test_CD_130D_f001_der[] = {
        0x30, 0x81, 0xe9, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xdb, 0x30, 0x81, 0xd8,
        0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x45,
        0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15, 0x24, 0x00, 0x01, 0x25,
        0x01, 0x0d, 0x13, 0x36, 0x02, 0x05, 0x01, 0xf0, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32,
        0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06,
        0x00, 0x25, 0x07, 0x76, 0x98, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01, 0x03, 0x80, 0x14, 0x62, 0xfa,
        0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0b,
        0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x21, 0x00, 0xd9, 0x91, 0xc9, 0xce, 0xaf, 0x8e, 0x81, 0x56, 0x10, 0x63,
        0x1d, 0x1d, 0x69, 0x3d, 0x0c, 0xbb, 0xb6, 0x17, 0x6e, 0x0b, 0x91, 0xba, 0x7b, 0x23, 0x57, 0xdc, 0x50, 0x80, 0x23, 0xc9,
        0x8e, 0xd0, 0x02, 0x20, 0x34, 0x5d, 0xeb, 0xd7, 0x38, 0xca, 0x8f, 0xfb, 0xa8, 0x4e, 0x8b, 0xe9, 0x5b, 0x66, 0x8b, 0x15,
        0xb9, 0x0a, 0x9a, 0xf2, 0x11, 0x82, 0x0a, 0x7a, 0xf2, 0x9f, 0x3b, 0xdd, 0xd8, 0x15, 0x51, 0x07
    };

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(Chip_Test_CD_130D_f001_der), outBuffer));

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    static const uint8_t Dac_Cert_Array[] = {
        0x30, 0x82, 0x01, 0xd0, 0x30, 0x82, 0x01, 0x76, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x68, 0xa3, 0xb6, 0x6e, 0x27,
        0x6b, 0x5b, 0x29, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x30, 0x31, 0x18, 0x30,
        0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0f, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20,
        0x50, 0x41, 0x49, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c,
        0x04, 0x31, 0x33, 0x30, 0x44, 0x30, 0x20, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x36, 0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34,
        0x33, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30,
        0x48, 0x31, 0x1a, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x11, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54,
        0x65, 0x73, 0x74, 0x20, 0x44, 0x41, 0x43, 0x20, 0x30, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01,
        0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x31, 0x33, 0x30, 0x44, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01,
        0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x02, 0x0c, 0x04, 0x46, 0x30, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04,
        0xd8, 0x19, 0x93, 0xac, 0xf1, 0xc8, 0x63, 0xbb, 0x04, 0x2b, 0x8c, 0x2e, 0x4d, 0xe4, 0x08, 0x39, 0x4f, 0xf9, 0x3e, 0xa3,
        0x89, 0x19, 0x96, 0x8c, 0x22, 0xa1, 0x0f, 0xeb, 0x4c, 0x20, 0x2a, 0x8a, 0x12, 0xff, 0xe4, 0xe6, 0x09, 0x4f, 0x13, 0x4b,
        0xa8, 0x35, 0x53, 0x2f, 0xa4, 0x9d, 0x8e, 0x79, 0x8c, 0x07, 0x01, 0x5c, 0x73, 0xff, 0x0d, 0x1c, 0x34, 0xfe, 0x14, 0x7f,
        0xbe, 0xc6, 0x70, 0xf8, 0xa3, 0x60, 0x30, 0x5e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02,
        0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07, 0x80, 0x30, 0x1d,
        0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x63, 0x9f, 0xbd, 0x91, 0x8c, 0x90, 0xed, 0x7a, 0xbf, 0x64, 0xcd,
        0xe8, 0x2e, 0x13, 0x10, 0x8e, 0xee, 0x5d, 0x6f, 0x8e, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
        0x80, 0x14, 0x3a, 0xb2, 0xfa, 0x06, 0x27, 0xcc, 0x4a, 0xdc, 0x84, 0xac, 0x9c, 0x95, 0x1e, 0x8c, 0x8f, 0xdc, 0x4c, 0x0d,
        0xec, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02,
        0x21, 0x00, 0xd6, 0xc3, 0xab, 0x6d, 0xbd, 0xd0, 0xa2, 0x45, 0xbc, 0xcf, 0xb7, 0x2d, 0x6f, 0x5e, 0xd1, 0xbe, 0xd7, 0xd4,
        0xea, 0xca, 0x9e, 0xf2, 0xa1, 0x09, 0x77, 0xc3, 0x43, 0xa2, 0x29, 0x67, 0x63, 0x3d, 0x02, 0x20, 0x2b, 0xb8, 0x39, 0xf8,
        0x31, 0xeb, 0x83, 0x31, 0xe2, 0x31, 0xeb, 0xd2, 0x5c, 0x82, 0xe4, 0xb7, 0x14, 0xfd, 0x62, 0x97, 0x1f, 0x64, 0xff, 0x51,
        0x43, 0x83, 0xb4, 0xa9, 0x19, 0x6f, 0x96, 0xcf
    };

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(Dac_Cert_Array), outBuffer));

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    static const uint8_t Pai_Cert_Array[] = {
        0x30, 0x82, 0x01, 0xa7, 0x30, 0x82, 0x01, 0x4e, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x01, 0x0f, 0x96, 0xa1, 0x42,
        0x5c, 0xe7, 0x0c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x1a, 0x31, 0x18, 0x30,
        0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0f, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20,
        0x50, 0x41, 0x41, 0x30, 0x20, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x36, 0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34, 0x33, 0x5a,
        0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x30, 0x31,
        0x18, 0x30, 0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0f, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73,
        0x74, 0x20, 0x50, 0x41, 0x49, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02,
        0x01, 0x0c, 0x04, 0x31, 0x33, 0x30, 0x44, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01,
        0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xdb, 0xbe, 0xac, 0xd5, 0xf3, 0xd6,
        0x32, 0x36, 0x04, 0x00, 0xe0, 0xa1, 0x16, 0x8d, 0xbf, 0x0c, 0x74, 0x8f, 0xdb, 0xd7, 0x03, 0x7f, 0x8c, 0x4e, 0x65, 0x58,
        0x27, 0x2f, 0xfa, 0x2d, 0x64, 0x61, 0x4e, 0xe5, 0x72, 0xb8, 0xa5, 0x87, 0x8c, 0x15, 0xc7, 0x25, 0x55, 0x47, 0xc4, 0xc9,
        0x62, 0xda, 0xd7, 0x02, 0x64, 0xd2, 0x84, 0x2d, 0xe8, 0x2d, 0xf1, 0x69, 0xcb, 0x56, 0x42, 0x86, 0xf8, 0x55, 0xa3, 0x66,
        0x30, 0x64, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xff, 0x02,
        0x01, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d,
        0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x3a, 0xb2, 0xfa, 0x06, 0x27, 0xcc, 0x4a, 0xdc, 0x84, 0xac, 0x9c,
        0x95, 0x1e, 0x8c, 0x8f, 0xdc, 0x4c, 0x0d, 0xec, 0x01, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
        0x80, 0x14, 0x78, 0x5c, 0xe7, 0x05, 0xb8, 0x6b, 0x8f, 0x4e, 0x6f, 0xc7, 0x93, 0xaa, 0x60, 0xcb, 0x43, 0xea, 0x69, 0x68,
        0x82, 0xd5, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02,
        0x20, 0x0f, 0x12, 0x2f, 0x8c, 0x83, 0xab, 0xdd, 0x35, 0x53, 0xbc, 0xcd, 0xe5, 0xec, 0x2d, 0x84, 0x99, 0x24, 0xb5, 0xe0,
        0x1a, 0x54, 0x45, 0x37, 0x78, 0xba, 0x88, 0xa8, 0x51, 0x18, 0xf8, 0x0e, 0x05, 0x02, 0x20, 0x59, 0x84, 0x16, 0xe1, 0xd1,
        0xee, 0x5e, 0xc6, 0x50, 0xcd, 0x5d, 0x58, 0x47, 0xb8, 0x10, 0x82, 0xa9, 0xa2, 0x2f, 0x4f, 0x31, 0x7e, 0xc7, 0x5d, 0xb6,
        0x6c, 0xc9, 0xc5, 0x70, 0xb8, 0x1e, 0xd8
    };

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(Pai_Cert_Array), outBuffer));

    return CHIP_NO_ERROR;
#endif
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

#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE

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

#else
    static const uint8_t Dac_PrivateKey_Array[] = { 0x65, 0x11, 0x78, 0x2e, 0xf7, 0xcd, 0x30, 0x59, 0xbd, 0x3a, 0xdd,
                                                    0x44, 0x6f, 0xc5, 0xdd, 0x92, 0xe5, 0xd8, 0x3c, 0x56, 0x9d, 0x67,
                                                    0x7a, 0x29, 0xc7, 0xe9, 0xa7, 0x8f, 0x67, 0xc5, 0x5f, 0xf2 };

    uint8_t dac_cert_array[Credentials::kMaxDERCertLength];
    uint8_t dac_cert_private_key_array[sizeof(Dac_PrivateKey_Array)];
    MutableByteSpan dacCert(dac_cert_array, Credentials::kMaxDERCertLength),
        dacPrivateKey(dac_cert_private_key_array, sizeof(dac_cert_private_key_array));

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(Dac_PrivateKey_Array), dacPrivateKey));
    ReturnErrorOnFailure(GetDeviceAttestationCert(dacCert));

#endif

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCert, dacPublicKey));

    ReturnErrorOnFailure(LoadKeypairFromRaw(dacPrivateKey, ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(signature.Bytes(), signature.Length()), outSignBuffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    setupDiscriminator = 3840;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    iterationCount = 1000;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    static const char spake2pSalt[] = "U1BBS0UyUCBLZXkgU2FsdA==";
    uint32_t aSpake2pSaltLen;

    if (!utils_base64decode((const uint8_t *) spake2pSalt, sizeof(spake2pSalt) - 1, saltBuf.size(), saltBuf.data(),
                            &aSpake2pSaltLen))
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    saltBuf = MutableByteSpan(saltBuf.data(), aSpake2pSaltLen);

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    static const char spake2pVerifier[] = "jdf0KbjmAwujViR8WckvvklJzE0UL+uIOiIjTtb3a5kE/"
                                          "WdbVWhmDFSSjLqFhiiCILxXQ4NVO3YBWTdkERnTlXbFmx+T/32FMRpZLPz8yqFXyALytJW7ZJfArBz0/CP9hA==";
    uint32_t aSpake2pVerifierLen;

    if (!utils_base64decode((const uint8_t *) spake2pVerifier, sizeof(spake2pVerifier) - 1, verifierBuf.size(), verifierBuf.data(),
                            &aSpake2pVerifierLen))
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    verifierBuf = MutableByteSpan(verifierBuf.data(), aSpake2pVerifierLen);
    verifierLen = aSpake2pVerifierLen;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    setupPasscode = 49808401;
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    static const char vendorName[] = "Bouffalo Lab";
    strncpy(buf, vendorName, bufSize);

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    vendorId = 0x130D;
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    strncpy(buf, CHIP_BLE_DEVICE_NAME, bufSize);

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    productId = 0xf001;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
#endif
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
#endif
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
#endif
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    strncpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, bufSize);

    return CHIP_NO_ERROR;
#endif
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

#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
    if (mfd_getManufacturingDate(&year, &month, &day))
    {
        return CHIP_NO_ERROR;
    }
#endif

    year  = (uint16_t) OS_YEAR;
    month = (uint8_t) OS_MONTH;
    day   = (uint8_t) OS_DAY;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    hardwareVersion = CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    strncpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING, bufSize);

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
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
#else
    constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

    VerifyOrReturnValue(uniqueIdSpan.size() >= sizeof(uniqueId), CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
    uniqueIdSpan.reduce_size(sizeof(uniqueId));

    return CHIP_NO_ERROR;
#endif
}

} // namespace DeviceLayer
} // namespace chip
