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
 * @brief Test code for the Find and Bind Initiator plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/test/script/afv2-scripted.h"

#ifdef EMBER_STACK_PROFILE
#undef EMBER_STACK_PROFILE
#endif

#include "app/framework/include/af.h"

#include "find-and-bind-initiator.h"
#include "../find-and-bind-target/find-and-bind-target.h"
#include "find-and-bind-test-configuration.h"

// -----------------------------------------------------------------------------
// CONSTANTS/GLOBALS.

#define EXTERNAL_BUFFER_DATA_SIZE (64)
static uint8_t externalBufferData[EXTERNAL_BUFFER_DATA_SIZE];
static uint16_t externalBufferLength;
static EmberApsFrame externalBufferApsFrame;

EmberAfClusterCommand *emAfCurrentCommand;

static bool containsIsReferringToTarget = false;

// TARGET1.
#define TARGET1_ENDPOINT       (1)
#define TARGET1_PROFILE_ID     (0x1234)
#define TARGET1_DEVICE_ID      (0xABCD)
#define TARGET1_DEVICE_VERSION (0xAC)
#define TARGET1_NETWORK_INDEX  (0)

#define TARGET1_SHORT (EMBER_ZIGBEE_COORDINATOR_ADDRESS)
static const EmberEUI64 target1Long = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, };

// Input clusters are the server clusters.
// Output clusters are the client clusters.
#define TARGET1_IN_CLUSTER_COUNT (3)
static const EmberAfClusterId target1InClusterList[TARGET1_IN_CLUSTER_COUNT] = {
  ZCL_IDENTIFY_CLUSTER_ID,
  ZCL_ON_OFF_CLUSTER_ID,
  ZCL_TIME_CLUSTER_ID,
};
#define TARGET1_OUT_CLUSTER_COUNT (2)
static const EmberAfClusterId target1OutClusterList[TARGET1_OUT_CLUSTER_COUNT] = {
  ZCL_IDENTIFY_CLUSTER_ID,
  ZCL_PARTITION_CLUSTER_ID,
};

// TARGET2.
#define TARGET2_ENDPOINT       (2)
#define TARGET2_PROFILE_ID     (0xF00D)
#define TARGET2_DEVICE_ID      (0xBEEF)
#define TARGET2_DEVICE_VERSION (0xDC)
#define TARGET2_NETWORK_INDEX  (0)

#define TARGET2_SHORT (0x8888)
static const EmberEUI64 target2Long = { 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, };

// Input clusters are the server clusters.
// Output clusters are the client clusters.
#define TARGET2_IN_CLUSTER_COUNT (2)
static const EmberAfClusterId target2InClusterList[TARGET2_IN_CLUSTER_COUNT] = {
  ZCL_IDENTIFY_CLUSTER_ID,
  ZCL_LEVEL_CONTROL_CLUSTER_ID,
};
#define TARGET2_OUT_CLUSTER_COUNT (2)
static const EmberAfClusterId target2OutClusterList[TARGET2_OUT_CLUSTER_COUNT] = {
  ZCL_IDENTIFY_CLUSTER_ID,
  ZCL_POLL_CONTROL_CLUSTER_ID,
};

// INITATOR.
#define INITIATOR_ENDPOINT       (1)
#define INITIATOR_PROFILE_ID     (0x1234)
#define INITIATOR_DEVICE_ID      (0xABCD)
#define INITIATOR_DEVICE_VERSION (0xAC)
#define INITIATOR_NETWORK_INDEX  (0)

#define INITIATOR_ENDPOINT_COUNT (1)
#define INITIATOR_CLUSTER_COUNT (5)
static EmberAfCluster cluster[INITIATOR_CLUSTER_COUNT] = {
  {
    .clusterId  = ZCL_IDENTIFY_CLUSTER_ID,
    .attributes = NULL,
    .mask       = CLUSTER_MASK_CLIENT,
  },
  {
    .clusterId  = ZCL_IDENTIFY_CLUSTER_ID,
    .attributes = NULL,
    .mask       = CLUSTER_MASK_SERVER,
  },
  {
    .clusterId  = ZCL_GROUPS_CLUSTER_ID,
    .attributes = NULL,
    .mask       = CLUSTER_MASK_CLIENT,
  },
  {
    .clusterId  = ZCL_ON_OFF_CLUSTER_ID,
    .attributes = NULL,
    .mask       = CLUSTER_MASK_CLIENT,
  },
  {
    .clusterId  = ZCL_LEVEL_CONTROL_CLUSTER_ID,
    .attributes = NULL,
    .mask       = CLUSTER_MASK_CLIENT,
  },
};
static EmberAfEndpointType endpointType = {
  .cluster      = cluster,
  .clusterCount = INITIATOR_CLUSTER_COUNT,
};
EmberAfDefinedEndpoint emAfEndpoints[INITIATOR_ENDPOINT_COUNT] = {
  {
    .endpoint      = INITIATOR_ENDPOINT,
    .profileId     = INITIATOR_PROFILE_ID,
    .deviceId      = INITIATOR_DEVICE_ID,
    .deviceVersion = INITIATOR_DEVICE_VERSION,
    .endpointType  = &endpointType,
    .networkIndex  = INITIATOR_NETWORK_INDEX,
  },
};
#define INVALID_GROUP_ID            0xFFFF
#define INITIATOR_GROUP_ID          0xACAC
#define INITIATOR_GROUP_NAME        "tuna"
#define INITIATOR_GROUP_NAME_LENGTH 4

// -----------------------------------------------------------------------------
// UTIL.

Parcel *makeBindingEntryParcel(EmberBindingType type,
                               uint8_t local,
                               uint16_t clusterId,
                               uint8_t remote,
                               const EmberEUI64 eui64,
                               uint8_t networkIndex)
{
  EmberBindingTableEntry entry = {
    .type = type,
    .local = local,
    .clusterId = clusterId,
    .remote = remote,
    .networkIndex = networkIndex,
  };
  MEMMOVE(entry.identifier, eui64, EUI64_SIZE);

  return bindingEntryToParcel(&entry);
}

// -----------------------------------------------------------------------------
// SEND UNICAST.

EmberStatus emberAfSendUnicastWithCallback(EmberOutgoingMessageType type,
                                           uint16_t indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t *message,
                                           EmberAfMessageSentFunction callback)
{
  functionCallCheck("emberAfSendUnicastWithCallback",
                    "iip",
                    type,
                    indexOrDestination,
                    makeMessage("s", message, messageLength));
  return EMBER_SUCCESS;
}

#define addSendUnicastCheck(type, indexOrDestination, message) \
  addSimpleCheck("emberAfSendUnicastWithCallback",             \
                 "iip",                                        \
                 (type),                                       \
                 (indexOrDestination),                         \
                 (message))

// -----------------------------------------------------------------------------
// BINDING.

#ifdef EMBER_BINDING_TABLE_SIZE
#undef EMBER_BINDING_TABLE_SIZE
#endif
#define EMBER_BINDING_TABLE_SIZE (10)

static EmberBindingTableEntry bindingTable[EMBER_BINDING_TABLE_SIZE];

EmberStatus emberGetBinding(uint8_t index, EmberBindingTableEntry *result)
{
  *result = bindingTable[index];
  return EMBER_SUCCESS;
}

EmberStatus emberSetBinding(uint8_t index, EmberBindingTableEntry *result)
{
  long *contents = functionCallCheck("setBinding",
                                     "iiii",
                                     result->type,
                                     result->local,
                                     result->clusterId,
                                     result->remote,
                                     (result->type == EMBER_UNICAST_BINDING
                                      ? makeMessage("s",
                                                    result->identifier,
                                                    EUI64_SIZE)
                                      : makeMessage("11",
                                                    result->identifier[1],
                                                    result->identifier[0])));

  EmberStatus status = (EmberStatus)contents[5];

  if (status == EMBER_SUCCESS) {
    bindingTable[index] = *result;
  }

  return status;
}

void emberSetBindingRemoteNodeId(uint8_t index, EmberNodeId nodeId)
{
}

#define addSetBindingCheck(type, local, clusterId, remote, identifier, status) \
  addSimpleCheck("setBinding",                                                 \
                 "iiiip!",                                                     \
                 (type),                                                       \
                 (local),                                                      \
                 (clusterId),                                                  \
                 (remote),                                                     \
                 (identifier),                                                 \
                 (status))

// -----------------------------------------------------------------------------
// COMPLETE CALLBACK.

void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  functionCallCheck("completeCallback", "i", status);
}

#define addCompleteCallbackCheck(status) \
  addSimpleCheck("completeCallback", "i", (status))

// -----------------------------------------------------------------------------
// FOUND TARGET CALLBACK.

bool emberAfPluginFindAndBindInitiatorBindTargetCallback(EmberNodeId nodeId,
                                                         EmberBindingTableEntry *entry,
                                                         uint8_t *groupName)
{
  long *contents = functionCallCheck("foundTargetCallback",
                                     "ip!!",
                                     nodeId,
                                     bindingEntryToParcel(entry));
  uint16_t groupId = (uint16_t)contents[2];
  bool doBind = (bool)contents[3];

  if (groupId != INVALID_GROUP_ID) {
    entry->type = EMBER_MULTICAST_BINDING;
    entry->identifier[0] = LOW_BYTE(groupId);
    entry->identifier[1] = HIGH_BYTE(groupId);
  }

  MEMMOVE(groupName + 1, INITIATOR_GROUP_NAME, INITIATOR_GROUP_NAME_LENGTH);
  groupName[0] = INITIATOR_GROUP_NAME_LENGTH;

  if (scriptDebug) {
    simPrint("found target 0x%04X on endpoint %d, cluster 0x%04X, doBind: %s",
             nodeId,
             entry->remote,
             entry->clusterId,
             (doBind ? "YES" : "NO"));
  }

  return doBind;
}

#define addFoundTargetCallbackCheck(nodeId,  \
                                    binding, \
                                    groupId, \
                                    doBind)  \
  addSimpleCheck("foundTargetCallback",      \
                 "ip!!",                     \
                 (nodeId),                   \
                 (binding),                  \
                 (groupId),                  \
                 (doBind))

// -----------------------------------------------------------------------------
// SERVICE DISCOVERY ACTION.

static EmberAfServiceDiscoveryCallback *discoveryCallback = NULL;

static void callDiscoveryCallback(Action *action,
                                  EmberAfServiceDiscoveryStatus status,
                                  uint8_t *responseData)
{
  EmberAfServiceDiscoveryResult discoveryResult = {
    .status       = status,
    .responseData = responseData,
  };

  scriptAssert(action, discoveryCallback);
  discoveryCallback(&discoveryResult);
}

PRINTER(simpleDescriptorServiceDiscovery)
{
  EmberAfServiceDiscoveryStatus status
    = (EmberAfServiceDiscoveryStatus)action->contents[0];
  Parcel *inClusterListParcel = (Parcel *)action->contents[2];
  Parcel *outClusterListParcel = (Parcel *)action->contents[4];
  EmberAfClusterList list = {
    .inClusterCount = (uint8_t)action->contents[1],
    .inClusterList
      = (EmberAfClusterId *)inClusterListParcel->contents,

    .outClusterCount = (uint8_t)action->contents[3],
    .outClusterList
      = (EmberAfClusterId *)outClusterListParcel->contents,

    .profileId = (EmberAfProfileId)action->contents[5],
    .deviceId  = (uint16_t)action->contents[6],
    .endpoint  = (uint8_t)action->contents[7],
  };
  fprintf(stderr, " received simple descriptor service discovery response");
  fprintf(stderr, " inClusters (%d):", list.inClusterCount);
  printParcelBytes(inClusterListParcel);
  fprintf(stderr, " outClusters (%d):", list.outClusterCount);
  printParcelBytes(outClusterListParcel);
  fprintf(stderr, " endpoint: %d", list.endpoint);
  fprintf(stderr, " with status 0x%02X", status);
}

