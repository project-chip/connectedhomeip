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
 * @brief Routines for finding and joining any viable network via scanning, rather
 * than joining a specific network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/common/form-and-join.h"
#include "network-find.h"

#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
#include "app/framework/util/util.h"    // emberAfGetPageFrom8bitEncodedChanPg() etc
#endif

//------------------------------------------------------------------------------
// Forward Declaration

#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
static EmberStatus startSearchForUnusedNetwork(bool allChannels);
#endif
static EmberStatus startSearchForJoinableNetwork(uint32_t channelMask,
                                                 bool allChannels);
EmberStatus emberAfStartSearchForJoinableNetworkAllChannels(void);

//------------------------------------------------------------------------------
// Globals

enum {
  NETWORK_FIND_NONE,
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  NETWORK_FIND_FORM,
  NETWORK_FIND_FORM_ALL_CHANNELS,
  #ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
  // Only with sub-GHz support do we have the secondary interface (even that
  // only on dual-PHY architectures but that if difficult to guard with a macro,
  // especially in a Host-NCP configuration).
  NETWORK_FIND_PHY2,
  NETWORK_FIND_PHY2_ALL_CHANNELS,
  #endif
#endif
  // NOTE: Important!
  // In order to resume the right kind of joinable network search, we have two
  // different waiting states. The code (see the two macros below) assumes that
  // they are in this specific order. Keep that in mind when editing states.
  NETWORK_FIND_JOIN,
  NETWORK_FIND_JOIN_ALL_CHANNELS,
  NETWORK_FIND_WAIT,
  NETWORK_FIND_WAIT_ALL_CHANNELS
};

// Macros to switch to the right flavor of join/wait state from the current state.
// See also the comment in the enum list above.
#define JOIN_STATE_TO_WAIT_STATE(state) ((state) - NETWORK_FIND_JOIN + NETWORK_FIND_WAIT)
#define WAIT_STATE_TO_JOIN_STATE(state) ((state) - NETWORK_FIND_WAIT + NETWORK_FIND_JOIN)

#if defined(EMBER_SCRIPTED_TEST)
  #define HIDDEN
#else
  #define HIDDEN static
#endif

HIDDEN uint8_t state = NETWORK_FIND_NONE;

#if defined(EMBER_SCRIPTED_TEST)
  #define EMBER_AF_PLUGIN_NETWORK_FIND_DURATION 5
extern uint32_t testFrameworkChannelMask;
  #define page0mask testFrameworkChannelMask

#elif defined(EMBER_AF_PLUGIN_TEST_HARNESS)
const uint32_t testHarnessOriginalChannelMask = EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
uint32_t testHarnessChannelMask = EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
  #define page0mask testHarnessChannelMask

#else
static uint32_t page0mask = EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK;
#endif

#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
static uint8_t currentPage = 0;
static uint32_t page28to31mask[EMBER_MAX_SUGBHZ_PAGE_NUMBER - EMBER_MIN_SUGBHZ_PAGE_NUMBER + 1] = EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_CHANNEL_MASK;
  #define CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, channelMaskArray) \
  ((((uint32_t)(page)) << EMBER_MAX_CHANNELS_PER_PAGE) | channelMaskArray[(page) - EMBER_MIN_SUGBHZ_PAGE_NUMBER])
  #define CHANNEL_28_TO_31_PAGE_MASK(page) \
  CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, page28to31mask)

static const uint32_t page28to31maskAllChannels[] = EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_ALL_CHANNELS_MASK;
  #define CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(page) \
  CHANNEL_28_TO_31_PAGE_MASK_WITH_PARAMETER(page, page28to31maskAllChannels)

// Search mode bitmask for "find unused" and "find joinable".
// Permitted values are set by the FIND_AND_JOIN_MODE_... macros.
// The default value is to search all supported channels of all supported pages.
static uint8_t searchMode =
  FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ | FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ;
#endif // EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ

#if defined(EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_CALLBACK)
  #define GET_RADIO_TX_POWER(pgChan)    emberAfPluginNetworkFindGetRadioPowerForChannelCallback(pgChan)
#elif defined(EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ)
  #define GET_RADIO_TX_POWER(pgChan)    (emberAfGetPageFrom8bitEncodedChanPg(pgChan) == 0 \
                                         ? EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER    \
                                         : EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ_RADIO_TX_POWER)
#else
  #define GET_RADIO_TX_POWER(pgChan)    EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER
#endif

static uint8_t extendedPanIds[EMBER_SUPPORTED_NETWORKS][EXTENDED_PAN_ID_SIZE];

EmberEventControl emberAfPluginNetworkFindTickEventControl;

//------------------------------------------------------------------------------

void emberAfPluginNetworkFindInitCallback(void)
{
  uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE] = EMBER_AF_PLUGIN_NETWORK_FIND_EXTENDED_PAN_ID;
  uint8_t i;
  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    MEMMOVE(extendedPanIds[i], extendedPanId, EXTENDED_PAN_ID_SIZE);
  }
}

void emberAfPluginFormAndJoinUnusedPanIdFoundCallback(EmberPanId panId, uint8_t channel)
{
  emberAfUnusedPanIdFoundCallback(panId, channel);
}

void emberAfUnusedPanIdFoundCallback(EmberPanId panId, uint8_t channel)
{
  EmberNetworkParameters networkParams = { 0 };
  EmberStatus status;

  emberAfGetFormAndJoinExtendedPanIdCallback(networkParams.extendedPanId);
  networkParams.panId = panId;
  networkParams.radioChannel = channel;
  networkParams.radioTxPower = GET_RADIO_TX_POWER(channel);

  status = emberAfFormNetwork(&networkParams);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p error 0x%x %p", "Form",
                      status,
                      "aborting");
    emberAfAppFlush();
    emberAfScanErrorCallback(status);
  }
}

void emberAfJoinableNetworkFoundCallback(EmberZigbeeNetwork *networkFound,
                                         uint8_t lqi,
                                         int8_t rssi)
{
  EmberStatus status = EMBER_ERR_FATAL;

  // NOTE: It's not necessary to check the networkFound->extendedPanId here
  // because the form-and-join utilities ensure this handler is only called
  // when the beacon in the found network has the same EPID as what we asked
  // for when we initiated the scan.  If the scan was requested for the EPID of
  // all zeroes, that's a special case that means any network is OK.  Either
  // way we can trust that it's OK to try joining the network params found in
  // the beacon.

  if (emberAfPluginNetworkFindJoinCallback(networkFound, lqi, rssi)) {
    // Now construct the network parameters to join
    EmberNetworkParameters networkParams;
    MEMSET(&networkParams, 0, sizeof(EmberNetworkParameters));
    MEMMOVE(networkParams.extendedPanId,
            networkFound->extendedPanId,
            EXTENDED_PAN_ID_SIZE);
    networkParams.panId = networkFound->panId;
    networkParams.radioChannel = networkFound->channel;
    networkParams.radioTxPower = GET_RADIO_TX_POWER(networkFound->channel);

    emberAfAppPrintln("Nwk found, ch %d, panId 0x%2X, joining",
                      networkFound->channel,
                      networkFound->panId);

    status = emberAfJoinNetwork(&networkParams);
  }

  // Note that if the application wants to skip this network or if the join
  // fails, we can't continue the scan from here (by calling
  // emberScanForNextJoinableNetwork) because that's the function that called
  // this handler in the first place, and we don't want to recurse.
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p error 0x%x, %p",
                      "Join",
                      status,
                      "aborting");
    emberAfAppFlush();
    emberEventControlSetActive(emberAfPluginNetworkFindTickEventControl);
  }
}

void emberAfPluginFormAndJoinNetworkFoundCallback(EmberZigbeeNetwork *networkFound,
                                                  uint8_t lqi,
                                                  int8_t rssi)
{
  emberAfJoinableNetworkFoundCallback(networkFound, lqi, rssi);
}

