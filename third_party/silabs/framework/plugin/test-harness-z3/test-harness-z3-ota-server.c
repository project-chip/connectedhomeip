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
 * @brief ZigBee 3.0 ota server test harness functionality
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "test-harness-z3-core.h"
#include "app/framework/plugin/ota-server/ota-server.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
// -----------------------------------------------------------------------------
extern void emberAfSetNoReplyForNextMessage(bool set);
static EmberAfOtaImageId loadedOtaImageId = {
  INVALID_MANUFACTURER_ID,
  INVALID_DEVICE_ID,
  INVALID_FIRMWARE_VERSION,
  INVALID_EUI64,
};
static EmberAfOtaHeader loadedOTAFileHeader = {
  // Magic Number omitted since it is always the same.
  .headerVersion = 0x0100,
  .headerLength  = 0x0038,
  .fieldControl  = 0x0000,
  .manufacturerId = 0xffff,
  .imageTypeId    = 0xffff,           // a.k.a. Device ID
  .firmwareVersion = 0xffffffff,
  .zigbeeStackVersion = 0x0002,

  /**
   * @brief The spec. does NOT require that the string be NULL terminated in the
   *   header stored on disk.  Therefore we make sure we can support a
   *   32-character string without a NULL terminator by adding +1 in the data
   *   structure.
   */
  //.headerString[EMBER_AF_OTA_MAX_HEADER_STRING_LENGTH + 1] = {0},

  /**
   * @brief When reading the header this will be the complete length of
   *  the file. When writing the header, this must be set to
   *  the length of the MFG image data portion including all tags.
   */
  .imageSize = 0,

  /**
   * @brief The remaining four fields are optional. The field control should be checked
   *   to determine if their values are valid.
   */
  .securityCredentials = 0,
  .upgradeFileDestination = { 0xff },
  .minimumHardwareVersion = 0,
  .maximumHardwareVersion = 0
};

EmberAfOtaImageId emberAfOtaStorageSearchCallback(int16u manufacturerId,
                                                  int16u imageTypeId,
                                                  const int16u* hardwareVersion)
{
  EmberAfOtaImageId InvalidId = {
    INVALID_MANUFACTURER_ID,
    INVALID_DEVICE_ID,
    INVALID_FIRMWARE_VERSION,
    INVALID_EUI64,
  };

  if (loadedOTAFileHeader.manufacturerId == manufacturerId
      && loadedOTAFileHeader.imageTypeId == imageTypeId) {
    return loadedOtaImageId;
  }
  (void)hardwareVersion;
  return InvalidId;
}
void emAfOtaLoadFileCommand(void)
{
  // TODO: dummy file for the test harness
}

EmberAfOtaStorageStatus emberAfOtaStorageGetFullHeaderCallback(const EmberAfOtaImageId* id,
                                                               EmberAfOtaHeader* returnData)
{
  if (id->manufacturerId == loadedOtaImageId.manufacturerId
      && id->imageTypeId == loadedOtaImageId.imageTypeId) {
    return EMBER_AF_OTA_STORAGE_SUCCESS;
  }

  // If the requested image cannot be found, then an error should be returned.
  return EMBER_AF_OTA_STORAGE_ERROR;
}

/** @brief Ota Storage Read Image Data
 *
 * This callback reads data from the specified OTA file and returns that data to
 * the caller.
 *
 * @param id This is a pointer to the image id for the OTA file to retrieve data
 * from.  Ver.: always
 * @param offset This is the offset relative to the start of the image where the
 * data should be read from.  Ver.: always
 * @param length This is the length of data that will be read.  Ver.: always
 * @param returnData This is a pointer to where the data read out of the file
 * will be written to  Ver.: always
 * @param returnedLength This is a pointer to a variable where the actual length
 * of data read will be written to.  A short read may occur if the end of file
 * was reached.  Ver.: always
 */
EmberAfOtaStorageStatus emberAfOtaStorageReadImageDataCallback(const EmberAfOtaImageId* id,
                                                               uint32_t offset,
                                                               uint32_t length,
                                                               uint8_t* returnData,
                                                               uint32_t* returnedLength)
{
  // If the requested image cannot be found, then an error should be returned.
  if (id != NULL) {
    if (id->manufacturerId == loadedOtaImageId.manufacturerId
        && id->imageTypeId == loadedOtaImageId.imageTypeId) {
      emberAfSetNoReplyForNextMessage(true);
      return EMBER_AF_OTA_STORAGE_SUCCESS;
    }
  }
  return EMBER_AF_OTA_STORAGE_ERROR;
}
/** @brief Ota Storage Get Total Image Size
 *
 * This function returns the total size of the ZigBee Over-the-air file with the
 * passed parameters.  If no file is found with those parameters, 0 is returned.
 *
 * @param id A pointer to the image identifier for the OTA file to retrieve
 * information for.  Ver.: always
 */
uint32_t emberAfOtaStorageGetTotalImageSizeCallback(const EmberAfOtaImageId* id)
{
  if (id != NULL) {
    if (id->manufacturerId == loadedOtaImageId.manufacturerId
        && id->imageTypeId == loadedOtaImageId.imageTypeId) {
      return loadedOTAFileHeader.imageSize;
    }
  }
  // On failure this should return an image size of zero.
  return 0;
}

