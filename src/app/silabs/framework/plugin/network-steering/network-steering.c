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
 * @brief General Procedure
 *
 * 1. Setup stack security with an Install code key (if available)
 *    If not available, skip to step 9.
 * 2. Set the channel mask to the Primary set of channels.
 * 3. Perform a scan of open networks on a single channel.
 * 4. For all beacons collected on that channel, filter out PAN IDs that are not open
 *    for joining or don't have the right stack profile (2), or are duplicates.
 * 5. For each discovered network, try to join.
 * 6. Repeat steps 3-5 until all channels in the mask are tried.
 * 7. If no networks are successfully joined, setup secondary channel mask
 *    (all channels) and repeat steps 3-6 with.
 * 8. If no networks are successfully joined, change the security
 *    to use the default well-known "ZigbeeAlliance09" key.
 * 9. Repeat steps 2-7
 *    (This will again try primary channel mask first, then secondary).
 * 10. If no networks are successsully joined, setup security to use
 *     distributed link key.
 * 11. Repeat steps 2.7.
 * 12. If no networks are joined successfully fail the whole process and return
 *     control to the application.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#ifndef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS

#include "app/framework/security/af-security.h" // emAfClearLinkKeyTable()
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"

#if defined(EMBER_AF_API_SCAN_DISPATCH)
  #include EMBER_AF_API_SCAN_DISPATCH
#elif defined(EMBER_TEST)
  #include "../scan-dispatch/scan-dispatch.h"
#endif

#if defined(EMBER_AF_API_UPDATE_TC_LINK_KEY)
  #include EMBER_AF_API_UPDATE_TC_LINK_KEY
#elif defined(EMBER_TEST)
  #include "../update-tc-link-key/update-tc-link-key.h"
#endif

#ifdef EMBER_TEST
  #define HIDDEN
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER 3
#else
  #define HIDDEN static
#endif

//============================================================================
// Globals

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK \
  (BIT32(11) | BIT32(14))
#endif

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION 5
#endif

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S (180)
#endif

#ifdef  EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_CALLBACK
  #define GET_RADIO_TX_POWER(channel) emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(channel)
#else
  #define GET_RADIO_TX_POWER(channel) EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER
#endif

const char * emAfPluginNetworkSteeringStateNames[] = {
  "None",
  // These next two states are only run if explicitly configured to do so
  // See emAfPluginNetworkSteeringSetConfiguredKey()
  "Scan Primary Channels and use Configured Key",
  "Scan Secondary Channels and use Configured Key",
  "Scan Primary Channels and use Install Code",
  "Scan Secondary Channels and use Install Code",
  "Scan Primary Channels and Use Centralized Key",
  "Scan Secondary Channels and Use Centralized Key",
  "Scan Primary Channels and Use Distributed Key",
  "Scan Secondary Channels and Use Distributed Key",
};

#define LAST_JOINING_STATE \
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED

EmberAfPluginNetworkSteeringJoiningState emAfPluginNetworkSteeringState
  = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;

const uint8_t emAfNetworkSteeringPluginName[] = "NWK Steering";
#define PLUGIN_NAME emAfNetworkSteeringPluginName

// #define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
  #define debugPrint(...)   emberAfCorePrint(__VA_ARGS__)
  #define debugExec(x) do { x; } while (0)
#else
  #define debugPrintln(...)
  #define debugPrint(...)
  #define debugExec(x)
#endif

#define SECONDARY_CHANNEL_MASK EMBER_ALL_802_15_4_CHANNELS_MASK

#define REQUIRED_STACK_PROFILE 2

static const EmberKeyData defaultLinkKey = {
  { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,
    0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 }
};
static const EmberKeyData distributedTestKey = {
  { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF }
};

// These parameters allow for filtering which networks to find or which specific
// key to use
static bool gFilterByExtendedPanId = false;
static uint8_t gExtendedPanIdToFilterOn[8];
static bool gUseConfiguredKey = false;
static EmberKeyData gConfiguredKey = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static bool printedMaxPanIdsWarning = false;
uint8_t emAfPluginNetworkSteeringPanIdIndex = 0;
uint8_t emAfPluginNetworkSteeringCurrentChannel;

