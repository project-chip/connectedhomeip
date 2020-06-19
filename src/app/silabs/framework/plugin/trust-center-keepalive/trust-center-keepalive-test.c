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
 * @brief Test code for the Trust Center Keepalive plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "trust-center-keepalive.h"

#include "app/framework/test/test-framework.h"

//------------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginTrustCenterKeepaliveTickNetworkEventControls[EM_AF_MAX_SUPPORTED_NETWORKS_FOR_UNIT_TESTS];

EmberAfClusterCommand *emAfCurrentCommand = NULL;

typedef enum {
  SERVERLESS_KEEP_ALIVE_NOT_SUPPORTED,
  SERVERLESS_KEEP_ALIVE_DISABLED,
  SERVERLESS_KEEP_ALIVE_ENABLED
} ServerlessKeepAliveState;

typedef struct {
  ServerlessKeepAliveState serverlessKeepAliveEnabled;
  bool discoveryExpected;
  bool discoveryReceived;
  EmberNodeId target;
  EmberAfProfileId profileId;
  EmberAfClusterId clusterId;
  bool serverCluster;
  EmberAfServiceDiscoveryCallback* callback;
  bool rejoinInitiated;
} ServiceDiscoveryParameters;

static ServiceDiscoveryParameters serviceDiscoveryParameters = {
  false,
};

static uint8_t readAttributeRequestMessage[] = {
  0,  // frame control
  TEST_ZCL_SEQUENCE,
  0,  // command:  Read attributes
  LOW_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID)
};

//------------------------------------------------------------------------------
// Forward Declaration

void emberAfPluginTrustCenterKeepaliveStackStatusCallback(EmberStatus status);
void emberAfPluginTrustCenterKeepaliveInitCallback(void);

//------------------------------------------------------------------------------
// Stubs
EmberStatus emberSetBeaconClassificationParams(EmberBeaconClassificationParams* param)
{
  return EMBER_SUCCESS;
}
EmberStatus emberGetBeaconClassificationParams(EmberBeaconClassificationParams* param)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfFindDevicesByProfileAndCluster(EmberNodeId target,
                                                  EmberAfProfileId profileId,
                                                  EmberAfClusterId clusterId,
                                                  bool serverCluster,
                                                  EmberAfServiceDiscoveryCallback *callback)
{
  expect(serviceDiscoveryParameters.discoveryExpected);
  // Check the profile ID if not looking for wildcard profile ID
  if (profileId != 0xFFFF) {
    expectComparisonHex(serviceDiscoveryParameters.profileId,
                        profileId,
                        "Expected Profile ID",
                        "Actual Profile ID");
  }
  expectComparisonHex(serviceDiscoveryParameters.clusterId,
                      clusterId,
                      "Expected Cluster ID",
                      "Actual Cluster ID");
  expectComparisonDecimal(serviceDiscoveryParameters.serverCluster,
                          serverCluster,
                          "Expected Server Cluster",
                          "Actual Server Cluster");
  expectComparisonHex(serviceDiscoveryParameters.target,
                      target,
                      "Expected Node ID",
                      "Target Node ID");
  serviceDiscoveryParameters.discoveryReceived = true;
  serviceDiscoveryParameters.callback = callback;

  serviceDiscoveryParameters.discoveryExpected = false;
  return EMBER_SUCCESS;
}

void emberAfNetworkEventControlSetInactive(EmberEventControl *controls)
{
}

uint8_t emberAfPrimaryEndpointForNetworkIndex(uint8_t networkIndex)
{
  return 1;
}

uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void)
{
  return 1;
}

EmberStatus emberFindAndRejoinNetworkWithReason(bool haveCurrentNetworkKey,
                                                uint32_t channelMask,
                                                EmberRejoinReason reason)
{
  serviceDiscoveryParameters.rejoinInitiated = true;
  return EMBER_SUCCESS;
}

EmberStatus emberStartWritingStackTokens(void)
{
  return EMBER_SUCCESS;
}

EmberStatus emberStopWritingStackTokens(void)
{
  return EMBER_SUCCESS;
}

bool emberAfPluginTrustCenterKeepaliveTimeoutCallback(void)
{
  return false;
}

bool emberAfPluginTrustCenterKeepaliveServerlessIsSupportedCallback(void)
{
  return (serviceDiscoveryParameters.serverlessKeepAliveEnabled == SERVERLESS_KEEP_ALIVE_NOT_SUPPORTED) ? false : true;
}

bool emberAfTrustCenterKeepaliveServerlessIsEnabledCallback(void)
{
  return (serviceDiscoveryParameters.serverlessKeepAliveEnabled == SERVERLESS_KEEP_ALIVE_ENABLED) ? true : false;
}

bool emberAfTrustCenterKeepaliveOverwriteDefaultTimingCallback(uint16_t *baseTimeSeconds, uint16_t *jitterTimeSeconds)
{
  return false;
}

