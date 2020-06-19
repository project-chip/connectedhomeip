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
 * @brief A sample policy file that implements the callbacks for the
 * Zigbee Over-the-air bootload cluster server.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-server/ota-server.h"
#include "app/framework/plugin/ota-server/ota-server-dynamic-block-period.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

typedef enum {
  UPGRADE_IF_SERVER_HAS_NEWER = 0,
  DOWNGRADE_IF_SERVER_HAS_OLDER = 1,
  REINSTALL_IF_SERVER_HAS_SAME = 2,
  NO_NEXT_VERSION = 3,
} NextVersionPolicy;
#define QUERY_POLICY_MAX NO_NEXT_VERSION

#ifdef EMBER_COMMAND_INTEPRETER_HAS_DESCRIPTION_FIELD
static const char * nextVersionPolicyStrings[] = {
  "Upgrade if server has newer",
  "Downgrade if server has older",
  "Reinstall if server has same",
  "No next version",
};
#endif // EMBER_COMMAND_INTEPRETER_HAS_DESCRIPTION_FIELD

static NextVersionPolicy nextVersionPolicy = UPGRADE_IF_SERVER_HAS_NEWER;

// Image Block Response Message Format
// Status Code: 1-byte
// Manuf Code:  2-bytes
// Image Type:  2-bytes
// File Ver:    4-bytes
// File Offset: 4-bytes
// Data Size:   1-byte
// Data:        variable
#define IMAGE_BLOCK_RESPONSE_OVERHEAD (EMBER_AF_ZCL_OVERHEAD + 14)

#if defined EM_AF_TEST_HARNESS_CODE
typedef enum {
  SEND_BLOCK = 0,
  DELAY_DOWNLOAD_ONCE = 1,
  ABORT_DOWNLOAD = 2,
} ImageBlockRequestPolicy;
#define BLOCK_REQUEST_POLICY_MAX ABORT_DOWNLOAD

static ImageBlockRequestPolicy imageBlockRequestPolicy = SEND_BLOCK;

const char * imageBlockRequestPolicyStrings[] = {
  "Send block",
  "Delay download once",
  "Abort download",
};

#define IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS (2 * 60)
#endif

typedef enum {
  UPGRADE_NOW = 0,
  UPGRADE_SOON = 1,
  UPGRADE_ASK_ME_LATER = 2,
  UPGRADE_ABORT = 3,
} UpgradePolicy;
#define UPGRADE_POLICY_MAX  UPGRADE_ABORT

UpgradePolicy upgradePolicy = UPGRADE_NOW;
#define UPGRADE_SOON_TIME_SECONDS (2 * 60)

const char * upgradePolicyStrings[] = {
  "Upgrade Now",
  "Upgrade In a few minutes",
  "Ask me later to upgrade",
  "Abort upgrade",
};

// This corresponds to the enumerated UpgradePolicy list.
const uint32_t upgradeTimes[] = {
  0,                            // Now
  UPGRADE_SOON_TIME_SECONDS,    // in a little while
  0xFFFFFFFFL,                  // go ask your father (wait forever)
  0,                            // unused
};

#if defined EM_AF_TEST_HARNESS_CODE
static uint16_t missedBlockModulus = 0;
#endif

#if defined(EMBER_AF_PLUGIN_OTA_SERVER_PAGE_REQUEST_SUPPORT)
  #define PAGE_REQUEST_STATUS_CODE EMBER_ZCL_STATUS_SUCCESS
  #define PAGE_REQUEST_COMPILE_TIME_SUPPORT "yes"
#else
  #define PAGE_REQUEST_STATUS_CODE EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND
  #define PAGE_REQUEST_COMPILE_TIME_SUPPORT "no"
#endif

static uint8_t pageRequestStatus = PAGE_REQUEST_STATUS_CODE;

static uint16_t otaMinimumBlockPeriodMs = 0;

#define SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST                   \
  (EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_CLIENT \
   | EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_SERVER)

#ifdef EMBER_TEST
uint8_t testClientDelayUnit = OTA_SERVER_DO_NOT_OVERRIDE_CLIENT_DELAY_UNITS;
#endif // EMBER_TEST

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void emAfOtaServerPolicyPrint(void)
{
  otaPrintln("OTA Server Policies");
  otaPrintln("Query Policy: %p (%d)",
             nextVersionPolicyStrings[nextVersionPolicy],
             nextVersionPolicy);

#if defined EM_AF_TEST_HARNESS_CODE
  otaPrintln("Image Block Request Policy: %p (%d)",
             imageBlockRequestPolicyStrings[imageBlockRequestPolicy],
             imageBlockRequestPolicy);
  if (imageBlockRequestPolicy == DELAY_DOWNLOAD_ONCE) {
    otaPrintln("  Delay time: %d seconds",
               IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS);
  }
  otaPrintln("Page Request Block Missed Modulus: %d", missedBlockModulus);
#else
  otaPrintln("Image Block Request Policy: Send block");
#endif
  otaPrintln("Image Block Request Min Period: %d milliseconds",
             otaMinimumBlockPeriodMs);
  emberAfCoreFlush();

  otaPrintln("Page Request Code Compiled in: %p",
             PAGE_REQUEST_COMPILE_TIME_SUPPORT);
  otaPrintln("Page Request Runtime Status Code: 0x%X",
             pageRequestStatus);
  emberAfCoreFlush();

  otaPrintln("Upgrade Request Policy: %p (%d)",
             upgradePolicyStrings[upgradePolicy],
             upgradePolicy);
  if (upgradePolicy == UPGRADE_SOON) {
    otaPrintln("  (%d seconds)", UPGRADE_SOON_TIME_SECONDS);
  }
  emberAfCoreFlush();
}

