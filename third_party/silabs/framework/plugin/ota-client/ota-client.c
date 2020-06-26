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
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading device specific file.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/ota-common/ota.h"

#include "app/framework/util/af-main.h"

#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#include "app/framework/plugin/ota-client-policy/ota-client-policy.h"
#include "ota-client-signature-verify.h"
#include "ota-client-page-request.h"
#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"

#if defined(EZSP_HOST)
// For emberIeeeAddressRequest()
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else
  #include "stack/include/ember.h"
#endif

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_SET_IMAGE_STAMP)            \
  && !defined(EZSP_HOST)                                           \
  && defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_IMAGE_STAMP_ATTRIBUTE) \
  && !defined(EMBER_TEST)

    #define USE_IMAGE_STAMP
    #include "hal/micro/cortexm3/memmap.h"
    #include "hal/micro/cortexm3/memmap-tables.h"
#endif

//------------------------------------------------------------------------------
// Globals

enum BootloadState{
  BOOTLOAD_STATE_NONE,
  BOOTLOAD_STATE_DELAY,
  BOOTLOAD_STATE_DISCOVER_SERVER,
  BOOTLOAD_STATE_GET_SERVER_EUI,
  BOOTLOAD_STATE_OBTAIN_LINK_KEY,
  BOOTLOAD_STATE_QUERY_NEXT_IMAGE,
  BOOTLOAD_STATE_DOWNLOAD,
  BOOTLOAD_STATE_VERIFY_IMAGE,
  BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE,
  BOOTLOAD_STATE_COUNTDOWN_TO_UPGRADE,
  BOOTLOAD_STATE_UPGRADE_VIA_OUT_OF_BAND,
};
typedef uint8_t BootloadState;

static const char * bootloadStateNames[] = {
  "None",
  "Random Delay before start",
  "Discovering OTA Server",
  "Get OTA Server EUI",
  "Obtain link key",
  "Querying Next Image",
  "Downloading Image",
  "Verifying Image",
  "Waiting for Upgrade message",
  "Countdown to Upgrade",
  "Waiting to upgrade via out-of-band mechanism",
};

// This relates the bootload state above to the status that is externally
// reported via the attribute.
static const uint8_t bootloadStateToExternalState[] = {
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_NORMAL,
  OTA_UPGRADE_STATUS_DOWNLOAD_IN_PROGRESS,
  OTA_UPGRADE_STATUS_DOWNLOAD_COMPLETE,
  OTA_UPGRADE_STATUS_WAIT,
  OTA_UPGRADE_STATUS_COUNTDOWN,
  OTA_UPGRADE_STATUS_WAIT_TO_UPGRADE_VIA_OUT_OF_BAND,
};

#define UNDEFINED_ENDPOINT 0xFF

static BootloadState currentBootloadState = BOOTLOAD_STATE_NONE;

static uint8_t myEndpoint = UNDEFINED_ENDPOINT;
static uint8_t serverEndpoint = UNDEFINED_ENDPOINT;
static EmberNodeId serverNodeId = EMBER_UNKNOWN_NODE_ID;
static uint8_t errors = 0;
static uint32_t totalImageSize = 0;
static EmberAfOtaImageId currentDownloadFile;
static uint16_t hardwareVersion;

#define WAIT_FOR_UPGRADE_MESSAGE 0xFFFFFFFF

#define ZCL_COMMAND_ID_INDEX 2

#define IMAGE_NOTIFY_NO_PAYLOAD              0
#define IMAGE_NOTIFY_MANUFACTURER_ONLY_TYPE  1
#define IMAGE_NOTIFY_MFG_AND_IMAGE_TYPE      2
#define IMAGE_NOTIFY_FULL_VERSION_TYPE       3
#define IMAGE_NOTIFY_LAST_VALID_TYPE         IMAGE_NOTIFY_FULL_VERSION_TYPE

// These lengths correspond to the #defines above.
static const uint8_t imageNotifyPayloadLengths[] = {
  (EMBER_AF_ZCL_OVERHEAD + 2),  // Payload and Jitter only.
  (EMBER_AF_ZCL_OVERHEAD + 4),  // and MFG ID
  (EMBER_AF_ZCL_OVERHEAD + 6),  // and Image Type ID
  (EMBER_AF_ZCL_OVERHEAD + 10), // and Version
};

// Values per the spec.
#define IMAGE_BLOCK_RESPONSE_SUCCESS_MIN_LENGTH      (EMBER_AF_ZCL_OVERHEAD + 14)
#define UPGRADE_END_RESPONSE_MIN_LENGTH              (EMBER_AF_ZCL_OVERHEAD + 16)
#define QUERY_NEXT_IMAGE_SUCCESS_RESPONSE_MIN_LENGTH (EMBER_AF_ZCL_OVERHEAD + 13)

static bool waitingForResponse = false;
static uint32_t nextEventTimer;

#define WAITING_FOR_MESSAGE      true
#define NO_MESSAGE_RESPONSE_WAIT false

// Maximum amount of data that the client is willing to accept in one packet
// The server can always give us less data (sizing the response based on its
// send parameters, eg. source routing).  But it can't give us more.
// Since no
#define MAX_CLIENT_DATA_SIZE      63   // in bytes

// We only support the Zigbbe Pro stack version.
#define STACK_VERSION ZIGBEE_PRO_STACK_VERSION

// Handy defines to make the code more readable.
#define TIMEOUT_REACHED  true
#define START_NEW_TIMER  false

// queryNextImage() defines
typedef enum {
  DELAY_BEFORE_SEND_QUERY = 0,
  SEND_QUERY_NOW = 1,
} SendNextQueryState;

// This defines how long to wait for a message response before considering it
// an error.  It also defines how long a sleepy stays awake waiting for
// a message.
#if EMBER_AF_DISCOVERY_TIMEOUT_QS >= 3 * 4
#define MESSAGE_TIMEOUT_MS ((EMBER_AF_DISCOVERY_TIMEOUT_QS * (MILLISECOND_TICKS_PER_SECOND / 4)) + 100)
#else
#define MESSAGE_TIMEOUT_MS 3000L
#endif

#define IMAGE_BLOCK_ABORT_LENGTH                   (EMBER_AF_ZCL_OVERHEAD + 1)
#define IMAGE_BLOCK_RESPONSE_SUCCESS_MIN_LENGTH    (EMBER_AF_ZCL_OVERHEAD + 14)

// This is the length WITHOUT the Minimum Block Request Period
#define IMAGE_BLOCK_RESPONSE_WAIT_FOR_DATA_LENGTH_MIN  (EMBER_AF_ZCL_OVERHEAD + 8)

#define IMAGE_BLOCK_RESPONSE_WAIT_FOR_DATA_MIN_BLOCK_RESPONSE_LENGTH \
  (IMAGE_BLOCK_RESPONSE_WAIT_FOR_DATA_LENGTH_MIN + 2)

// Maximum wait time that the client would wait to retrive data or to be
// upgraded. If the server is not ready for the operation, it should tell the
// client to wait (again).  But the client would only wait up to the value
// defined below.  Regarding upgrading, current OTA spec recommends that the
// client should query the server every 60 mins even though it is told to wait
// longer.
#define TIMEOUT_MAX_WAIT_TIME_MS   (60 * MINUTES_IN_MS)

// If the server sent us an invalid delay time for the an image block
// download, this is how long we will delay before getting the
// next block.
#define CALCULATE_TIME_ERROR_IMAGE_BLOCK_DELAY_MS (5 * MINUTES_IN_MS)

// If the server sent us an invalid delay time for the upgrade end response,
// we use this value for the next request.
#define CALCULATE_TIME_ERROR_UPGRADE_END_RESPONSE_DELAY_MS (1 * HOURS_IN_MS)

// When the server asks us to wait indefinitely to apply an upgrade,
// we will ask them again in this much time.
#define WAIT_FOR_UPGRADE_DELAY_MS (1 * HOURS_IN_MS)

// When the Bootloading cluster's activation policy is set to activate via out-
// of-band means only, this is the amount of time we wait for the out-of-band
// action to happen before we complain
#define WAIT_FOR_OUT_OF_BAND_ACTIVATION_MS  (1 * MINUTES_IN_MS)

// How often a print is done indicating client download progress.
// e.g. "Download 5% complete"
#define DOWNLOAD_PERCENTAGE_UPDATE_RATE 5

#define MESSAGE_TIMEOUT_BACKOFF_MS (2 * SECONDS_IN_MS)

// Even when we are told to upgrade immediately, we want to insure there
// is a chance for the APS retries and ZCL response to get back to their
// senders.
#define IMMEDIATE_UPGRADE_DELAY_MS (3 * SECONDS_IN_MS)

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_USE_PAGE_REQUEST)
  #define USE_PAGE_REQUEST_DEFAULT true
#else
  #define USE_PAGE_REQUEST_DEFAULT false
#endif

// HA 1.2 feature
// Updated in ZSE 1.2b to be in seconds rather than milliseconds
// The unit of this variable is dictated by MIN_BLOCK_PERIOD_UNITS
static uint16_t minBlockRequestPeriod;

// This is not a CONST because even if the client supports page request,
// the server may not.  So we will dynamically turn off sending page requests
// when it is enabled and the server doesn't support it.
static bool usePageRequest = USE_PAGE_REQUEST_DEFAULT;

typedef enum {
  NO_CUSTOM_VERIFY,
  NEW_CUSTOM_VERIFY,
  CUSTOM_VERIFY_IN_PROGRESS,
} CustomVerifyStatus;
static CustomVerifyStatus customVerifyStatus = NO_CUSTOM_VERIFY;

// TODO: need to gate this when we're better equipped to do so
// with the generated CLI
//#if defined(EMBER_TEST)
// If set to 100, will not stop download.
uint8_t emAfOtaClientStopDownloadPercentage = 100;
//#endif

// The spec says pick a random number up to 5 minutes.  We limit
// ourselves to 4.25 minutes to make it easier to mask out the upper
// bits and limit the number retrieved from emberGetPseudoRandomNumber()
#define MAXIMUM_RANDOM_DELAY_SECONDS_MASK 0x00FF

typedef enum {
  SERVER_ACTIVATION,      // (D) The server can tell us to activate new image
  OUT_OF_BAND_ACTIVATION, // Image activation allowed only by out-of-band method
} UpgradeActivationPolicy;

typedef enum {
  UPGRADE_AFTER_TIMEOUT,    // (D) Activate the image after timeout
  NO_UPGRADE_AFTER_TIMEOUT, // Don't activate after timeout
} UpgradeTimeoutPolicy;

// The Minimum Block Period attribute's unit has gone from milliseconds to
// seconds and back to milliseconds. While the timelines would suggest that
// SE1.2b uses seconds and everything else uses milliseconds, the workgroup has
// determined that the OTA ClusterRevision determines if the client is using
// milliseconds or seconds (revision 2 for seconds, 1 and 3 for milliseconds)