void emberAfPluginNetworkFindTickEventHandler(void)
{
  EmberStatus status = EMBER_SUCCESS;
  emberEventControlSetInactive(emberAfPluginNetworkFindTickEventControl);
  if (state == NETWORK_FIND_JOIN || state == NETWORK_FIND_JOIN_ALL_CHANNELS) {
    // If the tick fires while we're searching for a joinable network, it means
    // we need to keep searching.  This can happen if the join fails or if the
    // application decided to leave the network because it was the wrong
    // network.
    emberAfAppPrintln("Continue %p search", "joinable network");
    emberScanForNextJoinableNetwork();
#if defined(EMBER_AF_HAS_COORDINATOR_NETWORK) && defined(EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ)
  } else if (state == NETWORK_FIND_PHY2 || state == NETWORK_FIND_PHY2_ALL_CHANNELS) {
    state = NETWORK_FIND_NONE;
#endif
  } else {
    // In (almost) all other cases, we're done and can clean up.
    emberAfAppPrintln("Network find complete");
    emberFormAndJoinCleanup(EMBER_SUCCESS);
#if defined(EMBER_AF_HAS_COORDINATOR_NETWORK) && defined(EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ)
    // If the tick has fired while forming AND we are running on a dual-PHY
    // device, then after forming the network on 2.4GHz we need to kick off
    // a new energy scan on sub-GHz bands and start a second interface.
    if ((state == NETWORK_FIND_FORM || state == NETWORK_FIND_FORM_ALL_CHANNELS)
        && currentPage == 0
        && (searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) != 0) {
      const uint8_t phyInterfaceCount = emberGetPhyInterfaceCount();
      if (phyInterfaceCount > 1 && phyInterfaceCount != 0xFF) {
        currentPage = EMBER_MIN_SUGBHZ_PAGE_NUMBER;
        status = emberScanForUnusedPanId(CHANNEL_28_TO_31_PAGE_MASK(currentPage),
                                         EMBER_AF_PLUGIN_NETWORK_FIND_DURATION);
        if (status == EMBER_SUCCESS) {
          state = NETWORK_FIND_PHY2;
          // We are not quite finished yet, avoid calling the finished callback.
          return;
        }
      }
    }
#endif
    state = NETWORK_FIND_NONE;
    emberAfPluginNetworkFindFinishedCallback(status);
  }
}

static void printScanAllChannelsPrompt(void)
{
  emberAfAppPrintln("%p, no suitable primary channel found.",
                    "Network find complete");
  emberAfAppPrintln("Scanning again, using all channels.");
}

void emberAfScanErrorCallback(EmberStatus status)
{
  switch (status) {
    case EMBER_NO_BEACONS:
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
      // Finished scanning the current page, try the next page if available
      emberAfCorePrintln("%p %p on page %d", "Form", "and join scan done", currentPage);
      if ((searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ)
          && currentPage < EMBER_MAX_SUGBHZ_PAGE_NUMBER) {
        bool const allChannels = (state == NETWORK_FIND_JOIN_ALL_CHANNELS);
        uint32_t newPageMask;
        if (currentPage == 0) {
          currentPage = EMBER_MIN_SUGBHZ_PAGE_NUMBER;
        } else {
          currentPage++;
        }
        state = NETWORK_FIND_NONE;        // have to reset state before calling the fn
        newPageMask = allChannels
                      ? CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage)
                      : CHANNEL_28_TO_31_PAGE_MASK(currentPage);
        status = startSearchForJoinableNetwork(newPageMask, allChannels);
        if (status == EMBER_SUCCESS) {
          return;       // not 'break', we want to bail out immediately
        }
      }
#else
      emberAfCorePrintln("%p %p", "Form", "and join scan done");
#endif
      if (state == NETWORK_FIND_JOIN
          && emberAfPluginNetworkFindGetEnableScanningAllChannelsCallback()) {
        printScanAllChannelsPrompt();
        state = NETWORK_FIND_NONE;        // have to reset state before calling the fn
        status = emberAfStartSearchForJoinableNetworkAllChannels();
        if (status == EMBER_SUCCESS) {
          return;       // not 'break', we want to bail out immediately
        }
      }
      break;
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
    case EMBER_NOT_FOUND:
      if ((state == NETWORK_FIND_FORM
  #ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
           || state == NETWORK_FIND_PHY2
  #endif
           )
          && emberAfPluginNetworkFindGetEnableScanningAllChannelsCallback()) {
        printScanAllChannelsPrompt();
        if (state == NETWORK_FIND_FORM) {
          state = NETWORK_FIND_NONE;      // have to reset state before calling the fn
          status = startSearchForUnusedNetwork(true);
  #ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
        } else {
          state = NETWORK_FIND_PHY2_ALL_CHANNELS;
          currentPage = EMBER_MIN_SUGBHZ_PAGE_NUMBER;
          status = emberScanForUnusedPanId(CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage),
                                           EMBER_AF_PLUGIN_NETWORK_FIND_DURATION);
  #endif
        }
        if (status == EMBER_SUCCESS) {
          return;       // not 'break', we want to bail out immediately
        }
      } else {
        // Translate the error status for backwards compatibility...
        status = EMBER_ERR_FATAL;
      }
      // ...and deliberately fall through
#endif
    default:
      emberAfCorePrintln("%p error 0x%x", "Scan", status);
  }
  emberAfCoreFlush();
  state = NETWORK_FIND_NONE;
  emberAfAppPrintln("%p (scan error).",
                    "Network find complete");
  emberAfPluginNetworkFindFinishedCallback(status);
}

static bool findInitialChannelMask(bool allChannels,
                                   uint32_t *initialChannelMask)
{
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
  if (searchMode & FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ) {
    currentPage = 0;
    // Continue in the common part after the if statement
  } else if (searchMode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) {
    currentPage = EMBER_MIN_SUGBHZ_PAGE_NUMBER;
    *initialChannelMask = allChannels
                          ? CHANNEL_28_TO_31_PAGE_ALL_CHANNELS_MASK(currentPage)
                          : CHANNEL_28_TO_31_PAGE_MASK(currentPage);
    return true;
  } else {
    emberAfAppPrint("Invalid search mode");
    return false;
  }
#endif

  // A common part for a 2.4 GHz only mode or no sub-GHz support at all
  *initialChannelMask = allChannels
                        ? EMBER_ALL_802_15_4_CHANNELS_MASK
                        : page0mask;
  return true;
}

#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
static EmberStatus startSearchForUnusedNetwork(bool allChannels)
{
  EmberStatus status;
  uint32_t initialChannelMask;

  if (state != NETWORK_FIND_NONE) {
    emberAfAppPrintln("%pForm and join ongoing, state = %d", "Error: ", state);
    return EMBER_INVALID_CALL;
  }

  if (!findInitialChannelMask(allChannels, &initialChannelMask)) {
    return EMBER_INVALID_CALL;
  }

  emberAfAppPrintln("Search for %s network", "unused");

  status = emberScanForUnusedPanId(initialChannelMask,
                                   EMBER_AF_PLUGIN_NETWORK_FIND_DURATION);
  if (status == EMBER_SUCCESS) {
    state = allChannels
            ? NETWORK_FIND_FORM_ALL_CHANNELS
            : NETWORK_FIND_FORM;
  }
  return status;
}
#endif

EmberStatus emberAfFindUnusedPanIdAndFormCallback(void)
{
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  return startSearchForUnusedNetwork(false);
#else
  return EMBER_INVALID_CALL;
#endif
}

static EmberStatus startSearchForJoinableNetwork(uint32_t channelMask,
                                                 bool allChannels)
{
  EmberStatus status;
  uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];

  if (state != NETWORK_FIND_NONE) {
    emberAfAppPrintln("%pForm and join ongoing", "Error: ");
    return EMBER_INVALID_CALL;
  }

  emberAfGetFormAndJoinExtendedPanIdCallback(extendedPanId);
  status = emberScanForJoinableNetwork(channelMask, extendedPanId);
  if (status == EMBER_SUCCESS) {
    state = allChannels
            ? NETWORK_FIND_JOIN_ALL_CHANNELS
            : NETWORK_FIND_JOIN;
  }
  return status;
}

void emberAfPluginNetworkFindStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP) {
    // If we had been searching for an unused network so that we could form,
    // we're done.  If we were searching for a joinable network and have
    // successfully joined, we give the application some time to determine if
    // this is the correct network.  If so, we'll eventually time out and clean
    // up the state machine.  If not, the application will tell the stack to
    // leave, we'll get an EMBER_NETWORK_DOWN, and we'll continue searching.
    switch (state) {
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
      case NETWORK_FIND_FORM:
      case NETWORK_FIND_FORM_ALL_CHANNELS:
        emberAfPluginNetworkFindTickEventHandler();
        break;
#endif
      case NETWORK_FIND_JOIN:
      case NETWORK_FIND_JOIN_ALL_CHANNELS:
        state = JOIN_STATE_TO_WAIT_STATE(state);
        emberEventControlSetDelayMinutes(emberAfPluginNetworkFindTickEventControl,
                                         EMBER_AF_PLUGIN_NETWORK_FIND_JOINABLE_SCAN_TIMEOUT_MINUTES);
        break;
      default:
        // MISRA requires to have a default case
        break;
    }
  } else {
    // If we get something other than EMBER_NETWORK_UP while trying to join or
    // while waiting for the application to determine if this is the right
    // network, we need to continue searching for a joinable network.
    switch (state) {
      case NETWORK_FIND_WAIT:
      case NETWORK_FIND_WAIT_ALL_CHANNELS:
        state = WAIT_STATE_TO_JOIN_STATE(state);
      // Deliberate fall-through.
      case NETWORK_FIND_JOIN:
      case NETWORK_FIND_JOIN_ALL_CHANNELS:
        emberAfPluginNetworkFindTickEventHandler();
        break;
      default:
        // MISRA requires a default case
        break;
    }
  }
}

void emberAfGetFormAndJoinExtendedPanIdCallback(uint8_t *resultLocation)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  MEMMOVE(resultLocation, extendedPanIds[networkIndex], EXTENDED_PAN_ID_SIZE);
}

void emberAfSetFormAndJoinExtendedPanIdCallback(const uint8_t *extendedPanId)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  MEMMOVE(extendedPanIds[networkIndex], extendedPanId, EXTENDED_PAN_ID_SIZE);
}

// Code to compliantly search for all channels once we've searched on the
// preferred channels.
// It is assumed that appropriate channel masks per page are set prior to calling.
static EmberStatus startSearchForJoinableNetworkCallbackCommon(bool allChannels)
{
  uint32_t initialChannelMask;

  if (!findInitialChannelMask(allChannels,
                              &initialChannelMask)) {
    return EMBER_INVALID_CALL;
  }

  emberAfAppPrintln("Search for %s network", "joinable");

  return startSearchForJoinableNetwork(initialChannelMask, allChannels);
}

EmberStatus emberAfStartSearchForJoinableNetworkCallback(void)
{
  return startSearchForJoinableNetworkCallbackCommon(false);
}

EmberStatus emberAfStartSearchForJoinableNetworkAllChannels(void)
{
  return startSearchForJoinableNetworkCallbackCommon(true);
}

