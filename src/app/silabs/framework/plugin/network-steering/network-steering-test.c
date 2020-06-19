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
 * @brief Unit test for network steering procedure.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/test/script/afv2-scripted.h"

#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"
#include "app/framework/plugin/scan-dispatch/scan-dispatch.h"

// Called by the update-tc-link-key plugin.
void emberAfPluginUpdateTcLinkKeyStatusCallback(EmberKeyStatus keyStatus);

// -----------------------------------------------------------------------------
// STUBS.

void emAfClearLinkKeyTable(void)
{
}

static EmAfZigbeeProNetwork zigbeeNetwork
  = { EMBER_ROUTER, EMBER_AF_SECURITY_PROFILE_Z3 };
const EmAfZigbeeProNetwork *emAfCurrentZigbeeProNetwork = &zigbeeNetwork;

int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel)
{
  return 0;
}

EmberStatus emberGetExtendedSecurityBitmask(EmberExtendedSecurityBitmask* mask)
{
  return EMBER_SUCCESS;
}

EmberStatus emberSetExtendedSecurityBitmask(EmberExtendedSecurityBitmask mask)
{
  return EMBER_SUCCESS;
}

bool emberAfPluginNetworkSteeringGetDistributedKeyCallback(EmberKeyData * key)
{
  return false;
}

EmberStatus emberGetKey(EmberKeyType type,
                        EmberKeyStruct* keyStruct)
{
  return EMBER_SUCCESS;
}

// -----------------------------------------------------------------------------
// SCRIPTED STUBS.

EmberNodeType emberAfPluginNetworkSteeringGetNodeTypeCallback(EmberAfPluginNetworkSteeringJoiningState state)
{
  long *contents = functionCallCheck("getNodeTypeCallback",
                                     "i!",
                                     state);
  return contents[1];
}

bool emberAfPluginUpdateTcLinkKeyStop(void)
{
  functionCallCheck("updateTcLinkKeyStop", "");
  return true;
}

EmberStatus emberAfPermitJoin(uint8_t duration, bool broadcast)
{
  functionCallCheck("permitJoin",
                    "ii",
                    duration,
                    broadcast);
  return EMBER_SUCCESS;
}

void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  functionCallCheck("networkSteeringCompleteCallback",
                    "iiii",
                    status,
                    totalBeacons,
                    joinAttempts,
                    finalState);
}

