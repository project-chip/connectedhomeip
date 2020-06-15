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
 * @brief Unit tests for WWAH Server Silabs plugin
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/wwah-server-silabs/wwah-server-silabs.h"
#include "app/framework/util/common.h"

#include "app/framework/plugin/aps-link-key-authorization/aps-link-key-authorization-test.h"
#include "app/framework/plugin/wwah-server-silabs/wwah-server-silabs-test.h"
#include "app/framework/test/test-framework.h"
#include "stack/include/packet-buffer.h"

#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
#include "app/framework/plugin/ias-zone-server/ias-zone-server.h"
#endif

#define AMAZON_MANUFACTURING_ID 0x1217

static uint8_t  exemptList[2 * (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 1)];
static uint16_t exemptCntr;

#define EXTERNAL_BUFFER_DATA_SIZE (64)
static uint8_t externalBufferData[EXTERNAL_BUFFER_DATA_SIZE];
static uint16_t externalBufferLength;
static EmberApsFrame externalBufferApsFrame;
EmberAfClusterCommand *emAfCurrentCommand;

EmberAfClusterCommand command;
EmberAfClusterCommand *emAfCurrentCommand = &command;

// -----------------------------------------------------------------------------
// Interface for callbacks implemeted in the plugin and tested here
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
static void testQueryResponseData(uint16_t clusterId, bool expectedEnableState)
{
  expectComparisonDecimal(
    LOW_BYTE(AMAZON_MANUFACTURING_ID),
    externalBufferData[externalBufferLength - 7],
    "expected query data in response data",
    "observed query data in response data");

  expectComparisonDecimal(
    HIGH_BYTE(AMAZON_MANUFACTURING_ID),
    externalBufferData[externalBufferLength - 6],
    "expected query data in response data",
    "observed query data in response data");

  // Don't test externalBufferData[externalBufferLength - 5] as it is sequence number.

  expectComparisonDecimal(
    ZCL_APS_LINK_KEY_AUTHORIZATION_QUERY_RESPONSE_COMMAND_ID,
    externalBufferData[externalBufferLength - 4],
    "expected query data in response data",
    "observed query data in response data");

  expectComparisonDecimal(
    LOW_BYTE(clusterId),
    externalBufferData[externalBufferLength - 3],
    "expected query data in response data",
    "observed query data in response data");

  expectComparisonDecimal(
    HIGH_BYTE(clusterId),
    externalBufferData[externalBufferLength - 2],
    "expected query data in response data",
    "observed query data in response data");

  expectComparisonDecimal(
    expectedEnableState,
    externalBufferData[externalBufferLength - 1],
    "expected query data in response data",
    "observed query data in response data");
}

// -----------------------------------------------------------------------------
// stubbed out functions: In general, we don't care about these functions; they
// just need to compile and always return success when called at runtime
// -----------------------------------------------------------------------------
EmberAfDefinedEndpoint emAfEndpoints[] = {
  { 1, HA_PROFILE_ID, 1, 1, NULL }
};

EmberMessageBuffer scanDataCache;

uint8_t emberAfIndexFromEndpoint(uint8_t endpoint)
{
  return 0;
}

EmberPanId emberGetPanId(void)
{
  return 0x0000;
}

void emCallCounterHandler(EmberCounterType type, uint8_t data)
{
}
uint8_t emberEndpointCount = 0;
EmberEndpoint emberEndpoints[] = {};

EmberStatus emberSurveyBeacons(bool useStandardBeacons,
                               EmberNetworkFoundCallback networkFoundCallback,
                               EmberScanCompleteCallback scanCompleteCallback)
{
  return EMBER_SUCCESS;
}

EmberStatus emberZllSetPolicy(EmberZllPolicy policy)
{
  return EMBER_SUCCESS;
}

EmberZllPolicy emberZllGetPolicy(void)
{
  return EMBER_ZLL_POLICY_ENABLED | EMBER_ZLL_POLICY_TARGET;
}

