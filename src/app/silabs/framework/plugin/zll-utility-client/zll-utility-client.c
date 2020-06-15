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
 * @brief Routines for the ZLL Utility Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

bool emberAfZllCommissioningClusterEndpointInformationCallback(uint8_t *ieeeAddress,
                                                               uint16_t networkAddress,
                                                               uint8_t endpointId,
                                                               uint16_t profileId,
                                                               uint16_t deviceId,
                                                               uint8_t version)
{
  emberAfZllCommissioningClusterPrint("RX: EndpointInformation ");
  emberAfZllCommissioningClusterDebugExec(emberAfPrintBigEndianEui64(ieeeAddress));
  emberAfZllCommissioningClusterPrintln(", 0x%2x, 0x%x, 0x%2x, 0x%2x, 0x%x",
                                        networkAddress,
                                        endpointId,
                                        profileId,
                                        deviceId,
                                        version);
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfZllCommissioningClusterGetGroupIdentifiersResponseCallback(uint8_t total,
                                                                       uint8_t startIndex,
                                                                       uint8_t count,
                                                                       uint8_t *groupInformationRecordList)
{
  uint16_t groupInformationRecordListLen = (emberAfCurrentCommand()->bufLen
                                            - (emberAfCurrentCommand()->payloadStartIndex
                                               + sizeof(total)
                                               + sizeof(startIndex)
                                               + sizeof(count)));
  uint16_t groupInformationRecordListIndex = 0;
  uint8_t i;

  emberAfZllCommissioningClusterPrint("RX: GetGroupIdentifiersResponse 0x%x, 0x%x, 0x%x,",
                                      total,
                                      startIndex,
                                      count);

  for (i = 0; i < count; i++) {
    uint16_t groupId;
    uint8_t groupType;
    groupId = emberAfGetInt16u(groupInformationRecordList, groupInformationRecordListIndex, groupInformationRecordListLen);
    groupInformationRecordListIndex += 2;
    groupType = emberAfGetInt8u(groupInformationRecordList, groupInformationRecordListIndex, groupInformationRecordListLen);
    groupInformationRecordListIndex++;
    emberAfZllCommissioningClusterPrint(" [0x%2x 0x%x]", groupId, groupType);
  }

  emberAfZllCommissioningClusterPrintln("");
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfZllCommissioningClusterGetEndpointListResponseCallback(uint8_t total,
                                                                   uint8_t startIndex,
                                                                   uint8_t count,
                                                                   uint8_t *endpointInformationRecordList)
{
  uint16_t endpointInformationRecordListLen = (emberAfCurrentCommand()->bufLen
                                               - (emberAfCurrentCommand()->payloadStartIndex
                                                  + sizeof(total)
                                                  + sizeof(startIndex)
                                                  + sizeof(count)));
  uint16_t endpointInformationRecordListIndex = 0;
  uint8_t i;

  emberAfZllCommissioningClusterPrint("RX: GetEndpointListResponse 0x%x, 0x%x, 0x%x,",
                                      total,
                                      startIndex,
                                      count);

  for (i = 0; i < count; i++) {
    uint16_t networkAddress;
    uint8_t endpointId;
    uint16_t profileId;
    uint16_t deviceId;
    uint8_t version;
    networkAddress = emberAfGetInt16u(endpointInformationRecordList, endpointInformationRecordListIndex, endpointInformationRecordListLen);
    endpointInformationRecordListIndex += 2;
    endpointId = emberAfGetInt8u(endpointInformationRecordList, endpointInformationRecordListIndex, endpointInformationRecordListLen);
    endpointInformationRecordListIndex++;
    profileId = emberAfGetInt16u(endpointInformationRecordList, endpointInformationRecordListIndex, endpointInformationRecordListLen);
    endpointInformationRecordListIndex += 2;
    deviceId = emberAfGetInt16u(endpointInformationRecordList, endpointInformationRecordListIndex, endpointInformationRecordListLen);
    endpointInformationRecordListIndex += 2;
    version = emberAfGetInt8u(endpointInformationRecordList, endpointInformationRecordListIndex, endpointInformationRecordListLen);
    endpointInformationRecordListIndex++;
    emberAfZllCommissioningClusterPrint(" [0x%2x 0x%x 0x%2x 0x%2x 0x%x]",
                                        networkAddress,
                                        endpointId,
                                        profileId,
                                        deviceId,
                                        version);
  }

  emberAfZllCommissioningClusterPrintln("");
  emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
  return true;
}
