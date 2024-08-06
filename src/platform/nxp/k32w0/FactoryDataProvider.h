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

#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.h>
#include <src/lib/core/CHIPError.h>

#include <vector>

#include "CHIPPlatformConfig.h"

#include <vector>

/* Grab symbol for the base address from the linker file. */
extern uint32_t __MATTER_FACTORY_DATA_START[];
extern uint32_t __MATTER_FACTORY_DATA_SIZE[];

namespace chip {
namespace DeviceLayer {

#define CHIP_FACTORY_DATA_ERROR(e)                                                                                                 \
    ChipError(ChipError::Range::kLastRange, ((uint8_t) ChipError::Range::kLastRange << 2) | e, __FILE__, __LINE__)

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
        kMaxId
    };

    static uint32_t kFactoryDataStart;
    static uint32_t kFactoryDataSize;
    static uint32_t kFactoryDataPayloadStart;
    static constexpr uint32_t kLengthOffset = 1;
    static constexpr uint32_t kValueOffset  = 3;
    static constexpr uint32_t kHashLen      = 4;
    static constexpr size_t kHashId         = 0xCE47BA5E;

    virtual ~FactoryDataProvider();

    virtual CHIP_ERROR Init()                                                                          = 0;
    virtual CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) = 0;
    CHIP_ERROR Validate();

    CHIP_ERROR SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length, uint32_t * offset = nullptr);

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
    Header mHeader;
};

extern FactoryDataProvider & FactoryDataPrvd();

extern FactoryDataProviderImpl & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip
