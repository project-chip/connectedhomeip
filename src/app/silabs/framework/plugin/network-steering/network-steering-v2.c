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
 * @brief This file implements the Zigbee 3.0 network steering procedure using
 * only one 15.4 scan.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"

#ifdef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS

#include "app/framework/security/af-security.h" // emAfClearLinkKeyTable()
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"
#include "stack/include/network-formation.h"

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
  "Scan Primary Channels and use Install Code",
  "Scan Primary Channels and Use Centralized Key",
  "Scan Primary Channels and Use Distributed Key",
  "Scan Secondary Channels and use Install Code",
  "Scan Secondary Channels and Use Centralized Key",
  "Scan Secondary Channels and Use Distributed Key",
  // The Use All Keys states are run only if the plugin option is selected, in
  // which case the other states are not run
  "Scan Primary Channels and Use All Keys (install code, centralized, distributed)",
  "Scan Secondary Channels and Use All Keys (install code, centralized, distributed)",
};

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

#define SECONDARY_CHANNEL_MASK                                                        \
  ((EMBER_ALL_802_15_4_CHANNELS_MASK ^ EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK) \
   & EMBER_ALL_802_15_4_CHANNELS_MASK)

#define WILDCARD_EUI  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

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
static uint32_t scheduleScanChannelMask = 0;

// We make these into variables so that they can be changed at run time.
// This is very useful for unit and interop tests.
uint32_t emAfPluginNetworkSteeringPrimaryChannelMask
  = EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK;
uint32_t emAfPluginNetworkSteeringSecondaryChannelMask
  = SECONDARY_CHANNEL_MASK;

uint8_t emAfPluginNetworkSteeringTotalBeacons = 0;
uint8_t emAfPluginNetworkSteeringJoinAttempts = 0;
EmberKeyData emberPluginNetworkSteeringDistributedKey;

NetworkSteeringState steeringState = { 0 };
// Shorthand names
#define BEACON_ITERATOR         steeringState.beaconIterator
#define CURRENT_CHANNEL_MASK    steeringState.currentChannelMask

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

#ifdef EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS
  #define FIRST_PRIMARY_STATE     EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS
  #define FIRST_SECONDARY_STATE   EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS
#else // EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS
  #define FIRST_PRIMARY_STATE     EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE
  #define FIRST_SECONDARY_STATE   EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE
#endif // EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS

//============================================================================
// Externs

#ifdef EZSP_HOST
 #define setMaxBeaconsToStore(n)  { \
    uint8_t val = n;                \
    (void)ezspSetValue(EZSP_VALUE_MAX_BEACONS_TO_STORE, 1, &val); }
#else // EZSP_HOST
 #define setMaxBeaconsToStore(n)  { \
    (void)emberSetNumBeaconsToStore(n); }
#endif // EZSP_HOST

//============================================================================
// Forward Declarations

static void cleanupAndStop(EmberStatus status);
static void stateMachineRun(void);
static void tryToJoinNetwork(void);
static EmberStatus goToNextState(void);
static EmberStatus setupSecurity(void);
static uint32_t jitterTimeDelayMs();
HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results);
bool emIsWellKnownKey(EmberKeyData key);
static EmberStatus scheduleScan(uint32_t channelMask);

// Callback declarations for the sake of the compiler during unit tests.
int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel);
bool emberAfPluginNetworkSteeringGetDistributedKeyCallback(EmberKeyData * key);
EmberNodeType emberAfPluginNetworkSteeringGetNodeTypeCallback(EmberAfPluginNetworkSteeringJoiningState state);

//============================================================================
// State Machine