EmberStatus emberAfSetFormAndJoinChannelMask(uint8_t page, uint32_t mask)
{
  switch (page) {
    case 0:
      // Make sure the bitmask only contains the permitted bits
      if ((mask & ~EMBER_ALL_802_15_4_CHANNELS_MASK) != 0) {
        return EMBER_MAC_INVALID_CHANNEL_MASK;
      }
      page0mask = mask;
      break;
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
    case 28:
    case 30:
    case 31:
      if ((mask & ~EMBER_ALL_SUBGHZ_CHANNELS_MASK_FOR_PAGES_28_30_31) != 0) {
        return EMBER_MAC_INVALID_CHANNEL_MASK;
      }
      page28to31mask[page - EMBER_MIN_SUGBHZ_PAGE_NUMBER] = mask;
      break;
    case 29:
      if ((mask & ~EMBER_ALL_SUBGHZ_CHANNELS_MASK_FOR_PAGES_29) != 0) {
        return EMBER_MAC_INVALID_CHANNEL_MASK;
      }
      page28to31mask[page - EMBER_MIN_SUGBHZ_PAGE_NUMBER] = mask;
      break;
#endif
    default:
      // It would be nice to have an "invalid page" error, but this will have to do:
      return EMBER_PHY_INVALID_CHANNEL;
  }

  return EMBER_SUCCESS;
}

uint32_t emberAfGetFormAndJoinChannelMask(uint8_t page)
{
  switch (page) {
    case 0:
      return page0mask;
      break;
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
    case 28:
    case 29:
    case 30:
    case 31:
      return page28to31mask[page - EMBER_MIN_SUGBHZ_PAGE_NUMBER];
      break;
#endif
    default:
      return 0xFFFFFFFFU;
  }
}

EmberStatus emberAfSetFormAndJoinSearchMode(uint8_t mode)
{
  switch (mode) {
    case FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ:
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
    case FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ:
    case FIND_AND_JOIN_MODE_ALLOW_BOTH:
      searchMode = mode;
#endif
      break;
    default:
      return EMBER_MAC_INVALID_CHANNEL_MASK;
  }

  return EMBER_SUCCESS;
}

uint8_t emberAfGetFormAndJoinSearchMode(void)
{
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
  return searchMode;
#else
  return FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ;
#endif
}

/** @brief Is the search for an unused network currently in progress scanning
 *         all channels?
 * @return true if yes, false if the current scan is on preferred channels only
 */
bool emAfIsCurrentSearchForUnusedNetworkScanningAllChannels(void)
{
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  #ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
  return state == NETWORK_FIND_FORM_ALL_CHANNELS
         || state == NETWORK_FIND_PHY2_ALL_CHANNELS;
  #else
  return state == NETWORK_FIND_FORM_ALL_CHANNELS;
  #endif
#else
  return false;
#endif
}

/** @brief Returns the channel mask for the current scan.
 * Similar to emberAfGetFormAndJoinChannelMask(), but may return the configured
 * channel mask or all channels mask, depending on the current scan state.
 */
uint32_t emAfGetSearchForUnusedNetworkChannelMask(uint8_t page)
{
  if (emAfIsCurrentSearchForUnusedNetworkScanningAllChannels()) {
    switch (page) {
      case 0:
        return EMBER_ALL_802_15_4_CHANNELS_MASK;
        break;
#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
      case 28:
      case 29:
      case 30:
      case 31:
        return page28to31maskAllChannels[page - EMBER_MIN_SUGBHZ_PAGE_NUMBER];
        break;
#endif
      default:
        return 0xFFFFFFFFU;
    }
  }

  return emberAfGetFormAndJoinChannelMask(page);
}

#ifdef EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
/** @brief Secondary interface formed callback.
 * Called after forming the network to notify the plugin that the secondary
 * interface (in case of a dual-PHY implementation) has been formed. It is used
 * by the plugin to reset its internal state. Strictly for internal use only.
 */
void emAfSecondaryInterfaceFormedCallback(EmberStatus status)
{
  (void) status;        // unreferenced parameter
  #ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  if (state == NETWORK_FIND_PHY2 || state == NETWORK_FIND_PHY2_ALL_CHANNELS) {
    // It should be enough to reset the find status, but we may still have
    // the timer running so let's delegate the job to the tick handler which
    // will also take care of killing the timer.
    emberAfPluginNetworkFindTickEventHandler();
  }
  #endif
}
#endif // EMBER_AF_PLUGIN_NETWORK_FIND_SUB_GHZ