// We make these into variables so that they can be changed at run time.
// This is very useful for unit and interop tests.
uint32_t emAfPluginNetworkSteeringPrimaryChannelMask
  = EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK;
uint32_t emAfPluginNetworkSteeringSecondaryChannelMask
  = SECONDARY_CHANNEL_MASK;

uint8_t emAfPluginNetworkSteeringTotalBeacons = 0;
uint8_t emAfPluginNetworkSteeringJoinAttempts = 0;
EmberKeyData emberPluginNetworkSteeringDistributedKey;

static uint32_t currentChannelMask = 0;

EmberEventControl emberAfPluginNetworkSteeringFinishSteeringEventControl;
#define finishSteeringEvent (emberAfPluginNetworkSteeringFinishSteeringEventControl)

// TODO: good value for this?
// Let's try jittering our TCLK update and permit join broadcast to cut down
// on commission-time traffic.
#define FINISH_STEERING_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND << 1)
#define FINISH_STEERING_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND << 2)
#define randomJitterMS()                                               \
  ((emberGetPseudoRandomNumber()                                       \
    % (FINISH_STEERING_JITTER_MAX_MS - FINISH_STEERING_JITTER_MIN_MS)) \
   + FINISH_STEERING_JITTER_MIN_MS)
#define UPDATE_TC_LINK_KEY_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND * 10)
#define UPDATE_TC_LINK_KEY_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND * 40)

// This is an attribute specified in the BDB.
#define VERIFY_KEY_TIMEOUT_MS (5 * MILLISECOND_TICKS_PER_SECOND)

EmberAfPluginNetworkSteeringOptions emAfPluginNetworkSteeringOptionsMask
  = EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NONE;

//============================================================================
// Forward Declarations

static void cleanupAndStop(EmberStatus status);
static EmberStatus stateMachineRun(void);
static uint8_t getNextChannel(void);
static EmberStatus tryNextMethod(void);
static EmberStatus setupSecurity(void);
static uint32_t jitterTimeDelayMs();
HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results);
bool emIsWellKnownKey(EmberKeyData key);

// Callback declarations for the sake of the compiler during unit tests.
int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel);
bool emberAfPluginNetworkSteeringGetDistributedKeyCallback(EmberKeyData * key);
EmberNodeType emberAfPluginNetworkSteeringGetNodeTypeCallback(EmberAfPluginNetworkSteeringJoiningState state);

//============================================================================
// State Machine

static bool addPanIdCandidate(uint16_t panId)
{
  uint16_t* panIdPointer = emAfPluginNetworkSteeringGetStoredPanIdPointer(0);
  if (panIdPointer == NULL) {
    emberAfCorePrintln("Error: %p could not get memory pointer for stored PAN IDs",
                       PLUGIN_NAME);
    return false;
  }
  uint8_t maxNetworks = emAfPluginNetworkSteeringGetMaxPossiblePanIds();
  uint8_t i;
  for (i = 0; i < maxNetworks; i++) {
    if (panId == *panIdPointer) {
      // We already know about this PAN, no point in recording it twice.
      debugPrintln("Ignoring duplicate PAN ID 0x%2X", panId);
      return true;
    } else if (*panIdPointer == EMBER_AF_INVALID_PAN_ID) {
      *panIdPointer = panId;
      debugPrintln("Stored PAN ID 0x%2X", *panIdPointer);
      return true;
    }
    panIdPointer++;
  }

  if (!printedMaxPanIdsWarning) {
    emberAfCorePrintln("Warning: %p Max PANs reached (%d)",
                       PLUGIN_NAME,
                       maxNetworks);
    printedMaxPanIdsWarning = true;
  }
  return true;
}

static void clearPanIdCandidates(void)
{
  printedMaxPanIdsWarning = false;
  emAfPluginNetworkSteeringClearStoredPanIds();
  emAfPluginNetworkSteeringPanIdIndex = 0;
}

static uint16_t getNextCandidate(void)
{
  debugPrintln("Getting candidate at index %d", emAfPluginNetworkSteeringPanIdIndex);
  uint16_t* pointer =
    emAfPluginNetworkSteeringGetStoredPanIdPointer(emAfPluginNetworkSteeringPanIdIndex);
  if (pointer == NULL) {
    debugPrintln("Error: %p could not get pointer to stored PANs", PLUGIN_NAME);
    return EMBER_AF_INVALID_PAN_ID;
  }
  emAfPluginNetworkSteeringPanIdIndex++;
  return *pointer;
}

