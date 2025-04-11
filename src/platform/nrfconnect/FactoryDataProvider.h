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
#include <platform/DeviceInstanceInfoProvider.h>

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <crypto/CHIPCryptoPALPSA.h>
#endif

#ifdef CONFIG_FPROTECT
#include <fprotect.h>
#endif // if CONFIG_FPROTECT

#if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1
#include <pm_config.h>
#define FACTORY_DATA_ADDRESS PM_FACTORY_DATA_ADDRESS
#define FACTORY_DATA_SIZE PM_FACTORY_DATA_SIZE
#else
#include <zephyr/storage/flash_map.h>
#define FACTORY_DATA_SIZE DT_REG_SIZE(DT_ALIAS(factory_data))
#define FACTORY_DATA_ADDRESS DT_REG_ADDR(DT_ALIAS(factory_data))
#endif // if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1

#include <system/SystemError.h>
#include <zephyr/drivers/flash.h>

#include "FactoryDataParser.h"

namespace chip {
namespace DeviceLayer {

struct InternalFlashFactoryData
{
    CHIP_ERROR GetFactoryDataPartition(uint8_t *& data, size_t & dataSize)
    {
        data     = reinterpret_cast<uint8_t *>(FACTORY_DATA_ADDRESS);
        dataSize = FACTORY_DATA_SIZE;
        return CHIP_NO_ERROR;
    }

#ifdef CONFIG_CHIP_FACTORY_DATA_WRITE_PROTECT
#define TO_STR_IMPL(x) #x
#define TO_STR(x) TO_STR_IMPL(x)
    // These two helpers allows to get the factory data memory block which shall be protected with fprotect, so that:
    // 1) it is aligned to the multiple of CONFIG_FPROTECT_BLOCK_SIZE (which differs between nRF families)
    // 2) it does not exceed the settings partition start address
    // Note that this block can overlap with app partition but this is not a problem since we do not aim to modify
    // the application code at runtime anyway.
    static constexpr size_t FactoryDataBlockBegin()
    {
        // calculate the nearest multiple of CONFIG_FPROTECT_BLOCK_SIZE smaller than FACTORY_DATA_ADDRESS
        return FACTORY_DATA_ADDRESS & (-CONFIG_FPROTECT_BLOCK_SIZE);
    }