void tryToJoinNetwork()
{
  EmberStatus status = EMBER_SUCCESS;
  EmberNodeType nodeType;
  int8_t radioTxPower;

  emberAfCorePrintln("Examining beacon on channel %d with panId 0x%2X",
                     BEACON_ITERATOR.beacon.channel,
                     BEACON_ITERATOR.beacon.panId);

  if (!(CURRENT_CHANNEL_MASK & BIT32(BEACON_ITERATOR.beacon.channel))) {
    emberAfCorePrintln("This beacon is not part of the current "
                       "channel mask (0x%4X)."
                       " Getting next beacon whose channel bitmask is set.",
                       CURRENT_CHANNEL_MASK);
  }

  bool filteredOut = false;
  if (gFilterByExtendedPanId) {
    if (0 != MEMCOMPARE(BEACON_ITERATOR.beacon.extendedPanId,
                        gExtendedPanIdToFilterOn,
                        COUNTOF(gExtendedPanIdToFilterOn))) {
      debugPrint(". Skipping since we are looking for xpan: ");
      debugExec(emberAfPrintBigEndianEui64(gExtendedPanIdToFilterOn));
      debugPrintln("");
      filteredOut = true;
    }
  }

  // If the retrieved beacon channel is not set in the mask or we've filtered it
  // out, get the next valid beacon
  while ((!(CURRENT_CHANNEL_MASK & BIT32(BEACON_ITERATOR.beacon.channel)))
         || filteredOut) {
    status = emberGetNextBeacon(&(BEACON_ITERATOR.beacon));

    if (status != EMBER_SUCCESS) {
      break;
    }

    filteredOut = (gFilterByExtendedPanId && MEMCOMPARE(BEACON_ITERATOR.beacon.extendedPanId,
                                                        gExtendedPanIdToFilterOn,
                                                        COUNTOF(gExtendedPanIdToFilterOn)));
  }

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("No beacons left on current mask. Moving to next state");

    status = goToNextState();
    if (status != EMBER_SUCCESS) {
      return;
    }

    stateMachineRun();
    return;
  }

  emberAfCorePrintln("%p joining 0x%2x on channel %d",
                     PLUGIN_NAME,
                     BEACON_ITERATOR.beacon.panId,
                     BEACON_ITERATOR.beacon.channel);

  nodeType = emberAfPluginNetworkSteeringGetNodeTypeCallback(emAfPluginNetworkSteeringState);
  radioTxPower = GET_RADIO_TX_POWER(BEACON_ITERATOR.beacon.channel);
  status = emberJoinNetworkDirectly(nodeType,
                                    &BEACON_ITERATOR.beacon,
                                    radioTxPower,
                                    true);  // clearBeaconsAfterNetworkUp
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
  emberAfCorePrintln("%p stack status 0x%X", PLUGIN_NAME, status);

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
  } else {
    if (status == EMBER_NETWORK_UP) {
      emberAfCorePrintln("%p network joined.", PLUGIN_NAME);
      if (!emAfPluginNetworkSteeringStateUsesDistributedKey()
          && !(emAfPluginNetworkSteeringOptionsMask
               & EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE)) {
        emAfPluginNetworkSteeringStateSetUpdateTclk();
      }
      emberEventControlSetDelayMS(finishSteeringEvent, randomJitterMS());
    } else {
      if (emAfPluginNetworkSteeringState > EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE
          && emAfPluginNetworkSteeringState < EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED) {
        emberAfCorePrintln("%p trying next network.", PLUGIN_NAME);

        status = emberGetNextBeacon(&(BEACON_ITERATOR.beacon));
        if (status != EMBER_SUCCESS) {
          emberAfCorePrintln("No beacons left on current mask. "
                             "Moving to next state");

          status = goToNextState();
          if (status != EMBER_SUCCESS) {
            return;
          }

          stateMachineRun();
        } else {
          tryToJoinNetwork();
        }
      } else {
        cleanupAndStop(EMBER_NO_LINK_KEY_RECEIVED);
      }
    }
  }
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

static EmberStatus scheduleScan(uint32_t channelMask)
{
  EmberStatus status;
  EmberAfPluginScanDispatchScanData scanData;
  scheduleScanChannelMask = channelMask;
  // Kick off a scan and record beacons
  setMaxBeaconsToStore(EMBER_MAX_BEACONS_TO_STORE);

  MEMSET(&steeringState, 0, sizeof(steeringState));

  scanData.scanType = EMBER_ACTIVE_SCAN;
  scanData.channelMask = channelMask;
  scanData.duration = EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION;
  scanData.handler = scanResultsHandler;
  status = emberAfPluginScanDispatchScheduleScan(&scanData);
  emberAfCorePrintln("%s: issuing scan on %s channels (mask 0x%4X)",
                     PLUGIN_NAME,
                     channelMask == emAfPluginNetworkSteeringPrimaryChannelMask
                     ? "primary"
                     : "secondary",
                     channelMask);
  return status;
}

