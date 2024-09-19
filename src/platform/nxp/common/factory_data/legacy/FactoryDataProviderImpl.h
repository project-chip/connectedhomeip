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

#if !CHIP_USE_PLAIN_DAC_KEY
#include "sss_crypto.h"
#endif

extern "C" {
#include "HWParameter.h"
}

/* This flag should be defined to run SSS_RunApiTest tests.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST
#define CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST 0
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
    static FactoryDataProviderImpl sInstance;

    FactoryDataProviderImpl();
    ~FactoryDataProviderImpl();

    CHIP_ERROR Init() override;
    CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR

    CHIP_ERROR PreResetCheck() override;
    CHIP_ERROR PostResetCheck() override;

    static StorageKeyName GetTargetVersionKey() { return StorageKeyName::FromConst("nxp/tgt-sw-ver"); }

    static CHIP_ERROR GetTargetVersion(uint32_t & version);
    static CHIP_ERROR SaveTargetVersion(uint32_t & version);
#endif

private:
#if !CHIP_USE_PLAIN_DAC_KEY

    CHIP_ERROR SSS_InitContext();
    CHIP_ERROR SSS_ImportPrivateKeyBlob();
    CHIP_ERROR SSS_Sign(uint8_t * digest, Crypto::P256ECDSASignature & signature);
    /*!
     * \brief Convert DAC private key to an SSS encrypted blob and update factory data if not already done
     *
     * @note This API should be called in manufacturing process context to replace
     *       DAC private key with an SSS encrypted blob. The conversion will be a
     *       one-time-only operation.
     * @retval #CHIP_NO_ERROR if factory data was updated successfully.
     */
    CHIP_ERROR SSS_ConvertDacKey();

    /*!
     * \brief Check and export an SSS encrypted blob from the DAC private key found in factory data if needed
     *
     * @param data        Pointer to an allocated buffer
     * @param dataLen     Pointer to a variable that will store the blob length
     * @param offset      Offset of private key from the start of factory data payload address (after header)
     * @param isNeeded    Will be set to true if conversion is needed
     *
     * @retval #CHIP_NO_ERROR if conversion to blob was successful.
     */
    CHIP_ERROR SSS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset, bool & isNeeded);

    /*!
     * \brief Replace DAC private key with the specified SSS encrypted blob
     *
     * @note A new hash has to be computed and written in the factory data header.
     * @param data        Pointer to a RAM buffer that duplicates the current factory data
     * @param blob        Pointer to blob data
     * @param blobLen     Blob length
     * @param offset      Offset of private key from the start of factory data payload address (after header)
     *                    Extracted with SSS_ConvertToBlob.
     *
     * @retval #CHIP_NO_ERROR if conversion to blob was successful.
     */
    CHIP_ERROR ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t offset);

#if CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST
    void SSS_RunApiTest();
#endif
#endif // CHIP_USE_PLAIN_DAC_KEY

private:
    extendedAppFactoryData_t * mFactoryData = nullptr;
#if !CHIP_USE_PLAIN_DAC_KEY
    sss_sscp_object_t mContext;
#endif
};

} // namespace DeviceLayer
} // namespace chip