void emberAfPluginTrustCenterKeepaliveConnectivityEstablishedCallback(void)
{
  return;
}

void emberSetHubConnectivity(bool connectivity)
{
  return;
}

EmberStatus emberNetworkState(void)
{
  return EMBER_JOINED_NETWORK;
}

//------------------------------------------------------------------------------

static void findAndQueryTest(void)
{
  emberAfPluginTrustCenterKeepaliveStackStatusCallback(EMBER_NETWORK_UP);
  expectComparisonDecimal(emberAfPluginTrustCenterKeepaliveTickNetworkEventControls[EM_AF_DEFAULT_NETWORK_INDEX_FOR_UNIT_TESTS].status,
                          EMBER_EVENT_MS_TIME,
                          "Event Status",
                          "EMBER_EVENT_MS_TIME");
  serviceDiscoveryParameters.discoveryExpected = true;
  serviceDiscoveryParameters.serverlessKeepAliveEnabled = SERVERLESS_KEEP_ALIVE_NOT_SUPPORTED;
  serviceDiscoveryParameters.rejoinInitiated = false;
  serviceDiscoveryParameters.serverCluster = true;
  serviceDiscoveryParameters.profileId = SE_PROFILE_ID;
  serviceDiscoveryParameters.clusterId = ZCL_KEEPALIVE_CLUSTER_ID;
  serviceDiscoveryParameters.target = EMBER_TRUST_CENTER_NODE_ID;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  // Pretend the TC didn't answer...
  EmberAfServiceDiscoveryResult result = {
    EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT,
    serviceDiscoveryParameters.clusterId,
    serviceDiscoveryParameters.target,
    NULL,  // response data
  };
  serviceDiscoveryParameters.callback(&result);

  // ... device should retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and eventually rejoin.
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(!serviceDiscoveryParameters.discoveryReceived);
  expect(serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.rejoinInitiated = false;

  // Restart TC Keep Alive after rejoin
  emberAfPluginTrustCenterKeepaliveStackStatusCallback(EMBER_NETWORK_UP);

  // TC now responds with an empty list of endpoints (i.e. unsupported) ...
  uint8_t list[] = { 250, 37, 18, 102 };
  EmberAfEndpointList endpointListStruct = {
    0,
    NULL,
  };
  result.status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE;
  result.responseData = &endpointListStruct;
  serviceDiscoveryParameters.callback(&result);

  // ... device should retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and eventually rejoin.
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(!serviceDiscoveryParameters.discoveryReceived);
  expect(serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.rejoinInitiated = false;

  // Restart TC Keep Alive after rejoin
  emberAfPluginTrustCenterKeepaliveStackStatusCallback(EMBER_NETWORK_UP);

  // TC now responds with the same empty list of endpoints, however,
  // with serverless operation supported, but disabled ...
  serviceDiscoveryParameters.serverlessKeepAliveEnabled = SERVERLESS_KEEP_ALIVE_DISABLED;
  serviceDiscoveryParameters.callback(&result);

  // ... neither discovery nor rejoin is expected.
  serviceDiscoveryParameters.discoveryExpected = false;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(!serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);

  // TC now responds with the same empty list of endpoints, however,
  // with serverless operation supported, and enabled ...
  serviceDiscoveryParameters.serverlessKeepAliveEnabled = SERVERLESS_KEEP_ALIVE_ENABLED;
  serviceDiscoveryParameters.callback(&result);
  emberAfPluginTrustCenterKeepaliveEnable();

  // ... device should retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and rejoin is never expected ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  // ... unless no response is received ...
  result.status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT;
  serviceDiscoveryParameters.callback(&result);

  // ... in which case it starts to retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and retry ...
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(serviceDiscoveryParameters.discoveryReceived);
  expect(!serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.discoveryReceived = false;

  serviceDiscoveryParameters.callback(&result);

  // ... and eventually rejoin.
  serviceDiscoveryParameters.discoveryExpected = true;
  emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler();
  expect(!serviceDiscoveryParameters.discoveryReceived);
  expect(serviceDiscoveryParameters.rejoinInitiated);
  serviceDiscoveryParameters.rejoinInitiated = false;

  // Restart TC Keep Alive after rejoin
  emberAfPluginTrustCenterKeepaliveStackStatusCallback(EMBER_NETWORK_UP);

  result.status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE;
  endpointListStruct.count = 4;
  endpointListStruct.list = list;
  serviceDiscoveryParameters.callback(&result);
  verifyMessage("Read Attributes Request",
                sizeof(readAttributeRequestMessage),
                readAttributeRequestMessage);
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "find-and-query", findAndQueryTest },
    { NULL },
  };

  emEmberTestSetExternalBuffer();

  emberAfPluginTrustCenterKeepaliveInitCallback();

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-trust-center-keepalive-test",
                                        allTests);
}