// This function is called after a channel mask has been scanned
HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results)
{
  if (emberAfPluginScanDispatchScanResultsAreComplete(results)
      || emberAfPluginScanDispatchScanResultsAreFailure(results)) {
    if (results->status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: Scan complete handler returned 0x%X",
                         results->status);
      // To match the change in network-steering, and scan-dispatch code
      CLEARBIT(scheduleScanChannelMask, results->channel);
      scheduleScan(scheduleScanChannelMask);
      return;
    }

    emAfPluginNetworkSteeringTotalBeacons = emberGetNumStoredBeacons();

    emberAfCorePrintln("%s scan complete. Beacons heard: %d",
                       PLUGIN_NAME,
                       emAfPluginNetworkSteeringTotalBeacons);

    if (emAfPluginNetworkSteeringTotalBeacons) {
      stateMachineRun();
    } else {
      switch (emAfPluginNetworkSteeringState) {
        case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE:
        case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED:
        case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED:
        case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS:
          cleanupAndStop(EMBER_NO_BEACONS);
          return;
          break;
        default:
          break;
      }

      emAfPluginNetworkSteeringState = FIRST_SECONDARY_STATE;

      (void)scheduleScan(emAfPluginNetworkSteeringSecondaryChannelMask);

      return;
    }
  }
}

static EmberStatus goToNextState(void)
{
  emAfPluginNetworkSteeringState++;

  // If there are no more states, return error
  if ((emAfPluginNetworkSteeringState
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS)
      || (emAfPluginNetworkSteeringState
          == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED)) {
    cleanupAndStop(EMBER_JOIN_FAILED);
    return EMBER_JOIN_FAILED;
  }

  // When we're on the final scan-primary-channels state,
  // EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED,
  // goToNextState simply increments emAfPluginNetworkSteeringState and then we
  // call stateMachineRun. We take this opportunity to check if we need to scan
  // the secondary mask now
  if ((emAfPluginNetworkSteeringState
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE)
      || (emAfPluginNetworkSteeringState
          == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS)) {
    if (emAfPluginNetworkSteeringSecondaryChannelMask == 0) {
      cleanupAndStop(EMBER_JOIN_FAILED);
      return EMBER_JOIN_FAILED;
    }

    (void)scheduleScan(emAfPluginNetworkSteeringSecondaryChannelMask);

    // We need to return any error code, so that the code that called this
    // function doesn't continue doing its routine
    return EMBER_MAC_SCANNING;
  }

  return EMBER_SUCCESS;
}

static void cleanupAndStop(EmberStatus status)
{
  emberAfCorePrintln("%p Stop.  Cleaning up.", PLUGIN_NAME);
  emberAfPluginNetworkSteeringCompleteCallback(status,
                                               emAfPluginNetworkSteeringTotalBeacons,
                                               emAfPluginNetworkSteeringJoinAttempts,
                                               emAfPluginNetworkSteeringState);
  emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
  emAfPluginNetworkSteeringJoinAttempts = 0;
  emAfPluginNetworkSteeringTotalBeacons = 0;
  emberEventControlSetInactive(finishSteeringEvent);
}

// This function is called only on state transitions. This function sets up
// security, configures parameters, and then makes a call to start joining
// networks
static void stateMachineRun(void)
{
  EmberStatus status;
  emberAfCorePrintln("%p State: %p",
                     PLUGIN_NAME,
                     emAfPluginNetworkSteeringStateNames[emAfPluginNetworkSteeringState]);

  status = setupSecurity();

  while (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: %p could not setup security: 0x%X",
                       PLUGIN_NAME,
                       status);

    status = goToNextState();
    if (status != EMBER_SUCCESS) {
      return;
    }

    status = setupSecurity();
  }

  // Set CURRENT_CHANNEL_MASK properly so that we can iterate over beacons
  // correctly
  switch (emAfPluginNetworkSteeringState) {
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED:
    case EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS:
      CURRENT_CHANNEL_MASK = emAfPluginNetworkSteeringPrimaryChannelMask;
      break;
    default:
      CURRENT_CHANNEL_MASK = emAfPluginNetworkSteeringSecondaryChannelMask;
      break;
  }

  // Clear our stored beacon so we can iterate from the start
  status = emberGetFirstBeacon(&BEACON_ITERATOR);

  if (status != EMBER_SUCCESS) {
    cleanupAndStop(EMBER_ERR_FATAL);
    return;
  }

  tryToJoinNetwork();
}