EmberStatus emberAfPluginScanDispatchScheduleScan(EmberAfPluginScanDispatchScanData *data)
{
  functionCallCheck("scheduleScan", "ii", data->scanType, data->channelMask);

  if (scriptDebug) {
    simPrint("scheduleScan with scanType 0x%02X and channelMask 0x%08X",
             data->scanType, data->channelMask);
  }

  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginUpdateTcLinkKeyStart(void)
{
  functionCallCheck("updateTcLinkKeyStart", "");

  return EMBER_SUCCESS;
}

EmberStatus emberJoinNetwork(EmberNodeType nodeType,
                             EmberNetworkParameters *parameters)
{
  functionCallCheck("joinNetwork",
                    "iii", // nodeType, panId, channel
                    nodeType,
                    parameters->panId,
                    parameters->radioChannel);

  return EMBER_SUCCESS;
}

EmberStatus emberLeaveNetwork(void)
{
  functionCallCheck("leaveNetwork", "");

  return EMBER_SUCCESS;
}

EmberStatus emberSetInitialSecurityState(EmberInitialSecurityState *state)
{
  long *contents = functionCallCheck("setInitialSecurityState",
                                     "ip", // bitmask, preconfiguredKey
                                     state->bitmask,
                                     makeMessage("s",
                                                 &state->preconfiguredKey,
                                                 EMBER_ENCRYPTION_KEY_SIZE));
  EmberStatus returnStatus = (EmberStatus)contents[2];

  return returnStatus;
}

void emberAfPluginUpdateTcLinkKeySetDelay(uint32_t delayMs)
{
  functionCallCheck("updateTcLinkKeyStart", "");
}

void emberAfPluginUpdateTcLinkKeySetInactive(void)
{
  functionCallCheck("updateTcLinkKeyStop", "");
}

#define addSetInitialSecurityStateCheck(mask, preconfiguredKey, returnStatus) \
  addSimpleCheck("setInitialSecurityState",                                   \
                 "ip!i",                                                      \
                 (mask),                                                      \
                 (preconfiguredKey),                                          \
                 (returnStatus))

// -----------------------------------------------------------------------------
// ACTIONS.

PRINTER(start)
{
  fprintf(stderr, " starting network steering");
}

PERFORMER(start)
{
  EmberStatus expectedReturnStatus = (EmberStatus)action->contents[0];
  EmberStatus actualReturnStatus = emberAfPluginNetworkSteeringStart();

  scriptAssert(action, actualReturnStatus == expectedReturnStatus);
}

ACTION(start, i); // expectedReturnStatus

#define addStartAction(expectedReturnStatus) \
  addAction(&startActionType, (expectedReturnStatus))

PRINTER(scanResults)
{
  uint8_t statusOrRssi = (uint8_t)action->contents[0];
  uint8_t channelOrLqi = (uint8_t)action->contents[1];
  EmberPanId panId     = (EmberPanId)action->contents[2];
  bool complete        = (bool)action->contents[3];

  fprintf(stderr, " statusOrRssi: 0x%02X, channelOrLqi: %d, panId: 0x%04X",
          statusOrRssi, channelOrLqi, panId);
  fprintf(stderr, " complete: %s", (complete ? "YES" : "NO"));
}

PERFORMER(scanResults)
{
  uint8_t statusOrRssi = (uint8_t)action->contents[0];
  uint8_t channelOrLqi = (uint8_t)action->contents[1];
  EmberPanId panId     = (EmberPanId)action->contents[2];
  bool complete        = (bool)action->contents[3];
  EmberZigbeeNetwork network;
  EmberAfPluginScanDispatchScanResults results = {
    .status = statusOrRssi,
    .rssi = statusOrRssi,
    .channel = channelOrLqi,
    .lqi = channelOrLqi,
    .network = &network,
    .mask = (EMBER_ACTIVE_SCAN
             | (complete
                ? EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE
                : 0)),
  };

  network.panId        = panId;
  network.allowingJoin = true;
  network.stackProfile = 2;

  // Defined in network-steering.c.
  extern void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results);
  scanResultsHandler(&results);
}

ACTION(scanResults, iiii); // statusOrRssi, channelOrLqi, panId, complete

#define addScanResultsAction(statusOrRssi, channelOrLqi, panId, complete) \
  addAction(&scanResultsActionType,                                       \
            (statusOrRssi),                                               \
            (channelOrLqi),                                               \
            (panId),                                                      \
            (complete))

PRINTER(stackStatus)
{
  EmberStatus status = (EmberStatus)action->contents[0];
  fprintf(stderr, " status: 0x%02X", status);
}

PERFORMER(stackStatus)
{
  EmberStatus status = (EmberStatus)action->contents[0];

  // Normally called by the framework-generated code.
  extern void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status);
  emberAfPluginNetworkSteeringStackStatusCallback(status);
}

ACTION(stackStatus, i); // status

#define addStackStatusAction(status)          \
  addAction(&stackStatusActionType, (status)) \

// -----------------------------------------------------------------------------
// UTIL.

#define ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY         \
  {                                                    \
    { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,  \
      0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 } \
  }

#define ZIGBEE_3_DISTRIBUTED_SECURITY_LINK_KEY         \
  {                                                    \
    { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,  \
      0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF } \
  }

#define NETWORK_STEERING_TEST_CONFIGURED_KEY           \
  {                                                    \
    { 0x63, 0x6F, 0x6E, 0x67, 0x72, 0x65, 0x73, 0x73,  \
      0x20, 0x73, 0x74, 0x72, 0x65, 0x65, 0x74, 0x2E } \
  }

static Parcel *centralizedKey = NULL;
static Parcel *distributedKey = NULL;
static Parcel *configuredKey  = NULL;