PERFORMER(simpleDescriptorServiceDiscovery)
{
  EmberAfServiceDiscoveryStatus status
    = (EmberAfServiceDiscoveryStatus)action->contents[0];
  Parcel *inClusterListParcel = (Parcel *)action->contents[2];
  Parcel *outClusterListParcel = (Parcel *)action->contents[4];
  EmberAfClusterList list = {
    .inClusterCount = (uint8_t)action->contents[1],
    .inClusterList
      = (EmberAfClusterId *)inClusterListParcel->contents,

    .outClusterCount = (uint8_t)action->contents[3],
    .outClusterList
      = (EmberAfClusterId *)outClusterListParcel->contents,

    .profileId = (EmberAfProfileId)action->contents[5],
    .deviceId  = (uint16_t)action->contents[6],
    .endpoint  = (uint8_t)action->contents[7],
  };

  callDiscoveryCallback(action, status, (uint8_t *)&list);
}

ACTION(simpleDescriptorServiceDiscovery, iipipiii);

#define addSimpleDescriptorServiceDiscoveryAction(status,           \
                                                  inClusterCount,   \
                                                  inClusterList,    \
                                                  outClusterCount,  \
                                                  outClusterList,   \
                                                  profileId,        \
                                                  deviceId,         \
                                                  endpoint)         \
  addAction(&simpleDescriptorServiceDiscoveryActionType,            \
            (status),                                               \
            (inClusterCount),                                       \
            makeMessage("s", inClusterList, inClusterCount << 1),   \
            (outClusterCount),                                      \
            makeMessage("s", outClusterList, outClusterCount << 1), \
            (profileId),                                            \
            (deviceId),                                             \
            (endpoint))

PRINTER(findIeeeServiceDiscovery)
{
  EmberAfServiceDiscoveryStatus status
    = (EmberAfServiceDiscoveryStatus)action->contents[0];
  Parcel *eui64 = (Parcel *)action->contents[1];
  fprintf(stderr, " received find ieee service discovery response from");
  printParcelBytes(eui64);
  fprintf(stderr, " with status 0x%02X", status);
}

PERFORMER(findIeeeServiceDiscovery)
{
  EmberAfServiceDiscoveryStatus status
    = (EmberAfServiceDiscoveryStatus)action->contents[0];
  Parcel *eui64 = (Parcel *)action->contents[1];

  callDiscoveryCallback(action, status, eui64->contents);
}

ACTION(findIeeeServiceDiscovery, ip);

#define addFindIeeeServiceDiscoveryAction(status, ieee) \
  addAction(&findIeeeServiceDiscoveryActionType,        \
            (status),                                   \
            makeMessage("s", (ieee), EUI64_SIZE))

// -----------------------------------------------------------------------------
// SIMPLE DESCRIPTOR CHECK.

EmberStatus emberAfFindClustersByDeviceAndEndpoint(EmberNodeId target,
                                                   uint8_t targetEndpoint,
                                                   EmberAfServiceDiscoveryCallback *callback)
{
  long *contents = functionCallCheck("simpleDescriptor",
                                     "ii",
                                     target,
                                     targetEndpoint);
  EmberStatus status = (EmberStatus)contents[2];

  discoveryCallback = callback;

  return status;
}

#define addSimpleDescriptorCheck(nodeId, nodeEndpoint, status) \
  addSimpleCheck("simpleDescriptor",                           \
                 "ii!",                                        \
                 (nodeId),                                     \
                 (nodeEndpoint),                               \
                 (status))

// -----------------------------------------------------------------------------
// FIND IEEE CHECK.

EmberStatus emberAfFindIeeeAddress(EmberNodeId shortAddress,
                                   EmberAfServiceDiscoveryCallback *callback)
{
  long *values = functionCallCheck("findIeee", "i", shortAddress);
  EmberStatus status = (EmberStatus)values[1];

  discoveryCallback = callback;

  return status;
}

#define addFindIeeeCheck(shortAddress, status) \
  addSimpleCheck("findIeee", "i!", (shortAddress), (status))

// -----------------------------------------------------------------------------
// MESSAGE CHECK.

EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t *message,
                                             EmberAfMessageSentFunction callback)
{
  long *contents = functionCallCheck("message",
                                     "iiiiii",
                                     destination,
                                     apsFrame->profileId,
                                     apsFrame->clusterId,
                                     apsFrame->sourceEndpoint,
                                     apsFrame->destinationEndpoint,
                                     false); // isUnicast?
  EmberStatus status = (EmberStatus)contents[6];

  return status;
}

#define addMessageCheck(destination, \
                        profileId,   \
                        clusterId,   \
                        srcEndpoint, \
                        dstEndpoint, \
                        isUnicast,   \
                        status)      \
  addSimpleCheck("message",          \
                 "iiiiii!",          \
                 (destination),      \
                 (profileId),        \
                 (clusterId),        \
                 (srcEndpoint),      \
                 (dstEndpoint),      \
                 (isUnicast),        \
                 (status))

// -----------------------------------------------------------------------------
// INITIATOR START ACTION.

PRINTER(initiatorStart)
{
  uint8_t endpoint = (uint8_t)action->contents[0];
  EmberStatus status = (EmberStatus)action->contents[1];
  fprintf(stderr, " INITIATOR tried to start on endpoint %d (0x%02X)",
          endpoint, status);
}

PERFORMER(initiatorStart)
{
  uint8_t endpoint = (uint8_t)action->contents[0];
  EmberStatus status = (EmberStatus)action->contents[1];
  scriptAssert(action, emberAfPluginFindAndBindInitiatorStart(endpoint) == status);
}

ACTION(initiatorStart, ii); // endpoint, status

#define addInitiatorStartAction(endpoint, status) \
  addAction(&initiatorStartActionType, (endpoint), (status))

// -----------------------------------------------------------------------------
// TARGET RESPONSE ACTION.

PRINTER(targetResponse)
{
  //uint8_t dstEndpoint = (uint8_t)action->contents[0];
  //EmberAfProfileId profileId = (EmberAfProfileId)action->contents[1];
  EmberAfClusterId clusterId = (EmberAfClusterId)action->contents[2];
  //uint8_t commandId = (uint8_t)action->contents[3];
  //uint8_t direction = (uint8_t)action->contents[4];
  EmberNodeId source = (EmberNodeId)action->contents[5];
  uint8_t srcEndpoint = (uint8_t)action->contents[6];
  fprintf(stderr, " target rsp (node 0x%04X, endpoint %d, cluster 0x%04X)",
          source, srcEndpoint, clusterId);
}

PERFORMER(targetResponse)
{
  uint8_t dstEndpoint = (uint8_t)action->contents[0];
  EmberAfProfileId profileId = (EmberAfProfileId)action->contents[1];
  EmberNodeId source = (EmberNodeId)action->contents[2];
  uint8_t srcEndpoint = (uint8_t)action->contents[3];

  EmberApsFrame apsFrame = {
    .profileId           = profileId,
    .clusterId           = ZCL_IDENTIFY_CLUSTER_ID,
    .sourceEndpoint      = srcEndpoint,
    .destinationEndpoint = dstEndpoint,
  };
  EmberAfClusterCommand clusterCommand = {
    .apsFrame        = &apsFrame,
    .type            = EMBER_INCOMING_UNICAST,
    .source          = source,
    .clusterSpecific = true,
    .commandId       = ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID,
    .direction       = ZCL_DIRECTION_SERVER_TO_CLIENT,
  };

  emAfCurrentCommand = &clusterCommand;

  // We don't really care about the timeout argument.
  scriptAssert(action, emberAfIdentifyClusterIdentifyQueryResponseCallback(0));
}

ACTION(targetResponse, iiii);

#define addTargetResponseAction(dstEndpoint, \
                                profileId,   \
                                source,      \
                                srcEndpoint) \
  addAction(&targetResponseActionType,       \
            (dstEndpoint),                   \
            (profileId),                     \
            (source),                        \
            (srcEndpoint))

// -----------------------------------------------------------------------------
// TARGET IDENTIFY CHECK.

// These are slotted in the index of the endpoint number.
static uint16_t targetIdentifyTimes[2 + 1] = { 0, 0, 0, };

PRINTER(targetIdentify)
{
  uint16_t expectedTargetIdentifyTime = (uint16_t)action->contents[0];
  uint8_t endpoint = (uint8_t)action->contents[1];
  fprintf(stderr, " targetIdentifyTime is %d and it should be %d (endpoint %d)",
          targetIdentifyTimes[endpoint], expectedTargetIdentifyTime, endpoint);
}
PERFORMER(targetIdentify)
{
  uint16_t expectedTargetIdentifyTime = (uint16_t)action->contents[0];
  uint8_t endpoint = (uint8_t)action->contents[1];
  scriptAssert(action,
               targetIdentifyTimes[endpoint] == expectedTargetIdentifyTime);
}

ACTION(targetIdentify, ii);

#define addTargetIdentifyCheck(targetIdentifyTime, endpoint) \
  addAction(&targetIdentifyActionType, (targetIdentifyTime), (endpoint))

// -----------------------------------------------------------------------------
// WRITE ATTRIBUTE CHECK.

EmberAfStatus emberAfWriteServerAttribute(uint8_t endpoint,
                                          EmberAfClusterId cluster,
                                          EmberAfAttributeId attributeID,
                                          uint8_t* dataPtr,
                                          uint8_t dataType)
{
  uint16_t data = (dataPtr[1] << 0x08) | (dataPtr[0] << 0x00);

  functionCallCheck("writeAttribute",
                    "iiii",
                    endpoint,
                    cluster,
                    attributeID,
                    data);

  if ((endpoint == TARGET1_ENDPOINT || endpoint == TARGET2_ENDPOINT)
      && cluster == ZCL_IDENTIFY_CLUSTER_ID
      && attributeID == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID) {
    targetIdentifyTimes[endpoint] = data;
  }

  return EMBER_ZCL_STATUS_SUCCESS;
}

#define addWriteAttributeCheck(endpoint, cluster, attributeID, data) \
  addSimpleCheck("writeAttribute",                                   \
                 "iiii",                                             \
                 (endpoint),                                         \
                 (cluster),                                          \
                 (attributeID),                                      \
                 (data))

// -----------------------------------------------------------------------------
// TARGET START ACTION.

PRINTER(targetStart)
{
  uint8_t endpoint = (uint8_t)action->contents[0];
  EmberAfStatus status = (EmberAfStatus)action->contents[1];
  fprintf(stderr, " TARGET tried to start on endpoint %d (0x%02X)",
          endpoint, status);
}

PERFORMER(targetStart)
{
  uint8_t endpoint = (uint8_t)action->contents[0];
  EmberAfStatus status = (EmberAfStatus)action->contents[1];

  containsIsReferringToTarget = true;
  scriptAssert(action, emberAfPluginFindAndBindTargetStart(endpoint) == status);
  containsIsReferringToTarget = false;
}

