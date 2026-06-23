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
#pragma once

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/core/CHIPError.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

#include "psa/crypto_types.h"

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
#include <lib/support/DefaultStorageKeyAllocator.h>
#endif

#include <platform/nxp/common/factory_data/legacy/FactoryDataDriver.h>

#include <vector>

#include CHIP_PLATFORM_CONFIG_INCLUDE

namespace chip {
namespace DeviceLayer {

#define CHIP_FACTORY_DATA_ERROR(e)                                                                                                 \
    CHIP_GENERIC_ERROR(ChipError::Range::kLastRange, ((uint8_t) ChipError::Range::kLastRange << 2) | e)

#define CHIP_FACTORY_DATA_SHA_CHECK CHIP_FACTORY_DATA_ERROR(0x01)
#define CHIP_FACTORY_DATA_HEADER_READ CHIP_FACTORY_DATA_ERROR(0x02)
#define CHIP_FACTORY_DATA_HASH_ID CHIP_FACTORY_DATA_ERROR(0x03)
#define CHIP_FACTORY_DATA_PDM_RESTORE CHIP_FACTORY_DATA_ERROR(0x04)
#define CHIP_FACTORY_DATA_NULL CHIP_FACTORY_DATA_ERROR(0x05)
#define CHIP_FACTORY_DATA_FLASH_ERASE CHIP_FACTORY_DATA_ERROR(0x06)
#define CHIP_FACTORY_DATA_FLASH_PROGRAM CHIP_FACTORY_DATA_ERROR(0x07)
#define CHIP_FACTORY_DATA_INTERNAL_FLASH_READ CHIP_FACTORY_DATA_ERROR(0x08)
#define CHIP_FACTORY_DATA_PDM_SAVE_RECORD CHIP_FACTORY_DATA_ERROR(0x09)
#define CHIP_FACTORY_DATA_PDM_READ_RECORD CHIP_FACTORY_DATA_ERROR(0x0A)
#define CHIP_FACTORY_DATA_RESTORE_MECHANISM CHIP_FACTORY_DATA_ERROR(0x0B)

// Forward declaration to define the getter for factory data provider impl instance
class FactoryDataProviderImpl;

/**
 * @brief This class provides Commissionable data, Device Attestation Credentials,
 *        and Device Instance Info.
 */

class FactoryDataProvider : public DeviceInstanceInfoProvider,
                            public CommissionableDataProvider,
                            public Credentials::DeviceAttestationCredentialsProvider
{
public:
    struct Header
    {
        uint32_t hashId;
        uint32_t size;
        uint8_t hash[4];
    };
    enum KeyType
    {
        kHwKey  = 0U,
        kSftKey = 1U,
    };

    struct FactoryDataConfig
    {
        uint32_t start;
        uint32_t size;
        uint32_t payload;
    };

    enum EncryptionMode
    {
        encrypt_none = 0U,
        encrypt_ecb  = 1U,
        encrypt_cbc  = 2U
    };

    enum AESKeySize
    {
        aes_128 = 128u,
        aes_256 = 256U
    };

    // Default factory data IDs
    enum FactoryDataId
    {
        kVerifierId = 1,
        kSaltId,
        kIcId,
        kDacPrivateKeyId,
        kDacCertificateId,
        kPaiCertificateId,
        kDiscriminatorId,
        kSetupPasscodeId,
        kVidId,
        kPidId,
        kCertDeclarationId,
        kVendorNameId,
        kProductNameId,
        kSerialNumberId,
        kManufacturingDateId,
        kHardwareVersionId,
        kHardwareVersionStrId,
        kUniqueId,
        kPartNumber,
        kProductURL,
        kProductLabel,
        kProductFinish,
        kProductPrimaryColor,
        kEl2GoBlob,
        kEl2GoDacKeyId,
        kEl2GoDacCertId,

        kMaxId
    };

    static uint32_t kFactoryDataMaxSize;
    static constexpr uint32_t kLengthOffset = 1;
    static constexpr uint32_t kValueOffset  = 3;
    static constexpr uint32_t kHashLen      = 4;
    static constexpr size_t kHashId         = 0xCE47BA5E;

    virtual ~FactoryDataProvider();

    virtual CHIP_ERROR Init() = 0;
    virtual CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer);
    virtual CHIP_ERROR Validate();

    virtual CHIP_ERROR SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                                   uint32_t * offset = nullptr);
    virtual CHIP_ERROR SetAesKey(const uint8_t * keyAes, AESKeySize keySize);
    virtual CHIP_ERROR SetEncryptionMode(EncryptionMode mode);
    virtual CHIP_ERROR EncryptFactoryData(uint8_t * FactoryDataBuff) { return CHIP_NO_ERROR; };
    virtual CHIP_ERROR DecryptFactoryData(uint8_t * FactoryDataBuff) { return CHIP_NO_ERROR; };
    virtual CHIP_ERROR SetKeyType(KeyType type) { return CHIP_NO_ERROR; };
    virtual CHIP_ERROR SetCbcInitialVector(const uint8_t * iv, uint16_t ivSize);