static bool determineNextSoftwareVersion(uint32_t versionServerHas,
                                         uint32_t versionClientHas)
{
  // Our system here controls whether we tell the client to
  // (A) upgrade, because we have a newer version
  // (B) downgrade, because we have an older version we want to install
  // (C) reinstall, because we have the same version you have currently
  // (D) do nothing (no 'next' image is avaiable)

  switch (nextVersionPolicy) {
    case UPGRADE_IF_SERVER_HAS_NEWER:
      if (versionServerHas > versionClientHas) {
        return true;
      }
      break;
    case DOWNGRADE_IF_SERVER_HAS_OLDER:
      if (versionServerHas < versionClientHas) {
        return true;
      }
      break;
    case REINSTALL_IF_SERVER_HAS_SAME:
      if (versionServerHas == versionClientHas) {
        return true;
      }
      break;
    case NO_NEXT_VERSION:
    default:
      break;
  }
  return false;
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

  otaPrintln("QueryNextImageRequest mfgId:0x%2x imageTypeId:0x%2x, fw:0x%4x",
             currentImageId->manufacturerId,
             currentImageId->imageTypeId,
             currentImageId->firmwareVersion);
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
          otaPrintln("Hardware version 0x%02X does not fall within the min (0x%02X) and max (0x%02X) hardware versions in the file.",
                     *hardwareVersion,
                     header.minimumHardwareVersion,
                     header.maximumHardwareVersion);
          hardwareVersionMismatch = true;
        }
      }
    }
    // "!hardwareVersionMismatch" does not mean the hardware
    // versions match.  It just means we don't *disqualify* the image
    // as a potential upgrade candidate because the hardware is out
    // of range.
    if (!hardwareVersionMismatch) {
      status = (determineNextSoftwareVersion(nextUpgradeImageId->firmwareVersion,
                                             currentImageId->firmwareVersion)
                ? EMBER_ZCL_STATUS_SUCCESS
                : EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        otaPrintln("Next fw version is: 0x%4X",
                   nextUpgradeImageId->firmwareVersion);
      }
    }
  }
  return status;
}

uint8_t emberAfOtaServerBlockSizeCallback(EmberNodeId clientNodeId)
{
  // This function provides a way for the server to potentially
  // adjust the block size based on the client who is requesting.
  // In other words if we are using source routing we will limit
  // data returned by enough to put a source route into the message.

  EmberApsFrame apsFrame;
  uint8_t maxSize;
  apsFrame.options = EMBER_APS_OPTION_NONE;

  if (emberAfIsCurrentSecurityProfileSmartEnergy()) {
    apsFrame.options |= EMBER_APS_OPTION_ENCRYPTION;
  }

  maxSize = emberAfMaximumApsPayloadLength(EMBER_OUTGOING_DIRECT,
                                           clientNodeId,
                                           &apsFrame);
  maxSize -= IMAGE_BLOCK_RESPONSE_OVERHEAD;
  return maxSize;
}