ACTION(targetStart, ii); // endpoint, status

#define addTargetStartAction(endpoint, status) \
  addAction(&targetStartActionType, (endpoint), (status))

// -----------------------------------------------------------------------------
// NO TARGETS.

static void noTargets(void)
{
  // Start the initiator without starting the target.
  addTargetIdentifyCheck(0,
                         TARGET1_ENDPOINT);
  addInitiatorStartAction(TARGET1_ENDPOINT,
                          EMBER_SUCCESS);
  // Catch the broadcast of the identify query.
  addMessageCheck(0xFFFF,                   // BDB wants this broadcasted to sleepy's
                  0,                        // profile id...eh?
                  ZCL_IDENTIFY_CLUSTER_ID,
                  INITIATOR_ENDPOINT,       // src endpoint
                  EMBER_BROADCAST_ENDPOINT, // dst endpoint
                  false,                    // isUnicast?
                  EMBER_SUCCESS);

  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS - 1);

  // ...since no responses come back in, we fail.
  addCompleteCallbackCheck(EMBER_ERR_FATAL);

  addRunAction(10);

  // The initiator should be able to start back up successfully again.
  addInitiatorStartAction(TARGET1_ENDPOINT,
                          EMBER_SUCCESS);
  addMessageCheck(0xFFFF,                   // BDB wants this broadcasted to sleepy's
                  0,                        // profile id...eh?
                  ZCL_IDENTIFY_CLUSTER_ID,
                  INITIATOR_ENDPOINT,       // src endpoint
                  EMBER_BROADCAST_ENDPOINT, // dst endpoint
                  false,                    // isUnicast?
                  EMBER_SUCCESS);
  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS - 1);
  addCompleteCallbackCheck(EMBER_ERR_FATAL);

  runScript();
}

// -----------------------------------------------------------------------------
// ONE TARGET.

// Returns the number of ticks used.
static void negativeTargetResponseCases(void)
{
  // The initiator should not care about bad responses like...
  // ...bad destination endpoint...
  addTargetResponseAction(0xAC,               // dst endpoint
                          INITIATOR_PROFILE_ID,
                          TARGET1_SHORT,      // src short
                          TARGET1_ENDPOINT);  // src endpoint
  // ...bad profile ID...
  addTargetResponseAction(INITIATOR_ENDPOINT, // dst endpoint
                          0xACDC,
                          TARGET1_SHORT,      // src short
                          TARGET1_ENDPOINT);  // src endpoint
}

static void oneTarget()
{
  // Start the target. It should try to write to the identify time attribute.
  addTargetStartAction(TARGET1_ENDPOINT,
                       EMBER_ZCL_STATUS_SUCCESS);
  addWriteAttributeCheck(TARGET1_ENDPOINT,
                         ZCL_IDENTIFY_CLUSTER_ID,
                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                         EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME,
                         TARGET1_ENDPOINT);

  // If we wait too long, the target should stop identifying...
  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(0,
                         TARGET1_ENDPOINT);

  // ...so fire up the target again.
  addTargetStartAction(TARGET1_ENDPOINT, EMBER_ZCL_STATUS_SUCCESS);
  addWriteAttributeCheck(TARGET1_ENDPOINT,
                         ZCL_IDENTIFY_CLUSTER_ID,
                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                         EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME,
                         TARGET1_ENDPOINT);

  // Start the initiator.
  addInitiatorStartAction(INITIATOR_ENDPOINT, EMBER_SUCCESS);

  // Catch the broadcast of the identify query.
  addMessageCheck(0xFFFF,                   // BDB wants this broadcasted to sleepy's
                  0,                        // profile id...eh?
                  ZCL_IDENTIFY_CLUSTER_ID,
                  INITIATOR_ENDPOINT,       // src endpoint
                  EMBER_BROADCAST_ENDPOINT, // dst endpoint
                  false,                    // isUnicast?
                  EMBER_SUCCESS);

  // The target should respond since it is identifying.
  addTargetResponseAction(INITIATOR_ENDPOINT, // dst endpoint
                          INITIATOR_PROFILE_ID,
                          TARGET1_SHORT,      // src short
                          TARGET1_ENDPOINT);  // src endpoint

  // Test some negative target response cases.
  negativeTargetResponseCases();

  // Wait for the initiator to check the responses.
  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS - 1);

  // The initiator should start the binding process by getting the target's
  // long address for the binding table entry.
  addFindIeeeCheck(TARGET1_SHORT,
                   EMBER_SUCCESS);
  addFindIeeeServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                    target1Long);

  // The initiator should then send a simple descriptor request to get
  // the possible clusters to bind with on the target.
  addSimpleDescriptorCheck(TARGET1_SHORT,    // dst short
                           TARGET1_ENDPOINT, // dst endpoint
                           EMBER_SUCCESS);
  addSimpleDescriptorServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                            TARGET1_IN_CLUSTER_COUNT,
                                            target1InClusterList,
                                            TARGET1_OUT_CLUSTER_COUNT,
                                            target1OutClusterList,
                                            TARGET1_PROFILE_ID,
                                            TARGET1_DEVICE_ID,
                                            TARGET1_ENDPOINT);

  // The application should get asked if it would like to create bindings
  // for any of the matching clusters on the target.
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INITIATOR_GROUP_ID,
                              false); // doBind?
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_ON_OFF_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INVALID_GROUP_ID,
                              true); // doBind?

  // Since we return the invalid group id, we will not send an AddGroup to the
  // remote binding and the binding should be made unicast.
  addSetBindingCheck(EMBER_UNICAST_BINDING,
                     INITIATOR_ENDPOINT, // local endpoint
                     ZCL_ON_OFF_CLUSTER_ID,
                     TARGET1_ENDPOINT,   // remote endpoint
                     makeMessage("s", target1Long, EUI64_SIZE),
                     EMBER_SUCCESS);

  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INVALID_GROUP_ID,
                              false); // doBind?

  // Finally, the complete callback should be called.
  addCompleteCallbackCheck(EMBER_SUCCESS);

  runScript();
}