    virtual void UpdateKeyAttributes(psa_key_attributes_t & attrs)
    { /* Empty Default implementation - can be overridden by derived classes */
    }

    virtual CHIP_ERROR FactoryReset() { return CHIP_NO_ERROR; };
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    using RestoreMechanism = CHIP_ERROR (*)(void);

    CHIP_ERROR ValidateWithRestore();
    void RegisterRestoreMechanism(RestoreMechanism mechanism);

    virtual CHIP_ERROR PreResetCheck();
    virtual CHIP_ERROR PostResetCheck();

    StorageKeyName GetTargetVersionKey() { return StorageKeyName::FromConst("nxp/tgt-sw-ver"); }

    CHIP_ERROR GetTargetVersion(uint32_t & version);
    CHIP_ERROR SaveTargetVersion(uint32_t & version);
#endif

    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override;

    // ===== Members functions that implement the DeviceAttestationCredentialsProvider
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

    // ===== Members functions that implement the GenericDeviceInstanceInfoProvider
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
    CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish) override;
    CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor) override;

protected:
    // Use when factory data are encrypted using aes key
    const uint8_t * pAesKey = nullptr;
    // AES key size in bit
    AESKeySize pAESKeySize;
    EncryptionMode encryptMode       = encrypt_ecb;
    const uint8_t * cbcInitialVector = nullptr;

    Header mHeader;
    FactoryDataConfig mConfig;
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    std::vector<RestoreMechanism> mRestoreMechanisms;
    FactoryDataDriver * mFactoryDataDriver = nullptr;
#endif

    /**
     * @brief Blob context containing PSA-translatable fields only
     */
    struct BlobContext
    {
        uint32_t keyId;
        uint32_t psaPermittedAlgorithm;
        uint32_t psaUsage;
        uint16_t psaKeyType;
        uint32_t psaKeyBits;
        uint32_t psaKeyLifetime;
    };

#if CONFIG_CHIP_ENABLE_EL2GO_FACTORY_DATA
    /**
     * @brief Parses and imports EL2GO blobs from a memory area into PSA Crypto keystore
     *
     * Scans a memory area containing one or more EL2GO (EdgeLock 2GO) blobs, extracts
     * PSA key attributes from each blob, and imports them into the PSA Crypto keystore.
     * System-reserved blobs (OEM, RKTH, OTP, BATCH_FLOW keys) are automatically skipped.
     *
     * @param[in]  blobArea      Pointer to memory area containing EL2GO blobs (must not be NULL)
     * @param[in]  blobAreaSize  Size of the blob memory area in bytes (must be > 0)
     * @param[out] blobsImported Number of successfully imported blobs (must not be NULL)
     * @param[out] dacKeyId      DAC private key ID found in blobs (must not be NULL)
     * @param[out] dacCertId     DAC certificate ID found in blobs (must not be NULL)
     *
     * @return CHIP_NO_ERROR on success
     * @return CHIP_ERROR_INVALID_ARGUMENT if parameters are invalid or blob format is incorrect
     * @return CHIP_ERROR_BUFFER_TOO_SMALL if blob is truncated
     * @return CHIP_ERROR_INTERNAL if PSA import fails
     * @return CHIP_ERROR_NO_MEMORY if too many blobs to track for cleanup
     * @return CHIP_ERROR_KEY_NOT_FOUND if DAC key or certificate not found in blobs
     *
     * @note On error, all successfully imported keys are destroyed and blobsImported is set to 0
     * @note If a key already exists, it is destroyed and replaced
     * @note Function stops at first error and cleans up all imported keys
     */
    CHIP_ERROR ParseEl2GoBlobs(const uint8_t * blobArea, size_t blobAreaSize, size_t * blobsImported, uint32_t * dacKeyId,
                               uint32_t * dacCertId);

    /**
     * @brief Parses an EL2GO blob and extracts PSA-relevant metadata
     *
     * Parses a TLV-encoded EL2GO blob and populates a BlobContext structure
     * with only the fields that can be converted to PSA key attributes.
     * Other blob fields (IV, signature, etc.) are validated but not returned.
     *
     * @param[in] blob      Pointer to blob data
     * @param[in] maxSize   Maximum size to prevent reading beyond valid memory
     * @param[out] blobCtx  Parsed blob context with PSA-translatable fields
     * @param[out] actualSize Optional: actual blob size (can be nullptr)
     *
     * @return CHIP_NO_ERROR on success
     * @return CHIP_ERROR_INVALID_ARGUMENT if blob is null, maxSize is 0,
     *         or value sizes don't match expected lengths
     * @return CHIP_ERROR_BUFFER_TOO_SMALL if blob is truncated
     *
     * @note Non-PSA fields (magic, IV, signature) are validated during parsing
     *       but not stored in blobCtx
     */
    CHIP_ERROR ParseEl2GoBlob(const uint8_t * blob, size_t maxSize, BlobContext & blobCtx, size_t * actualSize);
