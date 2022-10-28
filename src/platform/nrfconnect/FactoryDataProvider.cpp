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
#include <crypto/CHIPCryptoPAL.h>

#if CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
#include <credentials/CertificationDeclaration.h>
#include <platform/Zephyr/ZephyrConfig.h>
#endif

#include <logging/log.h>

namespace chip {
namespace {

CHIP_ERROR LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(serializedKeypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serializedKeypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serializedKeypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serializedKeypair);
}

CHIP_ERROR GetFactoryDataString(const FactoryDataString & str, char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < str.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!str.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(buf, str.data, str.len);
    buf[str.len] = 0;

    return CHIP_NO_ERROR;
}

} // namespace

namespace DeviceLayer {

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::Init()
{
    uint8_t * factoryData = nullptr;
    size_t factoryDataSize;

    CHIP_ERROR error = mFlashFactoryData.ProtectFactoryDataPartitionAgainstWrite();

    // Protection against write for external storage is not supported.
    if (error == CHIP_ERROR_NOT_IMPLEMENTED)
    {
        ChipLogProgress(DeviceLayer, "The device does not support hardware protection against write.");
        error = CHIP_NO_ERROR;
    }
    else if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to protect the factory data partition.");
        return error;
    }

    error = mFlashFactoryData.GetFactoryDataPartition(factoryData, factoryDataSize);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to read factory data partition");
        return error;
    }

    if (!ParseFactoryData(factoryData, factoryDataSize, &mFactoryData))
    {
        ChipLogError(DeviceLayer, "Failed to parse factory data");
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    // Check if factory data version is correct
    if (mFactoryData.version != CONFIG_CHIP_FACTORY_DATA_VERSION)
    {
        ChipLogError(DeviceLayer, "Factory data version mismatch. Flash version: %d vs code version: %d", mFactoryData.version,
                     CONFIG_CHIP_FACTORY_DATA_VERSION);
        return CHIP_ERROR_VERSION_MISMATCH;
    }

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
#if CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
    size_t cdLen = 0;

    if (Internal::ZephyrConfig::ReadConfigValueBin(Internal::ZephyrConfig::kConfigKey_CertificationDeclaration,
                                                   reinterpret_cast<uint8_t *>(outBuffer.data()), outBuffer.size(),
                                                   cdLen) == CHIP_NO_ERROR)
    {
        outBuffer.reduce_size(cdLen);
        return CHIP_NO_ERROR;
    }
#endif
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);

    outBuffer.reduce_size(mFactoryData.dac_cert.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.pai_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.pai_cert.data, mFactoryData.pai_cert.len);

    outBuffer.reduce_size(mFactoryData.pai_cert.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SignWithDeviceAttestationKey(const ByteSpan & messageToSign,
                                                                               MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    ReturnErrorCodeIf(!mFactoryData.dac_priv_key.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Extract public key from DAC cert.
    ByteSpan dacCertSpan{ reinterpret_cast<uint8_t *>(mFactoryData.dac_cert.data), mFactoryData.dac_cert.len };
    chip::Crypto::P256PublicKey dacPublicKey;

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));
    ReturnErrorOnFailure(
        LoadKeypairFromRaw(ByteSpan(reinterpret_cast<uint8_t *>(mFactoryData.dac_priv_key.data), mFactoryData.dac_priv_key.len),
                           ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    VerifyOrReturnError(mFactoryData.discriminatorPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupDiscriminator = mFactoryData.discriminator;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    ReturnErrorCodeIf(mFactoryData.spake2_it == 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    iterationCount = mFactoryData.spake2_it;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    ReturnErrorCodeIf(saltBuf.size() < mFactoryData.spake2_salt.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2_salt.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(saltBuf.data(), mFactoryData.spake2_salt.data, mFactoryData.spake2_salt.len);

    saltBuf.reduce_size(mFactoryData.spake2_salt.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    ReturnErrorCodeIf(verifierBuf.size() < mFactoryData.spake2_verifier.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2_verifier.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(verifierBuf.data(), mFactoryData.spake2_verifier.data, mFactoryData.spake2_verifier.len);

    verifierLen = mFactoryData.spake2_verifier.len;

    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSetupPasscode(uint32_t & setupPasscode)
{
    ReturnErrorCodeIf(mFactoryData.passcode == 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupPasscode = mFactoryData.passcode;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetVendorName(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.vendor_name, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetVendorId(uint16_t & vendorId)
{
    VerifyOrReturnError(mFactoryData.vendorIdPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    vendorId = mFactoryData.vendor_id;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductName(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_name, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductId(uint16_t & productId)
{
    VerifyOrReturnError(mFactoryData.productIdPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    productId = mFactoryData.product_id;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetPartNumber(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.part_number, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductURL(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_url, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetProductLabel(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.product_label, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetSerialNumber(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.sn, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    VerifyOrReturnError(mFactoryData.date_year != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    year  = mFactoryData.date_year;
    month = mFactoryData.date_month;
    day   = mFactoryData.date_day;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetHardwareVersion(uint16_t & hardwareVersion)
{
    VerifyOrReturnError(mFactoryData.hwVerPresent, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    hardwareVersion = mFactoryData.hw_ver;
    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return GetFactoryDataString(mFactoryData.hw_ver_str, buf, bufSize);
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ReturnErrorCodeIf(uniqueIdSpan.size() < mFactoryData.rd_uid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.rd_uid.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(uniqueIdSpan.data(), mFactoryData.rd_uid.data, mFactoryData.rd_uid.len);

    return CHIP_NO_ERROR;
}

template <class FlashFactoryData>
CHIP_ERROR FactoryDataProvider<FlashFactoryData>::GetEnableKey(MutableByteSpan & enableKey)
{
    ReturnErrorCodeIf(!mFactoryData.enable_key.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    ReturnErrorCodeIf(enableKey.size() < mFactoryData.enable_key.len, CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(enableKey.data(), mFactoryData.enable_key.data, mFactoryData.enable_key.len);

    enableKey.reduce_size(mFactoryData.enable_key.len);

    return CHIP_NO_ERROR;
}

// Fully instantiate the template class in whatever compilation unit includes this file.
template class FactoryDataProvider<InternalFlashFactoryData>;
template class FactoryDataProvider<ExternalFlashFactoryData>;

} // namespace DeviceLayer
} // namespace chip