// -----------------------------------------------------------------------------
// TWO TARGETS.

static void twoTargets(uint16_t groupId)
{
  // Start target1...
  addTargetStartAction(TARGET1_ENDPOINT,
                       EMBER_ZCL_STATUS_SUCCESS);
  addWriteAttributeCheck(TARGET1_ENDPOINT,
                         ZCL_IDENTIFY_CLUSTER_ID,
                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                         EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME,
                         TARGET1_ENDPOINT);
  // ...and target2.
  addTargetStartAction(TARGET2_ENDPOINT,
                       EMBER_ZCL_STATUS_SUCCESS);
  addWriteAttributeCheck(TARGET2_ENDPOINT,
                         ZCL_IDENTIFY_CLUSTER_ID,
                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                         EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME,
                         TARGET2_ENDPOINT);

  // Start the initiator.
  addInitiatorStartAction(INITIATOR_ENDPOINT,
                          EMBER_SUCCESS);

  // Catch the broadcast of the identify query.
  addMessageCheck(0xFFFF,                   // BDB wants this broadcasted to sleepy's
                  0,                        // profile id...eh?
                  ZCL_IDENTIFY_CLUSTER_ID,
                  INITIATOR_ENDPOINT,       // src endpoint
                  EMBER_BROADCAST_ENDPOINT, // dst endpoint
                  false,                    // isUnicast?
                  EMBER_SUCCESS);

  // Both targets should respond since they are identifying.
  addTargetResponseAction(INITIATOR_ENDPOINT, // dst endpoint
                          INITIATOR_PROFILE_ID,
                          TARGET1_SHORT,      // src node
                          TARGET1_ENDPOINT);  // src endpoint
  addTargetResponseAction(INITIATOR_ENDPOINT, // dst endpoint
                          INITIATOR_PROFILE_ID,
                          TARGET2_SHORT,      // src node
                          TARGET2_ENDPOINT);  // src endpoint

  // Wait for the initiator to check the responses.
  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS - 3);

  // The initiator should handle the first target...
  addFindIeeeCheck(TARGET1_SHORT,
                   EMBER_SUCCESS);
  addFindIeeeServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                    target1Long);
  addSimpleDescriptorCheck(TARGET1_SHORT,
                           TARGET1_ENDPOINT,
                           EMBER_SUCCESS);
  addSimpleDescriptorServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                            TARGET1_IN_CLUSTER_COUNT,
                                            target1InClusterList,
                                            TARGET1_OUT_CLUSTER_COUNT,
                                            target1OutClusterList,
                                            TARGET1_PROFILE_ID,
                                            TARGET1_DEVICE_ID,
                                            TARGET1_ENDPOINT);
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              groupId,
                              false); // doBind?
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_ON_OFF_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              groupId,
                              true); // doBind?

  // ...sending an AddGroup command if the groupId is valid...
  if (groupId != INVALID_GROUP_ID) {
    addSendUnicastCheck(EMBER_OUTGOING_DIRECT,
                        TARGET1_SHORT,
                        makeMessage("111<21p",
                                    0x01, // zcl fc
                                    0x00, // zcl sequence
                                    ZCL_ADD_GROUP_COMMAND_ID,
                                    groupId,
                                    INITIATOR_GROUP_NAME_LENGTH,
                                    makeStringMessage(INITIATOR_GROUP_NAME)));
  }

  // ...and create one binding with it since the application said so...
  addSetBindingCheck((groupId == INVALID_GROUP_ID
                      ? EMBER_UNICAST_BINDING
                      : EMBER_MULTICAST_BINDING),
                     INITIATOR_ENDPOINT, // local endpoint
                     ZCL_ON_OFF_CLUSTER_ID,
                     TARGET1_ENDPOINT,   // remote endpoint
                     (groupId == INVALID_GROUP_ID
                      ? makeMessage("s", target1Long, EUI64_SIZE)
                      : makeMessage("<2", groupId)),
                     EMBER_SUCCESS);

  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              groupId,
                              false); // doBind?

  // ...and then the initiator should handle the second target...
  addFindIeeeCheck(TARGET2_SHORT,
                   EMBER_SUCCESS);
  addFindIeeeServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                    target2Long);
  addSimpleDescriptorCheck(TARGET2_SHORT,
                           TARGET2_ENDPOINT,
                           EMBER_SUCCESS);
  addSimpleDescriptorServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                            TARGET2_IN_CLUSTER_COUNT,
                                            target2InClusterList,
                                            TARGET2_OUT_CLUSTER_COUNT,
                                            target2OutClusterList,
                                            TARGET2_PROFILE_ID,
                                            TARGET2_DEVICE_ID,
                                            TARGET2_ENDPOINT);
  addFoundTargetCallbackCheck(TARGET2_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET2_ENDPOINT,
                                                     target2Long,
                                                     0x00), // network index
                              groupId,
                              false); // doBind?
  addFoundTargetCallbackCheck(TARGET2_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_LEVEL_CONTROL_CLUSTER_ID,
                                                     TARGET2_ENDPOINT,
                                                     target2Long,
                                                     0x00), // network index
                              groupId,
                              true); // doBind?

  // ...sending an AddGroup command if the groupId is valid...
  if (groupId != INVALID_GROUP_ID) {
    addSendUnicastCheck(EMBER_OUTGOING_DIRECT,
                        TARGET2_SHORT,
                        makeMessage("111<21p",
                                    0x01, // zcl fc
                                    0x00, // zcl sequence
                                    ZCL_ADD_GROUP_COMMAND_ID,
                                    groupId,
                                    INITIATOR_GROUP_NAME_LENGTH,
                                    makeStringMessage(INITIATOR_GROUP_NAME)));
  }

  // ...and create one binding with it since the application said so.
  addSetBindingCheck((groupId == INVALID_GROUP_ID
                      ? EMBER_UNICAST_BINDING
                      : EMBER_MULTICAST_BINDING),
                     INITIATOR_ENDPOINT, // local endpoint
                     ZCL_LEVEL_CONTROL_CLUSTER_ID,
                     TARGET2_ENDPOINT,   // remote endpoint
                     (groupId == INVALID_GROUP_ID
                      ? makeMessage("s", target2Long, EUI64_SIZE)
                      : makeMessage("<2", groupId)),
                     EMBER_SUCCESS);

  addFoundTargetCallbackCheck(TARGET2_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET2_ENDPOINT,
                                                     target2Long,
                                                     0x00), // network index
                              groupId,
                              false); // doBind?

  // Finally, the complete callback should be called.
  addCompleteCallbackCheck(EMBER_SUCCESS);

  runScript();
}

static void twoTargetsRegular(void)
{
  twoTargets(INVALID_GROUP_ID);
}

static void twoTargetsMulticast(void)
{
  twoTargets(INITIATOR_GROUP_ID);
}

// -----------------------------------------------------------------------------
// DOUBLE BIND TEST.

// EMAPPFWKV2-1295 - we should check to make sure we are not rewritting the
// same bindings.

static void bindings(void)
{
  // Start target.
  addTargetStartAction(TARGET1_ENDPOINT,
                       EMBER_ZCL_STATUS_SUCCESS);
  addWriteAttributeCheck(TARGET1_ENDPOINT,
                         ZCL_IDENTIFY_CLUSTER_ID,
                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                         EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME);
  addTargetIdentifyCheck(EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME,
                         TARGET1_ENDPOINT);

  // Start initiator.
  addInitiatorStartAction(INITIATOR_ENDPOINT, EMBER_SUCCESS);
  addMessageCheck(0xFFFF,                   // BDB wants this broadcasted to sleepy's
                  0,                        // profile id...eh?
                  ZCL_IDENTIFY_CLUSTER_ID,
                  INITIATOR_ENDPOINT,       // src endpoint
                  EMBER_BROADCAST_ENDPOINT, // dst endpoint
                  false,                    // isUnicast?
                  EMBER_SUCCESS);

  // The target should respond since it is identifying.
  addTargetResponseAction(INITIATOR_ENDPOINT, // dst endpoint
                          INITIATOR_PROFILE_ID,
                          TARGET1_SHORT,      // src short
                          TARGET1_ENDPOINT);  // src endpoint

  // Wait for the initiator to check the responses, catch ieee and simple
  // descriptor handshakes.
  addRunAction(EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_TARGET_RESPONSES_DELAY_MS - 1);

  addFindIeeeCheck(TARGET1_SHORT,
                   EMBER_SUCCESS);
  addFindIeeeServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                    target1Long);

  addSimpleDescriptorCheck(TARGET1_SHORT,    // dst short
                           TARGET1_ENDPOINT, // dst endpoint
                           EMBER_SUCCESS);
  addSimpleDescriptorServiceDiscoveryAction(EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                                            TARGET1_IN_CLUSTER_COUNT,
                                            target1InClusterList,
                                            TARGET1_OUT_CLUSTER_COUNT,
                                            target1OutClusterList,
                                            TARGET1_PROFILE_ID,
                                            TARGET1_DEVICE_ID,
                                            TARGET1_ENDPOINT);

  // Aggressively create bindings. We should only be able to
  // make one binding for the identify cluster!
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INVALID_GROUP_ID,
                              true); // doBind?
  addSetBindingCheck(EMBER_UNICAST_BINDING,
                     INITIATOR_ENDPOINT, // local endpoint
                     ZCL_IDENTIFY_CLUSTER_ID,
                     TARGET1_ENDPOINT,   // remote endpoint
                     makeMessage("s", target1Long, EUI64_SIZE),
                     EMBER_SUCCESS);

  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_ON_OFF_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INVALID_GROUP_ID,
                              true); // doBind?
  addSetBindingCheck(EMBER_UNICAST_BINDING,
                     INITIATOR_ENDPOINT, // local endpoint
                     ZCL_ON_OFF_CLUSTER_ID,
                     TARGET1_ENDPOINT,   // remote endpoint
                     makeMessage("s", target1Long, EUI64_SIZE),
                     EMBER_SUCCESS);
  addFoundTargetCallbackCheck(TARGET1_SHORT,
                              makeBindingEntryParcel(EMBER_UNICAST_BINDING,
                                                     INITIATOR_ENDPOINT,
                                                     ZCL_IDENTIFY_CLUSTER_ID,
                                                     TARGET1_ENDPOINT,
                                                     target1Long,
                                                     0x00), // network index
                              INVALID_GROUP_ID,
                              true); // doBind?

  // Finally, the complete callback should be called.
  addCompleteCallbackCheck(EMBER_SUCCESS);

  runScript();
}

// -----------------------------------------------------------------------------
// MAIN.

extern void emberAfPluginFindAndBindInitiatorCheckTargetResponsesEventHandler(void);
extern EmberEventControl emberAfPluginFindAndBindInitiatorCheckTargetResponsesEventControl;
static EmberEventData findAndBindEventData[] = {
  { &emberAfPluginFindAndBindInitiatorCheckTargetResponsesEventControl,
    emberAfPluginFindAndBindInitiatorCheckTargetResponsesEventHandler, },
  { NULL,
    NULL, },
};
static EmberTaskId findAndBindTaskId;