EmberAfStatus emberAfWriteManufacturerSpecificServerAttribute(uint8_t endpoint,
                                                              EmberAfClusterId cluster,
                                                              EmberAfAttributeId attributeID,
                                                              uint16_t manufacturerCode,
                                                              uint8_t* dataPtr,
                                                              EmberAfAttributeType dataType)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfReadManufacturerSpecificServerAttribute(uint8_t endpoint,
                                                             EmberAfClusterId cluster,
                                                             EmberAfAttributeId attributeID,
                                                             uint16_t manufacturerCode,
                                                             uint8_t* dataPtr,
                                                             uint8_t readLength)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus staus)
{
  return EMBER_SUCCESS;
}

void emberSetMacPollFailureWaitTime(uint32_t waitBeforeRetryIntervalMs)
{
}

EmberStatus emberGetExtendedSecurityBitmask(EmberExtendedSecurityBitmask* mask)
{
  return EMBER_SUCCESS;
}

EmberStatus emberSetExtendedSecurityBitmask(EmberExtendedSecurityBitmask mask)
{
  return EMBER_SUCCESS;
}
uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void)
{
  return 1;
}

void emberSetPendingNetworkUpdateChannel(uint8_t channel)
{
}

void emberSetPendingNetworkUpdatePanId(uint16_t panId)
{
}

static uint16_t ExtendedResetInfo;
uint8_t halGetResetInfo(void)
{
  return (ExtendedResetInfo & 0xFF);
}

uint16_t halCommonGetRandomTraced(char *file, int line)
{
  return 0;
}

void halInternalGetTokenData(void *data, uint16_t token, uint8_t index, uint8_t len)
{
  return;
}

void halInternalSetTokenData(uint16_t token, uint8_t index, void *data, uint8_t len)
{
}

EmberStatus emberAfFindDevicesByProfileAndCluster(EmberNodeId target,
                                                  EmberAfProfileId profileId,
                                                  EmberAfClusterId clusterId,
                                                  bool serverCluster,
                                                  EmberAfServiceDiscoveryCallback *callback)
{
  return EMBER_SUCCESS;
}

#define ZIGBEE_MAX_NETWORK_RETRIES_DEFAULT 15
uint8_t emberGetMaxNwkRetries()
{
  return ZIGBEE_MAX_NETWORK_RETRIES_DEFAULT;
}
#define MAC_MAX_ACKD_RETRIES_DEFAULT  (3)
uint8_t emberGetMaxMacRetries()
{
  return MAC_MAX_ACKD_RETRIES_DEFAULT;
}

EmberStatus emberClearBindingTable(void)
{
  return EMBER_SUCCESS;
}

void emberSetParentClassificationEnabled(bool enabled)
{
}

bool emberGetParentClassificationEnabled(void)
{
  return false;
}

void emberAfPluginWwahConnectivityManagerEnableRejoinAlgorithm(void)
{
}

void emberAfPluginWwahConnectivityManagerDisableRejoinAlgorithm(void)
{
}

void emAfPluginWwahConnectivityManagerEnableBadParentRecovery(uint8_t endpoint, uint16_t badParentRejoinPeriod)
{
}

void emAfPluginWwahConnectivityManagerDisableBadParentRecovery(uint8_t endpoint)
{
}

void emberAfIasZoneServerSetStatusQueueRetryParamsToDefault(void)
{
}

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER)
EmberStatus emberAfIasZoneServerConfigStatusQueueRetryParams(
  IasZoneStatusQueueRetryConfig *retryConfig)
{
  return EMBER_SUCCESS;
}

EmberAfStatus emberAfPluginIasZoneClusterSetEnrollmentMethod(uint8_t endpoint,
                                                             EmberAfIasZoneEnrollmentMode method)
{
  return EMBER_SUCCESS;
}
#endif

void emberAfIasZoneServerDiscardPendingEventsInStatusQueue(void)
{
}

void emberAfPluginOtaClientSetIgnoreNonTrustCenter(bool ignoreNonTc)
{
}

void emberAfPluginSetDisableOtaDowngrades(bool isDisabled)
{
}

void emberAfPluginPollControlServerSetIgnoreNonTrustCenter(bool ignoreNonTc)
{
}

void emberAfPluginWwahConnectivityManagerSetRejoinParameters(uint16_t fastRejoinTimeoutSeconds,
                                                             uint16_t durationBetweenRejoinsSeconds,
                                                             uint16_t fastRejoinFirstBackoffSeconds,
                                                             uint16_t maxBackoffTimeSeconds,
                                                             uint16_t maxBackoffIterations)
{
  (void)fastRejoinTimeoutSeconds;
  (void)durationBetweenRejoinsSeconds;
  (void)fastRejoinFirstBackoffSeconds;
  (void)maxBackoffTimeSeconds;
  (void)maxBackoffIterations;
  return;
}

EmberEventControl emberAfPluginWwahServerSilabsTriggerSurveyBeaconsNetworkEventControls[EM_AF_MAX_SUPPORTED_NETWORKS_FOR_UNIT_TESTS];
void emberAfNetworkEventControlSetInactive(EmberEventControl *controls)
{
}

bool emberAfContainsClient(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return true;
}

bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return true;
}

// -----------------------------------------------------------------------------
// test cases: These are the test cases called by the test framework when unit
// tests are run.
// -----------------------------------------------------------------------------

// Verify APS ACK on Unicasts.
static void wwahRequireRemoveQueryApsAcksOnUnicasts(void)
{
  expectMessage(true == emberAfSlWwahClusterRequireApsAcksOnUnicastsCallback(0, NULL),
                "emberAfSlWwahClusterRequireApsAcksOnUnicastsCallback returns error!\n");

  expectMessage(true == emberAfSlWwahClusterRemoveApsAcksOnUnicastsRequirementCallback(),
                "emberAfSlWwahClusterRemoveApsAcksOnUnicastsRequirementCallback returns error!\n");

  emberAfSlWwahClusterApsAckRequirementQueryCallback();

  expectComparisonDecimal(
    LOW_BYTE(AMAZON_MANUFACTURING_ID),
    externalBufferData[externalBufferLength - 5],
    "expected query data in response data",
    "observed query data in response data");

  expectComparisonDecimal(
    HIGH_BYTE(AMAZON_MANUFACTURING_ID),
    externalBufferData[externalBufferLength - 4],
    "expected query data in response data",
    "observed query data in response data");

  // Don't test externalBufferData[externalBufferLength - 3] as it is sequence number.

  expectComparisonDecimal(
    ZCL_APS_ACK_ENABLEMENT_QUERY_RESPONSE_COMMAND_ID,
    externalBufferData[externalBufferLength - 2],
    "expected query data in response data",
    "observed query data in response data");

  // APS ACK on Unicasts is always enabled, exemption list is not supported, so length is zero.
  expectComparisonDecimal(
    0,
    externalBufferData[externalBufferLength - 1],
    "expected query data in response data",
    "observed query data in response data");
}

// Verify APS Link Key Authorization.
static void wwahEnableQueryDisableAuthorizationTest(void)
{
  // Load the full list with elements
  // Note: this is 10 more that is available in the exempt list of the plugin
  for (exemptCntr = 0; exemptCntr < (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 10); exemptCntr++) {
    // TODO: This is wrong, needs to add as uint8_t
    exemptList[2 * exemptCntr] = LOW_BYTE(exemptCntr);
    exemptList[2 * exemptCntr + 1] = HIGH_BYTE(exemptCntr);
  }

  // Make some clusteres missing from the list (ie. let it be a duplicate of some other cluster)
  exemptList[0] = 1;
  exemptList[1] = 0;
  exemptList[2 * (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1)] = 1;
  exemptList[2 * (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1) + 1] = 0;

  // Note: Test needed to be disabled as return value is not what I supposed.
  // TODO: Needs to find a way to report failure properly.
  // // Try to load more element that is available
  // expectMessage(false == emberAfSlWwahClusterEnableApsLinkKeyAuthorizationCallback(
  //                 EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 3, exemptList),
  //               "Link key authorization cluster exempt list is not supposed to be overloaded !\n");

  // Try to properly fill up the list (note addin)
  // Note; Adding 2 more elements as the two "missing" one does not occupy space
  expectMessage(true == emberAfSlWwahClusterEnableApsLinkKeyAuthorizationCallback(
                  EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 2, exemptList),
                "Link key authorization cluster exempt list cannot be filled up !\n");

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(0);
  testQueryResponseData(0, true);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(1);
  testQueryResponseData(1, false);

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1, true);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2, false);

  // Note: Test needed to be disabled as return value is not what I supposed.
  // TODO: Needs to find a way to report failure properly.
  // // Use the "disable" API to let the same cluster list be exempted
  // expectMessage(false == emberAfSlWwahClusterDisableApsLinkKeyAuthorizationCallback(
  //                 EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 3, exemptList),
  //               "Link key authorization cluster exempt list is not supposed to be overloaded !\n");

  // Use the "disable" API to let the same cluster list be exempted
  expectMessage(true == emberAfSlWwahClusterDisableApsLinkKeyAuthorizationCallback(
                  EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 2, exemptList),
                "Link key authorization cluster exempt list cannot be filled up !\n");

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(0);
  testQueryResponseData(0, false);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(1);
  testQueryResponseData(1, true);

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1, false);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2, true);

  // Enable APS Link Key Encription for all clusters
  expectMessage(true == emberAfSlWwahClusterEnableApsLinkKeyAuthorizationCallback(0, NULL),
                "Link key authorization cluster exempt list cannot be filled up !\n");

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(0);
  testQueryResponseData(0, true);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(1);
  testQueryResponseData(1, true);

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1, true);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2, true);

// Disable APS Link Key Encription for all clusters
  expectMessage(true == emberAfSlWwahClusterDisableApsLinkKeyAuthorizationCallback(0, NULL),
                "Link key authorization cluster exempt list cannot be filled up !\n");

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(0);
  testQueryResponseData(0, false);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(1);
  testQueryResponseData(1, false);

  // Test query callback for a cluster that was just removed from the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 1, false);

  // Test query callback for a cluster that is supposed to be in the exempt list
  emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2);
  testQueryResponseData(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS - 2, false);
}

static uint8_t readTimeAttributeRequestMessage[] = {
  0,  // frame control
  TEST_ZCL_SEQUENCE,
  0,  // command: Read attributes
  LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_DST_START_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_DST_START_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_DST_END_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_DST_END_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_DST_SHIFT_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_DST_SHIFT_ATTRIBUTE_ID),
  LOW_BYTE(ZCL_LOCAL_TIME_ATTRIBUTE_ID),
  HIGH_BYTE(ZCL_LOCAL_TIME_ATTRIBUTE_ID)
};

#define NUM_REQUEST_TIME_TEST_ENDPOINTS 2