#endif

private:
#if CONFIG_CHIP_ENABLE_EL2GO_FACTORY_DATA
    // EL2GO blob magic signature constants
    static constexpr size_t kBlobMagicTlvSize = 0x0D;
    static constexpr uint32_t kBlobMagicTlv1  = 0x400B6564U;
    static constexpr uint32_t kBlobMagicTlv2  = 0x67656C6FU;
    static constexpr uint32_t kBlobMagicTlv3  = 0x636B3267U;
    static constexpr uint8_t kBlobMagicTlv4   = 0x6FU;

    // System Keys not imported
    static constexpr uint32_t kOemKeyId       = 0x7FFF817BU;
    static constexpr uint32_t kRkthKeyId      = 0x7FFF817AU;
    static constexpr uint32_t kOtpDataKeyId   = 0x7FFF817CU;
    static constexpr uint32_t kBatchFlowKeyId = 0x7FFF8181U;

    // EL2GO Blob TLV Tag Constants
    static constexpr uint8_t kTagMagic              = 0x40U;
    static constexpr uint8_t kTagKeyId              = 0x41U;
    static constexpr uint8_t kTagPermittedAlgorithm = 0x42U;
    static constexpr uint8_t kTagKeyUsageFlags      = 0x43U;
    static constexpr uint8_t kTagKeyType            = 0x44U;
    static constexpr uint8_t kTagKeyBits            = 0x45U;
    static constexpr uint8_t kTagKeyLifetime        = 0x46U;
    static constexpr uint8_t kTagDeviceLifecycle    = 0x47U;
    static constexpr uint8_t kTagWrappingKeyId      = 0x50U;
    static constexpr uint8_t kTagWrappingAlgorithm  = 0x51U;
    static constexpr uint8_t kTagIv                 = 0x52U;
    static constexpr uint8_t kTagSignatureKeyId     = 0x53U;
    static constexpr uint8_t kTagSignatureAlgorithm = 0x54U;
    static constexpr uint8_t kTagPlain              = 0x55U;
    static constexpr uint8_t kTagSignature          = 0x5EU;

    /**
     * @brief Converts BlobContext to PSA key attributes
     *
     * @param[in] blobCtx   Parsed blob context
     * @param[out] psaAttrs PSA key attributes structure
     *
     * @return CHIP_NO_ERROR on success
     */
    CHIP_ERROR BlobContextToPsaAttributes(const BlobContext & blobCtx, psa_key_attributes_t & psaAttrs);

    /**
     * @brief Checks if a memory location contains the EL2GO blob magic signature
     *
     * Verifies whether the specified memory location starts with the 13-byte
     * EL2GO blob magic TLV signature ("edgelock2go" encoded as TLV).
     *
     * @param[in] ptr       Pointer to the memory location to check
     * @param[in] endPtr    Pointer to the end of the valid memory region
     *
     * @return true if the magic signature is found and bounds are valid
     * @return false if signature doesn't match or insufficient bytes remain
     *
     * @note Performs bounds checking to prevent buffer overruns
     */
    bool IsBlobMagic(const uint8_t * ptr, const uint8_t * endPtr);

    /**
     * @brief Reads a 32-bit unsigned integer from a byte array (big-endian)
     *
     * Extracts a 32-bit value from a byte array in big-endian (MSB first) byte order,
     * as used in EL2GO blob format.
     *
     * @param[in] input     Pointer to a byte array (must have at least 4 bytes)
     *
     * @return The 32-bit unsigned integer value
     *
     * @warning Caller must ensure 'input' points to at least 4 readable bytes
     *
     * @note Byte order: input[0] is MSB, input[3] is LSB
     */
    uint32_t GetUint32(const uint8_t * input);

    /**
     * @brief Reads a 16-bit unsigned integer from a byte array (big-endian)
     *
     * Extracts a 16-bit value from a byte array in big-endian (MSB first) byte order,
     * as used in EL2GO blob format.
     *
     * @param[in] input Pointer to a byte array (must have at least 2 bytes)
     *
     * @return The 16-bit unsigned integer value
     *
     * @warning Caller must ensure 'input' points to at least 2 readable bytes
     *
     * @note Byte order: input[0] is MSB, input[1] is LSB
     */
    uint16_t GetUint16(const uint8_t * input);
#endif
};

extern FactoryDataProvider & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip
