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
 * @brief APIs and defines for the Network Creator plugin, which handles forming
 *        a network per the Zigbee Base Device Behavior specification.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"

#include "network-creator.h"
#include "network-creator-composite.h"

#if defined(EMBER_AF_API_SCAN_DISPATCH)
  #include EMBER_AF_API_SCAN_DISPATCH
#elif defined(EMBER_TEST)
  #include "../scan-dispatch/scan-dispatch.h"
#endif

#if defined(EMBER_AF_API_NETWORK_CREATOR_SECURITY)
  #include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#elif defined(EMBER_TEST)
  #include "../network-creator-security/network-creator-security.h"
#endif

#ifdef EMBER_TEST
  #ifndef EMBER_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION
    #define EMBER_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION 0x04
  #endif
  #ifndef EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK
    #define EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK 0x02108800
  #endif
  #ifndef EMBER_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER
    #define EMBER_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER 3
  #endif
  #define HIDDEN
#else
  #define HIDDEN static
#endif

//#define EM_AF_PLUGIN_NETWORK_CREATOR_DEBUG
#ifdef EM_AF_PLUGIN_NETWORK_CREATOR_DEBUG
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

// -----------------------------------------------------------------------------
// Globals

uint32_t emAfPluginNetworkCreatorPrimaryChannelMask = EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK;

// The Base Device spec (13-0402) says, by default, define the secondary
// channel mask to be all channels XOR the primary mask. See 6.2, table 2.
#define SECONDARY_CHANNEL_MASK \
  (EMBER_ALL_802_15_4_CHANNELS_MASK ^ EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK)
uint32_t emAfPluginNetworkCreatorSecondaryChannelMask = (uint32_t)SECONDARY_CHANNEL_MASK;

static uint32_t currentChannelMask;

static EmAfPluginNetworkCreatorChannelComposite channelComposites[EMBER_NUM_802_15_4_CHANNELS];

static bool doFormCentralizedNetwork = true;

static uint8_t stateFlags = 0;
#define clearStateFlags() (stateFlags = 0)

#define STATE_FLAGS_WAITING_FOR_SCAN (0x01)
#define waitingForScan() (stateFlags & STATE_FLAGS_WAITING_FOR_SCAN)

#define STATE_FLAGS_MASK_IS_SECONDARY (0x02)
#define maskIsSecondary() (stateFlags & STATE_FLAGS_MASK_IS_SECONDARY)

// -----------------------------------------------------------------------------
// Declarations

// For the sake of the compiler in unit tests.
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels);
EmberPanId emberAfPluginNetworkCreatorGetPanIdCallback(void);
int8_t emberAfPluginNetworkCreatorGetRadioPowerCallback(void);

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static EmberStatus scheduleScans(void);

static void cleanChannelComposites(void);
static void updateChannelComposites(int8_t rssi,
                                    EmAfPluginNetworkCreatorChannelCompositeMetric metric,
                                    uint8_t channel);
static void maybeClearChannelBitOfMaxRssiReading(uint8_t channel);

static void cleanupAndStop(EmberStatus status);
static void fillExtendedPanId(uint8_t *extendedPanId);
static uint32_t makeLeastWorstChannelMask(void);

// -----------------------------------------------------------------------------
// Public API Definitions

EmberStatus emberAfPluginNetworkCreatorStart(bool centralizedNetwork)
{
  if (waitingForScan() || (emberAfNetworkState() != EMBER_NO_NETWORK)) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                       "Cannot start. State",
                       stateFlags);
    return EMBER_INVALID_CALL;
  }

  doFormCentralizedNetwork = centralizedNetwork;

  // Reset channel masks and composites and state.
  currentChannelMask = emAfPluginNetworkCreatorPrimaryChannelMask;
  cleanChannelComposites();
  clearStateFlags();

  return scheduleScans();
}

void emberAfPluginNetworkCreatorStop(void)
{
  cleanupAndStop(EMBER_ERR_FATAL);
}

EmberStatus emberAfPluginNetworkCreatorNetworkForm(bool centralizedNetwork,
                                                   EmberPanId panId,
                                                   int8_t radioTxPower,
                                                   uint8_t channel)
{
  EmberStatus status;
  EmberNetworkParameters networkParameters;

  networkParameters.panId = panId;
  networkParameters.radioTxPower = radioTxPower;
  fillExtendedPanId(networkParameters.extendedPanId);
  networkParameters.radioChannel = channel;

  if ((channel > EMBER_MAX_802_15_4_CHANNEL_NUMBER)
      || (channel < EMBER_MIN_802_15_4_CHANNEL_NUMBER)) {
    emberAfCorePrintln("%p: Error: Channel %d is out of range",
                       EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                       channel);
    return EMBER_BAD_ARGUMENT;
  }

  doFormCentralizedNetwork = centralizedNetwork;

  status = emberAfPluginNetworkCreatorSecurityStart(centralizedNetwork);
  if (status == EMBER_SUCCESS) {
    status = emberFormNetwork(&networkParameters);
    emberAfCorePrintln("%p: Form. Channel: %d. Status: 0x%X",
                       EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                       channel,
                       status);
  }

  return status;
}