// NOTE: clients who desire to treat the field as seconds should set the
// appropriate OTA client cluster revision when configuring the application
typedef enum {
  DISABLE_FEATURE = 0,
  MILLISECONDS    = 1,              // (D)
  SECONDS         = SECONDS_IN_MS,
} MinimumBlockPeriodUnit;

#ifdef EMBER_TEST_BLOCK_PERIOD_IN_SECONDS
  #undef EMBER_AF_PLUGIN_OTA_CLIENT_MIN_BLOCK_PERIOD_UNITS
  #define EMBER_AF_PLUGIN_OTA_CLIENT_MIN_BLOCK_PERIOD_UNITS SECONDS
#endif // EMBER_TEST_BLOCK_PERIOD_IN_SECONDS

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_MIN_BLOCK_PERIOD_UNITS)
  #define MIN_BLOCK_PERIOD_UNITS EMBER_AF_PLUGIN_OTA_CLIENT_MIN_BLOCK_PERIOD_UNITS
#else
  #define MIN_BLOCK_PERIOD_UNITS MILLISECONDS
#endif

#if defined(EMBER_TEST_OTA_CLIENT_IGNORE_NON_TRUST_CENTERS) \
  || defined(EMBER_AF_PLUGIN_OTA_CLIENT_IGNORE_NON_TRUST_CENTER)
static bool ignoreNonTrustCenter = true;
#else
static bool ignoreNonTrustCenter = false;
#endif

static bool disableOtaDowngrades = false;

//------------------------------------------------------------------------------
// Forward Declarations

static void recordUpgradeStatus(BootloadState state);
static void putImageInfoInMessage(void);
static void startServerDiscovery(void);
static void euiLookupErrorOccurred(void);
static void euiLookup(void);
static void queryNextImage(SendNextQueryState nextQuery,
                           bool queryErrorOccurred);
static void continueImageDownload(void);
static void continueImageVerification(EmberAfImageVerifyStatus status);
static void askServerToRunUpgrade(bool timeout);
static void runUpgrade(void);
static uint32_t updateCurrentOffset(uint32_t currentOffset);
static void updateDownloadFileVersion(uint32_t version);
static bool downloadAndVerifyFinish(EmberAfOtaDownloadResult result);
static void determineNextState(void);
static EmberAfStatus imageNotifyParse(bool broadcast,
                                      uint8_t* buffer,
                                      uint8_t index,
                                      uint8_t length);
static EmberAfStatus queryNextImageResponseParse(uint8_t* buffer,
                                                 uint8_t index,
                                                 uint8_t length);
static EmberAfStatus imageBlockResponseParse(uint8_t* buffer,
                                             uint8_t index,
                                             uint8_t length);
static EmberAfStatus upgradeEndResponseParse(uint8_t status,
                                             uint8_t* buffer,
                                             uint8_t length);
static bool calculateTimer(uint32_t currentTime,
                           uint32_t targetTime,
                           uint32_t* returnTime);

static UpgradeActivationPolicy getUpgradeActivationPolicy();
static UpgradeTimeoutPolicy getUpgradeTimeoutPolicy();

static UpgradeActivationPolicy getUpgradeActivationPolicy();
static UpgradeTimeoutPolicy getUpgradeTimeoutPolicy();
static void sendMessage(uint8_t cmdId,
                        uint8_t upgradeEndStatus,
                        uint32_t timer);
static void updateDownloadedZigbeeStackVersionAttribute(void);
static void updateImageTypeIdAttribute(uint16_t imageTypeId);

//------------------------------------------------------------------------------

static void updateCurrentImageAttributes(EmberAfOtaImageId* imageId)
{
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_MANUFACTURER_ID_ATTRIBUTE)
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_MANUFACTURER_ID_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&(imageId->manufacturerId),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
#endif

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_IMAGE_TYPE_ID_ATTRIBUTE)
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_IMAGE_TYPE_ID_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&(imageId->imageTypeId),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
#endif

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_CURRENT_FILE_VERSION_ATTRIBUTE)
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_CURRENT_FILE_VERSION_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&(imageId->firmwareVersion),
                        ZCL_INT32U_ATTRIBUTE_TYPE);
#endif
}

static void updateMinBlockRequestPeriodAttribute(uint16_t value)
{
  minBlockRequestPeriod = value;

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_MINIMUM_BLOCK_REQUEST_PERIOD_ATTRIBUTE)
  {
    emberAfWriteAttribute(myEndpoint,
                          ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                          ZCL_MINIMUM_BLOCK_REQUEST_PERIOD_ATTRIBUTE_ID,
                          CLUSTER_MASK_CLIENT,
                          (uint8_t*)&minBlockRequestPeriod,
                          ZCL_INT16U_ATTRIBUTE_TYPE);
  }
#endif
}

void emberAfOtaBootloadClusterClientInitCallback(uint8_t endpoint)
{
  if (myEndpoint != UNDEFINED_ENDPOINT) {
    // We have already been initialized
    return;
  }
  myEndpoint = endpoint;

  emberAfOtaStorageInitCallback();

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_CURRENT_ZIGBEE_STACK_VERSION_ATTRIBUTE)
  {
    uint16_t currentZigbeeStackVersion = ZIGBEE_PRO_STACK_VERSION;
    emberAfWriteAttribute(myEndpoint,
                          ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                          ZCL_CURRENT_ZIGBEE_STACK_VERSION_ATTRIBUTE_ID,
                          CLUSTER_MASK_CLIENT,
                          (uint8_t*)&currentZigbeeStackVersion,
                          ZCL_INT16U_ATTRIBUTE_TYPE);
  }
#endif

  // Assume we can download as fast as we want until told otherwise by server.
  updateMinBlockRequestPeriodAttribute(0); // minBlockRequestPeriod.

#if defined(USE_IMAGE_STAMP)
  {
    // Unfortunately the AAT data struct is declared const,
    // so we must copy out the image stamp to satisfy the compiler.
    uint8_t data[IMAGE_STAMP_SIZE];
    MEMMOVE(data, halAppAddressTable.imageStamp, IMAGE_STAMP_SIZE);
    emberAfWriteAttribute(myEndpoint,
                          ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                          ZCL_IMAGE_STAMP_ATTRIBUTE_ID,
                          CLUSTER_MASK_CLIENT,
                          data,
                          ZCL_INT32U_ATTRIBUTE_TYPE);
  }
#endif

  {
    EmberAfOtaImageId id;
    uint16_t hwVer;
    emberAfOtaClientVersionInfoCallback(&id,
                                        &hwVer);
    id.imageTypeId = 0xFFFF;
    updateCurrentImageAttributes(&id);
  }

  updateDownloadedZigbeeStackVersionAttribute();
}

// Returns whether or not a timer has been set.  For timer = 0, will return false
static bool setTimer(uint32_t timeMs)
{
  uint32_t timer;

  // When waiting for page request replies I want to use the timer that is defined
  // in App. Builder and passed through to here since I am expecting multiple
  // messages coming back from the server.
  if (waitingForResponse
      && emAfGetCurrentPageRequestStatus() != EM_AF_WAITING_PAGE_REQUEST_REPLIES) {
    timer = MESSAGE_TIMEOUT_MS;
    nextEventTimer = timeMs;
  } else {
    timer = timeMs;
    nextEventTimer = 0;
  }

  // A timer set at 0 means we are not counting down to some event.
  // We may still be in an active state of waiting, but are waiting
  // for an event to fire rather than a timer to expire.  For example, service
  // discovery will generate a callback when it is complete.  No need for
  // keeping track of time here as well.
  if (timer != 0) {
    //    otaPrintln("Setting timer: 0x%4X ms", timer);
    EmberAfEventPollControl pollControl = EMBER_AF_SHORT_POLL;
    EmberAfEventSleepControl sleepControl = EMBER_AF_OK_TO_SLEEP;
    if (currentBootloadState == BOOTLOAD_STATE_VERIFY_IMAGE) {
      sleepControl = EMBER_AF_STAY_AWAKE;
    } else if (!waitingForResponse
               && (emAfGetCurrentPageRequestStatus()
                   == EM_AF_NO_PAGE_REQUEST)) {
      pollControl = EMBER_AF_LONG_POLL;
    }
    emberAfScheduleClientTickExtended(myEndpoint,
                                      ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                                      timer,
                                      pollControl,
                                      sleepControl);
  }
  return (timer != 0);
}

void emberAfPluginOtaClientStackStatusCallback(EmberStatus status)
{
#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_AUTO_START)
  uint16_t randomDelaySeconds;
  if (status != EMBER_NETWORK_UP
      || currentBootloadState != BOOTLOAD_STATE_NONE) {
    return;
  }
  randomDelaySeconds = (emberGetPseudoRandomNumber()
                        & MAXIMUM_RANDOM_DELAY_SECONDS_MASK);
  // EMAPPFWKV2-1516: make sure we don't have a delay time of 0 seconds.
  if (randomDelaySeconds == 0) {
    randomDelaySeconds = 1;
  }
  recordUpgradeStatus(BOOTLOAD_STATE_DELAY);
  otaPrintln("Delaying %d seconds before starting OTA client", randomDelaySeconds);
  // the setTimer(uint32_t) function wants milliseconds
  setTimer(randomDelaySeconds * MILLISECOND_TICKS_PER_SECOND);
#endif
}

static void otaClientTick(void)
{
  bool errorOccurred = false;
  // Getting here means either we timed out our last operation,
  // or we need to kick off a periodic event.

  emAfPageRequestTimerExpired();

  if (waitingForResponse) {
    otaPrintln("Timeout waiting for message.");
    errorOccurred = true;
    errors++;
    waitingForResponse = false;

    if (currentBootloadState == BOOTLOAD_STATE_DOWNLOAD) {
      // Especially important if the download delay is 0, we want
      // to backoff a little and make sure we don't continue to
      // blast the server with requests.
      setTimer((nextEventTimer > MESSAGE_TIMEOUT_BACKOFF_MS)
               ? nextEventTimer
               : MESSAGE_TIMEOUT_BACKOFF_MS);
      return;
    }
  }

  switch (currentBootloadState) {
    case BOOTLOAD_STATE_DELAY:
      emberAfOtaClientStartCallback();
      break;
    case BOOTLOAD_STATE_DISCOVER_SERVER:
      startServerDiscovery();
      break;
    case BOOTLOAD_STATE_GET_SERVER_EUI:
      euiLookupErrorOccurred();
      break;
    case BOOTLOAD_STATE_QUERY_NEXT_IMAGE:
      queryNextImage(SEND_QUERY_NOW,
                     errorOccurred);
      break;
    case BOOTLOAD_STATE_DOWNLOAD:
      continueImageDownload();
      break;
    case BOOTLOAD_STATE_VERIFY_IMAGE:
      continueImageVerification(EMBER_AF_IMAGE_VERIFY_IN_PROGRESS);
      break;
    case BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE:
      askServerToRunUpgrade(TIMEOUT_REACHED);
      break;
    case BOOTLOAD_STATE_COUNTDOWN_TO_UPGRADE:
      runUpgrade();
    case BOOTLOAD_STATE_UPGRADE_VIA_OUT_OF_BAND:
    // Does nothing right now after timeout
    default:
      // Do nothing.  Invalid state
      break;
  }
}

