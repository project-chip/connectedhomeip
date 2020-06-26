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
 * @brief CLI for the Simple Metering Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"

#if !defined(EMBER_AF_GENERATE_CLI)

void emAfPluginSimpleMeteringClientCliSchSnapshot(void);
void emAfPluginSimpleMeteringClientCliStartSampling(void);
void emAfPluginSimpleMeteringClientCliGetSampledData(void);
void emAfPluginSimpleMeteringClientCliLocalChangeSupply(void);

EmberCommandEntry emberAfPluginSimpleMeteringClientCommands[] = {
  emberCommandEntryAction("sch-snapshot",
                          emAfPluginSimpleMeteringClientCliSchSnapshot,
                          "vuuwuuuwwuw",
                          "Schedule a snapshot."),
  emberCommandEntryAction("start-sampling",
                          emAfPluginSimpleMeteringClientCliStartSampling,
                          "vuuwwuvv",
                          "Send a start sampling command to a metering server."),
  emberCommandEntryAction("get-sampled-data",
                          emAfPluginSimpleMeteringClientCliGetSampledData,
                          "vuuvwuv",
                          "Send a start sampling command to a metering server."),
  emberCommandEntryAction("local-change-supply",
                          emAfPluginSimpleMeteringClientCliLocalChangeSupply,
                          "vuuu",
                          "Send a start sampling command to a metering server."),
  emberCommandEntryTerminator(),
};

#endif /* !defined(EMBER_AF_GENERATE_CLI) */

void emAfPluginSimpleMeteringClientCliSchSnapshot(void)
{
  uint8_t payload[13];
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t issuerId = (uint32_t)emberUnsignedCommandArgument(3);
  uint8_t commandIndex = (uint8_t)emberUnsignedCommandArgument(4);
  uint8_t numberOfCommands = (uint8_t)emberUnsignedCommandArgument(5);
  uint8_t snapshotScheduleId = (uint8_t)emberUnsignedCommandArgument(6);
  uint32_t startTime = (uint32_t)emberUnsignedCommandArgument(7);
  uint32_t snapshotSchedule = (uint32_t)emberUnsignedCommandArgument(8);
  uint8_t snapshotType = (uint8_t)emberUnsignedCommandArgument(9);
  uint32_t snapshotCause = (uint32_t)emberUnsignedCommandArgument(10);

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  payload[0] = snapshotScheduleId;
  emberAfCopyInt32u((uint8_t *)payload, 1, startTime);
  emberAfCopyInt24u((uint8_t *)payload, 5, snapshotSchedule);
  payload[8] = snapshotType;
  emberAfCopyInt32u((uint8_t *)payload, 9, snapshotCause);

  emberAfFillCommandSimpleMeteringClusterScheduleSnapshot(issuerId,
                                                          commandIndex,
                                                          numberOfCommands,
                                                          payload,
                                                          13);

  (void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
}

void emAfPluginSimpleMeteringClientCliStartSampling(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t issuerId = (uint32_t)emberUnsignedCommandArgument(3);
  uint32_t startTime = (uint32_t)emberUnsignedCommandArgument(4);
  uint8_t sampleType = (uint8_t)emberUnsignedCommandArgument(5);
  uint16_t sampleRequestInterval = (uint16_t)emberUnsignedCommandArgument(6);
  uint16_t maxNumberOfSamples = (uint16_t)emberUnsignedCommandArgument(7);

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                            ZCL_SIMPLE_METERING_CLUSTER_ID,         \
                            ZCL_START_SAMPLING_COMMAND_ID,          \
                            "wwuvv",                                \
                            issuerId,                               \
                            startTime,                              \
                            sampleType,                             \
                            sampleRequestInterval,                  \
                            maxNumberOfSamples);
  // emberAfFillCommandSimpleMeteringClusterStartSampling(issuerId,
  //                                                      startTime,
  //                                                      sampleType,
  //                                                      sampleRequestInterval,
  //                                                      maxNumberOfSamples);
  (void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
}

void emAfPluginSimpleMeteringClientCliGetSampledData(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint16_t sampleId = (uint16_t)emberUnsignedCommandArgument(3);
  uint32_t startTime = (uint32_t)emberUnsignedCommandArgument(4);
  uint8_t sampleType = (uint8_t)emberUnsignedCommandArgument(5);
  uint16_t numberOfSamples = (uint16_t)emberUnsignedCommandArgument(6);

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK), \
                            ZCL_SIMPLE_METERING_CLUSTER_ID,        \
                            ZCL_GET_SAMPLED_DATA_COMMAND_ID,       \
                            "vwuv",                                \
                            sampleId,                              \
                            startTime,                             \
                            sampleType,                            \
                            numberOfSamples);
  (void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
}

void emAfPluginSimpleMeteringClientCliLocalChangeSupply(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t proposedChange = (uint8_t)emberUnsignedCommandArgument(3);

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK), \
                            ZCL_SIMPLE_METERING_CLUSTER_ID,        \
                            ZCL_LOCAL_CHANGE_SUPPLY_COMMAND_ID,    \
                            "u",                                   \
                            proposedChange);

  (void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
}
