/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

extern "C" {
#include "HWParameter.h"
}

/* This flag should be defined to run RunApiTest tests.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_API_TEST
#define CHIP_DEVICE_CONFIG_ENABLE_API_TEST 0
#endif

namespace chip {
namespace DeviceLayer {

/**
 * This class extends the default FactoryDataProvider functionality
 * by leveraging the secure subsystem for signing messages.
 * CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR - enables factory data OTA
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    FactoryDataProviderImpl();
    ~FactoryDataProviderImpl();

    CHIP_ERROR Init() override;
    void UpdateKeyAttributes(psa_key_attributes_t & attrs) override;

private:
#if CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
    CHIP_ERROR InitContext();
    CHIP_ERROR ImportPrivateKeyBlob();
    /*!
     * \brief Convert DAC private key to an encrypted blob and update factory data if not already done
     *
     * @note This API should be called in manufacturing process context to replace
     *       DAC private key with an encrypted blob. The conversion will be a
     *       one-time-only operation.
     * @retval #CHIP_NO_ERROR if factory data was updated successfully.
     */
    CHIP_ERROR ConvertDacKey();

    /*!
     * \brief Check and export an encrypted blob from the DAC private key found in factory data if needed
     *
     * @param data        Pointer to an allocated buffer
     * @param dataLen     Pointer to a variable that will store the blob length
     * @param offset      Offset of private key from the start of factory data payload address (after header)
     * @param isNeeded    Will be set to true if conversion is needed
     *
     * @retval #CHIP_NO_ERROR if conversion to blob was successful.
     */
    CHIP_ERROR ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset, bool & isNeeded);

    /*!
     * \brief Replace DAC private key with the specified encrypted blob
     *
     * @note A new hash has to be computed and written in the factory data header.
     * @param data        Pointer to a RAM buffer that duplicates the current factory data
     * @param blob        Pointer to blob data
     * @param blobLen     Blob length
     * @param offset      Offset of private key from the start of factory data payload address (after header)
     *                    Extracted with ConvertToBlob.
     *
     * @retval #CHIP_NO_ERROR if conversion to blob was successful.
     */
    CHIP_ERROR ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t offset);
#endif // CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION

#if CHIP_DEVICE_CONFIG_ENABLE_API_TEST
    void RunApiTest();
#endif

private:
    extendedAppFactoryData_t * mFactoryData = nullptr;
#if CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
    psa_key_id_t dac_private_key_id = PSA_KEY_ID_NULL;
#endif
};

FactoryDataProvider & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip
