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
 * @brief This file handle how the application can configure and interact with the OTA
 * cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#ifdef HAL_CONFIG
#include "hal-config.h"
#include "ember-hal-config.h"
#endif
#include "ota-client-policy.h"

#ifdef EMBER_AF_PLUGIN_SLOT_MANAGER
 #include "slot-manager.h"
 #define slotManagerBeginImageValidation(...) \
  (void)emberAfPluginSlotManagerImageIsValidReset(__VA_ARGS__)
 #define slotManagerBootSlot(...) emberAfPluginSlotManagerBootSlot(__VA_ARGS__)
#else
 #define slotManagerBeginImageValidation(...)
 #define slotManagerBootSlot(...)
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_CLIENT)

// Right now this sample policy only supports a single set of version
// information for the device, which will be supplied to the OTA cluster in
// order to query and download a new image when it is available.  This does not
// support multiple products with multiple download images.

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_INCLUDE_HARDWARE_VERSION)
  #define HARDWARE_VERSION EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_HARDWARE_VERSION
#else
  #define HARDWARE_VERSION EMBER_AF_INVALID_HARDWARE_VERSION
#endif

//------------------------------------------------------------------------------
// Globals
EmberAfImageVerifyStatus slotManagerContinueImageValidation();

//------------------------------------------------------------------------------

void emberAfOtaClientVersionInfoCallback(EmberAfOtaImageId* currentImageInfo,
                                         uint16_t* hardwareVersion)
{
  // This callback is fired when a new query and download is initiated.
  // The application will fill in the currentImageInfo with their manufacturer
  // ID, image type ID, and current software version number to use in that
  // query. The deviceSpecificFileEui64 can be ignored.

  // It may be necessary to dynamically determine this data by talking to
  // another device, as is the case with a host talking to an NCP device.

  // The OTA client plugin will cache the data returned by this callback
  // and use it for the subsequent transaction, which could be a query
  // or a query and download.  Therefore it is possible to instruct the
  // OTA client cluster code to query about multiple images by returning
  // different values.

  MEMSET(currentImageInfo, 0, sizeof(EmberAfOtaImageId));
  currentImageInfo->manufacturerId  = EMBER_AF_MANUFACTURER_CODE;
  currentImageInfo->imageTypeId     = EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_IMAGE_TYPE_ID;
  currentImageInfo->firmwareVersion = EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION;

  if (hardwareVersion != NULL) {
    *hardwareVersion = HARDWARE_VERSION;
  }
}

EmberAfImageVerifyStatus emberAfOtaClientCustomVerifyCallback(bool newVerification,
                                                              const EmberAfOtaImageId* id)
{
  // Manufacturing specific checks can be made to the image in this function to
  // determine if it is valid.  This function is called AFTER cryptographic
  // checks have passed.  If the cryptographic checks failed, this function will
  // never be called.

  // The function shall return one of the following based on its own
  // verification process.
  // 1) EMBER_AF_IMAGE_GOOD - the image has passed all checks
  // 2) EMBER_AF_IMAGE_BAD  - the image is not valid
  // 3) EMBER_AF_IMAGE_VERIFY_IN_PROGRESS - the image is valid so far, but more
  //      checks are needed.  This callback shall be re-executed later to
  //      continue verification.  This allows other code in the framework to run.

  // Note that EBL verification is an SoC-only feature.

#if !defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_EBL_VERIFICATION)
  uint16_t pages;
  EmberAfImageVerifyStatus status;
  uint32_t slot;

  // If we're using slots, we'll need to call a different set of APIs
  slot = emAfOtaStorageGetSlot();

  // For sleepies, we must re-initalize the EEPROM / bootloader
  // after each nap/hibernate.  This call will only re-initalize the EEPROM
  // if that is the case.
  emberAfEepromInitCallback();

  if (newVerification) {
    otaPrintln("Starting EBL verification");
    if (INVALID_SLOT != slot) {
      slotManagerBeginImageValidation(slot);
    } else {
      halAppBootloaderImageIsValidReset();
    }
  }

  if (INVALID_SLOT != slot) {
    status = slotManagerContinueImageValidation();
    if (status == EMBER_AF_IMAGE_VERIFY_IN_PROGRESS) {
      return status;
    } else if (status == EMBER_AF_IMAGE_BAD) {
      otaPrintln("EBL failed verification.");
      return status;
    }
  } else {
    pages = halAppBootloaderImageIsValid();
    if (pages == BL_IMAGE_IS_VALID_CONTINUE) {
      return EMBER_AF_IMAGE_VERIFY_IN_PROGRESS;
    } else if (pages == 0) {
      otaPrintln("EBL failed verification.");
      return EMBER_AF_IMAGE_BAD;
    }
  }
  otaPrintln("EBL passed verification.");
#endif

  return EMBER_AF_IMAGE_GOOD;
}

bool emberAfOtaClientDownloadCompleteCallback(EmberAfOtaDownloadResult result,
                                              const EmberAfOtaImageId* id)
{
  // At this point the image has been completely downloaded and cryptographic
  // checks (if applicable) have been performed.
  // Manufacturer verification callback has also been called and passed.

  if (result != EMBER_AF_OTA_DOWNLOAD_AND_VERIFY_SUCCESS) {
    emberAfOtaBootloadClusterPrintln("Download failed.");

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_DELETE_FAILED_DOWNLOADS)
    if (result != EMBER_AF_OTA_ERASE_FAILED) {
      EmberAfOtaStorageStatus status = emberAfOtaStorageClearTempDataCallback();
      emberAfOtaBootloadClusterPrintln("Deleting failed download, status: 0x%X",
                                       status);
    }
#endif

    return true;   // return value is ignored
  }

  // If the client wants to abort for some reason then it can do so here
  // and return false.  Otherwise it should give the "go ahead" by returning
  // true.

  return true;
}

void emberAfOtaClientBootloadCallback(const EmberAfOtaImageId* id)
{
  // OTA Server has told us to bootload.
  // Any final preperation prior to the bootload should be done here.
  // It is assumed that the device will reset in most all cases.

  uint32_t offset;
  uint32_t endOffset;
  uint32_t slot;

  if (EMBER_AF_OTA_STORAGE_SUCCESS
      != emAfOtaStorageGetTagOffsetAndSize(id,
                                           OTA_TAG_UPGRADE_IMAGE,
                                           &offset,
                                           &endOffset)) {
    emberAfCoreFlush();
    otaPrintln("Image does not contain an Upgrade Image Tag (0x%2X). Skipping "
               "upgrade.", OTA_TAG_UPGRADE_IMAGE);
    return;
  }

  otaPrintln("Executing bootload callback.");
  emberSerialWaitSend(APP_SERIAL);

  // If we're using slots, we'll need to use a different set of APIs
  slot = emAfOtaStorageGetSlot();

  // These routines will NOT return unless we failed to launch the bootloader.
  if (INVALID_SLOT != slot) {
    slotManagerBootSlot(slot);
  } else {
    emberAfOtaBootloadCallback(id, OTA_TAG_UPGRADE_IMAGE);
  }
}

EmberAfImageVerifyStatus slotManagerContinueImageValidation()
{
#ifdef EMBER_AF_PLUGIN_SLOT_MANAGER
  uint8_t slotManagerResult;

  slotManagerResult = emberAfPluginSlotManagerImageIsValid();

  if (slotManagerResult == SLOT_MANAGER_CONTINUE) {
    return EMBER_AF_IMAGE_VERIFY_IN_PROGRESS;
  } else if (slotManagerResult != SLOT_MANAGER_SUCCESS) {
    return EMBER_AF_IMAGE_BAD;
  } else {
    return EMBER_AF_IMAGE_GOOD;
  }
#else
  return EMBER_AF_IMAGE_BAD;
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER
}

#endif // defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_CLIENT)