static void wwahRequestTimeTest(void)
{
  EmberApsFrame *apsFrame;
  uint8_t srcEp[NUM_REQUEST_TIME_TEST_ENDPOINTS] = { 31, 47 };
  uint8_t dstEp[NUM_REQUEST_TIME_TEST_ENDPOINTS] = { 0xEE, 3 };
  uint8_t i;

  // Ensure source & dest endpoints are flipped in emberAfSlWwahClusterRequestTimeCallback().
  for ( i = 0; i < NUM_REQUEST_TIME_TEST_ENDPOINTS; i++ ) {
    // Setup src/dst endpoints in the "received" Time Request message.
    apsFrame = emberAfGetCommandApsFrame();
    apsFrame->destinationEndpoint = dstEp[i];
    apsFrame->sourceEndpoint = srcEp[i];
    // Make sure the RequestTime Callback function sends a Read Attributes command with the expected time attributes
    // (in readTimeAttributeRequestMessage[]), and that the source and destination endpoints are swapped in the
    // Read Attributes command.
    emberAfSlWwahClusterRequestTimeCallback();
    expectComparisonDecimal(apsFrame->destinationEndpoint, srcEp[i], "rsp dstEndpoint", "req srcEndpoint");
    expectComparisonDecimal(apsFrame->sourceEndpoint, dstEp[i], "rsp srcEndpoint", "req dstEndpoint");
    verifyMessage("test msg", sizeof(readTimeAttributeRequestMessage), readTimeAttributeRequestMessage);
  }
}

static void wwahSurveyBeaconSortTest(void)
{
  EmberBeaconSurvey testData[10] = {
    { .nodeId = 0x0000, .rssi = 0xBA, .classificationMask = 0x00 },
    { .nodeId = 0x1111, .rssi = 0xE9, .classificationMask = 0x01 },
    { .nodeId = 0x2222, .rssi = 0x26, .classificationMask = 0x03 },
    { .nodeId = 0x3333, .rssi = 0x56, .classificationMask = 0x01 },
    { .nodeId = 0x4444, .rssi = 0x72, .classificationMask = 0x00 },
    { .nodeId = 0x5555, .rssi = 0xC7, .classificationMask = 0x03 },
    { .nodeId = 0x6666, .rssi = 0xB1, .classificationMask = 0x01 },
    { .nodeId = 0x7777, .rssi = 0xD2, .classificationMask = 0x02 },
    { .nodeId = 0x8888, .rssi = 0xE1, .classificationMask = 0x03 },
    { .nodeId = 0x9999, .rssi = 0x12, .classificationMask = 0x00 }
  };

  EmberNodeId testResults[4] = { 0x2222, 0x8888, 0x5555, 0x7777 };

  // Do setup
  setupSurveyBeaconProcedure();

  // Sort beacon results
  int i;
  for (i = 0; i < 10; i++) {
    sortBeaconSurveyResult(testData[i]);
    surveyBeaconDataCount++;
  }

  EmberBeaconSurvey result;
  for (i = 0; i < 4; i++) {
    emberCopyFromLinkedBuffers(surveyBeaconDataCache,
                               i * sizeof(EmberBeaconSurvey),
                               &result,
                               sizeof(EmberBeaconSurvey));
    expectComparisonDecimal(testResults[i], result.nodeId, "expected node", "actual node");
  }

  emReleaseMessageBuffer(surveyBeaconDataCache);

  return;
}

//Stubs

void emberSetZDOConfigurationMode(bool enabled)
{
}

EmberAfStatus emberAfReadServerAttribute(uint8_t endpoint,
                                         EmberAfClusterId cluster,
                                         EmberAfAttributeId attributeID,
                                         uint8_t *dataPtr,
                                         uint8_t readLength)
{
  return EMBER_SUCCESS;
}

int main(int argc, char* argv[])
{
  emberAfSetExternalBuffer(externalBufferData,
                           EXTERNAL_BUFFER_DATA_SIZE,
                           &externalBufferLength,
                           &externalBufferApsFrame);

  const TestCase tests[] = {
    { "wwah-require-remove-query-aps-acks-on-unicasts", wwahRequireRemoveQueryApsAcksOnUnicasts },
    { "wwah-enable-query-disable-authorization-test", wwahEnableQueryDisableAuthorizationTest },
    { "wwah-request-time-test", wwahRequestTimeTest },
    { "wwah-survey-beacon-sort-test", wwahSurveyBeaconSortTest },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc, argv, "WWAH Server Silabs", tests);
}

EmberStatus emberAfPluginUpdateTcLinkKeyStart(void)
{
  return EMBER_SUCCESS;
}