bool emberAfIsOtaImageIdValid(const EmberAfOtaImageId* idToCompare)
{
  EmberAfOtaImageId InvalidId = {
    INVALID_MANUFACTURER_ID,
    INVALID_DEVICE_ID,
    INVALID_FIRMWARE_VERSION,
    INVALID_EUI64,
  };

  if (idToCompare != NULL) {
    // Check if the loaded image is a valid?
    if (idToCompare->manufacturerId != InvalidId.manufacturerId
        && idToCompare->imageTypeId != InvalidId.imageTypeId
        /*&& idToCompare->firmwareVersion != InvalidId.firmwareVersion*/) {
      // Then check if the requested image does match the loaded?
      if (idToCompare->manufacturerId == loadedOtaImageId.manufacturerId
          && idToCompare->imageTypeId == loadedOtaImageId.imageTypeId) {
        return true;
      }
    }
  }
  return false;
}

// OTA Server commands
// plugin test-harness z3 ota-server activate <shortAddress:2> <endpoint:1> <manufacturereCode:2> <imageType:2> <fileVersion:4> <fileSize:4>
//static EmberAfOtaImageId otaImageId;
void emAfPluginTestHarnessZ3OtaServerActivateCommand(void)
{
  EmberStatus status = EMBER_INVALID_CALL;
#ifndef EZSP_HOST
  EmberNodeId shortAddress = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  loadedOTAFileHeader.manufacturerId = (uint16_t)emberUnsignedCommandArgument(2);
  loadedOTAFileHeader.imageTypeId = (uint16_t)emberUnsignedCommandArgument(3);
  loadedOTAFileHeader.firmwareVersion = (uint32_t)emberUnsignedCommandArgument(4);
  loadedOTAFileHeader.imageSize = (uint32_t)emberUnsignedCommandArgument(5);

  loadedOtaImageId.manufacturerId = loadedOTAFileHeader.manufacturerId;
  loadedOtaImageId.imageTypeId = loadedOTAFileHeader.imageTypeId;
  loadedOtaImageId.firmwareVersion = loadedOTAFileHeader.firmwareVersion;

  status = emberAfOtaServerSendImageNotifyCallback(shortAddress,
                                                   endpoint,
                                                   0x03,//payloadType,
                                                   60,//jitter,
                                                   &loadedOtaImageId);

#endif /* EZSP_HOST */
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Ota Server activate",
                     status);
}

// plugin test-harness z3 ota-server deactivate
void emAfPluginTestHarnessZ3OtaServerDeActivateCommand(void)
{
  EmberStatus status = EMBER_INVALID_CALL;
#ifndef EZSP_HOST
  EmberAfOtaImageId InvalidId = {
    INVALID_MANUFACTURER_ID,
    INVALID_DEVICE_ID,
    INVALID_FIRMWARE_VERSION,
    INVALID_EUI64,
  };
  loadedOtaImageId = InvalidId;
  status = EMBER_SUCCESS;
#endif /* EZSP_HOST */

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Ota Server Deactivate",
                     status);
}

uint8_t emberAfOtaServerQueryCallback(const EmberAfOtaImageId* currentImageId,
                                      uint16_t* hardwareVersion,
                                      EmberAfOtaImageId* nextUpgradeImageId)
{
  // This function is called by the OTA cluster server to determine what
  // the 'next' version of software is for a particular device requesting
  // a new download image.  The server returns a status code indicating
  // EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE, or EMBER_ZCL_STATUS_SUCCESS
  // (new image is available).  It then also fills in the 'nextUpgradeImageId'
  // structure with the appropriate version.

  // The server can use whatever criteria it wants to dictate what
  // the 'next' version is and if it is currently available.
  // This sample does this based on a global policy value.

  uint8_t status = EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE;
  bool hardwareVersionMismatch = false;
  *nextUpgradeImageId
    = emberAfOtaStorageSearchCallback(currentImageId->manufacturerId,
                                      currentImageId->imageTypeId,
                                      hardwareVersion);

  if (emberAfIsOtaImageIdValid(nextUpgradeImageId)) {
    // We only perform a check if both the query and the
    // file have hardware version(s).  If one or the other doesn't
    // have them, we assume a match is still possible.
    if (hardwareVersion) {
      EmberAfOtaHeader header;
      emberAfOtaStorageGetFullHeaderCallback(nextUpgradeImageId,
                                             &header);
      if (header.fieldControl & HARDWARE_VERSIONS_PRESENT_MASK) {
        if (*hardwareVersion < header.minimumHardwareVersion
            || header.maximumHardwareVersion < *hardwareVersion) {
          hardwareVersionMismatch = true;
        }
      }
    }
    // "!hardwareVersionMismatch" does not mean the hardware
    // versions match.  It just means we don't *disqualify* the image
    // as a potential upgrade candidate because the hardware is out
    // of range.
    if (!hardwareVersionMismatch) {
      status = EMBER_ZCL_STATUS_SUCCESS;
    }
  }
  return status;
}

uint8_t emberAfOtaServerImageBlockRequestCallback(EmberAfImageBlockRequestCallbackStruct* data)
{
  // The Block response is generated from the Zigbee 3.0 Test Tool, hence just
  // return success
  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfOtaServerUpgradeEndRequestCallback(EmberNodeId source,
                                               uint8_t status,
                                               uint32_t* returnValue,
                                               const EmberAfOtaImageId* imageId)
{
  // Response is generated by the Zigbee 3.0 Test Tool
  emberAfSetNoReplyForNextMessage(true);
  return true;
}
