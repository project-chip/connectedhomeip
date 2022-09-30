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
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

#include "qvCHIP.h"

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

} // namespace

namespace DeviceLayer {

CHIP_ERROR FactoryDataProvider::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::MapQorvoError(qvStatus_t qStatus)
{
    switch (qStatus)
    {
    case QV_STATUS_NO_ERROR:
        return CHIP_NO_ERROR;
    case QV_STATUS_BUFFER_TOO_SMALL:
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case QV_STATUS_INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case QV_STATUS_INVALID_DATA:
        return CHIP_ERROR_INVALID_DATA_LIST;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_CERTIFICATION_DECLARATION, outBuffer.data(), outBuffer.size(), &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= CERTIFICATION_DECLARATION_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        outBuffer.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_CERTIFICATE, outBuffer.data(), outBuffer.size(), &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= DEVICE_ATTESTATION_CERTIFICATE_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        outBuffer.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_ATTESTATION_INTERMEDIATE_CERTIFICATE, outBuffer.data(), outBuffer.size(),
                                        &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= PRODUCT_ATTESTATION_INTERMEDIATE_CERTIFICATE_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        outBuffer.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t qorvoDacPrivKeyBuffer[DEVICE_ATTESTATION_PRIVATE_KEY_LEN];
    uint8_t qorvoDacPubKeyBuffer[DEVICE_ATTESTATION_PUBLIC_KEY_LEN];
    MutableByteSpan qorvoDacPrivateKey(qorvoDacPrivKeyBuffer, DEVICE_ATTESTATION_PRIVATE_KEY_LEN);
    MutableByteSpan qorvoDacPublicKey(qorvoDacPubKeyBuffer, DEVICE_ATTESTATION_PUBLIC_KEY_LEN);
    qvStatus_t status;
    uint32_t dataLength;
    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY, qorvoDacPrivateKey.data(), qorvoDacPrivateKey.size(),
                                        &dataLength);
    ;
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= DEVICE_ATTESTATION_PRIVATE_KEY_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        qorvoDacPrivateKey.reduce_size(dataLength);
    }
    ReturnErrorOnFailure(MapQorvoError(status));

    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_PUBLIC_KEY, qorvoDacPublicKey.data(), qorvoDacPublicKey.size(),
                                        &dataLength);
    ;
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= DEVICE_ATTESTATION_PUBLIC_KEY_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        qorvoDacPublicKey.reduce_size(dataLength);
    }
    ReturnErrorOnFailure(MapQorvoError(status));

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    ReturnErrorOnFailure(LoadKeypairFromRaw(qorvoDacPrivateKey, qorvoDacPublicKey, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SETUP_DISCRIMINATOR, (uint8_t *) &setupDiscriminator, SETUP_DISCRIMINATOR_LEN,
                                        &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == SETUP_DISCRIMINATOR_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_ITERATION_COUNT, (uint8_t *) &iterationCount, SPAKE2_ITERATION_COUNT_LEN,
                                        &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == SPAKE2_ITERATION_COUNT_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_SALT, saltBuf.data(), saltBuf.size(), &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= SPAKE2_SALT_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        saltBuf.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    qvStatus_t status;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_VERIFIER, verifierBuf.data(), verifierBuf.size(), (uint32_t *) &verifierLen);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(verifierLen <= SPAKE2_VERIFIER_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        verifierBuf.reduce_size(verifierLen);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_SETUP_PASSCODE, (uint8_t *) &setupPasscode, SETUP_PASSCODE_LEN, &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == SETUP_PASSCODE_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    return MapQorvoError(qvCHIP_FactoryDataGetValue(TAG_ID_VENDOR_NAME, (uint8_t *) buf, bufSize, NULL));
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_VENDOR_ID, (uint8_t *) &vendorId, VENDOR_ID_LEN, &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == VENDOR_ID_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    return MapQorvoError(qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_NAME, (uint8_t *) buf, bufSize, NULL));
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_ID, (uint8_t *) &productId, PRODUCT_ID_LEN, &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == PRODUCT_ID_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    return MapQorvoError(qvCHIP_FactoryDataGetValue(TAG_ID_SERIAL_NUMBER, (uint8_t *) buf, bufSize, NULL));
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    uint8_t buf[MANUFACTURING_DATE_LEN];
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_MANUFACTURING_DATE, buf, MANUFACTURING_DATE_LEN, &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == MANUFACTURING_DATE_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        uint8_t * ptr = buf;
        year          = *(uint16_t *) ptr;
        ptr += 2;
        month = *ptr;
        ptr += 1;
        day = *ptr;
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_HARDWARE_VERSION, (uint8_t *) &hardwareVersion, HARDWARE_VERSION_LEN, &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength == HARDWARE_VERSION_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return MapQorvoError(qvCHIP_FactoryDataGetValue(TAG_ID_HARDWARE_VERSION_STRING, (uint8_t *) buf, bufSize, NULL));
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_ROTATING_DEVICE_ID, uniqueIdSpan.data(), uniqueIdSpan.size(), &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= ROTATING_DEVICE_ID_UNIQUE_ID_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        uniqueIdSpan.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

CHIP_ERROR FactoryDataProvider::GetEnableKey(MutableByteSpan & enableKey)
{
    qvStatus_t status;
    uint32_t dataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_ENABLE_KEY, enableKey.data(), enableKey.size(), &dataLength);
    if (status == QV_STATUS_NO_ERROR)
    {
        VerifyOrReturnError(dataLength <= ENABLE_KEY_LEN, CHIP_ERROR_INVALID_LIST_LENGTH);
        enableKey.reduce_size(dataLength);
    }

    return MapQorvoError(status);
}

} // namespace DeviceLayer
} // namespace chip