    constexpr size_t FactoryDataBlockSize()
    {
        // calculate the factory data end address rounded up to the nearest multiple of CONFIG_FPROTECT_BLOCK_SIZE
        // and make sure we do not overlap with settings partition
        constexpr size_t kFactoryDataBlockEnd =
            (FACTORY_DATA_ADDRESS + FACTORY_DATA_SIZE + CONFIG_FPROTECT_BLOCK_SIZE - 1) & (-CONFIG_FPROTECT_BLOCK_SIZE);

        // Only the partition that is protected by fprotect must be aligned to fprotect block size
        constexpr size_t kSettingsBlockEnd = PM_SETTINGS_STORAGE_ADDRESS + PM_SETTINGS_STORAGE_SIZE;

        constexpr bool kOverlapsCheck =
            (kSettingsBlockEnd <= FactoryDataBlockBegin()) || (kFactoryDataBlockEnd <= PM_SETTINGS_STORAGE_ADDRESS);

        static_assert(kOverlapsCheck,
                      "FPROTECT memory block, which contains factory data"
                      "partition overlaps with the settings partition."
                      "Probably your settings partition size is not a multiple"
                      "of the atomic FPROTECT block size of " TO_STR(CONFIG_FPROTECT_BLOCK_SIZE) "kB");

        return kFactoryDataBlockEnd - FactoryDataBlockBegin();
    }
#undef TO_STR
#undef TO_STR_IMPL
    CHIP_ERROR ProtectFactoryDataPartitionAgainstWrite()
    {
#ifdef CONFIG_FPROTECT
        int ret = fprotect_area(FactoryDataBlockBegin(), FactoryDataBlockSize());
        return System::MapErrorZephyr(ret);
#else
        return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // if CONFIG_FPROTECT
    }
#else
    CHIP_ERROR ProtectFactoryDataPartitionAgainstWrite() { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
};

#if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1 && (defined(CONFIG_CHIP_QSPI_NOR) || defined(CONFIG_CHIP_SPI_NOR))
struct ExternalFlashFactoryData
{
    CHIP_ERROR GetFactoryDataPartition(uint8_t *& data, size_t & dataSize)
    {
        int ret = flash_read(mFlashDevice, FACTORY_DATA_ADDRESS, mFactoryDataBuffer, FACTORY_DATA_SIZE);

        if (ret != 0)
        {
            return CHIP_ERROR_READ_FAILED;
        }

        data     = mFactoryDataBuffer;
        dataSize = FACTORY_DATA_SIZE;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ProtectFactoryDataPartitionAgainstWrite() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    const struct device * mFlashDevice = DEVICE_DT_GET(DT_CHOSEN(nordic_pm_ext_flash));
    uint8_t mFactoryDataBuffer[FACTORY_DATA_SIZE];
};
#endif // if defined(USE_PARTITION_MANAGER) && USE_PARTITION_MANAGER == 1 && (defined(CONFIG_CHIP_QSPI_NOR) ||
       // defined(CONFIG_CHIP_SPI_NOR))

class FactoryDataProviderBase : public chip::Credentials::DeviceAttestationCredentialsProvider,
                                public CommissionableDataProvider,
                                public DeviceInstanceInfoProvider
{
public:
    /**
     * @brief Perform all operations needed to initialize factory data provider.
     *
     * @returns CHIP_NO_ERROR in case of a success, specific error code otherwise
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * @brief Get the EnableKey as MutableByteSpan
     *
     * @param enableKey MutableByteSpan object to obtain EnableKey
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain enable_key field, or the value cannot be read
     * out. CHIP_ERROR_BUFFER_TOO_SMALL if provided MutableByteSpan is too small
     */
    virtual CHIP_ERROR GetEnableKey(MutableByteSpan & enableKey) = 0;

    /**
     * @brief Get the user data in CBOR format as MutableByteSpan
     *
     * @param userData MutableByteSpan object to obtain all user data in CBOR format
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user field, or the value cannot be read out.
     * CHIP_ERROR_BUFFER_TOO_SMALL if provided MutableByteSpan is too small
     */
    virtual CHIP_ERROR GetUserData(MutableByteSpan & userData) = 0;

    /**
     * @brief Try to find user data key and return its value
     *
     * @param userKey A key name to be found
     * @param buf Buffer to store value of found key
     * @param len Length of the buffer. This value will be updated to the actual value if the key is read.
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user key field, or the value cannot be read
     * out. CHIP_ERROR_BUFFER_TOO_SMALL if provided buffer length is too small
     */
    virtual CHIP_ERROR GetUserKey(const char * userKey, void * buf, size_t & len) = 0;
};

template <class FlashFactoryData>
class FactoryDataProvider : public FactoryDataProviderBase
{
public:
    CHIP_ERROR Init() override;

    // ===== Members functions that implement the DeviceAttestationCredentialsProvider
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override;

    // ===== Members functions that implement the DeviceInstanceInfoProvider
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
    CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish) override;
    CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor) override;

    // ===== Members functions that are platform-specific
    CHIP_ERROR GetEnableKey(MutableByteSpan & enableKey) override;
    CHIP_ERROR GetUserData(MutableByteSpan & userData) override;
    CHIP_ERROR GetUserKey(const char * userKey, void * buf, size_t & len) override;

private:
    static constexpr uint16_t kFactoryDataPartitionSize    = FACTORY_DATA_SIZE;
    static constexpr uint32_t kFactoryDataPartitionAddress = FACTORY_DATA_ADDRESS;
    static constexpr uint8_t kDACPrivateKeyLength          = 32;
    static constexpr uint8_t kDACPublicKeyLength           = 65;

    struct FactoryData mFactoryData;
    FlashFactoryData mFlashFactoryData;
};

} // namespace DeviceLayer
} // namespace chip
