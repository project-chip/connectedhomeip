/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief This class provides Commissionable data and Device Attestation Credentials.
 *
 * This implementation is using the SDK fwk_factory_data_provider module.
 *
 * For more information on this module, the interface description available in
 * FactoryDataProvider/fwk_factory_data_provider.h inside the SDK can be checked.
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    CHIP_ERROR Init(void);
    CHIP_ERROR SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                           uint32_t * contentAddr = NULL);
    CHIP_ERROR SetEncryptionMode(EncryptionMode mode);

    void UpdateKeyAttributes(psa_key_attributes_t & attrs) override;

private:
    struct FactoryData
    {
        struct Header header;
        uint8_t factoryDataBuffer[PARTITION_SIZE(factory_partition) - sizeof(struct Header)];
    };

    FactoryData mFactoryData;
    const struct device * mFlashDevice = nullptr;

    CHIP_ERROR ReadEncryptedData(uint8_t * dest, uint8_t * source);

#if defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX)
    CHIP_ERROR ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t KeyAddr);
    CHIP_ERROR ELS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & addr);
    CHIP_ERROR ELS_ConvertDacKey();
#endif /* defined(CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION) && defined(CONFIG_SOC_SERIES_RW6XX) */
};

FactoryDataProvider & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip
