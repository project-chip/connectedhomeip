/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *          NFC Data Retrieval implementation for SE05x with NXP platforms
 */

#pragma once

#include <cstddef>
#include <lib/core/CHIPError.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace NFCCommissioning {

/**
 * @brief Class to handle NFC commissioning data retrieval from SE05x secure element
 *
 * This class manages the synchronization of credentials stored in the SE05x
 * secure element (via NFC commissioning) to the device's Key-Value Store.
 */
class NFCDataRetrievalInfo
{
public:
    NFCDataRetrievalInfo()  = default;
    ~NFCDataRetrievalInfo() = default;

    // Disable copy and move
    NFCDataRetrievalInfo(const NFCDataRetrievalInfo &)             = delete;
    NFCDataRetrievalInfo & operator=(const NFCDataRetrievalInfo &) = delete;
    NFCDataRetrievalInfo(NFCDataRetrievalInfo &&)                  = delete;
    NFCDataRetrievalInfo & operator=(NFCDataRetrievalInfo &&)      = delete;

    /**
     * @brief Initialize NFC data retrieval and synchronize SE05x data to KVS
     *
     * This function checks for NFC commissioned data in the SE05x secure element
     * and synchronizes it to the device's Key-Value Store if found.
     *
     * @return CHIP_NO_ERROR on success, appropriate error code otherwise
     */
    CHIP_ERROR Init();

    /**
     * @brief Get the remaining fail-safe timer value from SE05x
     *
     * Returns the remaining fail-safe timer duration if this is the first
     * read after NFC commissioning, otherwise returns 0.
     *
     * @return Remaining fail-safe time in seconds, or 0 if not applicable
     */
    uint32_t GetRemainingFailSafeTimerForSE05x();

    /**
     * @brief Get operational data set that is stored in SE05x during NFC commissioning
     * @param None
     * @return CHIP_NO_ERROR on success, appropriate error code on failure
     */
    CHIP_ERROR GetOperationalDataSetFormSE05x(char * opDataSet, size_t * opDataSetLen);

    /**
     * @brief Get the singleton instance
     * @return Reference to the NFCDataRetrievalInfo singleton
     */
    static NFCDataRetrievalInfo & GetInstance();

private:
    char op_data_set[256]  = { 0 };
    size_t op_data_set_len = sizeof(op_data_set);

#if defined(CONFIG_SE05X_HOST_GPIO)
    /**
     * @brief Check if device is commissioned and initialize GPIO notification if needed
     *
     * This function checks the commissioning status by looking for fabric index information
     * in the key-value store. If the device is not commissioned, it initializes GPIO
     * notification for NFC-based commissioning.
     *
     * @return CHIP_NO_ERROR if GPIO notification was successfully initialized
     *         CHIP_ERROR_INCORRECT_STATE if device is already commissioned
     *         Other CHIP_ERROR codes on failure
     */
    CHIP_ERROR CheckCommissioningStatusAndInitGPIO();
#endif

    /**
     * @brief Check if KVS already contains synchronized SE05x data
     * @return true if KVS is synchronized, false otherwise
     */
    bool IsKVSAlreadySynchronized();

    /**
     * @brief Synchronize operational credentials from SE05x to KVS
     * @return CHIP_NO_ERROR on success, appropriate error code otherwise
     */
    CHIP_ERROR SynchronizeOperationalCredentials();

    /**
     * @brief Synchronize network credentials from SE05x to KVS
     * @return CHIP_NO_ERROR on success, appropriate error code otherwise
     */
    CHIP_ERROR SynchronizeNetworkCredentials();

    /// Flag indicating if we're reading from SE05x for fail-safe timer handling
    uint32_t mSe05xReadFailSafe = 0;

    static NFCDataRetrievalInfo sInstance;
};

/**
 * @brief Get the NFCDataRetrievalInfo singleton instance
 * @return Reference to the NFCDataRetrievalInfo singleton
 */
inline NFCDataRetrievalInfo & NFCDataRetrievalInfoMgr()
{
    return NFCDataRetrievalInfo::GetInstance();
}

} // namespace NFCCommissioning
} // namespace DeviceLayer
} // namespace chip
