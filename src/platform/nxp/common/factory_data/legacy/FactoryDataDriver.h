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

namespace chip {
namespace DeviceLayer {

// Forward declaration to define the getter for factory data provider impl instance
class FactoryDataDriverImpl;

/**
 * @brief This interface provides the functions that should be implemented
 * to handle factory data update and factory data ram backup operations.
 * This interface must be implemented by each platform.
 */

class FactoryDataDriver
{
public:
    virtual ~FactoryDataDriver();

    /*!
     * \brief Initializes the FactoryDataDriver instance.
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR Init() = 0;

    /*!
     * \brief Checks whether the backup of the factory data exists (e.g in persistent storage).
     *
     * @retval true if backup exists otherwise return false.
     */
    virtual bool DoesBackupExist(uint16_t * size) = 0;

    /*!
     * \brief Deletes the backup of the factory data (e.g. from persistent storage).
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR DeleteBackup(void) = 0;

    /*!
     * \brief Allocates and initializes the factory data ram backup and copies
     * factory data into it.
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR InitRamBackup(void) = 0;

    /*!
     * \brief Clear and deallocate the factory data ram backup.
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR ClearRamBackup(void) = 0;

    /*!
     * \brief Read the factory data from persistent storage into the factory data
     * ram backup.
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR ReadBackupInRam(void) = 0;

    /*!
     * \brief Save / Backup the factory data into the persistent storage
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR BackupFactoryData(void) = 0;

    /*!
     * \brief Update / Write the factory data from the ram buffer
     *
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    virtual CHIP_ERROR UpdateFactoryData(void) = 0;

    /*!
     * \brief Update TLV value from factory data based on tag
     * @param tag         TLV tag of component to update
     * @param newValue    Reference to the new value of the TLV component
     * @retval CHIP_NO_ERROR if operation was successful.
     */
    CHIP_ERROR UpdateValueInRam(uint8_t tag, ByteSpan & newValue);

protected:
    uint8_t * mFactoryDataRamBuff = nullptr;
    uint32_t mSize                = 0;
    uint32_t mMaxSize             = 0;
};

extern FactoryDataDriver & FactoryDataDrv();

extern FactoryDataDriverImpl & FactoryDataDrvImpl();

} // namespace DeviceLayer
} // namespace chip
