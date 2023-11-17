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

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#define DEVICE_ATTESTATION_PRIVATE_KEY_LEN 32
#define DEVICE_ATTESTATION_PUBLIC_KEY_LEN 65

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t qorvoDacPrivKeyBuffer[DEVICE_ATTESTATION_PRIVATE_KEY_LEN];
    uint8_t qorvoDacPubKeyBuffer[DEVICE_ATTESTATION_PUBLIC_KEY_LEN];
    MutableByteSpan qorvoDacPrivateKey(qorvoDacPrivKeyBuffer, DEVICE_ATTESTATION_PRIVATE_KEY_LEN);
    MutableByteSpan qorvoDacPublicKey(qorvoDacPubKeyBuffer, DEVICE_ATTESTATION_PUBLIC_KEY_LEN);

    qvStatus_t status;
    uint32_t tlvDataLength;
    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY, qorvoDacPrivateKey.data(), qorvoDacPrivateKey.size(),
                                        &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    ReturnErrorOnFailure(MapQorvoError(status));

    VerifyOrReturnError(tlvDataLength <= qorvoDacPrivateKey.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    qorvoDacPrivateKey.reduce_size(tlvDataLength);

    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_PUBLIC_KEY, qorvoDacPublicKey.data(), qorvoDacPublicKey.size(),
                                        &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    ReturnErrorOnFailure(MapQorvoError(status));

    VerifyOrReturnError(tlvDataLength <= qorvoDacPublicKey.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    qorvoDacPublicKey.reduce_size(tlvDataLength);

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    ReturnErrorOnFailure(LoadKeypairFromRaw(qorvoDacPrivateKey, qorvoDacPublicKey, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SETUP_DISCRIMINATOR, (uint8_t *) &setupDiscriminator, sizeof(setupDiscriminator),
                                        &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(setupDiscriminator), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_ITERATION_COUNT, (uint8_t *) &iterationCount, sizeof(iterationCount),
                                        &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(iterationCount), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status =
        qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_SETUP_PASSCODE, (uint8_t *) &setupPasscode, sizeof(setupPasscode), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(setupPasscode), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_VENDOR_ID, (uint8_t *) &vendorId, sizeof(vendorId), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(vendorId), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_ID, (uint8_t *) &productId, sizeof(productId), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(productId), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status =
        qvCHIP_FactoryDataGetValue(TAG_ID_HARDWARE_VERSION, (uint8_t *) &hardwareVersion, sizeof(hardwareVersion), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(hardwareVersion), CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    uint8_t buf[sizeof(year) + sizeof(month) + sizeof(day)];

    status = qvCHIP_FactoryDataGetValue(TAG_ID_MANUFACTURING_DATE, buf, sizeof(buf), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength == sizeof(buf), CHIP_ERROR_INVALID_LIST_LENGTH);

    uint8_t * ptr = buf;
    year          = *(uint16_t *) ptr;
    ptr += 2;
    month = *ptr;
    ptr += 1;
    day = *ptr;

    return CHIP_NO_ERROR;
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

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_VENDOR_NAME, (uint8_t *) buf, bufSize, &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= bufSize, CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_NAME, (uint8_t *) buf, bufSize, &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= bufSize, CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SERIAL_NUMBER, (uint8_t *) buf, bufSize, &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= bufSize, CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_HARDWARE_VERSION_STRING, (uint8_t *) buf, bufSize, &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= bufSize, CHIP_ERROR_INVALID_LIST_LENGTH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierSpan, size_t & verifierLen)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_VERIFIER, verifierSpan.data(), verifierSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(verifierSpan.size() <= tlvDataLength, CHIP_ERROR_INVALID_LIST_LENGTH);

    verifierSpan.reduce_size(tlvDataLength);
    verifierLen = tlvDataLength;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBufferSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status =
        qvCHIP_FactoryDataGetValue(TAG_ID_CERTIFICATION_DECLARATION, outBufferSpan.data(), outBufferSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= outBufferSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    outBufferSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & firmwareInformationSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_FIRMWARE_INFORMATION, firmwareInformationSpan.data(), firmwareInformationSpan.size(),
                                        &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= firmwareInformationSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    firmwareInformationSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & attestationCertSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_DEVICE_ATTESTATION_CERTIFICATE, attestationCertSpan.data(),
                                        attestationCertSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= attestationCertSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    attestationCertSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & intermediateCertSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_PRODUCT_ATTESTATION_INTERMEDIATE_CERTIFICATE, intermediateCertSpan.data(),
                                        intermediateCertSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= intermediateCertSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    intermediateCertSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_SPAKE2_SALT, saltSpan.data(), saltSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= saltSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    saltSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_ROTATING_DEVICE_ID, uniqueIdSpan.data(), uniqueIdSpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= uniqueIdSpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    uniqueIdSpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetEnableKey(MutableByteSpan & enableKeySpan)
{
    qvStatus_t status;
    uint32_t tlvDataLength;

    status = qvCHIP_FactoryDataGetValue(TAG_ID_ENABLE_KEY, enableKeySpan.data(), enableKeySpan.size(), &tlvDataLength);

    VerifyOrReturnError(QV_STATUS_NO_ERROR == status, MapQorvoError(status));
    VerifyOrReturnError(tlvDataLength <= enableKeySpan.size(), CHIP_ERROR_INVALID_LIST_LENGTH);

    enableKeySpan.reduce_size(tlvDataLength);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
