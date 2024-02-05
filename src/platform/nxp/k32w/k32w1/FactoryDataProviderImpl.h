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
#include <platform/nxp/k32w/common/FactoryDataProvider.h>

#include "sss_crypto.h"

/* This flag should be defined when the factory data contains
 * the DAC private key in plain text. It usually occurs in
 * manufacturing.
 *
 * The init phase will use S200 to export an encrypted blob,
 * then overwrite the private key section from internal flash.
 *
 * Should be used one time only for securing the private key.
 * The manufacturer will then flash the real image, which shall
 * not define this flag.
 */
#ifndef CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
#define CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY 0
#endif

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
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    ~FactoryDataProviderImpl();

    CHIP_ERROR Init() override;
    CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

private:
    CHIP_ERROR SSS_InitContext();
    CHIP_ERROR SSS_ImportPrivateKeyBlob();
    CHIP_ERROR SSS_Sign(uint8_t * digest, Crypto::P256ECDSASignature & signature);
#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
    /*!
     * \brief Convert DAC private key to an SSS encrypted blob and update factory data
     *
     * @note This API should be called in manufacturing process context to replace
     *       DAC private key with an SSS encrypted blob. The conversion will be a
     *       one-time-only operation.
     * @retval #CHIP_NO_ERROR if factory data was updated successfully.
     */
    CHIP_ERROR SSS_ConvertDacKey();

    /*!
     * \brief Export an SSS encrypted blob from the DAC private key found in factory data
     *
     * @param data        Pointer to an allocated buffer
     * @param dataLen     Pointer to a variable that will store the blob length
     * @param offset      Offset of private key from the start of factory data payload address (after header)
     *
     * @retval #CHIP_NO_ERROR if conversion to blob was successful.
     */
    CHIP_ERROR SSS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset);

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
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_SSS_API_TEST
    void SSS_RunApiTest();
#endif

    sss_sscp_object_t mContext;
};

} // namespace DeviceLayer
} // namespace chip
