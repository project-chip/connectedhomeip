/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "CGSecureDACVendorProvider.h"
#include "FactoryDataProvider.h"
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
#include <platform/realtek/freertos/CHIPDevicePlatformConfig.h>

#include "CGCrypto.h"

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CGSecureDACVendorProvider::CGSecureDACVendorProvider()
{
    int ret = InitModule(0);
    if (ret) {
        ChipLogDetail(DeviceLayer, "Init Module failed %d", ret);
        this->initError = CHIP_ERROR_UNINITIALIZED;
    }

    this->initError = CHIP_NO_ERROR;
}

CGSecureDACVendorProvider::~CGSecureDACVendorProvider()
{
    int ret = ReleaseModule();
    if (ret != 0) {
        ChipLogDetail(DeviceLayer, "Release Module failed %d", ret);
    }
}

CHIP_ERROR CGSecureDACVendorProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

#if CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;
    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
#else
    const uint8_t kCdForAllExamples[] = {
            0x30, 0x81, 0xe8, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
            0x07, 0x02, 0xa0, 0x81, 0xda, 0x30, 0x81, 0xd7, 0x02, 0x01, 0x03, 0x31,
            0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,
            0x02, 0x01, 0x30, 0x44, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
            0x01, 0x07, 0x01, 0xa0, 0x37, 0x04, 0x35, 0x15, 0x24, 0x00, 0x01, 0x25,
            0x01, 0x16, 0x13, 0x36, 0x02, 0x05, 0x25, 0x1a, 0x18, 0x24, 0x03, 0x16,
            0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a,
            0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34, 0x24, 0x05,
            0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x01, 0x18,
            0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01, 0x03, 0x80, 0x14, 0x62, 0xfa, 0x82,
            0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd,
            0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48,
            0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86,
            0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x20,
            0x43, 0xa9, 0x95, 0xed, 0xbc, 0xaa, 0x0a, 0x4c, 0xaa, 0xa2, 0xb9, 0xaf,
            0xcc, 0x53, 0x93, 0xda, 0xef, 0xd3, 0x42, 0x45, 0x3d, 0xd9, 0x36, 0x4c,
            0x82, 0x67, 0xbf, 0xe6, 0x9f, 0xc8, 0xec, 0xb7, 0x02, 0x21, 0x00, 0xcc,
            0xc1, 0x07, 0xa5, 0x8f, 0xa6, 0x8a, 0x00, 0x6e, 0xf6, 0xdf, 0xff, 0x0a,
            0x10, 0x26, 0x02, 0x9f, 0x4b, 0x71, 0x1b, 0x53, 0x79, 0xc6, 0x29, 0x44,
            0x87, 0xe4, 0x9a, 0xd9, 0x71, 0x11, 0x81
        };

    return CopySpanToMutableSpan(ByteSpan(kCdForAllExamples), outBuffer);
#endif
}

CHIP_ERROR CGSecureDACVendorProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CGSecureDACVendorProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    int ret = 0;
    uint16_t dac_len = 0;

    // Step 1: Probe for certificate length
    ret = GetCert(CGCRYPTO_OBJECT_ID_DEVICE_CERT, nullptr, &dac_len);
    if (ret != CG_RTN_BUFFER_TOO_SMALL)
    {
        ChipLogDetail(DeviceLayer, "GetDeviceAttestationCert: failed to get DAC length, err=%d", ret);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    // Step 2: Check if output buffer is large enough
    VerifyOrReturnError(outBuffer.size() >= dac_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Step 3: Actually retrieve the certificate
    ret = GetCert(CGCRYPTO_OBJECT_ID_DEVICE_CERT, outBuffer.data(), &dac_len);
    if (ret != 0)
    {
        ChipLogDetail(DeviceLayer, "GetDeviceAttestationCert: failed to get DAC, err=%d", ret);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    outBuffer.reduce_size(dac_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CGSecureDACVendorProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    uint16_t pai_len = 0;
    int ret;

    // Step 1: Probe for certificate length
    ret = GetCert(CGCRYPTO_OBJECT_ID_TRUST_ANCHOR, nullptr, &pai_len);
    if (ret != CG_RTN_BUFFER_TOO_SMALL)
    {
        ChipLogDetail(DeviceLayer, "GetProductAttestationIntermediateCert: failed to get PAI length, err=%d", ret);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    // Step 2: Check if output buffer is large enough
    VerifyOrReturnError(outBuffer.size() >= pai_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Step 3: Retrieve the real certificate data
    ret = GetCert(CGCRYPTO_OBJECT_ID_TRUST_ANCHOR, outBuffer.data(), &pai_len);
    if (ret != 0)
    {
        ChipLogDetail(DeviceLayer, "GetProductAttestationIntermediateCert: failed to get PAI, err=%d", ret);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    outBuffer.reduce_size(pai_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CGSecureDACVendorProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    // Step 1: Validate input spans and signature buffer size
    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Step 2: Sign the data
    uint8_t asn1_signature_buffer[Crypto::kMax_ECDSA_Signature_Length_Der];
    MutableByteSpan asn1_signature_span(asn1_signature_buffer);
    uint16_t signature_length = asn1_signature_span.size();
    int ret = SignData(CGCRYPTO_KEY_ID_DEVICE_KEY, CGCRYPTO_HASH_TYPE_SHA_256,
                       messageToSign.data(), messageToSign.size(),
                       asn1_signature_span.data(), &signature_length);
    if (ret != 0)
    {
        ChipLogDetail(DeviceLayer, "SignWithDeviceAttestationKey: SignData failed, err=%d", ret);
        return CHIP_ERROR_INTERNAL;
    }
    asn1_signature_span.reduce_size(signature_length);

    // Step 3: Convert ASN.1 signature to raw format
    CHIP_ERROR err = Crypto::EcdsaAsn1SignatureToRaw(Crypto::kP256_FE_Length, asn1_signature_span, outSignBuffer);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(DeviceLayer, "SignWithDeviceAttestationKey: EcdsaAsn1SignatureToRaw failed, err=%" CHIP_ERROR_FORMAT, err);
        return err;
    }

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