uint8_t emberAfOtaServerImageBlockRequestCallback(EmberAfImageBlockRequestCallbackStruct* data)
{
  uint8_t status;
  uint16_t serverBlockPeriodValue = otaMinimumBlockPeriodMs;
  uint8_t clientDelayUnit;
  bool useSecondsDelay = false;

  if (SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST
      == (data->bitmask & SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST)) {
    clientDelayUnit = emberAfPluginOtaServerPolicyGetClientDelayUnits(
      data->source,
      data->sourceEui);

#ifdef EMBER_TEST
    if (OTA_SERVER_DO_NOT_OVERRIDE_CLIENT_DELAY_UNITS != testClientDelayUnit) {
      clientDelayUnit = testClientDelayUnit;
    }
#endif // EMBER_TEST

    switch (clientDelayUnit) {
      case OTA_SERVER_NO_RATE_LIMITING_FOR_CLIENT:
        return EMBER_ZCL_STATUS_SUCCESS;
        break;

      case OTA_SERVER_CLIENT_USES_MILLISECONDS:
        break;

      case OTA_SERVER_CLIENT_USES_SECONDS:
        useSecondsDelay = true;
        break;

      case OTA_SERVER_DISCOVER_CLIENT_DELAY_UNITS:
      {
        // If we support dynamic block request, check to see if we know how the
        // client treats the field. If we haven't tested the client yet or don't
        // have a spot for it as an active OTA session, we tell it to
        // WAIT_FOR_DATA
        status = emAfOtaServerCheckDynamicBlockPeriodDownload(data);
        if (EMBER_ZCL_STATUS_WAIT_FOR_DATA == status) {
          return EMBER_ZCL_STATUS_WAIT_FOR_DATA;
        }

        useSecondsDelay = emAfOtaServerDynamicBlockPeriodClientUsesSeconds(data->source);
      }
      break;

      default:
        break;
    }

    if (useSecondsDelay) {
      serverBlockPeriodValue /= 1000;
    }

    if (data->minBlockRequestPeriod != serverBlockPeriodValue) {
      data->minBlockRequestPeriod = serverBlockPeriodValue;
      return EMBER_ZCL_STATUS_WAIT_FOR_DATA;
    }
  }

#if defined EM_AF_TEST_HARNESS_CODE
  // TEST Harness code
  // This will artificially delay once or abort the download as a
  // demonstration.  The test cases use this.

  if (imageBlockRequestPolicy == DELAY_DOWNLOAD_ONCE) {
    data->waitTimeSecondsResponse = IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS;
    imageBlockRequestPolicy = SEND_BLOCK;
    return EMBER_ZCL_STATUS_WAIT_FOR_DATA;
  } else if (data->offset > 0 && imageBlockRequestPolicy == ABORT_DOWNLOAD) {
    // Only abort after the first block to insure the client handles
    // this correctly.
    return EMBER_ZCL_STATUS_ABORT;
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfOtaServerUpgradeEndRequestCallback(EmberNodeId source,
                                               uint8_t status,
                                               uint32_t* returnValue,
                                               const EmberAfOtaImageId* imageId)
{
  otaPrintln("Client 0x%2X indicated upgrade status: 0x%X",
             source,
             status);

  emAfOtaServerCompleteDynamicBlockPeriodDownload(source);

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    // If status != EMBER_ZCL_STATUS_SUCCESS then this callback is
    // only informative.  Return code will be ignored.
    return false;
  }

  otaPrintln("Upgrade End Response: %p", upgradePolicyStrings[upgradePolicy]);

  if (upgradePolicy == UPGRADE_ABORT) {
    return false;
  }

  *returnValue = upgradeTimes[upgradePolicy];
  return true;
}

void emAfOtaServerSetQueryPolicy(uint8_t value)
{
  if (value <= QUERY_POLICY_MAX) {
    nextVersionPolicy = (NextVersionPolicy)value;
  }
}

void emAfOtaServerSetBlockRequestPolicy(uint8_t value)
{
#if defined EM_AF_TEST_HARNESS_CODE
  if (value <= BLOCK_REQUEST_POLICY_MAX) {
    imageBlockRequestPolicy = (ImageBlockRequestPolicy)value;
  }
#else
  otaPrintln("Unsupported.");
#endif
}

uint8_t emberAfOtaPageRequestServerPolicyCallback(void)
{
  return pageRequestStatus;
}

void emAfOtaServerSetPageRequestPolicy(uint8_t value)
{
  // This allows test code to be compiled with support for page request but
  // tell requesting devices it doesn't support it.
  pageRequestStatus = (value
                       ? EMBER_ZCL_STATUS_SUCCESS
                       : EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emAfOtaServerSetUpgradePolicy(uint8_t value)
{
  if (value <= UPGRADE_POLICY_MAX) {
    upgradePolicy = (UpgradePolicy)value;
  }
}

#if defined EM_AF_TEST_HARNESS_CODE

// There is no reason in production why certain block responses
// for a page request would need a callback, so that's why we wrap
// this whole function in a #define.
bool emAfServerPageRequestTickCallback(uint16_t relativeOffset,
                                       uint8_t dataSize)
{
  uint16_t block = (relativeOffset + dataSize) / dataSize;
  emberAfCoreFlush();
  if (missedBlockModulus
      && (block % missedBlockModulus == 0)) {
    emberAfCoreFlush();
    otaPrintln("Inducing artificial failure for block %d", block);
    return false;
  }
  return true;
}

#endif

void emAfSetPageRequestMissedBlockModulus(uint16_t modulus)
{
#if defined(EM_AF_TEST_HARNESS_CODE)
  missedBlockModulus = modulus;
#else
  otaPrintln("Unsupported.");
#endif
}

void emAfOtaServerPolicySetMinBlockRequestPeriod(uint16_t minBlockPeriodMs)
{
  otaMinimumBlockPeriodMs = minBlockPeriodMs;
}