static Test tests[] = {
  { "no-targets-test", noTargets           },
  { "one-target-test", oneTarget           },
  { "two-targets-test", twoTargetsRegular   },
  { "two-targets-multicast-test", twoTargetsMulticast },
  { "bindings-test", bindings            },
  { NULL, NULL },
};

int main(int argc, char *argv[])
{
  Thunk test = parseTestArgument(argc, argv, tests);

  // Fake emberAfInit.
  {
    emberAfSetExternalBuffer(externalBufferData,
                             EXTERNAL_BUFFER_DATA_SIZE,
                             &externalBufferLength,
                             &externalBufferApsFrame);
    findAndBindTaskId = emberTaskInit(findAndBindEventData);
  }

  // Fake emInitializeBindingTable().
  {
    uint8_t i = 0;
    emberBindingTableSize = EMBER_BINDING_TABLE_SIZE;
    for (; i < EMBER_BINDING_TABLE_SIZE; i++) {
      bindingTable[i].type = EMBER_UNUSED_BINDING;
    }
  }

  // These cannot be the same due to logic in static bool contains() below.
  assert(TARGET1_ENDPOINT != TARGET2_ENDPOINT);

  test();

  fprintf(stderr, " ]\n");

  return 0;
}

// -----------------------------------------------------------------------------
// STUBS.

// Unless otherwise specified, these cluster definitions are for the initiator.
static bool contains(uint8_t endpoint, EmberAfClusterId clusterId, bool isServer)
{
  uint8_t endpointIndex, clusterIndex;
  EmberAfEndpointType *endpointType = NULL;
  EmberAfCluster *cluster = NULL;

  if (!containsIsReferringToTarget) {
    for (endpointIndex = 0;
         endpointIndex < INITIATOR_ENDPOINT_COUNT;
         endpointIndex++) {
      if (emAfEndpoints[endpointIndex].endpoint != endpoint) {
        continue;
      }
      endpointType = emAfEndpoints[endpointIndex].endpointType;

      for (clusterIndex = 0;
           clusterIndex < endpointType->clusterCount;
           clusterIndex++) {
        cluster = endpointType->cluster + clusterIndex;
        if (cluster->clusterId == clusterId
            && ((!isServer && cluster->mask == CLUSTER_MASK_CLIENT)
                || (isServer && cluster->mask == CLUSTER_MASK_SERVER))) {
          return true;
        }
      }
    }
  } else { // containsIsReferringToTarget
    if (endpoint == TARGET1_ENDPOINT) { // target1
      if (!isServer) { // client
        for (clusterIndex = 0;
             clusterIndex < TARGET1_IN_CLUSTER_COUNT;
             clusterIndex++) {
          if (target1InClusterList[clusterIndex] == clusterId) {
            return true;
          }
        }
      } else { // server
        for (clusterIndex = 0;
             clusterIndex < TARGET1_IN_CLUSTER_COUNT;
             clusterIndex++) {
          if (target1OutClusterList[clusterIndex] == clusterId) {
            return true;
          }
        }
      }
    } else if (endpoint == TARGET2_ENDPOINT) { // target2
      if (!isServer) { // client
        for (clusterIndex = 0;
             clusterIndex < TARGET2_IN_CLUSTER_COUNT;
             clusterIndex++) {
          if (target2InClusterList[clusterIndex] == clusterId) {
            return true;
          }
        }
      } else { // server
        for (clusterIndex = 0;
             clusterIndex < TARGET2_IN_CLUSTER_COUNT;
             clusterIndex++) {
          if (target2OutClusterList[clusterIndex] == clusterId) {
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool emberAfContainsClient(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return contains(endpoint, clusterId, false); // isServer?
}
bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return contains(endpoint, clusterId, true); // isServer?
}

uint8_t emberAfIndexFromEndpoint(uint8_t endpoint)
{
  return (endpoint == INITIATOR_ENDPOINT ? 0 : 0);
}
uint8_t emberAfNetworkIndexFromEndpoint(uint8_t endpoint)
{
  return (endpoint == INITIATOR_ENDPOINT ? INITIATOR_NETWORK_INDEX : 0);
}

EmberStatus emberAfSendInterPan(EmberPanId panId,
                                const EmberEUI64 destinationLongId,
                                EmberNodeId destinationShortId,
                                EmberMulticastId multicastId,
                                EmberAfClusterId clusterId,
                                EmberAfProfileId profileId,
                                uint16_t messageLength,
                                uint8_t* messageBytes)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastWithCallback(EmberMulticastId multicastId,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t *message,
                                             EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame *apsFrame,
                                                     uint16_t messageLength,
                                                     uint8_t* message,
                                                     EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastWithAliasWithCallback(EmberMulticastId multicastId,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastToBindings(EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message)
{
  return EMBER_SUCCESS;
}

uint8_t emberAfNextSequence(void)
{
  return 0;
}

uint8_t emberAfResponseType = 0;

uint8_t emberAfGetBindingTableSize(void)
{
  return emberBindingTableSize;
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
  return EMBER_SUCCESS;
}

void emAfApplyRetryOverride(EmberApsOption *options)
{
}

void emAfApplyDisableDefaultResponse(uint8_t *frame_control)
{
}

// -----------------------------------------------------------------------------
// CALLBACK.

void scriptTickCallback(void)
{
  uint8_t i;
  for (i = 0; i < sizeof(targetIdentifyTimes) / sizeof(uint16_t); i++) {
    if (targetIdentifyTimes[i]) {
      targetIdentifyTimes[i]--;
    }
  }

  emberRunTask(findAndBindTaskId);
}
