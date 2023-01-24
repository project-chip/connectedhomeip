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

#include <fprotect.h>
#include <pm_config.h>
#include <system/SystemError.h>
#include <zephyr/drivers/flash.h>

#include "FactoryDataParser.h"

namespace chip {
namespace DeviceLayer {

struct InternalFlashFactoryData
{
    CHIP_ERROR GetFactoryDataPartition(uint8_t *& data, size_t & dataSize)
    {
        data     = reinterpret_cast<uint8_t *>(PM_FACTORY_DATA_ADDRESS);
        dataSize = PM_FACTORY_DATA_SIZE;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ProtectFactoryDataPartitionAgainstWrite()
    {
        int ret = fprotect_area(PM_FACTORY_DATA_ADDRESS, PM_FACTORY_DATA_SIZE);
        return System::MapErrorZephyr(ret);
    }
};

struct ExternalFlashFactoryData
{
    CHIP_ERROR GetFactoryDataPartition(uint8_t *& data, size_t & dataSize)
    {
        int ret = flash_read(mFlashDevice, PM_FACTORY_DATA_ADDRESS, mFactoryDataBuffer, PM_FACTORY_DATA_SIZE);

        if (ret != 0)
        {
            return CHIP_ERROR_READ_FAILED;
        }

        data     = mFactoryDataBuffer;
        dataSize = PM_FACTORY_DATA_SIZE;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ProtectFactoryDataPartitionAgainstWrite() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    const struct device * mFlashDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
    uint8_t mFactoryDataBuffer[PM_FACTORY_DATA_SIZE];
};

template <class FlashFactoryData>
class FactoryDataProvider : public chip::Credentials::DeviceAttestationCredentialsProvider,
                            public CommissionableDataProvider,
                            public DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR Init();

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

    // ===== Members functions that are platform-specific
    CHIP_ERROR GetEnableKey(MutableByteSpan & enableKey);

    /**
     * @brief Get the user data in CBOR format as MutableByteSpan
     *
     * @param userData MutableByteSpan object to obtain all user data in CBOR format
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user field, or the value cannot be read out.
     * CHIP_ERROR_BUFFER_TOO_SMALL if provided MutableByteSpan is too small
     */
    CHIP_ERROR GetUserData(MutableByteSpan & userData);

    /**
     * @brief Try to find user data key and return its value
     *
     * @param userKey A key name to be found
     * @param buf Buffer to store value of found key
     * @param len Length of the buffer. This value will be updated to the actual value if the key is read.
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user field, or the value cannot be read out.
     * CHIP_ERROR_BUFFER_TOO_SMALL if provided buffer length is too small
     */
    CHIP_ERROR GetUserKey(const char * userKey, void * buf, size_t & len);

private:
    static constexpr uint16_t kFactoryDataPartitionSize    = PM_FACTORY_DATA_SIZE;
    static constexpr uint32_t kFactoryDataPartitionAddress = PM_FACTORY_DATA_ADDRESS;
    static constexpr uint8_t kDACPrivateKeyLength          = 32;
    static constexpr uint8_t kDACPublicKeyLength           = 65;

    struct FactoryData mFactoryData;
    FlashFactoryData mFlashFactoryData;
};

} // namespace DeviceLayer
} // namespace chip
