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
 * @brief Routines for the ZLL Utility Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"

static uint16_t getMaxLength(void)
{
  // Capturing the value of emberAfMaximumApsPayloadLength in an uint16_t
  // even though emberAfMaximumApsPayloadLength returns an uint8_t because
  // it needs to be comapred with EMBER_AF_RESPONSE_BUFFER_LEN(ranging from 74-10000)
  // to set the maxLength.
  uint16_t maxLength = emberAfMaximumApsPayloadLength(EMBER_OUTGOING_DIRECT,
                                                      emberAfResponseDestination,
                                                      &emberAfResponseApsFrame);
  if (EMBER_AF_RESPONSE_BUFFER_LEN < maxLength) {
    maxLength = EMBER_AF_RESPONSE_BUFFER_LEN;
  }
  return maxLength;
}

bool emberAfZllCommissioningClusterGetGroupIdentifiersRequestCallback(uint8_t startIndex)
{
  EmberStatus sendStatus;
  uint8_t endpoint = emberAfCurrentEndpoint();
  uint8_t total = emberAfPluginZllCommissioningServerGroupIdentifierCountCallback(endpoint);
  uint8_t i;
  uint8_t *count;
  uint16_t maxLength;

  emberAfZllCommissioningClusterPrintln("RX: GetGroupIdentifiersRequest 0x%x",
                                        startIndex);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                             | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_GET_GROUP_IDENTIFIERS_RESPONSE_COMMAND_ID,
                            "uu",
                            total,
                            startIndex);

  count = &appResponseData[appResponseLength];
  emberAfPutInt8uInResp(0); // temporary count

  maxLength = getMaxLength();
  for (i = startIndex; i < total && appResponseLength + 3 <= maxLength; i++) {
    EmberAfPluginZllCommissioningGroupInformationRecord record;
    if (emberAfPluginZllCommissioningServerGroupIdentifierCallback(endpoint,
                                                                   i,
                                                                   &record)) {
      emberAfPutInt16uInResp(record.groupId);
      emberAfPutInt8uInResp(record.groupType);
      (*count)++;
    }
  }

  sendStatus = emberAfSendResponse();
  if (EMBER_SUCCESS != sendStatus) {
    emberAfZllCommissioningClusterPrintln("ZLL: failed to send %s response: "
                                          "0x%x",
                                          "group_identifiers",
                                          sendStatus);
  }
  return true;
}

bool emberAfZllCommissioningClusterGetEndpointListRequestCallback(uint8_t startIndex)
{
  EmberStatus sendStatus;
  uint8_t endpoint = emberAfCurrentEndpoint();
  uint8_t total = emberAfPluginZllCommissioningServerEndpointInformationCountCallback(endpoint);
  uint8_t i;
  uint8_t *count;
  uint16_t maxLength;

  emberAfZllCommissioningClusterPrintln("RX: GetEndpointListRequest 0x%x",
                                        startIndex);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                             | EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_GET_ENDPOINT_LIST_RESPONSE_COMMAND_ID,
                            "uu",
                            total,
                            startIndex);

  count = &appResponseData[appResponseLength];
  emberAfPutInt8uInResp(0); // temporary count

  maxLength = getMaxLength();
  for (i = startIndex; i < total && appResponseLength + 8 <= maxLength; i++) {
    EmberAfPluginZllCommissioningEndpointInformationRecord record;
    if (emberAfPluginZllCommissioningServerEndpointInformationCallback(endpoint,
                                                                       i,
                                                                       &record)) {
      emberAfPutInt16uInResp(record.networkAddress);
      emberAfPutInt8uInResp(record.endpointId);
      emberAfPutInt16uInResp(record.profileId);
      emberAfPutInt16uInResp(record.deviceId);
      emberAfPutInt8uInResp(record.version);
      (*count)++;
    }
  }

  sendStatus = emberAfSendResponse();
  if (EMBER_SUCCESS != sendStatus) {
    emberAfZllCommissioningClusterPrintln("ZLL: failed to send %s response: "
                                          "0x%x",
                                          "endpoint_list",
                                          sendStatus);
  }
  return true;
}
