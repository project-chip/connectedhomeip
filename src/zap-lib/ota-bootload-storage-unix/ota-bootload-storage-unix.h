/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_STORAGE_UNIX_H__
#define __OTA_BOOTLOAD_STORAGE_UNIX_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE

/**
 * @addtogroup OTA_Bootload
 * @{
 */

/**
 * @addtogroup OTA_Bootload_Types
 * @{
 */

// -----------------------------------------------------------------------------
// Types

/** @} end addtogroup */

/**************************************************************************//**
 * This function outputs an OTA file header
 *
 * @param file The opened file handle to write to
 * @param fileHeaderInfo OTA bootloader header specification
 * @param imageDataSize Total size of the raw image data following the header
 *
 * @return Returns 0 on success, 1 on failure
 *****************************************************************************/
bool emberZclOtaBootloadStorageUnixWriteFileHeader(FILE *file,
                                                   EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                   size_t imageDataSize);

/**************************************************************************//**
 * This function reads an OTA file header from a file
 *
 * @param file The opened file handle to read from
 * @param fileHeaderInfo OTA bootloader header specification
 *
 * @return Returns 0 on success, 1 on failure
 *****************************************************************************/
bool emberZclOtaBootloadStorageUnixReadFileHeader(FILE *file,
                                                  EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo);

/** @} end addtogroup */

#endif // __OTA_BOOTLOAD_STORAGE_UNIX_H__
