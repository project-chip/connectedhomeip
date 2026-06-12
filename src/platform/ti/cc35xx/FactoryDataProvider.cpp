/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {

typedef struct
{
    uint32_t len;
    uint8_t * data;
} data_ptr;

typedef struct
{
    data_ptr serial_number;
    data_ptr vendor_id;
    data_ptr product_id;
    data_ptr vendor_name;
    data_ptr product_name;
    data_ptr manufacturing_date;
    data_ptr hw_ver;
    data_ptr hw_ver_str;
    data_ptr dac_cert;
    data_ptr dac_priv_key;
    data_ptr pai_cert;
    data_ptr rd_uniqueid;
    data_ptr spake2p_it;
    data_ptr spake2p_salt;
    data_ptr spake2p_verifier;
    data_ptr discriminator;
    data_ptr passcode;
} factoryData;

#if defined(__GNUC__)
factoryData __attribute__((section(".factory_data_struct"))) __attribute__((used)) mFactoryData =
#else

#error "compiler currently not supported"

#endif
    {};

CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR FactoryDataProvider::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & out_buffer)
{
    //-> format_version = 1
    //-> vendor_id = 0xFFF1
    //-> product_id_array = [ 0x8000, 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007, 0x8008, 0x8009, 0x800A, 0x800B,
    // 0x800C, 0x800D, 0x800E, 0x800F, 0x8010, 0x8011, 0x8012, 0x8013, 0x8014, 0x8015, 0x8016, 0x8017, 0x8018, 0x8019, 0x801A,
    // 0x801B, 0x801C, 0x801D, 0x801E, 0x801F, 0x8020, 0x8021, 0x8022, 0x8023, 0x8024, 0x8025, 0x8026, 0x8027, 0x8028, 0x8029,
    // 0x802A, 0x802B, 0x802C, 0x802D, 0x802E, 0x802F, 0x8030, 0x8031, 0x8032, 0x8033, 0x8034, 0x8035, 0x8036, 0x8037, 0x8038,
    // 0x8039, 0x803A, 0x803B, 0x803C, 0x803D, 0x803E, 0x803F, 0x8040, 0x8041, 0x8042, 0x8043, 0x8044, 0x8045, 0x8046, 0x8047,
    // 0x8048, 0x8049, 0x804A, 0x804B, 0x804C, 0x804D, 0x804E, 0x804F, 0x8050, 0x8051, 0x8052, 0x8053, 0x8054, 0x8055, 0x8056,
    // 0x8057, 0x8058, 0x8059, 0x805A, 0x805B, 0x805C, 0x805D, 0x805E, 0x805F, 0x8060, 0x8061, 0x8062, 0x8063 ]
    //-> device_type_id = 0x0016
    //-> certificate_id = "ZIG20142ZB330003-24"
    //-> security_level = 0
    //-> security_information = 0
    //-> version_number = 0x2694
    //-> certification_type = 0
    //-> dac_origin_vendor_id is not present
    //-> dac_origin_product_id is not present
    static const uint8_t kCdForAllExamples[] = {
        0x30, 0x82, 0x02, 0x19, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x82, 0x02, 0x0a, 0x30,
        0x82, 0x02, 0x06, 0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
        0x01, 0x30, 0x82, 0x01, 0x71, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x82, 0x01, 0x62,
        0x04, 0x82, 0x01, 0x5e, 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x05, 0x01, 0x80,
        0x05, 0x02, 0x80, 0x05, 0x03, 0x80, 0x05, 0x04, 0x80, 0x05, 0x05, 0x80, 0x05, 0x06, 0x80, 0x05, 0x07, 0x80, 0x05, 0x08,
        0x80, 0x05, 0x09, 0x80, 0x05, 0x0a, 0x80, 0x05, 0x0b, 0x80, 0x05, 0x0c, 0x80, 0x05, 0x0d, 0x80, 0x05, 0x0e, 0x80, 0x05,
        0x0f, 0x80, 0x05, 0x10, 0x80, 0x05, 0x11, 0x80, 0x05, 0x12, 0x80, 0x05, 0x13, 0x80, 0x05, 0x14, 0x80, 0x05, 0x15, 0x80,
        0x05, 0x16, 0x80, 0x05, 0x17, 0x80, 0x05, 0x18, 0x80, 0x05, 0x19, 0x80, 0x05, 0x1a, 0x80, 0x05, 0x1b, 0x80, 0x05, 0x1c,
        0x80, 0x05, 0x1d, 0x80, 0x05, 0x1e, 0x80, 0x05, 0x1f, 0x80, 0x05, 0x20, 0x80, 0x05, 0x21, 0x80, 0x05, 0x22, 0x80, 0x05,
        0x23, 0x80, 0x05, 0x24, 0x80, 0x05, 0x25, 0x80, 0x05, 0x26, 0x80, 0x05, 0x27, 0x80, 0x05, 0x28, 0x80, 0x05, 0x29, 0x80,
        0x05, 0x2a, 0x80, 0x05, 0x2b, 0x80, 0x05, 0x2c, 0x80, 0x05, 0x2d, 0x80, 0x05, 0x2e, 0x80, 0x05, 0x2f, 0x80, 0x05, 0x30,
        0x80, 0x05, 0x31, 0x80, 0x05, 0x32, 0x80, 0x05, 0x33, 0x80, 0x05, 0x34, 0x80, 0x05, 0x35, 0x80, 0x05, 0x36, 0x80, 0x05,
        0x37, 0x80, 0x05, 0x38, 0x80, 0x05, 0x39, 0x80, 0x05, 0x3a, 0x80, 0x05, 0x3b, 0x80, 0x05, 0x3c, 0x80, 0x05, 0x3d, 0x80,
        0x05, 0x3e, 0x80, 0x05, 0x3f, 0x80, 0x05, 0x40, 0x80, 0x05, 0x41, 0x80, 0x05, 0x42, 0x80, 0x05, 0x43, 0x80, 0x05, 0x44,
        0x80, 0x05, 0x45, 0x80, 0x05, 0x46, 0x80, 0x05, 0x47, 0x80, 0x05, 0x48, 0x80, 0x05, 0x49, 0x80, 0x05, 0x4a, 0x80, 0x05,
        0x4b, 0x80, 0x05, 0x4c, 0x80, 0x05, 0x4d, 0x80, 0x05, 0x4e, 0x80, 0x05, 0x4f, 0x80, 0x05, 0x50, 0x80, 0x05, 0x51, 0x80,
        0x05, 0x52, 0x80, 0x05, 0x53, 0x80, 0x05, 0x54, 0x80, 0x05, 0x55, 0x80, 0x05, 0x56, 0x80, 0x05, 0x57, 0x80, 0x05, 0x58,
        0x80, 0x05, 0x59, 0x80, 0x05, 0x5a, 0x80, 0x05, 0x5b, 0x80, 0x05, 0x5c, 0x80, 0x05, 0x5d, 0x80, 0x05, 0x5e, 0x80, 0x05,
        0x5f, 0x80, 0x05, 0x60, 0x80, 0x05, 0x61, 0x80, 0x05, 0x62, 0x80, 0x05, 0x63, 0x80, 0x18, 0x24, 0x03, 0x16, 0x2c, 0x04,
        0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34,
        0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01,
        0x03, 0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04,
        0xf3, 0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08,
        0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x20, 0x24, 0xe5, 0xd1, 0xf4, 0x7a, 0x7d,
        0x7b, 0x0d, 0x20, 0x6a, 0x26, 0xef, 0x69, 0x9b, 0x7c, 0x97, 0x57, 0xb7, 0x2d, 0x46, 0x90, 0x89, 0xde, 0x31, 0x92, 0xe6,
        0x78, 0xc7, 0x45, 0xe7, 0xf6, 0x0c, 0x02, 0x21, 0x00, 0xf8, 0xaa, 0x2f, 0xa7, 0x11, 0xfc, 0xb7, 0x9b, 0x97, 0xe3, 0x97,
        0xce, 0xda, 0x66, 0x7b, 0xae, 0x46, 0x4e, 0x2b, 0xd3, 0xff, 0xdf, 0xc3, 0xcc, 0xed, 0x7a, 0xa8, 0xca, 0x5f, 0x4c, 0x1a,
        0x7c,
    };

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, out_buffer);
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(outBuffer.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);
    outBuffer.reduce_size(mFactoryData.dac_cert.len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.pai_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(outBuffer.data(), mFactoryData.pai_cert.data, mFactoryData.pai_cert.len);
    outBuffer.reduce_size(mFactoryData.pai_cert.len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract public key from DAC cert.
    uint8_t dacBuf[600] = { 0 };
    MutableByteSpan dacCertSpan(dacBuf);
    ReturnErrorCodeIf(dacCertSpan.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(dacCertSpan.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);
    dacCertSpan.reduce_size(mFactoryData.dac_cert.len);
    chip::Crypto::P256PublicKey dacPublicKey;

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));

    uint8_t privKeyBuf[600] = { 0 };
    MutableByteSpan privKeySpan(privKeyBuf);
    ReturnErrorCodeIf(privKeySpan.size() < mFactoryData.dac_priv_key.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(privKeySpan.data(), mFactoryData.dac_priv_key.data, mFactoryData.dac_priv_key.len);
    privKeySpan.reduce_size(mFactoryData.dac_priv_key.len);

    uint8_t pubKeyBuf[600] = { 0 };
    MutableByteSpan pubKeySpan(pubKeyBuf);
    ReturnErrorCodeIf(pubKeySpan.size() < dacPublicKey.Length(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(pubKeySpan.data(), dacPublicKey.Bytes(), dacPublicKey.Length());
    pubKeySpan.reduce_size(dacPublicKey.Length());

    ReturnErrorOnFailure(LoadKeypairFromRaw(privKeySpan, pubKeySpan, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}
CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    ReturnErrorCodeIf(sizeof(setupDiscriminator) < mFactoryData.discriminator.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.discriminator.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&setupDiscriminator, mFactoryData.discriminator.data, mFactoryData.discriminator.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    ReturnErrorCodeIf(sizeof(iterationCount) < mFactoryData.spake2p_it.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_it.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(&iterationCount, 0, sizeof(iterationCount));
    memcpy(&iterationCount, mFactoryData.spake2p_it.data, mFactoryData.spake2p_it.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    ReturnErrorCodeIf(saltBuf.size() < mFactoryData.spake2p_salt.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_salt.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(saltBuf.data(), mFactoryData.spake2p_salt.data, mFactoryData.spake2p_salt.len);
    saltBuf.reduce_size(mFactoryData.spake2p_salt.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    ReturnErrorCodeIf(verifierBuf.size() < mFactoryData.spake2p_verifier.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_verifier.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(verifierBuf.data(), mFactoryData.spake2p_verifier.data, mFactoryData.spake2p_verifier.len);
    verifierLen = mFactoryData.spake2p_verifier.len;
    verifierBuf.reduce_size(verifierLen);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    ReturnErrorCodeIf(sizeof(setupPasscode) < mFactoryData.passcode.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.passcode.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&setupPasscode, mFactoryData.passcode.data, mFactoryData.passcode.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.vendor_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.vendor_name.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.vendor_name.data, mFactoryData.vendor_name.len);
    buf[mFactoryData.vendor_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    ReturnErrorCodeIf(sizeof(vendorId) < mFactoryData.vendor_id.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.vendor_id.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&vendorId, mFactoryData.vendor_id.data, mFactoryData.vendor_id.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.product_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.product_name.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.product_name.data, mFactoryData.product_name.len);
    buf[mFactoryData.product_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    ReturnErrorCodeIf(sizeof(productId) < mFactoryData.product_id.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.product_id.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&productId, mFactoryData.product_id.data, mFactoryData.product_id.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.serial_number.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.serial_number.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.serial_number.data, mFactoryData.serial_number.len);
    buf[mFactoryData.serial_number.len] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    ReturnErrorCodeIf(!mFactoryData.manufacturing_date.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    uint8_t tmp[10] = { 0 };
    memcpy(tmp, mFactoryData.manufacturing_date.data, 10);
    year  = ((tmp[0] - 0x30) * 1000) + ((tmp[1] - 0x30) * 100) + ((tmp[2] - 0x30) * 10) + (tmp[3] - 0x30);
    month = ((tmp[5] - 0x30) * 10) + (tmp[6] - 0x30);
    day   = ((tmp[8] - 0x30) * 10) + (tmp[9] - 0x30);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    ReturnErrorCodeIf(sizeof(hardwareVersion) < mFactoryData.hw_ver.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.hw_ver.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&hardwareVersion, mFactoryData.hw_ver.data, mFactoryData.hw_ver.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.hw_ver_str.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.hw_ver_str.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.hw_ver_str.data, mFactoryData.hw_ver_str.len);
    buf[mFactoryData.hw_ver_str.len] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ReturnErrorCodeIf(uniqueIdSpan.size() < mFactoryData.rd_uniqueid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.rd_uniqueid.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(uniqueIdSpan.data(), mFactoryData.rd_uniqueid.data, mFactoryData.rd_uniqueid.len);
    uniqueIdSpan.reduce_size(mFactoryData.rd_uniqueid.len);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