static void init(void)
{
  EmberKeyData zigbeeAlliance09 = ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY;
  EmberKeyData d0D1D2Etc = ZIGBEE_3_DISTRIBUTED_SECURITY_LINK_KEY;
  EmberKeyData congressStreet = NETWORK_STEERING_TEST_CONFIGURED_KEY;

  emAfPluginNetworkSteeringPrimaryChannelMask = BIT32(11) | BIT32(12);
  emAfPluginNetworkSteeringSecondaryChannelMask = BIT32(13) | BIT32(14);

  testFrameworkNetworkState = EMBER_NO_NETWORK;

  centralizedKey = makeMessage("s",
                               emberKeyContents(&zigbeeAlliance09),
                               EMBER_ENCRYPTION_KEY_SIZE);
  distributedKey = makeMessage("s",
                               emberKeyContents(&d0D1D2Etc),
                               EMBER_ENCRYPTION_KEY_SIZE);
  configuredKey  = makeMessage("s",
                               emberKeyContents(&congressStreet),
                               EMBER_ENCRYPTION_KEY_SIZE);
}

#define SECURITY_BITMASK              \
  (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
   | EMBER_HAVE_PRECONFIGURED_KEY     \
   | EMBER_REQUIRE_ENCRYPTED_KEY      \
   | EMBER_NO_FRAME_COUNTER_RESET)

/*static long getNetworkSteeringState(void)
   {
   return emAfPluginNetworkSteeringState;
   }
 */

// -----------------------------------------------------------------------------
// TESTS.

static void alreadyOnNetwork(void)
{
  init();
  testFrameworkNetworkState = EMBER_JOINED_NETWORK;

  // If we are already on a network, we should open the network with pjoin.
  // BDB wants the network to be open for 180 seconds by default.
  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("permitJoin", "ii", 180, true);

  addRunAction(100);
  runScript();
}

static void noNetworksTest(void)
{
  uint8_t i;

  init();

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  for (i = 0; i < 3; i++) {
    Parcel *key = (i == 2 ? distributedKey : centralizedKey);
    uint32_t securityBitmask = SECURITY_BITMASK;
    if (i == 0) {
      securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;
    } else if (i == 2) {
      securityBitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
    }

    // The procedure randomly selects a channel out of the current mask, so
    // hopefully we get lucky here.

    // First we scan the primary channels.
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

    // We don't find any networks...
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

    // ...so we go to the next channel on the primary mask.
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(12));

    // We don't find any networks...
    addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0, true);

    // ...so we go to the secondary mask.
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(13));

    // We don't find any networks...
    addScanResultsAction(EMBER_SUCCESS, BIT32(13), 0, true);

    // ...so we go to the next channel on the secondary mask.
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(14));

    // We don't find any networks...
    addScanResultsAction(EMBER_SUCCESS, BIT32(14), 0, true);

    // ...so we go to the next method.
  }

  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_NO_BEACONS,
                 0,  // beacons
                 0,  // join attempts
                 EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED);

  addRunAction(100);
  runScript();
}

static void primaryChannelsInstallCodeTest(void)
{
  uint8_t i;
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  for (i = 0; i < 3; i++) {
    Parcel *key = (i == 2 ? distributedKey : centralizedKey);
    uint32_t securityBitmask = SECURITY_BITMASK;
    if (i == 0) {
      securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;
    } else if (i == 2) {
      securityBitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
    }

    // The procedure randomly selects a channel out of the current mask, so
    // hopefully we get lucky here.

    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);

    // First we scan the the primary channels.
    state++;
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

    // We find two networks!
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0x1234, false);
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0xABCD, false);
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

    // We try to join the first network. We fail.
    addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
    addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0x1234, 11);
    addStackStatusAction(EMBER_NETWORK_DOWN);

    // We try to join the second network. We succeed!
    addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
    addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0xABCD, 11);
    addStackStatusAction(EMBER_NETWORK_UP);

    break;
  }

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // The TCLK is successfully received.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED);

  // The complete callback gets called a second time because of the
  // verify key success status.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND);
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_VERIFY_LINK_KEY_SUCCESS);

  // We should open the network for the BDB's 180 second window.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 2,  // beacons
                 2,  // join attempts
                 EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE);

  addRunAction(100);
  runScript();
}

static void secondaryChannelsCentralizedTest(void)
{
  uint8_t i;
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  for (i = 0; i < 3; i++) {
    Parcel *key = (i == 2 ? distributedKey : centralizedKey);
    uint32_t securityBitmask = SECURITY_BITMASK;
    if (i == 0) {
      securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;
    } else if (i == 2) {
      securityBitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
    }

    // The procedure randomly selects a channel out of the current mask, so
    // hopefully we get lucky here.

    // First we scan the the primary channels.
    state++;
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

    // We find a network!
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0x1234, false);
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

    // We try to join the network. We fail.
    addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_END_DEVICE);
    addSimpleCheck("joinNetwork", "iii", EMBER_END_DEVICE, 0x1234, 11);
    addStackStatusAction(EMBER_NETWORK_DOWN);

    // We continue scanning the primary channels.
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(12));

    // We do not find any networks.
    addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0, true);

    // We go to the secondary channels. We find one network.
    state++;
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(13));
    addScanResultsAction(EMBER_SUCCESS, BIT32(13), 0xABCD, false);
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

    // We fail in joining with an install code. We succeed when joining
    // a centralized network.
    addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_END_DEVICE);
    addSimpleCheck("joinNetwork", "iii", EMBER_END_DEVICE, 0xABCD, 13);

    if (i == 1) {
      addStackStatusAction(EMBER_NETWORK_UP);
      break;
    } else {
      addStackStatusAction(EMBER_NETWORK_DOWN);
    }

    // We continue scanning the secondary channels.
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(14));
    addScanResultsAction(EMBER_SUCCESS, BIT32(14), 0, true);

    // We should switch to looking on the primary channels for
    // centralized networks.
  }

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // The TCLK is successfully received.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED);

  // The complete callback gets called a second time because of the
  // verify key success status.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND);
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_VERIFY_LINK_KEY_SUCCESS);

  // We should open the network for the BDB's 180 second window.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 4,  // beacons
                 4,  // join attempts
                 EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED);

  addRunAction(100);
  runScript();
}

static void primaryChannelsDistributedTest(void)
{
  uint8_t i;
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  for (i = 0; i < 3; i++) {
    Parcel *key = (i == 2 ? distributedKey : centralizedKey);
    uint32_t securityBitmask = SECURITY_BITMASK;
    if (i == 0) {
      securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;
    } else if (i == 2) {
      securityBitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
    }

    // The procedure randomly selects a channel out of the current mask, so
    // hopefully we get lucky here.

    // First we scan the the primary channels.
    state++;
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

    // We do not find any networks.
    addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

    // We continue scanning the primary channels.
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(12));

    // We find a network!
    addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0x1234, false);
    addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0, true);

    // We try to join the network. We fail for install code and centralized
    // joining, but pass for distributed joining.
    addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
    addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0x1234, 12);
    if (i == 2) {
      addStackStatusAction(EMBER_NETWORK_UP);
      break;
    } else {
      addStackStatusAction(EMBER_NETWORK_DOWN);
    }

    // We go to the secondary channels. We do not find any networks.
    state++;
    addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(13));
    addScanResultsAction(EMBER_SUCCESS, BIT32(13), 0, true);
    addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(14));
    addScanResultsAction(EMBER_SUCCESS, BIT32(14), 0, true);

    // We should switch to looking on the primary channels for
    // centralized networks, and then destributed networks.
  }

  // The TCLK should not be updated since this is a distributed network
  // that we joined.

  // We should open the network for the BDB's 180 second window.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 3,  // beacons
                 3,  // join attempts
                 EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED);

  addRunAction(100);
  runScript();
}

static void failedTclkExchangeTest(void)
{
  uint32_t securityBitmask = SECURITY_BITMASK;
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;
  securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  // The procedure randomly selects a channel out of the current mask, so
  // hopefully we get lucky here.

  // We scan the the primary channels and successfully find and join
  // a network.
  state++;
  addSetInitialSecurityStateCheck(securityBitmask, centralizedKey, EMBER_SUCCESS);
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0x1234, false);
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);
  addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_END_DEVICE);
  addSimpleCheck("joinNetwork", "iii", EMBER_END_DEVICE, 0x1234, 11);
  addStackStatusAction(EMBER_NETWORK_UP);

  // We try to update our TCLK but we fail. According to the BDB, we should
  // leave the network.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TC_FAILED_TO_SEND_TC_KEY);
  addSimpleCheck("leaveNetwork", "");

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_NO_LINK_KEY_RECEIVED,
                 1,  // beacons
                 1,  // join attempts
                 (EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE
                  | EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK));

  addRunAction(100);
  runScript();
}