void emberAfPluginOtaStorageSimpleEepromEraseCompleteCallback(bool success)
{
  if (!success) {
    downloadAndVerifyFinish(EMBER_AF_OTA_ERASE_FAILED);
  } else if (currentBootloadState == BOOTLOAD_STATE_DOWNLOAD) {
    // If an erase was triggered for some other reason besides preparing
    // to start a download, we don't want to trigger the OTA client state
    // machine.
    otaClientTick();
  }
  updateDownloadedZigbeeStackVersionAttribute();
}

void emberAfOtaBootloadClusterClientTickCallback(uint8_t endpoint)
{
  // Since there is only one instance of the OTA bootload cluster allowed,
  // we don't care about the endpoint value.
  otaClientTick();
}

static void restartServerDiscoveryAfterDelay(void)
{
  setTimer(EMBER_AF_OTA_SERVER_DISCOVERY_DELAY_MS);
  recordUpgradeStatus(BOOTLOAD_STATE_DISCOVER_SERVER);
}

#if defined(EMBER_AF_HAS_SECURITY_PROFILE_SE)
static void otaClientPartnerLinkKeyCallback(bool success)
{
  if (success) {
    determineNextState();
    return;
  }

  restartServerDiscoveryAfterDelay();
}
#endif

static void getPartnerLinkKey(void)
{
#if defined(EMBER_AF_HAS_SECURITY_PROFILE_SE)
  EmberEUI64 serverEui64;
  uint8_t i;

  if (!emberAfIsCurrentSecurityProfileSmartEnergy()
      || serverNodeId == EMBER_TRUST_CENTER_NODE_ID) {
    goto partnerLinkKeyDone;
  }

  emberAfReadClientAttribute(myEndpoint,
                             ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                             ZCL_UPGRADE_SERVER_ID_ATTRIBUTE_ID,
                             serverEui64,
                             EUI64_SIZE);

  for (i = 0; i < emberAfGetKeyTableSize(); i++) {
    EmberKeyStruct keyStruct;
    if (EMBER_SUCCESS == emberGetKeyTableEntry(i, &keyStruct)
        && EMBER_APPLICATION_LINK_KEY == keyStruct.type
        && 0 == MEMCOMPARE(keyStruct.partnerEUI64, serverEui64, EUI64_SIZE)) {
      goto partnerLinkKeyDone;
    }
  }

  // Spec is not clear whether the actual key establishment endpoint
  // needs to be used, so we just use 1.
  if (EMBER_SUCCESS
      == emberAfInitiatePartnerLinkKeyExchange(serverNodeId,
                                               1,
                                               otaClientPartnerLinkKeyCallback)) {
    return;
  }

  // If we fail to initiate partner link key, it could be because
  // we don't have any link key spots or the server is no longer online.
  // We just retry the entire operation again.
  restartServerDiscoveryAfterDelay();
  return;

  partnerLinkKeyDone:
#endif

  determineNextState();
  return;
}

static uint32_t getCurrentOffset(void)
{
  uint32_t offset;
  uint8_t dataType = ZCL_INT32U_ATTRIBUTE_TYPE;
  emberAfReadAttribute(myEndpoint,
                       ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                       ZCL_FILE_OFFSET_ATTRIBUTE_ID,
                       CLUSTER_MASK_CLIENT,
                       (uint8_t*)&offset,
                       4,
                       &dataType);
  return offset;
}

static uint32_t updateCurrentOffset(uint32_t currentOffset)
{
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_FILE_OFFSET_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&currentOffset,
                        ZCL_INT32U_ATTRIBUTE_TYPE);
  return currentOffset;
}

static void updateDownloadFileVersion(uint32_t version)
{
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_DOWNLOADED_FILE_VERSION_ATTRIBUTE)
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_DOWNLOADED_FILE_VERSION_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)(&version),
                        ZCL_INT32U_ATTRIBUTE_TYPE);
#endif
  currentDownloadFile.firmwareVersion = version;
}

// It is expected this is called when registration has successfully
// completed.
void emberAfOtaClientStartCallback(void)
{
  // If we are already delaying we allow jumping immediately
  // to discovery and skipping the delay.  This benefits the
  // CLI as well as applications that might expect behavior
  // as in previous releases where discovery started as soon
  // as this API is called.
  if (currentBootloadState == BOOTLOAD_STATE_NONE
      || currentBootloadState == BOOTLOAD_STATE_DELAY
      || currentBootloadState == BOOTLOAD_STATE_DISCOVER_SERVER) {
    startServerDiscovery();
  }
}

void emAfOtaClientStop(void)
{
  downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
  recordUpgradeStatus(BOOTLOAD_STATE_NONE);
  waitingForResponse = false;

  emberAfDeactivateClientTick(myEndpoint, ZCL_OTA_BOOTLOAD_CLUSTER_ID);
}

void emAfOtaClientPrintState(void)
{
  otaPrintln(" State:   %p",
             bootloadStateNames[currentBootloadState]);
  otaPrintln(" Waiting for response: %p",
             (waitingForResponse ? "yes" : "no"));
  if (waitingForResponse) {
    otaPrintln(" Next Event Timer: %d ms", nextEventTimer);
  }
  otaPrintln(" Current Download Offset: 0x%4X (%d%%)",
             getCurrentOffset(),
             emAfCalculatePercentage(getCurrentOffset(),
                                     totalImageSize));
}

static void recordServerEuiAndGoToNextState(EmberEUI64 eui64)
{
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_UPGRADE_SERVER_ID_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        eui64,
                        ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);
  otaPrintln("OTA Cluster: setting IEEE address of OTA cluster");
  getPartnerLinkKey();;
}

void emAfOtaClientServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult *result)
{
  // We only look at the first result.  How multiple OTA servers are handled
  // has not been spelled out by the spec yet, but the spec does say to just
  // latch onto the first one, so we do that.
  const EmberAfEndpointList* epList =
    (const EmberAfEndpointList*)result->responseData;

  waitingForResponse = false;

  // Since the OTA cluster only uses broadcast discoveries for Match descriptor
  // and Unicast discoveries for the IEEE, we can differentiate the request
  // type based on the result.

  if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
    if (serverEndpoint == UNDEFINED_ENDPOINT) {
      // We did not find an OTA server yet, so wait a while before trying
      // again.  Hopefully one will appear on the network later.
      restartServerDiscoveryAfterDelay();
    } else {
      euiLookup();
    }
    return;
  } else if (result->status
             == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    // Per note above, we only care about the first responding server, so
    // only save the responder info if we don't already have a server endpoint.
    if (epList->count > 0 && serverEndpoint == UNDEFINED_ENDPOINT) {
#if (defined EMBER_AF_PLUGIN_OTA_CLIENT_IGNORE_LOOPBACK_SERVER)
      // The client is being told to ignore OTA server endpoints on the local
      // node so the "first" responder is really the first _remote_ server.
      if (result->matchAddress == emberAfGetNodeId()) {
        otaPrintln("Ignoring server from loopback address");
        return;
      }
#endif
      if (ignoreNonTrustCenter && result->matchAddress != 0x0000) {
        otaPrintln("Ignoring 0x%2X OTA server in search of TC",
                   result->matchAddress);
        return;
      }
      serverEndpoint = epList->list[0];
      serverNodeId = result->matchAddress;
      otaPrintln("Setting OTA Server to 0x%2X", serverNodeId);
    }
  } else if (result->status
             == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    // Assumed IEEE address request
    EmberEUI64 eui64;
    MEMMOVE(eui64, result->responseData, EUI64_SIZE);
    recordServerEuiAndGoToNextState(eui64);
  } else { // Assume Unicast timeout of IEEE address request
    euiLookupErrorOccurred();
  }
}

static void startServerDiscovery(void)
{
  EmberStatus status;
  uint8_t index;

  recordUpgradeStatus(BOOTLOAD_STATE_DISCOVER_SERVER);
  serverEndpoint = UNDEFINED_ENDPOINT;

  // Figure out the right app profile ID based on the endpoint descriptor
  index = emberAfIndexFromEndpoint(myEndpoint);
  if (0xFF == index) {
    otaPrintln("Invalid EP %d when trying to send OTA message.", myEndpoint);
    return;
  }

  status =
    emberAfFindDevicesByProfileAndCluster(
      EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
      emberAfProfileIdFromIndex(index),
      ZCL_OTA_BOOTLOAD_CLUSTER_ID,
      EMBER_AF_SERVER_CLUSTER_DISCOVERY,
      emAfOtaClientServiceDiscoveryCallback);

  if (status != EMBER_SUCCESS) {
    otaPrintln("Failed to initiate service discovery.");
    waitingForResponse = false;
  } else {
    waitingForResponse = true;
  }

  errors = 0;
  setTimer(EMBER_AF_OTA_SERVER_DISCOVERY_DELAY_MS);
}

static void euiLookupErrorOccurred(void)
{
  // The timer has expired and we don't know the server's EUI.
  // We treat the server discovery + EUI lookup as a single operation
  // and if either fails we delay a long time to allow any potential network
  // issues to be resolved.
  waitingForResponse = false;
  restartServerDiscoveryAfterDelay();
  return;
}

static void euiLookup(void)
{
  EmberEUI64 eui64;
  EmberStatus status;

  recordUpgradeStatus(BOOTLOAD_STATE_GET_SERVER_EUI);
  status = emberLookupEui64ByNodeId(serverNodeId, eui64);

  if (status != EMBER_SUCCESS) {
    // New discovery of the Server's EUI
    status = emberAfFindIeeeAddress(serverNodeId,
                                    emAfOtaClientServiceDiscoveryCallback);
    if (status != EMBER_SUCCESS) {
      // Discovery was never initiated.
      euiLookupErrorOccurred();
      return;
    }
    recordUpgradeStatus(BOOTLOAD_STATE_GET_SERVER_EUI);
    waitingForResponse = true;
    return; // allow the EUI to resolve
  } // Else
    // We happen to know the server's EUI without discovery, keep going.

  recordServerEuiAndGoToNextState(eui64);
}