void gotoNextChannel(void)
{
  EmberAfPluginScanDispatchScanData scanData;
  EmberStatus status;

  emAfPluginNetworkSteeringCurrentChannel = getNextChannel();
  if (emAfPluginNetworkSteeringCurrentChannel == 0) {
    debugPrintln("No more channels");
    tryNextMethod();
    return;
  }
  clearPanIdCandidates();

  scanData.scanType = EMBER_ACTIVE_SCAN;
  scanData.channelMask = BIT32(emAfPluginNetworkSteeringCurrentChannel);
  scanData.duration = EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION;
  scanData.handler = scanResultsHandler;
  status = emberAfPluginScanDispatchScheduleScan(&scanData);

  if (EMBER_SUCCESS != status) {
    emberAfCorePrintln("Error: %p start scan failed: 0x%X", PLUGIN_NAME, status);
    cleanupAndStop(status);
  } else {
    emberAfCorePrintln("Starting scan on channel %d",
                       emAfPluginNetworkSteeringCurrentChannel);
  }
}

void tryToJoinNetwork(void)
{
  EmberNetworkParameters networkParams;
  EmberStatus status;
  EmberNodeType nodeType;

  MEMSET(&networkParams, 0, sizeof(EmberNetworkParameters));

  networkParams.panId = getNextCandidate();
  if (networkParams.panId == EMBER_AF_INVALID_PAN_ID) {
    debugPrintln("No networks to join on channel %d", emAfPluginNetworkSteeringCurrentChannel);
    gotoNextChannel();
    return;
  }

  emberAfCorePrintln("%p joining 0x%2x", PLUGIN_NAME, networkParams.panId);
  networkParams.radioChannel = emAfPluginNetworkSteeringCurrentChannel;
  networkParams.radioTxPower = GET_RADIO_TX_POWER(emAfPluginNetworkSteeringCurrentChannel);
  nodeType = emberAfPluginNetworkSteeringGetNodeTypeCallback(emAfPluginNetworkSteeringState);
  status = emberJoinNetwork(nodeType, &networkParams);
  emAfPluginNetworkSteeringJoinAttempts++;
  if (EMBER_SUCCESS != status) {
    emberAfCorePrintln("Error: %p could not attempt join: 0x%X",
                       PLUGIN_NAME,
                       status);
    cleanupAndStop(status);
    return;
  }
}

//Description: Generates a random number between 10000-40000.
static uint32_t jitterTimeDelayMs()
{
  uint16_t seed;
  halStackSeedRandom((uint32_t)&seed);
  uint32_t jitterDelayMs = (emberGetPseudoRandomNumber() % (UPDATE_TC_LINK_KEY_JITTER_MAX_MS - UPDATE_TC_LINK_KEY_JITTER_MIN_MS + 1)) + UPDATE_TC_LINK_KEY_JITTER_MIN_MS;
  return jitterDelayMs;
}

void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status)
{
  if (emAfPluginNetworkSteeringState
      == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    EmberKeyStruct entry;
    EmberStatus keystatus = emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &entry);
    if (keystatus == EMBER_SUCCESS && emIsWellKnownKey(entry.key) && status == EMBER_NETWORK_UP) {
      emberAfPluginUpdateTcLinkKeySetDelay(jitterTimeDelayMs());
    } else if (status == EMBER_NETWORK_DOWN) {
      emberAfPluginUpdateTcLinkKeySetInactive();
    }
    return;
  } else if (status == EMBER_NETWORK_UP) {
    emberAfCorePrintln("%p network joined.", PLUGIN_NAME);
    if (!emAfPluginNetworkSteeringStateUsesDistributedKey()
        && !(emAfPluginNetworkSteeringOptionsMask
             & EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE)) {
      emAfPluginNetworkSteeringStateSetUpdateTclk();
    }
    emberEventControlSetDelayMS(finishSteeringEvent, randomJitterMS());
    return;
  }

  tryToJoinNetwork();
}