static void failedSecuritySetupTest(void)
{
  uint8_t i;
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  for (i = 0; i < 3; i++) {
    Parcel *key = (i == 2 ? distributedKey : centralizedKey);
    uint32_t securityBitmask = SECURITY_BITMASK;
    if (i == 0) {
      securityBitmask |= EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE;
    } else if (i == 2) {
      securityBitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
    }

    // The procedure randomly selects a channel out of the current mask, so
    // hopefully we get lucky here.

    // We try to join the network but our security setup fails because
    // we do not have an install code. We should go to the next method.
    state++;
    if (i == 0) {
      addSetInitialSecurityStateCheck(securityBitmask,
                                      key,
                                      EMBER_SECURITY_CONFIGURATION_INVALID);
    } else {
      addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
      // First we scan the the primary channels and find networks.
      addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));
      addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0x1234, false);
      addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0xABCD, false);
      addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);
      addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
      addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0x1234, 11);
      addStackStatusAction(EMBER_NETWORK_UP);
      break;
    }

    // We go to the secondary channels. We do not find any networks.
    state++;
    if (i == 0) {
      addSetInitialSecurityStateCheck(securityBitmask,
                                      key,
                                      EMBER_SECURITY_CONFIGURATION_INVALID);
    } else {
      addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(13));
      addScanResultsAction(EMBER_SUCCESS, BIT32(13), 0, true);
      addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(14));
      addScanResultsAction(EMBER_SUCCESS, BIT32(14), 0, true);
    }

    // We should switch to looking on the primary channels for
    // centralized networks.
  }

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // The TCLK is successfully received.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED);

  // The complete callback gets called a second time because of the
  // verify key success status.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND);
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_VERIFY_LINK_KEY_SUCCESS);

  // We should open the network for the BDB's 180 second window.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 2,  // beacons
                 1,  // join attempts
                 EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED);

  addRunAction(100);
  runScript();
}

static void failedVerifyKey(void)
{
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  Parcel *key = centralizedKey;
  uint32_t securityBitmask = (SECURITY_BITMASK
                              | EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE);

  // The procedure randomly selects a channel out of the current mask, so
  // hopefully we get lucky here.

  // We set up security.
  addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);

  // We schedule a scan on the first primary channel.
  state++;
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

  // We find a network.
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0xABCD, false);
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

  // We join the network.
  addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_END_DEVICE);
  addSimpleCheck("joinNetwork", "iii", EMBER_END_DEVICE, 0xABCD, 11);
  addStackStatusAction(EMBER_NETWORK_UP);

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // The TCLK is successfully received.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED);

  // The complete callback does not get called a second time since the
  // verify key was a failure.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);

  // We stop the update tclk process and leave the network.
  addSimpleCheck("updateTcLinkKeyStop", "");
  addSimpleCheck("leaveNetwork", "");

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_ERR_FATAL,
                 1,  // beacons
                 1,  // join attempts
                 (EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE
                  | EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK
                  | EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK));

  addRunAction(100);
  runScript();
}

static void failedScan(void)
{
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  Parcel *key = centralizedKey;
  uint32_t securityBitmask = (SECURITY_BITMASK
                              | EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE);

  // The procedure randomly selects a channel out of the current mask, so
  // hopefully we get lucky here.

  // We set up security.
  addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);

  // We schedule a scan on the first primary channel.
  state++;
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

  // The scan fails. We should resent our state.
  addScanResultsAction(EMBER_MAC_COMMAND_TRANSMIT_FAILURE, 11, 0, true);

  // We report failure and we used to reset our state. But now instaed we continue scanning on the next channel

  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(12));
  // We find a network.
  addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0xABCD, false);
  addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0, true);

  // We join the network.
  addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
  addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0xABCD, 12);
  addStackStatusAction(EMBER_NETWORK_UP);

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // We learn that the trust center is pre-21. Therefore, we don't update
  // the trust center link key.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_IS_PRE_R21);

  // We go ahead and open the network.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 1,    // beacons
                 1,    // join attempts
                 (EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE));

  addRunAction(100);
  runScript();
}

