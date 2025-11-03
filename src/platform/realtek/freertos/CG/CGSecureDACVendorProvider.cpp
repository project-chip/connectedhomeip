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

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CGSecureDACVendorProvider::CGSecureDACVendorProvider()
{
    cg_matter_data param{};
    ChipLogDetail(DeviceLayer, "secure_app_function_call InitModule start");
    secure_app_function_call(SECURE_APP_FUNCTION_INIT_MODULE, &param);

    if (param.status_code)
    {
        ChipLogError(DeviceLayer, "secure_app_function_call InitModule failed %d", param.status_code);
        this->initError = CHIP_ERROR_UNINITIALIZED;
    }
    else
    {
        this->initError = CHIP_NO_ERROR;
    }
}

CGSecureDACVendorProvider::~CGSecureDACVendorProvider()
{
    cg_matter_data param{};
    // Release Module
    ChipLogDetail(DeviceLayer, "secure_app_function_call ReleaseModule start");
    secure_app_function_call(SECURE_APP_FUNCTION_RELEASE_MODULE, &param);
    ChipLogDetail(DeviceLayer, "secure_app_function_call ReleaseModule end %d", param.status_code);
}

CHIP_ERROR CGSecureDACVendorProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;
    err                                   = CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
#else
    cg_matter_data param{};
    param.return_length = sizeof(param.return_data);

    /* Certification Declaration */
    ChipLogDetail(DeviceLayer, "secure_app_function_call GetCertificationDeclaration start");
    secure_app_function_call(SECURE_APP_FUNCTION_GET_CD, &param);
    if (param.status_code)
    {
        ChipLogError(DeviceLayer, "secure_app_function_call GetCertificationDeclaration failed %d, %d", param.status_code,
                     (int) param.return_length);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    ChipLogDetail(DeviceLayer, "secure_app_function_call GetCertificationDeclaration size: %d", (int) param.return_length);
    err = CopySpanToMutableSpan(ByteSpan(param.return_data, param.return_length), outBuffer);
#endif

    return err;
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

    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

    cg_matter_data param{};
    param.return_length = sizeof(param.return_data);
    /* DeviceAttestationCert */
    ChipLogDetail(DeviceLayer, "secure_app_function_call Get DeviceAttestationCert start");
    secure_app_function_call(SECURE_APP_FUNCTION_GET_DAC, &param);
    if (param.status_code)
    {
        ChipLogError(DeviceLayer, "secure_app_function_call GetDeviceAttestationCert failed %d, %d", param.status_code,
                     (int) param.return_length);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    ChipLogDetail(DeviceLayer, "secure_app_function_call Get DeviceAttestationCert size: %d", (int) param.return_length);
    err = CopySpanToMutableSpan(ByteSpan(param.return_data, param.return_length), outBuffer);

    return err;
}

CHIP_ERROR CGSecureDACVendorProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    cg_matter_data param{};
    param.return_length = sizeof(param.return_data);
    /* Product Attestation Intermediate Cert */
    ChipLogDetail(DeviceLayer, "secure_app_function_call Get Product Attestation Intermediate Cert start");
    secure_app_function_call(SECURE_APP_FUNCTION_GET_PAI_CERT, &param);
    if (param.status_code)
    {
        ChipLogError(DeviceLayer, "secure_app_function_call Get Product Attestation Intermediate Cert failed %d ,%d",
                     param.status_code, (int) param.return_length);
        return CHIP_ERROR_CERT_NOT_FOUND;
    }
    ChipLogDetail(DeviceLayer, "secure_app_function_call Get Product Attestation Intermediate Cert size: %d",
                  (int) param.return_length);
    err = CopySpanToMutableSpan(ByteSpan(param.return_data, param.return_length), outBuffer);

    return err;
}

CHIP_ERROR CGSecureDACVendorProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    if (!IsInitSuccess())
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    Crypto::P256ECDSASignature signature;

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    cg_matter_data param{};
    param.return_length = sizeof(param.return_data);
    /* Sign With Device Attestation Key */
    ChipLogDetail(DeviceLayer, "secure_app_function_call Sign start");
    VerifyOrReturnError(messageToSign.size() <= sizeof(param.input_data), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(param.input_data, messageToSign.data(), messageToSign.size());
    param.input_length = messageToSign.size();
    secure_app_function_call(SECURE_APP_FUNCTION_SIGN_WITH_DAKEY, &param);
    if (param.status_code)
    {
        ChipLogError(DeviceLayer, "secure_app_function_call Sign failed %d", param.status_code);
        return CHIP_ERROR_INVALID_SIGNATURE;
    }
    ChipLogDetail(DeviceLayer, "secure_app_function_call signature size: %d", (int) param.return_length);
    err = Crypto::EcdsaAsn1SignatureToRaw(Crypto::kP256_FE_Length, ByteSpan(param.return_data, param.return_length), outSignBuffer);
    return err;
}

} // namespace DeviceLayer
} // namespace chip