// Returns true if the key value is equal to defaultLinkKey
bool emIsWellKnownKey(EmberKeyData key)
{
  for (uint8_t i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    if (key.contents[i] != defaultLinkKey.contents[i]) {
      return false;
    }
  }
  return true;
}

static void scanCompleteCallback(uint8_t channel, EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: Scan complete handler returned 0x%X", status);
  }

  // EMAPPFWKV2-1462 - make sure we didn't cleanupAndStop() above.
  if (emAfPluginNetworkSteeringState
      != EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    tryToJoinNetwork();
  }
}

static void networkFoundCallback(EmberZigbeeNetwork *networkFound,
                                 uint8_t lqi,
                                 int8_t rssi)
{
  emAfPluginNetworkSteeringTotalBeacons++;

  if (!(networkFound->allowingJoin
        && networkFound->stackProfile == REQUIRED_STACK_PROFILE)) {
    return;
  }

  debugPrint("%p nwk found ch: %d, panID 0x%2X, xpan: ",
             PLUGIN_NAME,
             networkFound->channel,
             networkFound->panId);
  debugExec(emberAfPrintBigEndianEui64(networkFound->extendedPanId));

  // If we were told to filter by extended PAN ID, do so now
  if (gFilterByExtendedPanId) {
    if (0 != MEMCOMPARE(networkFound->extendedPanId,
                        gExtendedPanIdToFilterOn,
                        COUNTOF(gExtendedPanIdToFilterOn))) {
      debugPrint(". Skipping since we are looking for xpan: ");
      debugExec(emberAfPrintBigEndianEui64(gExtendedPanIdToFilterOn));
      debugPrintln("");
      return;
    }
  }

  debugPrintln("");

  if (!addPanIdCandidate(networkFound->panId)) {
    emberAfCorePrintln("Error: %p could not add candidate network.",
                       PLUGIN_NAME);
    cleanupAndStop(EMBER_ERR_FATAL);
    return;
  }
}

HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results)
{
  if (emberAfPluginScanDispatchScanResultsAreComplete(results)
      || emberAfPluginScanDispatchScanResultsAreFailure(results)) {
    scanCompleteCallback(results->channel, results->status);
  } else {
    networkFoundCallback(results->network,
                         results->lqi,
                         results->rssi);
  }
}

static EmberStatus tryNextMethod(void)
{
  emAfPluginNetworkSteeringState++;
  if (emAfPluginNetworkSteeringState > LAST_JOINING_STATE) {
    EmberStatus status = (emAfPluginNetworkSteeringTotalBeacons > 0
                          ? EMBER_JOIN_FAILED
                          : EMBER_NO_BEACONS);
    cleanupAndStop(status);
    return status;
  }
  return stateMachineRun();
}

static void cleanupAndStop(EmberStatus status)
{
  emberAfCorePrintln("%p Stop.  Cleaning up.", PLUGIN_NAME);
  emberAfPluginNetworkSteeringCompleteCallback(status,
                                               emAfPluginNetworkSteeringTotalBeacons,
                                               emAfPluginNetworkSteeringJoinAttempts,
                                               emAfPluginNetworkSteeringState);
  emAfPluginNetworkSteeringClearStoredPanIds();
  emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
  emAfPluginNetworkSteeringPanIdIndex = 0;
  emAfPluginNetworkSteeringJoinAttempts = 0;
  emAfPluginNetworkSteeringTotalBeacons = 0;
}

static uint8_t getNextChannel(void)
{
  if (emAfPluginNetworkSteeringCurrentChannel == 0) {
    emAfPluginNetworkSteeringCurrentChannel = (emberGetPseudoRandomNumber() & 0x0F)
                                              + EMBER_MIN_802_15_4_CHANNEL_NUMBER;
    debugPrintln("Randomly choosing a starting channel %d.", emAfPluginNetworkSteeringCurrentChannel);
  } else {
    emAfPluginNetworkSteeringCurrentChannel++;
  }
  while (currentChannelMask != 0) {
    if (BIT32(emAfPluginNetworkSteeringCurrentChannel) & currentChannelMask) {
      currentChannelMask &= ~(BIT32(emAfPluginNetworkSteeringCurrentChannel));
      return emAfPluginNetworkSteeringCurrentChannel;
    }
    emAfPluginNetworkSteeringCurrentChannel++;
    if (emAfPluginNetworkSteeringCurrentChannel > EMBER_MAX_802_15_4_CHANNEL_NUMBER) {
      emAfPluginNetworkSteeringCurrentChannel = EMBER_MIN_802_15_4_CHANNEL_NUMBER;
    }
  }
  return 0;
}