static void preR21TrustCenter(void)
{
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE - 1;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  Parcel *key = centralizedKey;
  uint32_t securityBitmask = (SECURITY_BITMASK
                              | EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE);

  // The procedure randomly selects a channel out of the current mask, so
  // hopefully we get lucky here.

  // We set up security.
  addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);

  // We schedule a scan on the first primary channel.
  state++;
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

  // We find a network.
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0xABCD, false);
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

  // We join the network.
  addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
  addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0xABCD, 11);
  addStackStatusAction(EMBER_NETWORK_UP);

  // The TCLK should be updated since this was not a distributed network that
  // we joined. We set a jitter timeout of up to 5 seconds.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND * 5);
  addSimpleCheck("updateTcLinkKeyStart", "");

  // We learn that the trust center is pre-21. Therefore, we don't update
  // the trust center link key.
  addSimpleAction("updateTcLinkKeyCompleteCallback(0x%02X)",
                  emberAfPluginUpdateTcLinkKeyStatusCallback,
                  1,
                  EMBER_TRUST_CENTER_IS_PRE_R21);

  // We go ahead and open the network.
  addWaitAction(MILLISECOND_TICKS_PER_SECOND << 2);
  addSimpleCheck("permitJoin", "ii", 180, true);

  // Done!
  addSimpleCheck("networkSteeringCompleteCallback",
                 "iiii",
                 EMBER_SUCCESS,
                 1,  // beacons
                 1,  // join attempts
                 (EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE));

  addRunAction(100);
  runScript();
}

static void configuredKeyTest(void)
{
  EmberAfPluginNetworkSteeringJoiningState state;

  init();
  state = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;

  addStartAction(EMBER_SUCCESS);
  addSimpleCheck("updateTcLinkKeyStop", "");

  Parcel *key = configuredKey;
  uint32_t securityBitmask = SECURITY_BITMASK;

  // Set the configured key to be used
  EmberKeyData configuredKeyData = NETWORK_STEERING_TEST_CONFIGURED_KEY;
  emAfPluginNetworkSteeringSetConfiguredKey(configuredKeyData.contents, true);

  // First we scan the the primary channels.
  state++;
  addSetInitialSecurityStateCheck(securityBitmask, key, EMBER_SUCCESS);
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(11));

  // We do not find any networks.
  addScanResultsAction(EMBER_SUCCESS, BIT32(11), 0, true);

  // We continue scanning the primary channels.
  addSimpleCheck("scheduleScan", "ii", EMBER_ACTIVE_SCAN, BIT32(12));

  // We find a network!
  addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0x1234, false);
  addScanResultsAction(EMBER_SUCCESS, BIT32(12), 0, true);

  // An attempt to join passes
  addSimpleCheck("getNodeTypeCallback", "i!", state, EMBER_ROUTER);
  addSimpleCheck("joinNetwork", "iii", EMBER_ROUTER, 0x1234, 12);
  addStackStatusAction(EMBER_NETWORK_UP);

  addRunAction(100);
  runScript();
}

// -----------------------------------------------------------------------------
// MAIN.

// These are declared in network-steering.c.
extern EmberEventControl emberAfPluginNetworkSteeringFinishSteeringEventControl;
extern void emberAfPluginNetworkSteeringFinishSteeringEventHandler(void);
static EmberEventData networkSteeringEventData[] = {
  {
    &emberAfPluginNetworkSteeringFinishSteeringEventControl,
    emberAfPluginNetworkSteeringFinishSteeringEventHandler,
  },
  {
    NULL,
    NULL,
  },
};
static EmberTaskId networkSteeringTaskId;

int main(int argc, char *argv[])
{
  Test tests[] = {
    { "already-on-network",
      alreadyOnNetwork },

    { "no-networks",
      noNetworksTest },

    { "primary-install-code",
      primaryChannelsInstallCodeTest },

    { "secondary-centralized",
      secondaryChannelsCentralizedTest },

    { "primary-distributed",
      primaryChannelsDistributedTest },

    { "failed-tclk-update",
      failedTclkExchangeTest },

    { "failed-security-setup",
      failedSecuritySetupTest },

    { "failed-verify-key",
      failedVerifyKey },

    { "failed-scan",
      failedScan },

    { "pre-r21-trust-center",
      preR21TrustCenter },

    { "configured-key",
      configuredKeyTest },

    { NULL, NULL, },
  };
  Thunk test = parseTestArgument(argc, argv, tests);

  // Fake emberAfInit.
  {
    networkSteeringTaskId = emberTaskInit(networkSteeringEventData);
  }

  test();

  fprintf(stderr, " done ]\n");
  return 0;
}
// -----------------------------------------------------------------------------
// CALLBACKS.

void scriptTickCallback(void)
{
  emberRunTask(networkSteeringTaskId);
}