static void putImageInfoInMessage(void)
{
  emberAfPutInt16uInResp(currentDownloadFile.manufacturerId);
  emberAfPutInt16uInResp(currentDownloadFile.imageTypeId);
  emberAfPutInt32uInResp(currentDownloadFile.firmwareVersion);
}

static BootloadState determineDownloadFileStatus(void)
{
  uint32_t currentOffset;
  EmberAfOtaStorageStatus
    status = emberAfOtaStorageCheckTempDataCallback(&currentOffset,
                                                    &totalImageSize,
                                                    &currentDownloadFile);

  if (status == EMBER_AF_OTA_STORAGE_PARTIAL_FILE_FOUND) {
    otaPrintFlush();
    otaPrintln("Partial file download found, continuing from offset 0x%4X",
               currentOffset);
    otaPrintFlush();
    updateCurrentOffset(currentOffset);
    updateDownloadFileVersion(currentDownloadFile.firmwareVersion);
    emAfPrintPercentageSetStartAndEnd(0, totalImageSize);
    return BOOTLOAD_STATE_DOWNLOAD;
  } else if (status == EMBER_AF_OTA_STORAGE_SUCCESS) {
    EmberAfOtaImageId currentVersionInfo;
    otaPrintln("Found fully downloaded file in storage (version 0x%4X).",
               currentDownloadFile.firmwareVersion);
    emberAfOtaClientVersionInfoCallback(&currentVersionInfo, NULL);
    if (currentVersionInfo.firmwareVersion != currentDownloadFile.firmwareVersion) {
      otaPrintln("Found file in storage with different version (0x%4X) than current version (0x%4X)",
                 currentDownloadFile.firmwareVersion,
                 currentVersionInfo.firmwareVersion);
      return BOOTLOAD_STATE_VERIFY_IMAGE;
    } else {
      otaPrintln("File in storage is same as current running version (0x%4X)",
                 currentVersionInfo.firmwareVersion);
    }
  } else {
    otaPrintln("No image found in storage.");
  }

  emberAfAppFlush();

  return BOOTLOAD_STATE_QUERY_NEXT_IMAGE;
}

static void determineNextState(void)
{
  currentBootloadState = determineDownloadFileStatus();

  switch (currentBootloadState) {
    case BOOTLOAD_STATE_QUERY_NEXT_IMAGE:
      queryNextImage(SEND_QUERY_NOW,
                     false); // query error?
      break;
    case BOOTLOAD_STATE_DOWNLOAD:
    {
      EmberAfOtaStorageStatus status = emberAfOtaStorageDriverPrepareToResumeDownloadCallback();
      errors = 0;
      recordUpgradeStatus(BOOTLOAD_STATE_DOWNLOAD);
      if (status == EMBER_AF_OTA_STORAGE_SUCCESS) {
        updateImageTypeIdAttribute(currentDownloadFile.imageTypeId);
        continueImageDownload();
      } else if (status != EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS) {
        downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
      } // For operation-in-progress we fall-through and wait for the callback
        // later.
    }
    break;
    case BOOTLOAD_STATE_VERIFY_IMAGE:
      updateImageTypeIdAttribute(currentDownloadFile.imageTypeId);
      continueImageVerification(EMBER_AF_IMAGE_UNKNOWN);
      break;
    case BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE:
      askServerToRunUpgrade(START_NEW_TIMER);
      break;
    default:
      // Do nothing.  No other states should reach here.
      break;
  }
}

static void recordUpgradeStatus(BootloadState state)
{
  uint8_t upgradeStatus = bootloadStateToExternalState[state];
  if (currentBootloadState != state) {
    emberAfCoreFlush();
    otaPrintln("Bootload state: %p",
               bootloadStateNames[state]);
    emberAfCoreFlush();
  }
  currentBootloadState = state;

  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_IMAGE_UPGRADE_STATUS_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&upgradeStatus,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);
}

static EmberAfStatus commandParse(bool defaultResponse,
                                  EmberAfClusterCommand* message)
{
  uint8_t commandId = (message->buffer[(ZCL_COMMAND_ID_INDEX
                                        + (uint8_t)defaultResponse)]);
  uint8_t index = EMBER_AF_ZCL_OVERHEAD;

  if (commandId > EM_AF_OTA_MAX_COMMAND_ID) {
    otaPrintln("Bad OTA command: 0x%X", commandId);
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  if (!defaultResponse && message->bufLen < emAfOtaMinMessageLengths[commandId]) {
    otaPrintln("OTA command 0x%X too short (len %d < min %d)",
               commandId,
               message->bufLen,
               emAfOtaMinMessageLengths[commandId]);
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }

  if (message->source != serverNodeId
      && currentBootloadState >= BOOTLOAD_STATE_DISCOVER_SERVER
      && commandId != ZCL_IMAGE_NOTIFY_COMMAND_ID) {
    otaPrintln("OTA command from unrecognized server 0x%2X.  My OTA server: 0x%2X",
               message->source,
               serverNodeId);
    return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
  }

  // While not all command validation has taken place at this point, we
  // flag that we are not waiting for a response anymore.  We want to make sure
  // that our sleepy can go to sleep if it wants.  The likelihood of an
  // invalid command followed by a properly formatted one is extremely low.
  // Either the server knows the correct format or it doesn't.
  waitingForResponse = false;

  // NOTE:  In order to support Default Response, we list the CLIENT commands here
  // as well in case the server used that to respond.
  switch (commandId) {
    case ZCL_IMAGE_NOTIFY_COMMAND_ID: {
      bool broadcast = (EMBER_INCOMING_UNICAST != message->type);
      if (currentBootloadState != BOOTLOAD_STATE_QUERY_NEXT_IMAGE) {
        if (currentBootloadState < BOOTLOAD_STATE_DISCOVER_SERVER) {
          // If the server wants us to upgrade before we complete our discovery
          // backoff, we should start the discovery right now.
          emberAfOtaClientStartCallback();
        } else if (currentBootloadState == BOOTLOAD_STATE_DISCOVER_SERVER) {
          // If we receive an image notfiy command while in the process of
          // discovering a server, we should immediately start discovery
          #ifdef EMBER_AF_PLUGIN_OTA_CLIENT_IGNORE_LOOPBACK_SERVER
          if (message->source == emberAfGetNodeId()) {
            return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
          }
          #endif
          otaPrintln("Got unexpected %p.  Start discovery.",
                     "Image Notify");
          startServerDiscovery();
          return EMBER_ZCL_STATUS_SUCCESS;
        } else {
          otaPrintln("Got unexpected %p.  Ignored.",
                     "Image notify");
          return EMBER_ZCL_STATUS_FAILURE;
        }
      }
      return imageNotifyParse(broadcast, message->buffer, index, message->bufLen);
    }
    case ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID:
    case ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID: {
      if (currentBootloadState != BOOTLOAD_STATE_QUERY_NEXT_IMAGE) {
        otaPrintln("Got unexpected %p.  Ignored.",
                   "Query next image response");
        return EMBER_ZCL_STATUS_FAILURE;
      }
      return queryNextImageResponseParse(message->buffer,
                                         index + defaultResponse,
                                         message->bufLen);
    }
    case ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID: {
      if (currentBootloadState != BOOTLOAD_STATE_DOWNLOAD) {
        otaPrintln("Got unexpected %p.  Ignored.",
                   "Image block response");
        return EMBER_ZCL_STATUS_FAILURE;
      }
      return imageBlockResponseParse(message->buffer,
                                     index + defaultResponse,
                                     message->bufLen);
    }
    case ZCL_UPGRADE_END_RESPONSE_COMMAND_ID:
    case ZCL_UPGRADE_END_REQUEST_COMMAND_ID: {
      if ((currentBootloadState != BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE)
          && (currentBootloadState != BOOTLOAD_STATE_UPGRADE_VIA_OUT_OF_BAND)) {
        otaPrintln("Got unexpected %p.  Ignored.",
                   "Upgrade end response");
        return EMBER_ZCL_STATUS_FAILURE;
      }
      return upgradeEndResponseParse((defaultResponse
                                      ? message->buffer[index]
                                      : EMBER_ZCL_STATUS_SUCCESS),
                                     message->buffer,
                                     message->bufLen);
    }
    default:
      // Fall through. Already printed info about the bad command ID.
      break;
  }
  return EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
}

bool emberAfOtaClientIncomingMessageRawCallback(EmberAfClusterCommand* message)
{
  EmberStatus sendStatus;
  EmberAfStatus zclStatus = commandParse(false,   // default response?
                                         message);
  if (zclStatus) {
    emberAfOtaBootloadClusterFlush();
    emberAfOtaBootloadClusterPrintln("%p: failed parsing OTA cmd 0x%x",
                                     "Error",
                                     message->commandId);
    if (message->type == EMBER_INCOMING_BROADCAST) {
      // We don't want to respond to invalid broadcast messages with
      // a default response.
      return EMBER_ZCL_STATUS_SUCCESS;
    }
    sendStatus = emberAfSendDefaultResponse(message, zclStatus);
    if (EMBER_SUCCESS != sendStatus) {
      emberAfOtaBootloadClusterPrintln("OTA: failed to send %s response: 0x%x",
                                       "default",
                                       sendStatus);
    }
  }

  return true;
}

void emberAfOtaBootloadClusterClientDefaultResponseCallback(uint8_t endpoint,
                                                            uint8_t commandId,
                                                            EmberAfStatus status)
{
  EmberAfClusterCommand* command = emberAfCurrentCommand();

  if (endpoint != myEndpoint) {
    return;
  }
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    // The only default response we care about is non-success values.
    // That will mean the server failed processing for some reason,
    // or wants to abort.  Successful responses will send a non-default
    // response message.
    return;
  }
  otaPrintln("OTA Default response to command ID 0x%X, status 0x%X",
             commandId,
             status);

  commandParse(true, // default response?
               command);
}

