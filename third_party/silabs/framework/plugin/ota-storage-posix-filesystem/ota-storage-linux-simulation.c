/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Simulation for the OTA Storage POSIX Filesystem plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/ota-common/ota.h"

#include "app/util/serial/command-interpreter2.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Globals

#define INVALID_MANUFACTURER_ID  0xFFFF
#define INVALID_DEVICE_ID        0xFFFF
#define INVALID_FIRMWARE_VERSION 0xFFFFFFFFL
#define INVALID_EUI64 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define INVALID_OTA_IMAGE_ID  \
  { INVALID_MANUFACTURER_ID,  \
    INVALID_DEVICE_ID,        \
    INVALID_FIRMWARE_VERSION, \
    INVALID_EUI64,            \
  }

EmberAfOtaImageId emberInvalidImageId = INVALID_OTA_IMAGE_ID;

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

void emAfOtaStorageClose(void)
{
}

void emAfOtaStorageInfoPrint(void)
{
}

EmberAfOtaStorageStatus emberAfOtaStorageCheckTempDataCallback(uint32_t* returnOffset,
                                                               uint32_t* returnTotalSize,
                                                               EmberAfOtaImageId* returnOtaImageId)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaStorageStatus emberAfOtaStorageClearTempDataCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

void emAfOtaLoadFileCommand(void)
{
}

EmberAfOtaStorageStatus emberAfOtaStorageDeleteImageCallback(const EmberAfOtaImageId* id)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

uint8_t emberAfOtaStorageGetCountCallback(void)
{
  return 0;
}

EmberAfOtaStorageStatus emberAfOtaStorageGetFullHeaderCallback(const EmberAfOtaImageId* id,
                                                               EmberAfOtaHeader* returnData)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

uint32_t emberAfOtaStorageGetTotalImageSizeCallback(const EmberAfOtaImageId* id)
{
  return 0;
}

EmberAfOtaStorageStatus emberAfOtaStorageInitCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaImageId emberAfOtaStorageIteratorFirstCallback(void)
{
  return emberInvalidImageId;
}

EmberAfOtaImageId emberAfOtaStorageIteratorNextCallback(void)
{
  return emberInvalidImageId;
}

EmberAfOtaStorageStatus emberAfOtaStorageReadImageDataCallback(const EmberAfOtaImageId* id,
                                                               uint32_t offset,
                                                               uint32_t length,
                                                               uint8_t* returnData,
                                                               uint32_t* returnedLength)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

EmberAfOtaImageId emberAfOtaStorageSearchCallback(uint16_t manufacturerId,
                                                  uint16_t manufacturerDeviceId,
                                                  const uint16_t* hardwareVersion)
{
  return emberInvalidImageId;
}
