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
 * @brief This file provides unit tests for functions defined in permit-join-manager.c
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/test/script/afv2-scripted.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
//============================================================================
// Globals
#define EMBER_AF_PLUGIN_PERMIT_JOIN_MANAGER_JOINING_DEVICES_QUEUE_LENGTH 4
#define EMBER_AF_PLUGIN_PERMIT_JOIN_MANAGER_PERMIT_JOINS_DURATION 40
#define EMBER_AF_PLUGIN_PERMIT_JOIN_MANAGER_DEVICE_ANNOUNCE_TIMEOUT 1000
//============================================================================
// Forward Declarations
static void deviceAnnounceTest(bool deviceAnnounceFound);
void emberAfPluginPermitJoinManagerTimeoutNetworkEventHandler(void);
void emberAfPluginPermitJoinManagerInitCallback(void);
EmberEventControl emberAfPluginPermitJoinManagerTimeoutNetworkEventControls[1];
bool emberAfPluginPermitJoinManagerZdoMessageReceivedCallback(EmberNodeId emberNodeId,
                                                              EmberApsFrame* apsFrame,
                                                              uint8_t* message,
                                                              uint16_t length);
void emberAfPluginPermitJoinManagerTrustCenterJoinCallback(EmberNodeId newNodeId,
                                                           EmberEUI64 newNodeEui64,
                                                           EmberNodeId parentOfNewNode,
                                                           EmberDeviceUpdate status,
                                                           EmberJoinDecision decision);
//------------------------------------------------------------------------------
// Event System stubs.
EmberEventData afEvents[] = {
  { &emberAfPluginPermitJoinManagerTimeoutNetworkEventControls[0], emberAfPluginPermitJoinManagerTimeoutNetworkEventHandler },
  { NULL, NULL }
};

//------------------------------------------------------------------------------
//Other stubs

void scriptTickCallback(void)
{
  emberRunEvents(afEvents);
}

//------------------------------------------------------------------------------

EmberStatus emberPermitJoiningRequest(EmberNodeId target,
                                      uint8_t duration,
                                      uint8_t authentication,
                                      EmberApsOption options)
{
  simpleScriptCheck("emberPermitJoiningRequest",
                    "emberPermitJoiningRequest",
                    "iiii",
                    43981,
                    40,
                    1, // TC significance is always 1
                    0);
  return EMBER_SUCCESS;
}

static void deviceAnnounceFound(void)
{
  deviceAnnounceTest(true);
}

static void noDeviceAnnounceTest(void)
{
  deviceAnnounceTest(false);
}

static void deviceAnnounceWithoutJoiningListEntry(void)
{
  EmberApsFrame apsFrame;
  MEMSET(&apsFrame, 0, sizeof(EmberApsFrame));
  apsFrame.clusterId = END_DEVICE_ANNOUNCE;
  uint8_t message[12] = { 0x81, 0xB7, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x8E };
  addSimpleAction("emberAfPluginPermitJoinManagerZdoMessageReceivedCallback()",
                  emberAfPluginPermitJoinManagerZdoMessageReceivedCallback,
                  4,
                  0xABCD,
                  &apsFrame,
                  message,
                  12);
  runScript();
}

static void endDeviceJoin(void)
{
  EmberApsFrame apsFrame;
  MEMSET(&apsFrame, 0, sizeof(EmberApsFrame));
  apsFrame.clusterId = END_DEVICE_ANNOUNCE;
  uint8_t message[12] = { 0x81, 0xB7, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xCC };
  addSimpleAction("emberAfPluginPermitJoinManagerZdoMessageReceivedCallback()",
                  emberAfPluginPermitJoinManagerZdoMessageReceivedCallback,
                  4,
                  0xABCD,
                  &apsFrame,
                  message,
                  12);
  runScript();
}
static void deviceAnnounceTest(bool deviceAnnounceFound)
{
  emberAfPluginPermitJoinManagerInitCallback();
  EmberEUI64 newNodeEui64 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A };
  EmberNodeId parentNodeId = 0x0005;

  emberAfPluginPermitJoinManagerTrustCenterJoinCallback(0xABCD,
                                                        newNodeEui64,
                                                        parentNodeId,
                                                        EMBER_STANDARD_SECURITY_UNSECURED_JOIN,
                                                        EMBER_USE_PRECONFIGURED_KEY);

  if (deviceAnnounceFound) {
    //Fake the device announce message. Look immediately for the permit Join Unicast.
    EmberApsFrame apsFrame;
    MEMSET(&apsFrame, 0, sizeof(EmberApsFrame));
    apsFrame.clusterId = END_DEVICE_ANNOUNCE;
    //ZDO transaction Sequence Number: 0x81
    //Network Address:0xC6B7
    //IEEE Address: 0x0000000000000003
    //Capability:0x8E(not an end device)
    uint8_t message[12] = { 0x81, 0xB7, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x8E };
    addSimpleAction("emberAfPluginPermitJoinManagerZdoMessageReceivedCallback()",
                    emberAfPluginPermitJoinManagerZdoMessageReceivedCallback,
                    4,
                    0xABCD,
                    &apsFrame,
                    message,
                    12);
  } else {
    //No device announce. Wait for Timer to fire.
    addWaitAction(1000);
  }

  addSimpleCheck("emberPermitJoiningRequest",
                 "iiii",
                 0xABCD,
                 EMBER_AF_PLUGIN_PERMIT_JOIN_MANAGER_PERMIT_JOINS_DURATION,
                 1, // TC significance is always 1
                 0);
  runScript();
}

int main(int argc, char* argv[])
{
  Test tests[] = {
    { "no-device-announce", noDeviceAnnounceTest },
    { "device-announce-found", deviceAnnounceFound },
    { "device-announce-no-entry", deviceAnnounceWithoutJoiningListEntry },
    { "sleepy-end-device", endDeviceJoin },
    { NULL, NULL },
  };

  // This test only runs correctly with network 0.
  // Ideally we should look into this.
  emberSetCurrentNetwork(0);

  Thunk test = parseTestArgument(argc, argv, tests);

  test();
  fprintf(stderr, " ]\n");

  return 0;
}