static EmberStatus setupSecurity(void)
{
  EmberStatus status;
  EmberInitialSecurityState state;
  EmberExtendedSecurityBitmask extended;
  bool tryAllKeys = ((emAfPluginNetworkSteeringState
                      == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS)
                     || (emAfPluginNetworkSteeringState
                         == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS));

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
          (emAfPluginNetworkSteeringStateUsesDistributedKey()
           ? emberKeyContents(&emberPluginNetworkSteeringDistributedKey)
           : emberKeyContents(&defaultLinkKey)),
          EMBER_ENCRYPTION_KEY_SIZE);

  status = emberSetInitialSecurityState(&state);

  // If we're trying all keys and the install code wasn't burned, setting
  // security will fail with EMBER_SECURITY_CONFIGURATION_INVALID. As a
  // consequence, we ignore the install code key and try again
  if ((status == EMBER_SECURITY_CONFIGURATION_INVALID) && tryAllKeys) {
    emberAfCorePrintln("%s: WARNING: emberSetInitialSecurityState failed. "
                       "Continuing without install code.",
                       emAfNetworkSteeringPluginName);
    state.bitmask &= ((uint16_t) ~((uint16_t)EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE));
    status = emberSetInitialSecurityState(&state);
  }

  if (status != EMBER_SUCCESS) {
    goto done;
  }

  extended = (EMBER_JOINER_GLOBAL_LINK_KEY
              | EMBER_EXT_NO_FRAME_COUNTER_RESET);

  if ((status = emberSetExtendedSecurityBitmask(extended)) != EMBER_SUCCESS) {
    goto done;
  }

  emAfClearLinkKeyTable();

  if (tryAllKeys) {
    EmberEUI64 wildcardEui = WILDCARD_EUI;

    // The install code derived key is handled by setting the
    // EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE bit above
    // Add the centralized and distributed keys into the transient key table
    status = emberAddTransientLinkKey(wildcardEui,
                                      (EmberKeyData*)&defaultLinkKey);
    if (status == EMBER_SUCCESS) {
      status = emberAddTransientLinkKey(wildcardEui,
                                        (EmberKeyData*)&distributedTestKey);
    }
  }

  done:
  return status;
}

EmberStatus emberAfPluginNetworkSteeringStart(void)
{
  EmberStatus status = EMBER_INVALID_CALL;
  uint32_t channelsToScan;

  if ((0 == emAfPluginNetworkSteeringPrimaryChannelMask)
      && (0 == emAfPluginNetworkSteeringSecondaryChannelMask)) {
    return EMBER_INVALID_CALL;
  }

  if (emAfProIsCurrentNetwork()
      && (emAfPluginNetworkSteeringState
          == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)) {
    if (emberAfNetworkState() == EMBER_NO_NETWORK) {
      if (emAfPluginNetworkSteeringPrimaryChannelMask) {
        emAfPluginNetworkSteeringState = FIRST_PRIMARY_STATE;
        channelsToScan = emAfPluginNetworkSteeringPrimaryChannelMask;
      } else {
        emAfPluginNetworkSteeringState = FIRST_SECONDARY_STATE;
        channelsToScan = emAfPluginNetworkSteeringSecondaryChannelMask;
      }

      // Stop any previous trust center link key update.
      emberAfPluginUpdateTcLinkKeyStop();

      status = scheduleScan(channelsToScan);
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
      {
        // If the trust center is pre-r21, then we don't update the link key.
        // If the key status is that the link key has been verified, then we
        // have successfully updated our trust center link key and we are done!
        emAfPluginNetworkSteeringStateClearVerifyTclk();
        uint32_t ms = randomJitterMS();
        emberEventControlSetDelayMS(finishSteeringEvent, ms);
        break;
      }
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

void emAfPluginNetworkSteeringCleanup(EmberStatus status)
{
  cleanupAndStop(status);
}

uint8_t emAfPluginNetworkSteeringGetCurrentChannel()
{
  return BEACON_ITERATOR.beacon.channel;
}

#endif // #ifdef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