// -----------------------------------------------------------------------------
// Private API Definitions

static void fillExtendedPanId(uint8_t *extendedPanId)
{
  uint8_t i;
  bool invalid = true;

  for (i = 0; i < EXTENDED_PAN_ID_SIZE && invalid; i++) {
    invalid = (emAfExtendedPanId[i] == 0x00 || emAfExtendedPanId[i] == 0xFF);
  }

  if (invalid) {
    for (i = 0; i < EXTENDED_PAN_ID_SIZE; i++) {
      extendedPanId[i] = emberGetPseudoRandomNumber();
    }
  } else {
    MEMMOVE(extendedPanId, emAfExtendedPanId, EXTENDED_PAN_ID_SIZE);
  }
}

static EmberStatus tryToFormNetwork(void)
{
  EmberNetworkParameters networkParameters;
  EmberStatus status;
  uint8_t channel = (emberGetPseudoRandomNumber() & 0x0F)
                    + EMBER_MIN_802_15_4_CHANNEL_NUMBER;
  EmberPanId panId = emberAfPluginNetworkCreatorGetPanIdCallback();

  networkParameters.panId = panId;
  networkParameters.radioTxPower = emberAfPluginNetworkCreatorGetRadioPowerCallback();
  fillExtendedPanId(networkParameters.extendedPanId);

  // If we are on our secondary channel mask and we didn't find any channels that
  // were below our channel composite thresholds, then we pick from the "least
  // worst channels" so that network formation is still possible.
  if (maskIsSecondary() && currentChannelMask == 0) {
    currentChannelMask = makeLeastWorstChannelMask();
  }

  status = emberAfPluginNetworkCreatorSecurityStart(doFormCentralizedNetwork);
  if (status == EMBER_SUCCESS) {
    status = EMBER_ERR_FATAL;
    while (currentChannelMask != 0 && status != EMBER_SUCCESS) {
      // Find the next channel on which to try forming a network.
      channel = (channel == EMBER_MAX_802_15_4_CHANNEL_NUMBER
                 ? EMBER_MIN_802_15_4_CHANNEL_NUMBER
                 : channel + 1);
      if (!READBIT(currentChannelMask, channel)) {
        continue;
      }

      // Try to form the network.
      networkParameters.radioChannel = channel;
      status = emberFormNetwork(&networkParameters);
      emberAfCorePrintln("%p: Form. Channel: %d. Status: 0x%X",
                         EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                         channel,
                         status);

      // If you pass, then tell the user.
      // Else, clear the channel bit that you just tried.
      if (status == EMBER_SUCCESS) {
        emberAfPluginNetworkCreatorCompleteCallback(&networkParameters,
                                                    (maskIsSecondary()
                                                     ? true
                                                     : false));
      } else {
        CLEARBIT(currentChannelMask, channel);
      }
    }
  }

  return status;
}

static void handleScanComplete(EmberAfPluginScanDispatchScanResults *results)
{
  EmberNetworkScanType scanType
    = emberAfPluginScanDispatchScanResultsGetScanType(results);

  // If then scan was unsuccessful...
  if (results->status != EMBER_SUCCESS) {
    // ...just turn off the channel on which the scan failed. The
    // network-creator will disregard this channel in the network
    // formation process.
    CLEARBIT(currentChannelMask, results->channel);
  } else {
    // If the scan was energy, then we have all of our scan data, so try
    // to form.
    if (scanType == EMBER_ENERGY_SCAN) {
      results->status = tryToFormNetwork();
      // If we were not successful...
      if (results->status != EMBER_SUCCESS) {
        // ...then try the secondary mask if we were on the primary...
        // ...else fail because we tried both masks.
        if (!maskIsSecondary()) {
          currentChannelMask = emAfPluginNetworkCreatorSecondaryChannelMask;
          SETBITS(stateFlags, STATE_FLAGS_MASK_IS_SECONDARY);
          scheduleScans();
        } else {
          cleanupAndStop(results->status);
        }
      } else {
        // If we were successful, then all done!
        cleanupAndStop(results->status);
      }
    }
  }
}

HIDDEN void scanHandler(EmberAfPluginScanDispatchScanResults *results)
{
  EmberNetworkScanType scanType
    = emberAfPluginScanDispatchScanResultsGetScanType(results);

  if (emberAfPluginScanDispatchScanResultsAreFailure(results)) {
    // If we are here, that means the call to emberStartScan was a failure
    // in the scan-dispatch plugin (see scan-dispatch.h). So fail.
    cleanupAndStop(results->status);
  } else { // success
    if (emberAfPluginScanDispatchScanResultsAreComplete(results)) {
      debugPrintln("Scan complete. Channel: %d. Status: 0x%X",
                   results->channel,
                   results->status);
      handleScanComplete(results);
    } else { // results
      if (scanType == EMBER_ACTIVE_SCAN) {
        debugPrintln("Found network!");
        debugPrintln("  PanId: 0x%2X, Channel: %d, PJoin: %p",
                     results->network->panId,
                     results->network->channel,
                     (results->network->allowingJoin ? "YES" : "NO"));
        debugPrintln("  lqi:  %d", results->lqi);
        debugPrintln("  rssi: %d", results->rssi);

        updateChannelComposites(results->rssi,
                                EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_BEACONS,
                                results->network->channel);
      } else if (scanType == EMBER_ENERGY_SCAN) {
        debugPrintln("Energy scan results.");
        debugPrintln("%p: Channel: %d. Rssi: %d",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     results->channel,
                     results->rssi);

        updateChannelComposites(results->rssi,
                                EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_RSSI,
                                results->channel);
      }
    }
  }
}

static EmberStatus scheduleScans()
{
  EmberStatus status;
  EmberAfPluginScanDispatchScanData data = {
    .channelMask = currentChannelMask,
    .duration    = EMBER_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION,
    .handler     = scanHandler,
  };

  // Active first.
  data.scanType = EMBER_ACTIVE_SCAN;
  status = emberAfPluginScanDispatchScheduleScan(&data);

  // Energy second.
  data.scanType = EMBER_ENERGY_SCAN;
  if (status == EMBER_SUCCESS) {
    status = emberAfPluginScanDispatchScheduleScan(&data);
  }

  return status;
}

static void updateChannelComposites(int8_t rssi,
                                    EmAfPluginNetworkCreatorChannelCompositeMetric metric,
                                    uint8_t channel)
{
  uint8_t channelIndex = channel - EMBER_MIN_802_15_4_CHANNEL_NUMBER;

  // Even if the channel bit is off, we still update its channel composite. This
  // is so we can use the channel composites to find a "least worst channel" if
  // necessary.

  // Update the network composite value for this channel.
  switch (metric) {
    case EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_BEACONS:
      channelComposites[channelIndex].beaconsHeard++;
      break;

    case EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_RSSI:
      if (rssi > channelComposites[channelIndex].maxRssiHeard) {
        channelComposites[channelIndex].maxRssiHeard = rssi;
        maybeClearChannelBitOfMaxRssiReading(channel);
      }
      break;

    default:
      debugPrintln("Unknown metric value: %d", metric);
  }

  // If the channel is over the composite threshold, disregard the channel.
  if (emAfPluginNetworkCreatorChannelCompositeIsAboveThreshold(channelComposites[channelIndex])) {
    CLEARBIT(currentChannelMask, channel);
  }
}

static void cleanupAndStop(EmberStatus status)
{
  emberAfCorePrintln("%p: Stop. Status: 0x%X. State: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     status,
                     stateFlags);
}

static void cleanChannelComposites(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_NUM_802_15_4_CHANNELS; i++) {
    channelComposites[i].beaconsHeard
      = 0;
    channelComposites[i].maxRssiHeard
      = EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI;
  }
}

static void maybeClearChannelBitOfMaxRssiReading(uint8_t channel)
{
  uint8_t i, channelsConsideredSoFar, maxIndex;

  // Find max RSSI index and how many channels have been considered so far.
  for (i = 0, channelsConsideredSoFar = 0, maxIndex = 0;
       i < EMBER_NUM_802_15_4_CHANNELS;
       i++) {
    // If we have received an RSSI reading from this channel,
    // and we are still considering this channel...
    if ((channelComposites[i].maxRssiHeard
         != EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI)
        && (READBIT(currentChannelMask,
                    i + EMBER_MIN_802_15_4_CHANNEL_NUMBER))) {
      // ...increment the channelsConsideredSoFar by 1 and see if this
      // channel has the maximum RSSI.
      channelsConsideredSoFar++;
      if (channelComposites[i].maxRssiHeard
          > channelComposites[maxIndex].maxRssiHeard) {
        maxIndex = i;
      }
    }
  }

  // If the number of channels considered so far is more than the number of
  // channels that you want to consider, then remove the channel with the
  // maximum RSSI. Also reset the analogous channel composite so that we remember
  // that we are throwing away this channel.
  if ((channelsConsideredSoFar
       > EM_AF_PLUGIN_NETWORK_CREATOR_CHANNELS_TO_CONSIDER)) {
    uint8_t channelToThrowAway
      = ((channelComposites[channel - EMBER_MIN_802_15_4_CHANNEL_NUMBER].maxRssiHeard
          > channelComposites[maxIndex].maxRssiHeard)
         ? channel
         : maxIndex + EMBER_MIN_802_15_4_CHANNEL_NUMBER);
    CLEARBIT(currentChannelMask, channelToThrowAway);
    channelComposites[channelToThrowAway - EMBER_MIN_802_15_4_CHANNEL_NUMBER].maxRssiHeard
      = EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI;
  }
}

static uint32_t makeLeastWorstChannelMask(void)
{
  uint32_t mask = 0;;
  for (uint8_t i = 0; i < EMBER_NUM_802_15_4_CHANNELS; i++) {
    if (channelComposites[i].maxRssiHeard
        != EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI) {
      SETBIT(mask, i + EMBER_MIN_802_15_4_CHANNEL_NUMBER);
    }
  }
  return mask;
}
