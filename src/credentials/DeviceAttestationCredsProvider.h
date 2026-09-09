/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <cstddef>
#include <cstdint>

#include <clusters/OperationalCredentials/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

// Keep the provider's source-level names while sharing the generated enum definitions.
// This changes ABI: profile bitmaps now use uint16_t, and document types use the
// generated certificate values (DAC = 1, PAI = 2) instead of the former 0/1 values.
using DeviceAttestationCertProfile       = app::Clusters::OperationalCredentials::AttestationCryptoProfileEnum;
using DeviceAttestationCertProfileBitmap = app::Clusters::OperationalCredentials::AttestationCryptoProfileBitmap;
using DeviceAttestationDocumentType      = app::Clusters::OperationalCredentials::CertificateChainTypeEnum;

// Each bitmap describes the corresponding certificate's subject public key capabilities,
// not its issuer's signature algorithm or the profile of the chain containing it.
// Phase 1 DAC keys (and device attestation signatures) remain ECDSA-P256.
struct DeviceAttestationProfileSupport
{
    BitMask<DeviceAttestationCertProfileBitmap> paaSupportedProfiles;
    BitMask<DeviceAttestationCertProfileBitmap> paiSupportedProfiles;
    BitMask<DeviceAttestationCertProfileBitmap> dacSupportedProfiles;
};

class DeviceAttestationCredentialsProvider
{
public:
    DeviceAttestationCredentialsProvider()          = default;
    virtual ~DeviceAttestationCredentialsProvider() = default;

    // Not copyable
    DeviceAttestationCredentialsProvider(const DeviceAttestationCredentialsProvider &)             = delete;
    DeviceAttestationCredentialsProvider & operator=(const DeviceAttestationCredentialsProvider &) = delete;

    /**
     * @brief Get the Certification Declaration body. Updates `out_cd_buffer`'s size on success
     *        to match the data size. If no Certification Declaration is available, sets
     *        `out_cd_buffer` to empty.
     *
     * @param[in,out] out_cd_buffer Buffer to receive the Certification Declaration body.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_cd_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) = 0;

    /**
     * @brief Get the Firmware Information body. Updates `out_firmware_info_buffer`'s size
     *        on success to match the data size. If no Firmware Information is available,
     *        sets `out_firmware_info_buffer` to empty.
     *
     * @param[in,out] out_firmware_info_buffer Buffer to receive the Firmware Information body.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_firmware_info_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if access fails.
     */
    virtual CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) = 0;

    /**
     * @brief Get the Device Attestation Certificate in DER format. Updates `out_dac_buffer`'s
     *        size on success to match the data size. If no Device Attestation Certificate
     *        is available, sets `out_dac_buffer` to empty.
     *
     * @param[in,out] out_dac_buffer Buffer to receive the Device Attestation Certificate.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_dac_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if
     *          access fails.
     */
    virtual CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) = 0;

    /**
     * @brief Get the Device Attestation Certificate in DER format for a given stored chain profile.
     *
     * The default implementation serves the legacy Matter profile through
     * GetDeviceAttestationCert() and reports unsupported profiles as not implemented.
     */
    virtual CHIP_ERROR GetDeviceAttestationCertForProfile(DeviceAttestationCertProfile profile, MutableByteSpan & out_dac_buffer);

    /**
     * @brief Get the PAI Certificate in DER format. Updates `out_pai_buffer`'s
     *        size on success to match the data size. If no PAI certificate
     *        is available, sets `out_pai_buffer` to empty.
     *
     * @param[in,out] out_pai_buffer Buffer to receive the PAI certificate.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_pai_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if
     *          access fails.
     */
    virtual CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) = 0;

    /**
     * @brief Get the PAI Certificate in DER format for a given stored chain profile.
     *
     * The default implementation serves the legacy Matter profile through
     * GetProductAttestationIntermediateCert() and reports unsupported profiles as not implemented.
     */
    virtual CHIP_ERROR GetProductAttestationIntermediateCertForProfile(DeviceAttestationCertProfile profile,
                                                                       MutableByteSpan & out_pai_buffer);

    /**
     * @brief Report which device attestation profiles are supported by this provider.
     *
     * The default implementation reports legacy Matter support for every attestation chain element.
     */
    virtual DeviceAttestationProfileSupport GetDeviceAttestationProfileSupport() const;

    /**
     * Select the strongest complete stored attestation chain for profile-aware requests.
     * This is a storage selector, independent of each certificate's public key profile.
     * Both PAI and DAC reads, including all segments, must use this same chain.
     * Providers must keep the selection stable while commissioning is in progress.
     *
     * The legacy default preserves source compatibility for existing providers. Providers
     * with multiple chains override this method based on their complete chain inventory.
     */
    virtual DeviceAttestationCertProfile GetPreferredDeviceAttestationChainProfile() const
    {
        return DeviceAttestationCertProfile::kEcdsaMatterLegacy;
    }

    /// Whether the provider reports a legacy chain and PQC PAA or PAI profiles, as required to enable PQC attestation.
    bool HasRequiredPqcCredentials() const;

    /**
     * @brief Read one segment of a device attestation document for a given stored chain profile.
     *
     * `profile` selects a chain, not the requested certificate's public key algorithm.
     * For example, an ML-DSA chain can contain an ECDSA PAI and DAC.
     *
     * On success, the implementation updates `out_document_buffer` to the bytes read starting at `offset` and sets
     * `out_document_size` to the size of the complete document. Implementations backed by persistent storage should read only
     * the requested segment into `out_document_buffer`.
     *
     * The default implementation supports legacy Matter documents at offset zero through the existing document getters.
     */
    virtual CHIP_ERROR GetDeviceAttestationDocumentSegment(DeviceAttestationDocumentType documentType,
                                                           DeviceAttestationCertProfile profile, size_t offset,
                                                           MutableByteSpan & out_document_buffer, size_t & out_document_size);

    /**
     * @brief Signs a message using the device attestation private key
     *
     * @param[in] message_to_sign The message to sign using the attestation private key.
     * @param[in,out] out_signature_buffer Buffer to receive the signature in raw <r,s> format.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_signature_buffer` is too small,
     *          or another CHIP_ERROR from the underlying implementation if signature fails.
     */
    virtual CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) = 0;
};

/**
 * Instance getter for the global DeviceAttestationCredentialsProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation credentials provider. Assume never null.
 */
DeviceAttestationCredentialsProvider * GetDeviceAttestationCredentialsProvider();

/**
 * Instance setter for the global DeviceAttestationCredentialsProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceAttestationCredentialsProvider to start returning with the getter
 */
void SetDeviceAttestationCredentialsProvider(DeviceAttestationCredentialsProvider * provider);

/**
 * Check if Instance is prepared
 */
bool IsDeviceAttestationCredentialsProviderSet();

} // namespace Credentials
} // namespace chip