static EmberStatus stateMachineRun(void)
{
  EmberStatus status = EMBER_SUCCESS;
  emberAfCorePrintln("%p State: %p",
                     PLUGIN_NAME,
                     emAfPluginNetworkSteeringStateNames[emAfPluginNetworkSteeringState]);

  if ((status = setupSecurity()) != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: %p could not setup security: 0x%X",
                       PLUGIN_NAME,
                       status);
    return tryNextMethod();
  }

  switch (emAfPluginNetworkSteeringState) {
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED:
      currentChannelMask = emAfPluginNetworkSteeringPrimaryChannelMask;
      break;
    default:
      currentChannelMask = emAfPluginNetworkSteeringSecondaryChannelMask;
      break;
  }

  debugPrintln("Channel Mask: 0x%4X", currentChannelMask);
  gotoNextChannel();
  return status;
}

static EmberStatus setupSecurity(void)
{
  EmberStatus status;
  EmberInitialSecurityState state;
  EmberExtendedSecurityBitmask extended;

  state.bitmask = (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY
                   | EMBER_HAVE_PRECONFIGURED_KEY
                   | EMBER_REQUIRE_ENCRYPTED_KEY
                   | EMBER_NO_FRAME_COUNTER_RESET
                   | (emAfPluginNetworkSteeringStateUsesInstallCodes()
                      ? EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE
                      : 0)
                   | (emAfPluginNetworkSteeringStateUsesDistributedKey()
                      ? EMBER_DISTRIBUTED_TRUST_CENTER_MODE
                      : 0)
                   );

  if (!emberAfPluginNetworkSteeringGetDistributedKeyCallback(&emberPluginNetworkSteeringDistributedKey)) {
    MEMMOVE(emberKeyContents(&emberPluginNetworkSteeringDistributedKey),
            emberKeyContents(&distributedTestKey),
            EMBER_ENCRYPTION_KEY_SIZE);
  }
  MEMMOVE(emberKeyContents(&(state.preconfiguredKey)),
          gUseConfiguredKey ? emberKeyContents(&(gConfiguredKey))
          : (emAfPluginNetworkSteeringStateUsesDistributedKey()
             ? emberKeyContents(&emberPluginNetworkSteeringDistributedKey)
             : emberKeyContents(&defaultLinkKey)),
          EMBER_ENCRYPTION_KEY_SIZE);

  if ((status = emberSetInitialSecurityState(&state))
      != EMBER_SUCCESS) {
    goto done;
  }

  extended = (EMBER_JOINER_GLOBAL_LINK_KEY
              | EMBER_EXT_NO_FRAME_COUNTER_RESET);

  if ((status = emberSetExtendedSecurityBitmask(extended)) != EMBER_SUCCESS) {
    goto done;
  }

  emAfClearLinkKeyTable();

  done:
  return status;
}

EmberStatus emberAfPluginNetworkSteeringStart(void)
{
  EmberStatus status = EMBER_INVALID_CALL;

  if (emAfProIsCurrentNetwork()
      && (emAfPluginNetworkSteeringState
          == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)) {
    if (emberAfNetworkState() == EMBER_NO_NETWORK) {
      emAfPluginNetworkSteeringState = gUseConfiguredKey
                                       ? EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED
                                       : EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE;

      // Stop any previous trust center link key update.
      emberAfPluginUpdateTcLinkKeyStop();

      status = stateMachineRun();
    } else {
      status = emberAfPermitJoin(EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S,
                                 true); // Broadcast permit join?
    }
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     emAfNetworkSteeringPluginName,
                     "Start",
                     status);

  return status;
}

EmberStatus emberAfPluginNetworkSteeringStop(void)
{
  if (emAfPluginNetworkSteeringState
      == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    return EMBER_INVALID_CALL;
  }
  cleanupAndStop(EMBER_JOIN_FAILED);
  return EMBER_SUCCESS;
}