// The buffer must point to the start of the image notify command,
// not the start of the ZCL frame.  It is assumed that the length
// of the buffer is minimum for image notify command.
static EmberAfStatus imageNotifyParse(bool broadcast,
                                      uint8_t* buffer,
                                      uint8_t index,
                                      uint8_t length)
{
  uint16_t manufacturerId;
  uint16_t imageTypeId;
  EmberAfOtaImageId myId;
  uint8_t payloadType = emberAfGetInt8u(buffer, index, length);
  uint8_t queryJitter = emberAfGetInt8u(buffer, index + 1, length);
  index += 2;

  if (!broadcast) {
    // Spec says to always respond to unicasts regardless of the parameters.
    otaPrintln("%p unicast, querying",
               "Image notify command");
    goto sendQuery;
  }

  emberAfOtaClientVersionInfoCallback(&myId, NULL);

  // We assume that if the message is broadcast then our ZCL
  // code will NOT send the default response.

  if (payloadType > IMAGE_NOTIFY_LAST_VALID_TYPE) {
    otaPrintln("%p %p payload type 0x%X",
               "Invalid",
               "Image notify command",
               payloadType);
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  if (queryJitter < 1 || queryJitter > 100) {
    otaPrintln("%p %p: out of range jitter %d",
               "Invalid",
               "Image notify command",
               queryJitter);
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  if (length != imageNotifyPayloadLengths[payloadType]) {
    otaPrintln("%p %p: payload length doesn't match type 0x%X (%d < %d)",
               "Invalid",
               "Image notify command",
               payloadType,
               length,
               imageNotifyPayloadLengths[payloadType]);
    // Although this truly is an error, we don't send a response because it could
    // be a broadcast.
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  if (payloadType >= IMAGE_NOTIFY_MANUFACTURER_ONLY_TYPE) {
    manufacturerId = emberAfGetInt16u(buffer, index, length);
    index += 2;
    if (manufacturerId != myId.manufacturerId) {
      otaPrintln("%p %p due to non-matching manufacturer ID",
                 "Ignoring",
                 "Image notify command");
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  if (payloadType >= IMAGE_NOTIFY_MFG_AND_IMAGE_TYPE) {
    imageTypeId = emberAfGetInt16u(buffer, index, length);
    index += 2;
    if (imageTypeId != myId.imageTypeId) {
      otaPrintln("%p %p due to non-matching image type ID",
                 "Ignoring",
                 "Image notify command");
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  if (payloadType >= IMAGE_NOTIFY_FULL_VERSION_TYPE) {
    // Could add some additional checking about the minimum allowed version
    // number, but it can still be caught after the download.
    uint32_t version = emberAfGetInt32u(buffer, index, length);
    index += 4;
    if (version == myId.firmwareVersion) {
      // Spec. says that if the firmware version matches, we should ignore.
      // A matching version number would be a re-install, which can only
      // be done via image notify by a unicast.  The server can force
      // a mass upgrade or downgrade by sending out a different version
      // than what devices have.
      otaPrintln("%p %p due to matching firmware version",
                 "Ignoring",
                 "Image notify command");
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  // Check QueryJitter value.  For QueryJitter value less than a 'must response'
  // value (value of 100), we need to introduce jitter in our reply by picking
  // a random number between 1 and 100.  We only send reply if the value picked
  // is less than or equal to the QueryJitter value.
  if (queryJitter < 100) {
    uint8_t random = (((uint8_t)emberGetPseudoRandomNumber()) % 100) + 1;
    if (random > queryJitter) {
      otaPrintln("%p %p, Rx'd Jitter (0x%x), Picked Jitter (0x%x)",
                 "Ignoring",
                 "Image notify command",
                 queryJitter,
                 random);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  sendQuery:
  // By saying "timeout reached" we want to indicate that a new query should kick off
  // immediately.
  queryNextImage(SEND_QUERY_NOW,
                 false);  // query error occurred?
  return EMBER_ZCL_STATUS_SUCCESS;
}

static void startDownload(uint32_t newVersion)
{
  EmberAfOtaStorageStatus status;
  otaPrintln("Starting download, Version 0x%4X",
             newVersion);
  emAfPrintPercentageSetStartAndEnd(0, totalImageSize);
  updateDownloadFileVersion(newVersion);
  updateCurrentOffset(0);
  updateMinBlockRequestPeriodAttribute(0);

  recordUpgradeStatus(BOOTLOAD_STATE_DOWNLOAD);

  errors = 0;

  status = emberAfOtaStorageClearTempDataCallback();

  if (status == EMBER_AF_OTA_STORAGE_SUCCESS) {
    // For the "OTA Storage Simple" plugin, there is only one image.
    // A call to clear the temp data (above) will also wipe out a fully
    // downloaded image.  So we don't actually have to call
    // emberAfOtaStorageDeleteImageCallback() as well.  We only worry
    // about a result of EMBER_AF_OPERATION_IN_PROGRESS returned from the first
    // delete.

    // For the "OTA POSIX Filesystem" plugin there may be multiple
    // images, so we may have to delete the temp data AND another image
    // on disk.  However we assume that delete operations are extremely
    // fast and thus won't return EMBER_AF_OPERATION_IN_PROGRESS.

    EmberAfOtaImageId oldImageId = emberAfOtaStorageSearchCallback(currentDownloadFile.manufacturerId,
                                                                   currentDownloadFile.imageTypeId,
                                                                   (hardwareVersion == 0xFFFF
                                                                    ? NULL
                                                                    : &hardwareVersion));

    if (emberAfIsOtaImageIdValid(&oldImageId)) {
      // Wipe out any existing file matching the same values as the one we
      // are going to download.

      status = emberAfOtaStorageDeleteImageCallback(&oldImageId);
      if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
        otaPrintln("Error: Failed to delete image.");
        downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
        return;
      }
    }
    continueImageDownload();
  } else if (status == EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS) {
    // Disable the timer since we are waiting for the erase event to complete
    // and execute our callback, not the timer to expire.
    emberAfDeactivateClientTick(myEndpoint, ZCL_OTA_BOOTLOAD_CLUSTER_ID);
  } else {
    otaPrintln("Error: Failed to erase old temp data.");
    downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
  }
}

// We expect that the minimum length for this command has already been checked.
static EmberAfStatus queryNextImageResponseParse(uint8_t* buffer,
                                                 uint8_t index,
                                                 uint8_t length)
{
  uint8_t status;
  EmberAfStatus zclStatus;
  EmberAfOtaImageId imageId;
  bool isError = false;

  status = emberAfGetInt8u(buffer, index, length);
  index++;

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    otaPrintln("%p returned 0x%X.  No new image to download.",
               "Query next image response",
               status);
    zclStatus = EMBER_ZCL_STATUS_SUCCESS;
    goto queryNextImageResponseDone;
  }
  if (length < QUERY_NEXT_IMAGE_SUCCESS_RESPONSE_MIN_LENGTH) {
    otaPrintln("%p too short (%d < %d)",
               "Query next image response",
               length,
               QUERY_NEXT_IMAGE_SUCCESS_RESPONSE_MIN_LENGTH);
    zclStatus = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
    isError = true;
    goto queryNextImageResponseDone;
  }
  otaPrintln("%p: New image is available for download.",
             "Query next image response");

  index += emAfOtaParseImageIdFromMessage(&imageId,
                                          &(buffer[index]),
                                          length - index);
  totalImageSize = emberAfGetInt32u(buffer, index, length);

  if (imageId.manufacturerId != currentDownloadFile.manufacturerId
      || imageId.imageTypeId != currentDownloadFile.imageTypeId
      || totalImageSize == 0) {
    otaPrintln("%p is not using my image info.",
               "Query next image response");
    isError = true;
    zclStatus = EMBER_ZCL_STATUS_INVALID_FIELD;
    goto queryNextImageResponseDone;
  } else if (totalImageSize > emberAfOtaStorageDriverMaxDownloadSizeCallback()) {
    otaPrintln("ERROR: Next Image is too big to store (0x%4X > 0x%4X)",
               totalImageSize,
               emberAfOtaStorageDriverMaxDownloadSizeCallback());
    isError = true;
    zclStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    goto queryNextImageResponseDone;
  } else if (imageId.firmwareVersion == currentDownloadFile.firmwareVersion) {
    otaPrintln("%p returned same FileVersion as currently installed.",
               "Query next image response");
    zclStatus = EMBER_ZCL_STATUS_SUCCESS;
    goto queryNextImageResponseDone;
  } else if (disableOtaDowngrades
             && imageId.firmwareVersion < currentDownloadFile.firmwareVersion) {
    otaPrintln("OTA Downgrades Disabled: FileVersion behind current installation");
    sendMessage(ZCL_UPGRADE_END_REQUEST_COMMAND_ID,
                EMBER_ZCL_STATUS_INVALID_IMAGE,
                EMBER_AF_RUN_UPGRADE_REQUEST_DELAY_MS);
    zclStatus = EMBER_ZCL_STATUS_SUCCESS;
    goto queryNextImageResponseDone;
  } else {
    startDownload(imageId.firmwareVersion);
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  queryNextImageResponseDone:
  queryNextImage(DELAY_BEFORE_SEND_QUERY,
                 isError);
  return zclStatus;
}

static void sendMessage(uint8_t cmdId, uint8_t upgradeEndStatus, uint32_t timer)
{
  uint8_t fieldControl = 0;
  uint8_t index;

  // Figure out the right app profile ID based on the endpoint descriptor
  index = emberAfIndexFromEndpoint(myEndpoint);
  if (0xFF == index) {
    otaPrintln("Invalid EP %d when trying to send OTA message.", myEndpoint);
    return;
  }

  uint8_t disable_default_response_mask = 0;

  if ( ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID == cmdId ) {
    //Optional "disable default response" - Image page request
    disable_default_response_mask = EMBER_AF_DEFAULT_RESPONSE_POLICY_REQUESTS;
  }

  // Basic ZCL header information
  appResponseLength = 0;
  emberAfResponseApsFrame.profileId = emberAfProfileIdFromIndex(index);
  emberAfResponseApsFrame.sourceEndpoint = myEndpoint;
  emberAfPutInt8uInResp(ZCL_CLUSTER_SPECIFIC_COMMAND
                        | disable_default_response_mask
                        | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  emberAfPutInt8uInResp(emberAfNextSequence());
  emberAfPutInt8uInResp(cmdId);
  waitingForResponse = true;
  switch (cmdId) {
    case ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID:
    {
      if (hardwareVersion != EMBER_AF_INVALID_HARDWARE_VERSION) {
        fieldControl |= OTA_FIELD_CONTROL_HW_VERSION_PRESENT_BIT;
      }
      emberAfPutInt8uInResp(fieldControl);
      putImageInfoInMessage();
      if (hardwareVersion != EMBER_AF_INVALID_HARDWARE_VERSION) {
        emberAfPutInt16uInResp(hardwareVersion);
      }
    }
    break;

    case ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID:
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_MINIMUM_BLOCK_REQUEST_PERIOD_ATTRIBUTE)
      if ((cmdId == ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID)
          && (DISABLE_FEATURE != MIN_BLOCK_PERIOD_UNITS)) {
        fieldControl |= OTA_FIELD_CONTROL_MIN_BLOCK_REQUEST_PRESENT_BIT;
      }
#endif
      emberAfPutInt8uInResp(fieldControl);
      putImageInfoInMessage();
      emberAfPutInt32uInResp(getCurrentOffset());
      emberAfPutInt8uInResp(usePageRequest
                            ? EM_AF_PAGE_REQUEST_BLOCK_SIZE
                            : MAX_CLIENT_DATA_SIZE);

      if (cmdId == ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID) {
        emberAfPutInt16uInResp(EMBER_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_SIZE);
        emberAfPutInt16uInResp(EMBER_AF_OTA_CLIENT_PAGE_REQUEST_SPACING_MS);
      } else {
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_MINIMUM_BLOCK_REQUEST_PERIOD_ATTRIBUTE)
        if ((cmdId != ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID)
            || (DISABLE_FEATURE != MIN_BLOCK_PERIOD_UNITS)) {
          emberAfPutInt16uInResp(minBlockRequestPeriod);
        }
#endif
      }
      break;

    case ZCL_UPGRADE_END_REQUEST_COMMAND_ID:
      otaPrintln("Sending Upgrade End request.");
      emberAfCoreFlush();
      emberAfPutInt8uInResp(upgradeEndStatus);
      putImageInfoInMessage();
      if (upgradeEndStatus != 0) {
        waitingForResponse = false;
      }
      break;

    case ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID:
    {
      EmberEUI64 myEui64;
      emberAfGetEui64(myEui64);
      emberAfPutBlockInResp(myEui64, EUI64_SIZE);
      putImageInfoInMessage();
      emberAfPutInt16uInResp(ZIGBEE_PRO_STACK_VERSION);
    }
    break;

    default:
      otaPrintln("%p: invalid cmdId 0x%x", "Error", cmdId);
      return;
  } //end switch statement

  emberAfResponseApsFrame.clusterId = ZCL_OTA_BOOTLOAD_CLUSTER_ID;
  emberAfResponseApsFrame.sourceEndpoint = myEndpoint;
  emberAfResponseApsFrame.destinationEndpoint = serverEndpoint;
  emberAfResponseApsFrame.options |= EMBER_APS_OPTION_RETRY;
  {
    EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, serverNodeId);
    if (status != EMBER_SUCCESS) {
      otaPrintln("Error:  Failed to send OTA command 0x%X, status: 0x%X",
                 cmdId, status);
    }
  }
  setTimer(timer);
}

static void queryNextImage(SendNextQueryState sendQuery,
                           bool queryErrorOccurred)
{
  recordUpgradeStatus(BOOTLOAD_STATE_QUERY_NEXT_IMAGE);

  if (serverNodeId == EMBER_UNKNOWN_NODE_ID) {
    otaPrintln("Server unknown, falling back to discovery.");
    restartServerDiscoveryAfterDelay();
    return;
  }

  if (queryErrorOccurred) {
    errors++;
    if (errors >= EMBER_AF_PLUGIN_OTA_CLIENT_QUERY_ERROR_THRESHOLD) {
      otaPrintln("Maximum query errors hit (%d), starting search for new OTA server.\n",
                 EMBER_AF_PLUGIN_OTA_CLIENT_QUERY_ERROR_THRESHOLD);
      restartServerDiscoveryAfterDelay();
      return;
    }
  }

  if (sendQuery == SEND_QUERY_NOW) {
    if (!queryErrorOccurred) {
      errors = 0;
    }

    // Ask the client what image info to use in the query and
    // subsequent download.
    emberAfOtaClientVersionInfoCallback(&currentDownloadFile,
                                        &hardwareVersion);

    sendMessage(ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID,
                0,                            // upgrade end status (ignored)
                EMBER_AF_OTA_QUERY_DELAY_MS);
  } else {
    setTimer(EMBER_AF_OTA_QUERY_DELAY_MS);
  }
}

static void continueImageDownload(void)
{
  uint8_t commandId = ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID;
  uint32_t timer = EMBER_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS
                   ? EMBER_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS
                   : MESSAGE_TIMEOUT_BACKOFF_MS;
  bool send = true;

  recordUpgradeStatus(BOOTLOAD_STATE_DOWNLOAD);

  if (errors >= EMBER_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_ERROR_THRESHOLD) {
    otaPrintln("Maximum number of download errors reach (%d), aborting.",
               errors);
    downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
    return;
  }

  if (usePageRequest) {
    // Set the current offset for page request
    // or, Get the current offset if retrying image blocks
    EmAfPageRequestClientStatus status = emAfGetCurrentPageRequestStatus();
    if (status == EM_AF_NO_PAGE_REQUEST) {
      timer = emAfInitPageRequestClient(getCurrentOffset(),
                                        totalImageSize);
      commandId = ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID;
    } else {
      uint32_t offset;
      if (EM_AF_PAGE_REQUEST_ERROR
          == emAfNextMissedBlockRequestOffset(&offset)) {
        // Server is unreachable because page request caused us to get 0
        // response packets from the server when we should have received
        // a lot.
        errors++;
        send = false;
      } else {
        updateCurrentOffset(offset);
      }
    }
  }

  if (send) {
    sendMessage(commandId,
                0,         // upgrade end status (ignored)
                timer);
  } else {
    setTimer(timer);
  }
}

// A callback fired by the verification code.
void emAfOtaVerifyStoredDataFinish(EmberAfImageVerifyStatus status)
{
  if (currentBootloadState == BOOTLOAD_STATE_VERIFY_IMAGE) {
    continueImageVerification(status);
  }
}

static bool downloadAndVerifyFinish(EmberAfOtaDownloadResult result)
{
  static const uint8_t zclStatusFromResult[] = {
    EMBER_ZCL_STATUS_SUCCESS,
    EMBER_ZCL_STATUS_ABORT,
    EMBER_ZCL_STATUS_INVALID_IMAGE,
    EMBER_ZCL_STATUS_ABORT,
    EMBER_ZCL_STATUS_ABORT,
    EMBER_ZCL_STATUS_ABORT,
  };
  uint8_t zclStatus;
  bool goAhead;

  if (currentBootloadState <= BOOTLOAD_STATE_QUERY_NEXT_IMAGE) {
    // We don't really care about the return code here.
    // The important thing is that we don't call the client's download
    // complete handler.
    return true;
  }

  goAhead = emberAfOtaClientDownloadCompleteCallback(result,
                                                     &currentDownloadFile);

  if (result == EMBER_AF_OTA_DOWNLOAD_AND_VERIFY_SUCCESS
      && !goAhead) {
    otaPrintln("Client verification failed.");
    result = EMBER_AF_OTA_CLIENT_ABORTED;
  }
  zclStatus = zclStatusFromResult[result];

  // We could automatically invalidate the image in temporary storage
  // when it is declared a bad image.
  // This can be done by the download complete callback instead.

  if (zclStatus == EMBER_ZCL_STATUS_SUCCESS) {
    askServerToRunUpgrade(START_NEW_TIMER);
  } else if (result != EMBER_AF_OTA_SERVER_ABORTED
             && currentBootloadState != BOOTLOAD_STATE_QUERY_NEXT_IMAGE) {
    // Report to the server that the download has failed.
    sendMessage(ZCL_UPGRADE_END_REQUEST_COMMAND_ID,
                zclStatus,
                EMBER_AF_RUN_UPGRADE_REQUEST_DELAY_MS);
  }
  updateDownloadedZigbeeStackVersionAttribute();

  if (zclStatus != EMBER_ZCL_STATUS_SUCCESS) {
    waitingForResponse = false;
    queryNextImage(DELAY_BEFORE_SEND_QUERY,
                   false);  // query error?
    // We've aborted! Update image type id attribute to indicate that we are not in an OTA process
    updateImageTypeIdAttribute(0xFFFF);
  }
  return (result == EMBER_ZCL_STATUS_SUCCESS);
}

static void continueImageVerification(EmberAfImageVerifyStatus status)
{
  recordUpgradeStatus(BOOTLOAD_STATE_VERIFY_IMAGE);

  if (status == EMBER_AF_IMAGE_UNKNOWN) {
    customVerifyStatus = NO_CUSTOM_VERIFY;
  } else if (status == EMBER_AF_IMAGE_GOOD) {
    // This is only called with status == GOOD when signature
    // verification has completed.  It is not called when
    // custom verification is done.
    customVerifyStatus = NEW_CUSTOM_VERIFY;
    status = EMBER_AF_IMAGE_VERIFY_IN_PROGRESS;
  }

  if (status == EMBER_AF_IMAGE_UNKNOWN
      || status == EMBER_AF_IMAGE_VERIFY_IN_PROGRESS) {
    uint32_t offset;
    uint32_t totalSize;
    EmberAfOtaImageId id;

    // First a basic sanity check of the image to insure
    // the file has completely downloaded and the file format
    // is correct.
    if (status == EMBER_AF_IMAGE_UNKNOWN
        && (EMBER_AF_OTA_STORAGE_SUCCESS
            != emberAfOtaStorageCheckTempDataCallback(&offset,
                                                      &totalSize,
                                                      &id))) {
      status = EMBER_AF_IMAGE_VERIFY_ERROR;
//      otaPrintln("emberAfOtaStorageCheckTempDataCallback() failed.");
      goto imageVerifyDone;
    }

    if (customVerifyStatus != NO_CUSTOM_VERIFY) {
      status = emberAfOtaClientCustomVerifyCallback((customVerifyStatus
                                                     == NEW_CUSTOM_VERIFY),
                                                    &currentDownloadFile);
      customVerifyStatus = CUSTOM_VERIFY_IN_PROGRESS;
    } else {
      status = emAfOtaImageSignatureVerify(MAX_DIGEST_CALCULATIONS_PER_CALL,
                                           &currentDownloadFile,
                                           (status == EMBER_AF_IMAGE_UNKNOWN
                                            ? EMBER_AF_NEW_IMAGE_VERIFICATION
                                            : EMBER_AF_CONTINUE_IMAGE_VERIFY));
    }

    if (status == EMBER_AF_IMAGE_VERIFY_IN_PROGRESS) {
      setTimer(EMBER_AF_PLUGIN_OTA_CLIENT_VERIFY_DELAY_MS);
      return;
    } else if (status == EMBER_AF_IMAGE_VERIFY_WAIT) {
      setTimer(0);
      return;
    } else if (status == EMBER_AF_NO_IMAGE_VERIFY_SUPPORT) {
      otaPrintln("No signature verification support, assuming image is okay.");
      customVerifyStatus = NEW_CUSTOM_VERIFY;
      setTimer(EMBER_AF_PLUGIN_OTA_CLIENT_VERIFY_DELAY_MS);
      return;
    } else {
      otaPrintln("%p verification %p: 0x%X",
                 (customVerifyStatus == NO_CUSTOM_VERIFY
                  ? "Signature"
                  : "Custom"),
                 (status == EMBER_AF_IMAGE_GOOD
                  ? "passed"
                  : "FAILED"),
                 status);
    }
  }

  imageVerifyDone:
  downloadAndVerifyFinish((status == EMBER_AF_IMAGE_GOOD
                           ? EMBER_AF_OTA_DOWNLOAD_AND_VERIFY_SUCCESS
                           : EMBER_AF_OTA_VERIFY_FAILED));
}

static void askServerToRunUpgrade(bool timeout)
{
  recordUpgradeStatus(BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE);

  if (!timeout) {
    errors = 0;
  }

  if (errors >= EMBER_AF_PLUGIN_OTA_CLIENT_UPGRADE_WAIT_THRESHOLD) {
    otaPrintln("Maximum upgrade requests made (%d) without response from "
               "server.", EMBER_AF_PLUGIN_OTA_CLIENT_UPGRADE_WAIT_THRESHOLD);
    UpgradeTimeoutPolicy upgradeTimeoutPolicy = getUpgradeTimeoutPolicy();
    if (UPGRADE_AFTER_TIMEOUT == upgradeTimeoutPolicy) {
      otaPrintln("Upgrading anyway");
      runUpgrade();
    } else {
      otaPrintln("Not applying upgrade due to UpgradeTimeoutPolicy of 0x%x",
                 upgradeTimeoutPolicy);
    }
    return;
  }
  sendMessage(ZCL_UPGRADE_END_REQUEST_COMMAND_ID,
              EMBER_ZCL_STATUS_SUCCESS,
              EMBER_AF_RUN_UPGRADE_REQUEST_DELAY_MS);
}

static bool storeData(uint32_t offset, uint32_t length, const uint8_t* data)
{
  return (EMBER_AF_OTA_STORAGE_SUCCESS
          == emberAfOtaStorageWriteTempDataCallback(offset, length, data));
}

static EmberAfStatus imageBlockResponseParse(uint8_t* buffer, uint8_t index, uint8_t length)
{
  EmberAfOtaImageId imageId;
  uint32_t offset;
  uint32_t currentOffset;
  uint8_t dataSize;
  uint32_t minBlockRequestPeriodMs = minBlockRequestPeriod * MIN_BLOCK_PERIOD_UNITS;
  uint32_t timerMs = (EMBER_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS >= minBlockRequestPeriodMs
                      ? EMBER_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS
                      : (minBlockRequestPeriodMs));
  uint8_t status;
  uint32_t nextOffset;
  const uint8_t* imageData;
  EmAfPageRequestClientStatus pageRequestStatus = EM_AF_NO_PAGE_REQUEST;

  if (NULL == buffer) {
    otaPrintln("OTA: ImageBlockReponse contains invalid or incomplete data");
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  status = emberAfGetInt8u(buffer, index, length);

  index++;

  if (status == EMBER_ZCL_STATUS_WAIT_FOR_DATA) {
    uint32_t currentTime;
    uint32_t requestTime;
    uint32_t calculatedTimerMs;

    if (length < IMAGE_BLOCK_RESPONSE_WAIT_FOR_DATA_LENGTH_MIN) {
      return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
    }
    currentTime = emberAfGetInt32u(buffer, index, length);
    index += 4;
    requestTime = emberAfGetInt32u(buffer, index, length);
    index += 4;

    if (!calculateTimer(currentTime, requestTime, &calculatedTimerMs)) {
      // Error printed by above function.
      calculatedTimerMs = CALCULATE_TIME_ERROR_IMAGE_BLOCK_DELAY_MS;
    }

    if ((DISABLE_FEATURE != MIN_BLOCK_PERIOD_UNITS)
        && (length >= IMAGE_BLOCK_RESPONSE_WAIT_FOR_DATA_MIN_BLOCK_RESPONSE_LENGTH)) {
      uint16_t temp = emberAfGetInt16u(buffer,
                                       index,
                                       length);
      emberAfCoreFlush();
      otaPrintln("Updating Min Block Request Period to %d ms",
                 temp * MIN_BLOCK_PERIOD_UNITS);
      updateMinBlockRequestPeriodAttribute(temp);
      calculatedTimerMs += temp * MIN_BLOCK_PERIOD_UNITS;
    }

    emberAfCoreFlush();
    otaPrintln("Download delay by server %d ms", calculatedTimerMs);
    emberAfCoreFlush();
    setTimer(calculatedTimerMs);
    // Abort the page request, else when the timer fires, it will be
    // misinterpreted as a page response timeout
    emAfAbortPageRequest();
    return EMBER_ZCL_STATUS_SUCCESS;
  } else if (status == EMBER_ZCL_STATUS_ABORT
             || status == EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE) {
    otaPrintln("Download aborted by server.");
    downloadAndVerifyFinish(EMBER_AF_OTA_SERVER_ABORTED);
    return EMBER_ZCL_STATUS_SUCCESS;
  } else if (status == EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND) {
    if (usePageRequest && emAfHandlingPageRequestClient()) {
      otaPrintln("Server doesn't support page request, only using block request.");
      usePageRequest = false;
      emAfAbortPageRequest();
      continueImageDownload();
      return EMBER_ZCL_STATUS_SUCCESS;
    } else {
      otaPrintln("Server returned 'unsupported cluster command'.");
      downloadAndVerifyFinish(EMBER_AF_OTA_SERVER_ABORTED);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  } else if (status != EMBER_ZCL_STATUS_SUCCESS) {
    otaPrintln("Unknown %p status code 0x%X",
               "Image block response",
               status);
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  } // Else status == success.  Keep going

  if (length < IMAGE_BLOCK_RESPONSE_SUCCESS_MIN_LENGTH) {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }

  index += emAfOtaParseImageIdFromMessage(&imageId,
                                          &(buffer[index]),
                                          length);
  offset = emberAfGetInt32u(buffer, index, length);
  index += 4;
  dataSize = emberAfGetInt8u(buffer, index, length);
  index += 1;
  imageData = buffer + index;

  if ((length - index) < dataSize) {
    otaPrintln("%p has data size (%d) smaller than actual packet size (%d).",
               "Image block response",
               dataSize,
               length - index);
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }

  if (!usePageRequest
      || emAfGetCurrentPageRequestStatus() != EM_AF_WAITING_PAGE_REQUEST_REPLIES) {
    // For normal image block request transactions, all blocks should be in order.
    // For page request, we may receive them out of order, or just miss packets.
    currentOffset = getCurrentOffset();
    if (offset != currentOffset) {
      otaPrintln("%p error: Expected offset 0x%4X, but got 0x%4X.  Ignoring",
                 "Image block response",
                 currentOffset,
                 offset);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  if (0 != MEMCOMPARE(&currentDownloadFile,
                      &imageId,
                      sizeof(EmberAfOtaImageId))
      || dataSize > MAX_CLIENT_DATA_SIZE) {
    otaPrintln("%p info did not match my expected info.  Dropping.",
               "Image block response");
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  // Configure image type id attribute if this is the first block we are reading
  if (currentOffset == 0x0000) {
    updateImageTypeIdAttribute(imageId.imageTypeId);
  }

  errors = 0;

  // We want to make sure we don't write the same block twice.  For
  // page-erase-required EEPROM parts this won't work.
  if (usePageRequest && emAfHandlingPageRequestClient()) {
    pageRequestStatus = emAfNoteReceivedBlockForPageRequestClient(offset);
    if (pageRequestStatus == EM_AF_BLOCK_ALREADY_RECEIVED) {
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  if (!storeData(offset, dataSize, imageData)) {
    otaPrintln("Failed to write to storage device!");
    downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
    // The downloadAndVerifyFinish() function will return it's only status
    // to the message.  No need to send ours.
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  nextOffset = offset + dataSize;

  if (usePageRequest && emAfHandlingPageRequestClient()) {
    if (pageRequestStatus == EM_AF_PAGE_REQUEST_ERROR) {
      downloadAndVerifyFinish(EMBER_AF_OTA_CLIENT_ABORTED);
      // We still return success to indicate we processed the message correctly.
      return EMBER_ZCL_STATUS_SUCCESS;
    } else if (pageRequestStatus == EM_AF_WAITING_PAGE_REQUEST_REPLIES) {
      return EMBER_ZCL_STATUS_SUCCESS;
    } else if (pageRequestStatus == EM_AF_PAGE_REQUEST_COMPLETE) {
      nextOffset = emAfGetFinishedPageRequestOffset();
    } else { // EM_AF_RETRY_MISSED_PACKETS
      timerMs = emAfGetPageRequestMissedPacketDelayMs();
    }
  }

  offset = updateCurrentOffset(nextOffset);

#if defined(EMBER_TEST)
  {
    uint8_t percentageComplete =
      emAfPrintPercentageUpdate("Download",
                                DOWNLOAD_PERCENTAGE_UPDATE_RATE,
                                offset);
    if (percentageComplete != 100 && percentageComplete >= emAfOtaClientStopDownloadPercentage) {
      otaPrintln("Artificially stopping download at %d%%", emAfOtaClientStopDownloadPercentage);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }
#else
  emAfPrintPercentageUpdate("Download",
                            DOWNLOAD_PERCENTAGE_UPDATE_RATE,
                            offset);
#endif

  if (offset >= totalImageSize) {
    emberAfOtaStorageFinishDownloadCallback(offset);
    continueImageVerification(EMBER_AF_IMAGE_UNKNOWN);
    return EMBER_ZCL_STATUS_SUCCESS;
  } else {
    if (!setTimer(timerMs)) {
      // Continue download right now
      continueImageDownload();
    } // else (fall through)
      // timer set, wait for next event to fire
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus upgradeEndResponseParse(uint8_t status,
                                             uint8_t* buffer,
                                             uint8_t length)
{
  EmberAfOtaImageId serverSentId;
  uint32_t waitTime;
  uint32_t currentTime, upgradeTime;
  uint8_t index = EMBER_AF_ZCL_OVERHEAD;

  if (status) {
    otaPrintln("Server aborted upgrade, status: 0x%X",
               status);
    downloadAndVerifyFinish(EMBER_AF_OTA_SERVER_ABORTED);
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  index += emAfOtaParseImageIdFromMessage(&serverSentId,
                                          &(buffer[index]),
                                          length);

  if ((serverSentId.manufacturerId != currentDownloadFile.manufacturerId)
      && (serverSentId.manufacturerId != MFG_ID_WILD_CARD)) {
    emberAfOtaBootloadClusterPrint("Error: %p had invalid %p: ",
                                   "Upgrade end response",
                                   "manufacturer ID");
    otaPrintln("0x%2X", serverSentId.manufacturerId);
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  if ((serverSentId.imageTypeId != currentDownloadFile.imageTypeId)
      && (serverSentId.imageTypeId != IMAGE_TYPE_WILD_CARD)) {
    emberAfOtaBootloadClusterPrint("Error: %p had invalid %p: ",
                                   "Upgrade end response",
                                   "image type ID");
    otaPrintln("0x%2X", serverSentId.imageTypeId);
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  if ((serverSentId.firmwareVersion != currentDownloadFile.firmwareVersion
       && serverSentId.firmwareVersion != FILE_VERSION_WILD_CARD)) {
    emberAfOtaBootloadClusterPrint("Error: %p had invalid %p: ",
                                   "Upgrade end response",
                                   "file version");
    otaPrintln("0x%4X", serverSentId.firmwareVersion);
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  currentTime = emberAfGetInt32u(buffer, index, length);
  index += 4;
  upgradeTime = emberAfGetInt32u(buffer, index, length);

  UpgradeActivationPolicy upgradeActivationPolicy = getUpgradeActivationPolicy();

  if (WAIT_FOR_UPGRADE_MESSAGE == upgradeTime) {
    switch (upgradeActivationPolicy) {
      case SERVER_ACTIVATION:
        recordUpgradeStatus(BOOTLOAD_STATE_WAITING_FOR_UPGRADE_MESSAGE);
        setTimer(WAIT_FOR_UPGRADE_DELAY_MS);
        break;
      case OUT_OF_BAND_ACTIVATION:
        recordUpgradeStatus(BOOTLOAD_STATE_UPGRADE_VIA_OUT_OF_BAND);
        setTimer(WAIT_FOR_OUT_OF_BAND_ACTIVATION_MS);
        break;
      default:
        break;
    }
    return EMBER_ZCL_STATUS_SUCCESS;
  } else {
    if (OUT_OF_BAND_ACTIVATION == upgradeActivationPolicy) {
      recordUpgradeStatus(BOOTLOAD_STATE_UPGRADE_VIA_OUT_OF_BAND);
      setTimer(WAIT_FOR_OUT_OF_BAND_ACTIVATION_MS);
      return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    }
  }

  // NOTE:  Current Time and Upgrade Time are in SECONDS since epoch.
  // Our timer uses MILISECONDS.  calculateTimer() will give us the MS delay.

  if (!calculateTimer(currentTime, upgradeTime, &waitTime)) {
    waitTime = CALCULATE_TIME_ERROR_UPGRADE_END_RESPONSE_DELAY_MS;
  } else {
    // Even when we are told to upgrade immediately, we want to insure there
    // is a chance for the APS retries and ZCL response to get back to their
    // senders.
    if (waitTime < IMMEDIATE_UPGRADE_DELAY_MS) {
      otaPrintln("Adding %d ms. delay for immediate upgrade.",
                 IMMEDIATE_UPGRADE_DELAY_MS);
      waitTime = IMMEDIATE_UPGRADE_DELAY_MS;
    }
  }
  emberAfPluginOtaClientPreBootloadCallback(myEndpoint,
                                            serverEndpoint,
                                            EMBER_ZIGBEE_COORDINATOR_ADDRESS);
  // Expect at this point waitTime != 0
  setTimer(waitTime);
  emberAfCoreFlush();
  otaPrintln("Countdown to upgrade: %d ms", waitTime);
  emberAfCoreFlush();
  recordUpgradeStatus(BOOTLOAD_STATE_COUNTDOWN_TO_UPGRADE);
  return EMBER_ZCL_STATUS_SUCCESS;
}

static bool calculateTimer(uint32_t currentTime,
                           uint32_t targetTime,
                           uint32_t* returnTimeMs)
{
  uint32_t timeOut = 0;
  bool validWaitTime = true;

  if (targetTime < currentTime) {
    otaPrintln("%p: invalid offset currentTime(0x%4X) > upgradeTime(0x%4X)",
               "Error",
               currentTime,
               targetTime);
    return false;
  } else {
    timeOut = targetTime - currentTime;
    otaPrintln("OTA Cluster: wait for %d s", timeOut);
  }
  otaPrintln("RXed timeOut 0x%4X s, MAX timeOut 0x%4X s",
             timeOut,
             TIMEOUT_MAX_WAIT_TIME_MS >> 10);   // divide by ~1000
                                                // save flash by doing a bit shift
  timeOut *= MILLISECOND_TICKS_PER_SECOND;

  if (timeOut > TIMEOUT_MAX_WAIT_TIME_MS) {
    timeOut = TIMEOUT_MAX_WAIT_TIME_MS;
  }
  *returnTimeMs = timeOut;
  return validWaitTime;
}

static void runUpgrade(void)
{
  emberAfCoreFlush();
  otaPrintln("Applying upgrade");
  emberAfCoreFlush();

  emberAfOtaClientBootloadCallback(&currentDownloadFile);

  // If we returned, then something is wrong with the upgrade.
  // It is expected that an invalid image file is deleted to prevent it
  // from being used a subsequent time.
  queryNextImage(DELAY_BEFORE_SEND_QUERY,
                 false);  // query error?
}

static void updateDownloadedZigbeeStackVersionAttribute(void)
{
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_DOWNLOADED_ZIGBEE_STACK_VERSION_ATTRIBUTE)
  uint16_t downloadedZigbeeStackVersion = 0;
  if (emAfOtaStorageGetZigbeeStackVersion(&currentDownloadFile, &downloadedZigbeeStackVersion) != EMBER_AF_OTA_STORAGE_SUCCESS) {
    downloadedZigbeeStackVersion = 0xFFFF;
  }
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_DOWNLOADED_ZIGBEE_STACK_VERSION_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&downloadedZigbeeStackVersion,
                        ZCL_INT16U_ATTRIBUTE_TYPE);
#endif //defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_DOWNLOADED_ZIGBEE_STACK_VERSION_ATTRIBUTE)
}

static void updateImageTypeIdAttribute(uint16_t imageTypeId)
{
#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_IMAGE_TYPE_ID_ATTRIBUTE)
  emberAfWriteAttribute(myEndpoint,
                        ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                        ZCL_IMAGE_TYPE_ID_ATTRIBUTE_ID,
                        CLUSTER_MASK_CLIENT,
                        (uint8_t*)&imageTypeId,
                        ZCL_INT16U_ATTRIBUTE_TYPE);
#endif //defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_IMAGE_TYPE_ID_ATTRIBUTE)
}

// Sends an image block request for a file the server should
// not have.  Test harness only (test case 9.5.6 - Missing File)
void emAfSendImageBlockRequestTest(void)
{
  if (currentBootloadState != BOOTLOAD_STATE_NONE) {
    otaPrintln("Image block request test only works when state is BOOTLOAD_STATE_NONE");
    return;
  }

  updateCurrentOffset(100);

  // Values from the test spec.
  currentDownloadFile.manufacturerId = 0xFFF0;
  currentDownloadFile.imageTypeId    = 0x0000;
  currentDownloadFile.firmwareVersion = 0xFFFFFFF0;
  sendMessage(ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID,
              0,   // upgrade end status (ignored)
              0);  // timer
}

void emAfSetPageRequest(bool pageRequestOn)
{
  usePageRequest = pageRequestOn;
}

bool emAfUsingPageRequest(void)
{
  return usePageRequest;
}

UpgradeActivationPolicy getUpgradeActivationPolicy()
{
  UpgradeActivationPolicy upgradeActivationPolicy = SERVER_ACTIVATION;

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_UPGRADE_ACTIVATION_POLICY_ATTRIBUTE)
  // SE 1.2b OTA updates - 15-0324-02
  // 11.10.12 - if the UpgradeTime is non-0xFFFFFFFF (begin timer) and the
  // upgradeActivationPolicy is 0x01 (out-of-band activation only), the client
  // shall send back a DefaultResponse of NOT_AUTHORIZED
  uint8_t dataType = ZCL_ENUM8_ATTRIBUTE_TYPE;
  emberAfReadAttribute(myEndpoint,
                       ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                       ZCL_UPGRADE_ACTIVATION_POLICY_ATTRIBUTE_ID,
                       CLUSTER_MASK_CLIENT,
                       (uint8_t*)&upgradeActivationPolicy,
                       sizeof(uint8_t),
                       &dataType);
#endif // ZCL_USING_OTA_BOOTLOAD_CLUSTER_UPGRADE_ACTIVATION_POLICY_ATTRIBUTE

  return upgradeActivationPolicy;
}

UpgradeTimeoutPolicy getUpgradeTimeoutPolicy()
{
  UpgradeTimeoutPolicy upgradeTimeoutPolicy = UPGRADE_AFTER_TIMEOUT;

#if defined(ZCL_USING_OTA_BOOTLOAD_CLUSTER_UPGRADE_TIMEOUT_POLICY_ATTRIBUTE)
  // SE 1.2b OTA updates - 15-0324-02
  // 11.10.13
  //
  // if UpgradeActivationPolicy is SERVER_ACTIVATION and UpgradeTimeoutPolicy is
  // UPGRADE_AFTER_TIMEOUT, then activate the image after timing out, as normal
  //
  // if UpgradeActivationPolicy is SERVER_ACTIVATION and UpgradeTimeoutPolicy is
  // NO_UPGRADE_AFTER_TIMEOUT, then do not activate the image after timeout
  //
  // if UpgradeActivationPolicy is OUT_OF_BAND_ACTIVATION and
  // UpgradeTimeoutPolicy is UPGRADE_AFTER_TIMEOUT, then retry mechanisms and
  // timeouts are manufacturer specific (we currently do nothing)
  //
  // if UpgradeActivationPolicy is OUT_OF_BAND_ACTIVATION and
  // UpgradeTimeoutPolicy is NO_UPGRADE_AFTER_TIMEOUT, then the image won't be
  // activated and the server shouldn't even be queried for whether to activate
  uint8_t dataType = ZCL_ENUM8_ATTRIBUTE_TYPE;
  emberAfReadAttribute(myEndpoint,
                       ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                       ZCL_UPGRADE_TIMEOUT_POLICY_ATTRIBUTE_ID,
                       CLUSTER_MASK_CLIENT,
                       (uint8_t*)&upgradeTimeoutPolicy,
                       sizeof(uint8_t),
                       &dataType);
#endif // ZCL_USING_OTA_BOOTLOAD_CLUSTER_UPGRADE_ACTIVATION_POLICY_ATTRIBUTE

  return upgradeTimeoutPolicy;
}

void emberAfOtaServerSendUpgradeRequest()
{
  sendMessage(ZCL_UPGRADE_END_REQUEST_COMMAND_ID,
              EMBER_ZCL_STATUS_SUCCESS,
              EMBER_AF_RUN_UPGRADE_REQUEST_DELAY_MS);
}

void emberAfPluginOtaClientSetIgnoreNonTrustCenter(bool ignoreNonTc)
{
  if (ignoreNonTrustCenter != ignoreNonTc) {
    ignoreNonTrustCenter = ignoreNonTc;
    emAfOtaClientStop();
  } // ignore otherwise.
}

bool emberAfPluginOtaClientGetIgnoreNonTrustCenter()
{
  return ignoreNonTrustCenter;
}

void emberAfPluginSetDisableOtaDowngrades(bool isDisabled)
{
  disableOtaDowngrades = isDisabled;
}

bool emberAfPluginGetDisableOtaDowngrades()
{
  return disableOtaDowngrades;
}