// =============================================================================
// Finish Steering

// At the end of the network steering process, we need to update the
// trust center link key (if we are in a centralized network) and broadcast
// a permit join to extend the network. This process needs to happen after
// we send our device announce and possibly our network timeout request if we
// are an end device.

void emberAfPluginNetworkSteeringFinishSteeringEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(finishSteeringEvent);

  if (emAfPluginNetworkSteeringStateVerifyTclk()) {
    // If we get here, then we have failed to verify the TCLK. Therefore,
    // we leave the network.
    emberAfPluginUpdateTcLinkKeyStop();
    emberLeaveNetwork();
    emberAfCorePrintln("%p: %p",
                       PLUGIN_NAME,
                       "Key verification failed. Leaving network");
    cleanupAndStop(EMBER_ERR_FATAL);
  } else if (emAfPluginNetworkSteeringStateUpdateTclk()) {
    // Start the process to update the TC link key. We will set another event
    // for the broadcast permit join.
    // Attempt a TC link key update now.
    emberAfPluginUpdateTcLinkKeySetDelay(0);
  } else {
    // Broadcast permit join to extend the network.
    // We are done!
    status = emberAfPermitJoin(EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S,
                               true); // Broadcast permit join?
    emberAfCorePrintln("%p: %p: 0x%X",
                       PLUGIN_NAME,
                       "Broadcasting permit join",
                       status);
    cleanupAndStop(status);
  }
}

void emberAfPluginUpdateTcLinkKeyStatusCallback(EmberKeyStatus keyStatus)
{
  if (emAfPluginNetworkSteeringStateUpdateTclk()) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       PLUGIN_NAME,
                       "Trust center link key update status",
                       keyStatus);
    switch (keyStatus) {
      case EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED:
        // Success! But we should still wait to make sure we verify the key.
        emAfPluginNetworkSteeringStateSetVerifyTclk();
        emberEventControlSetDelayMS(finishSteeringEvent, VERIFY_KEY_TIMEOUT_MS);
        return;
      case EMBER_TRUST_CENTER_IS_PRE_R21:
      case EMBER_VERIFY_LINK_KEY_SUCCESS:
        // If the trust center is pre-r21, then we don't update the link key.
        // If the key status is that the link key has been verified, then we
        // have successfully updated our trust center link key and we are done!
        emAfPluginNetworkSteeringStateClearVerifyTclk();
        emberEventControlSetDelayMS(finishSteeringEvent, randomJitterMS());
        break;
      default:
        // Failure!
        emberLeaveNetwork();
        cleanupAndStop(EMBER_NO_LINK_KEY_RECEIVED);
    }
    emAfPluginNetworkSteeringStateClearUpdateTclk();
  }

  return;
}

void emAfPluginNetworkSteeringSetChannelMask(uint32_t mask, bool secondaryMask)
{
  if (secondaryMask) {
    emAfPluginNetworkSteeringSecondaryChannelMask = mask;
  } else {
    emAfPluginNetworkSteeringPrimaryChannelMask = mask;
  }
}

void emAfPluginNetworkSteeringSetExtendedPanIdFilter(uint8_t* extendedPanId,
                                                     bool turnFilterOn)
{
  if (!extendedPanId) {
    return;
  }
  MEMCOPY(gExtendedPanIdToFilterOn,
          extendedPanId,
          COUNTOF(gExtendedPanIdToFilterOn));
  gFilterByExtendedPanId = turnFilterOn;
}

void emAfPluginNetworkSteeringSetConfiguredKey(uint8_t *key,
                                               bool useConfiguredKey)
{
  if (!key) {
    return;
  }
  MEMCOPY(gConfiguredKey.contents, key, EMBER_ENCRYPTION_KEY_SIZE);
  gUseConfiguredKey = useConfiguredKey;
}

void emAfPluginNetworkSteeringCleanup(EmberStatus status)
{
  cleanupAndStop(status);
}

uint8_t emAfPluginNetworkSteeringGetCurrentChannel()
{
  return emAfPluginNetworkSteeringCurrentChannel;
}

#endif // #ifndef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
