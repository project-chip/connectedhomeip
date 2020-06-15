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
 * @brief Place for common functions / definitions shared by Green Power Client/Server
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include EMBER_AF_API_GREEN_POWER_COMMON

extern bool emGpAddressMatch(const EmberGpAddress *a1, const EmberGpAddress *a2);

uint16_t emCopyAdditionalInfoBlockStructureToArray(uint8_t commandId,
                                                   EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlockIn,
                                                   uint8_t *additionalInfoBlockOut)
{
  uint8_t charCount = 0;
  uint8_t *additionalInfoBlockOutPtr = additionalInfoBlockOut;

  // copy GPD Additional information block length
  emberAfCopyInt8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->totalLengthOfAddInfoBlock));
  additionalInfoBlockOutPtr += sizeof(uint8_t);

  if ((additionalInfoBlockIn->totalLengthOfAddInfoBlock) != 0x00) {
    emberAfGreenPowerClusterPrintln("%d ", additionalInfoBlockIn->totalLengthOfAddInfoBlock);
    //if ((additionalInfoBlockIn->optionRecord.optionSelector != 0x00) && (recordIndex < additionalInfoBlockIn->totalNbOfOptionRecord) ) {
    // copy  option selector field
    //optionSelector = (additionalInfoBlockIn->optionRecord[recordIndex].optionID & 0xF0) | (additionalInfoBlockIn->optionRecord[recordIndex].optionLength & 0x0F);
    emberAfCopyInt8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->optionSelector));
    additionalInfoBlockOutPtr += sizeof(uint8_t);

    if (commandId == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING ) {
      emberAfCopyInt8u(additionalInfoBlockOutPtr,
                       0,
                       (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.reportIdentifier));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      emberAfCopyInt8u(additionalInfoBlockOutPtr,
                       0,
                       (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attrOffsetWithinReport));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      emberAfCopyInt16u(additionalInfoBlockOutPtr,
                        0,
                        (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.clusterID));
      additionalInfoBlockOutPtr += sizeof(uint16_t);
      emberAfCopyInt16u(additionalInfoBlockOutPtr,
                        0,
                        (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.attributeID));
      additionalInfoBlockOutPtr += sizeof(uint16_t);
      emberAfCopyInt8u(additionalInfoBlockOutPtr,
                       0,
                       (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attributeDataType));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      //attributeoptions = ((additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent << 1) | (additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.clientServer));
      emberAfCopyInt8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attributeOptions));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      //if ( additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent) {
      if (additionalInfoBlockIn->optionData.compactAttr.attributeOptions & 0x02) {
        emberAfCopyInt16u(additionalInfoBlockOutPtr,
                          0,
                          (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.manufacturerID));
        additionalInfoBlockOutPtr += sizeof(uint16_t);
      }
      emberAfGreenPowerClusterPrintln("%d %d %d 0x%2X 0x%2X %d %d 0x%2X",
                                      additionalInfoBlockIn->optionSelector,
                                      additionalInfoBlockIn->optionData.compactAttr.reportIdentifier,
                                      additionalInfoBlockIn->optionData.compactAttr.attrOffsetWithinReport,
                                      additionalInfoBlockIn->optionData.compactAttr.clusterID,
                                      additionalInfoBlockIn->optionData.compactAttr.attributeID,
                                      additionalInfoBlockIn->optionData.compactAttr.attributeDataType,
                                      additionalInfoBlockIn->optionData.compactAttr.attributeOptions,
                                      additionalInfoBlockIn->optionData.compactAttr.manufacturerID);
    } else {
      emberAfCopyInt8u(additionalInfoBlockOutPtr,
                       0,
                       (uint8_t)(additionalInfoBlockIn->optionData.genericSwitch.contactStatus));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      emberAfCopyInt8u(additionalInfoBlockOutPtr,
                       0,
                       (uint8_t)(additionalInfoBlockIn->optionData.genericSwitch.contactBitmask));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
    }

    emberAfGreenPowerClusterPrintln("%d 0x%2X 0x%2X",
                                    additionalInfoBlockIn->optionSelector,
                                    additionalInfoBlockIn->optionData.genericSwitch.contactBitmask,
                                    additionalInfoBlockIn->optionData.genericSwitch.contactStatus);
  }
  charCount = (uint16_t)(additionalInfoBlockOutPtr - additionalInfoBlockOut);
  if (additionalInfoBlockIn->totalLengthOfAddInfoBlock != (charCount - 1)) {
    emberAfGreenPowerClusterPrintln("[%s:%d] Error in Addiotional Information Block additionalInfoBlockIn->totalLengthOfAddInfoBlock = %d, charCount= %d",
                                    __FUNCTION__, __LINE__, additionalInfoBlockIn->totalLengthOfAddInfoBlock, charCount);
  }
  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpNotificationSmart(uint16_t options,
                                                                uint32_t gpdSrcId,
                                                                uint8_t* gpdIeee,
                                                                uint8_t  gpdEndpoint,
                                                                uint32_t gpdSecurityFrameCounter,
                                                                uint8_t  gpdCommandId,
                                                                uint8_t gpdCommandPayloadLength,
                                                                const uint8_t* gpdCommandPayload,
                                                                uint16_t gppShortAddress,
                                                                uint8_t  gppDistance)
{
  uint16_t charCount = 0;
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                          | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_NOTIFICATION_COMMAND_ID,
                                         "v",
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(gpdEndpoint);
    charCount += EUI64_SIZE + 1;
  }

  emberAfPutInt32uInResp(gpdSecurityFrameCounter);
  emberAfPutInt8uInResp(gpdCommandId);
  emberAfPutInt8uInResp(gpdCommandPayloadLength);
  emberAfPutBlockInResp(gpdCommandPayload, gpdCommandPayloadLength);

  charCount += 2 + 1 + gpdCommandPayloadLength + 1;
  if (options & EMBER_AF_GP_NOTIFICATION_OPTION_PROXY_INFO_PRESENT) {
    emberAfPutInt16uInResp(gppShortAddress);
    emberAfPutInt8uInResp(gppDistance);
    charCount += 3;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpPairingSearchSmart(uint16_t options,
                                                                 uint32_t gpdSrcId,
                                                                 uint8_t* gpdIeee,
                                                                 uint8_t endpoint)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                         ZCL_GREEN_POWER_CLUSTER_ID,             \
                                         ZCL_GP_PAIRING_SEARCH_COMMAND_ID,       \
                                         "v",                                    \
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpTunnelingStopSmart(uint8_t options,
                                                                 uint32_t gpdSrcId,
                                                                 uint8_t* gpdIeee,
                                                                 uint8_t endpoint,
                                                                 uint32_t gpdSecurityFrameCounter,
                                                                 uint16_t gppShortAddress,
                                                                 int8_t gppDistance)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                         ZCL_GREEN_POWER_CLUSTER_ID,             \
                                         ZCL_GP_TUNNELING_STOP_COMMAND_ID,       \
                                         "u",                                    \
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  emberAfPutInt32uInResp(gpdSecurityFrameCounter);
  emberAfPutInt16uInResp(gppShortAddress);
  emberAfPutInt8uInResp(gppDistance);

  charCount += 7;

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(uint16_t options,
                                                                             uint32_t gpdSrcId,
                                                                             uint8_t* gpdIeee,
                                                                             uint8_t endpoint,
                                                                             uint8_t macSequenceNumber,
                                                                             EmberGpSecurityLevel gpdfSecurityLevel,
                                                                             EmberGpSecurityFrameCounter gpdSecurityFrameCounter,
                                                                             uint8_t gpdCommandId,
                                                                             uint8_t gpdCommandPayloadLength,
                                                                             const uint8_t *gpdCommandPayload,
                                                                             EmberNodeId gppShortAddress,
                                                                             uint8_t gppLink,
                                                                             EmberGpMic mic)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;
  uint32_t securityFrameCounter = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                          | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID,
                                         "v",
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  if (gpdfSecurityLevel == 0 ) {
    securityFrameCounter = (uint32_t)macSequenceNumber;
  } else {
    securityFrameCounter = gpdSecurityFrameCounter;
  }
  emberAfPutInt32uInResp(securityFrameCounter);
  emberAfPutInt8uInResp(gpdCommandId);
  emberAfPutInt8uInResp(gpdCommandPayloadLength);
  emberAfPutBlockInResp(gpdCommandPayload, gpdCommandPayloadLength);
  charCount += 3 + gpdCommandPayloadLength + 1;
  if (options & EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT) {
    emberAfPutInt16uInResp(gppShortAddress);
    emberAfPutInt8uInResp(gppLink);
    charCount += 3;
  }

  if (options & EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED) {
    emberAfPutInt32uInResp(mic);
    charCount += 4;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpTranslationTableUpdateSmart(uint16_t options,
                                                                          uint32_t gpdSrcId,
                                                                          uint8_t* gpdIeee,
                                                                          uint8_t endpoint,
                                                                          uint8_t translationsLen,
                                                                          GpTranslationTableUpdateTranslation* translations,
                                                                          EmberGpTranslationTableAdditionalInfoBlockOptionRecordField* additionalInfoBlock)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);

  uint16_t charCount = 0;
  if (((appId != EMBER_GP_APPLICATION_SOURCE_ID)
       && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS))
      || (translations == NULL)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND               \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),     \
                                         ZCL_GREEN_POWER_CLUSTER_ID,                 \
                                         ZCL_GP_TRANSLATION_TABLE_UPDATE_COMMAND_ID, \
                                         "v",                                        \
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  emberAfPutInt8uInResp(translations->index);
  charCount += 1;
  emberAfPutInt8uInResp(translations->gpdCommandId);
  charCount += 1;
  emberAfPutInt8uInResp(translations->endpoint);
  charCount += 1;
  emberAfPutInt16uInResp(translations->profile);
  charCount += 2;
  emberAfPutInt16uInResp(translations->cluster);
  charCount += 2;
  emberAfPutInt8uInResp(translations->zigbeeCommandId);
  charCount += 1;
  emberAfPutStringInResp(translations->zigbeeCommandPayload);
  charCount += emberAfStringLength(translations->zigbeeCommandPayload) + 1;
  if (emberAfGreenPowerTTUpdateGetAdditionalInfoBlockPresent(options) != 0
      && additionalInfoBlock) {
    uint8_t tempBuffer[32] = { 0 }; // Two records maximum per cli
    uint16_t length = emCopyAdditionalInfoBlockStructureToArray(translations->gpdCommandId,
                                                                additionalInfoBlock,
                                                                tempBuffer);
    emberAfPutInt8uInResp(length);
    charCount += 1;
    emberAfPutBlockInResp(tempBuffer, length);
    charCount += length;
  }
  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpPairingConfigurationSmart(uint8_t actions,
                                                                        uint16_t options,
                                                                        uint32_t gpdSrcId,
                                                                        uint8_t* gpdIeee,
                                                                        uint8_t endpoint,
                                                                        uint8_t deviceId,
                                                                        uint8_t groupListCount,
                                                                        uint8_t* groupList,
                                                                        uint16_t gpdAssignedAlias,
                                                                        uint8_t groupcastRadius,
                                                                        uint8_t securityOptions,
                                                                        uint32_t gpdSecurityFrameCounter,
                                                                        uint8_t* gpdSecurityKey,
                                                                        uint8_t numberOfPairedEndpoints,
                                                                        uint8_t* pairedEndpoints,
                                                                        uint8_t applicationInformation,
                                                                        uint16_t manufacturerId,
                                                                        uint16_t modeId,
                                                                        uint8_t numberOfGpdCommands,
                                                                        uint8_t * gpdCommandIdList,
                                                                        uint8_t clusterIdListCount,
                                                                        uint16_t * clusterListServer,
                                                                        uint16_t * clusterListClient,
                                                                        uint8_t switchInformationLength,
                                                                        uint8_t genericSwitchConfiguration,
                                                                        uint8_t currentContactStatus,
                                                                        uint8_t totalNumberOfReports,
                                                                        uint8_t numberOfReports,
                                                                        uint8_t* reportDescriptorM)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND            \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),  \
                                         ZCL_GREEN_POWER_CLUSTER_ID,              \
                                         ZCL_GP_PAIRING_CONFIGURATION_COMMAND_ID, \
                                         "uv",                                    \
                                         actions,
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  emberAfPutInt8uInResp(deviceId);
  emberAfPutBlockInResp(groupList, groupListCount);
  charCount += 1 + groupListCount;

  if (options & EMBER_AF_GP_PAIRING_CONFIGURATION_OPTION_ASSIGNED_ALIAS) {
    emberAfPutInt16uInResp(gpdAssignedAlias);
    charCount += 2;
  }

  emberAfPutInt8uInResp(groupcastRadius);
  charCount += 1;

  if (options & EMBER_AF_GP_PAIRING_CONFIGURATION_OPTION_SECURITY_USE) {
    emberAfPutInt8uInResp(securityOptions);
    emberAfPutInt32uInResp(gpdSecurityFrameCounter);
    emberAfPutBlockInResp(gpdSecurityKey, EMBER_ENCRYPTION_KEY_SIZE);
    charCount += 3 + EMBER_ENCRYPTION_KEY_SIZE;
  }

  emberAfPutInt8uInResp(numberOfPairedEndpoints);
  charCount += 1;
  if (numberOfPairedEndpoints < EMBER_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
    emberAfPutBlockInResp(pairedEndpoints, numberOfPairedEndpoints);
    charCount += numberOfPairedEndpoints;
  }

  if (options & EMBER_AF_GP_PAIRING_CONFIGURATION_OPTION_APPLICATION_INFORMATION_PRESENT) {
    emberAfPutInt8uInResp(applicationInformation);
    if (applicationInformation & EMBER_AF_GP_APPLICATION_INFORMATION_MANUFACTURE_ID_PRESENT) {
      emberAfPutInt16uInResp(manufacturerId);
      charCount += 2;
    }

    if (applicationInformation & EMBER_AF_GP_APPLICATION_INFORMATION_MODEL_ID_PRESENT) {
      emberAfPutInt16uInResp(modeId);
      charCount += 2;
    }

    if (applicationInformation & EMBER_AF_GP_APPLICATION_INFORMATION_GPD_COMMANDS_PRESENT) {
      emberAfPutInt8uInResp(numberOfGpdCommands);
      emberAfPutBlockInResp(gpdCommandIdList, numberOfGpdCommands);
      charCount += 1 + numberOfGpdCommands;
    }

    if (applicationInformation & EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_PRESENT) {
      emberAfPutInt8uInResp(clusterIdListCount);
      emberAfPutBlockInResp((uint8_t*)clusterListServer, sizeof(EmberAfClusterId) * (clusterIdListCount & EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK));
      emberAfPutBlockInResp((uint8_t*)clusterListClient, sizeof(EmberAfClusterId) * ((clusterIdListCount & EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK) >> EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET));
      charCount += 1
                   + sizeof(EmberAfClusterId) * (clusterIdListCount & EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK)
                   + sizeof(EmberAfClusterId) * ((clusterIdListCount & EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK) >> EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET);
    }
  }
  if (applicationInformation & EMBER_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_PRESENT) { // Switch Information Present
    emberAfPutInt8uInResp(switchInformationLength);
    charCount += 1;
    emberAfPutInt8uInResp(genericSwitchConfiguration);
    charCount += 1;
    emberAfPutInt8uInResp(currentContactStatus);
    charCount += 1;
  }
  // Application description should add the reports
  if (actions == EMBER_ZCL_GP_PAIRING_CONFIGURATION_ACTION_APPLICATION_DESCRIPTION) {
    emberAfPutInt8uInResp(totalNumberOfReports);
    emberAfPutInt8uInResp(numberOfReports);
    uint8_t nextOffset = 0;
    for (uint8_t index = 0; index < numberOfReports; index++) {
      uint8_t * descPtr;  // pointing to each report descriptor
      uint8_t descLength; // of each descriptor
      descPtr = &reportDescriptorM[nextOffset];
      // the report descriptor array has following meaning for indexs
      // descPtr[0] is reportId, descPtr[1] is options
      // the bit 0 of option indicates timeout Period Present (all other bits reserved)
      // if timeout(uint16 type)is present, the remaining length is present at index 5 else it is present
      // at index 3 of descPtr
      if (descPtr[1] & EMBER_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT) {
        descLength = descPtr[4] + 5;
      } else {
        descLength = descPtr[2] + 3;
      }
      nextOffset += descLength;
    }
    // ptr to block and bytes to copy
    emberAfPutBlockInResp(reportDescriptorM, nextOffset);
    charCount += nextOffset;
  }
  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpSinkTableRequestSmart(uint8_t options,
                                                                    uint32_t gpdSrcId,
                                                                    uint8_t* gpdIeee,
                                                                    uint8_t endpoint,
                                                                    uint8_t index)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                         ZCL_GREEN_POWER_CLUSTER_ID,             \
                                         ZCL_GP_SINK_TABLE_REQUEST_COMMAND_ID,   \
                                         "uw",                                   \
                                         options,                                \
                                         gpdSrcId);

  if ( ((options & EMBER_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
        >> EMBER_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET) == EMBER_ZCL_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TABLE_ENTRIES_BY_GPD_ID) {
    if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
      emberAfPutInt32uInResp(gpdSrcId);
      charCount += 4;
    } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
      emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
      emberAfPutInt8uInResp(index);
      charCount += EUI64_SIZE + 1;
    }
  }
  if ( ((options & EMBER_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
        >> EMBER_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET) == EMBER_ZCL_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TABLE_ENTRIES_BY_INDEX) {
    emberAfPutInt8uInResp(index);
    charCount++;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpProxyTableResponseSmart(uint8_t status,
                                                                      uint8_t totalNumberOfNonEmptyProxyTableEntries,
                                                                      uint8_t startIndex,
                                                                      uint8_t entriesCount,
                                                                      uint8_t* proxyTableEntries)
{
  uint16_t charCount = 0;
  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                                          | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_PROXY_TABLE_RESPONSE_COMMAND_ID,
                                         "uuuu",
                                         status,
                                         totalNumberOfNonEmptyProxyTableEntries,
                                         startIndex,
                                         entriesCount);

  // FIXME: need to add proper fields to EmberGpProxyTableEntry / append.
  emberAfPutBlockInResp(proxyTableEntries, entriesCount);

  return charCount;
}

uint32_t emberAfFillCommandGreenPowerClusterGpNotificationResponseSmart(uint8_t options,
                                                                        uint32_t gpdSrcId,
                                                                        uint8_t* gpdIeee,
                                                                        uint8_t endpoint,
                                                                        uint32_t gpdSecurityFrameCounter)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_NOTIFICATION_RESPONSE_COMMAND_ID,
                                         "u",
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }
  emberAfPutInt32uInResp(gpdSecurityFrameCounter);
  charCount += 4;

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpPairingSmart(uint32_t options,
                                                           uint32_t gpdSrcId,
                                                           uint8_t* gpdIeee,
                                                           uint8_t endpoint,
                                                           uint8_t* sinkIeeeAddress,
                                                           uint16_t sinkNwkAddress,
                                                           uint16_t sinkGroupId,
                                                           uint8_t deviceId,
                                                           uint32_t gpdSecurityFrameCounter,
                                                           uint8_t* gpdKey,
                                                           uint16_t assignedAlias,
                                                           uint8_t groupcastRadius)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;
  uint8_t commMode = emberAfGreenPowerPairingOptionsGetCommMode(options);

  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }
  // The Disable default response sub-field of the Frame Control Field of
  // the ZCL header SHALL be set to 0b1.
  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                          | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_PAIRING_COMMAND_ID,
                                         "x",
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  if ((options & EMBER_AF_GP_PAIRING_OPTION_REMOVE_GPD) == 0x00) {
    if ((commMode == 0x00) || (commMode == 0x03)) {
      emberAfPutBlockInResp(sinkIeeeAddress, EUI64_SIZE);
      emberAfPutInt16uInResp(sinkNwkAddress);
      charCount += EUI64_SIZE + 2;
    } else if (commMode == 0x01 || commMode == 0x02) {
      emberAfPutInt16uInResp(sinkGroupId);
      charCount += 2;
    }
  }

  if (emberAfGreenPowerPairingOptionsGetAddSink(options)) {
    emberAfPutInt8uInResp(deviceId);
    charCount++;
  }
  if (options & EMBER_AF_GP_PAIRING_OPTION_GPD_SECURITY_FRAME_COUNTER_PRESENT) {
    emberAfPutInt32uInResp(gpdSecurityFrameCounter);
    charCount += 4;
  }
  if (options & EMBER_AF_GP_PAIRING_OPTION_GPD_SECURITY_KEY_PRESENT) {
    emberAfPutBlockInResp(gpdKey, EMBER_ENCRYPTION_KEY_SIZE);
    charCount += EMBER_ENCRYPTION_KEY_SIZE;
  }
  if (options & EMBER_AF_GP_PAIRING_OPTION_ASSIGNED_ALIAS_PRESENT) {
    emberAfPutInt16uInResp(assignedAlias);
    charCount += 2;
  }
  if (options & EMBER_AF_GP_PAIRING_OPTION_GROUPCAST_RADIUS_PRESENT) {
    emberAfPutInt8uInResp(groupcastRadius);
    charCount++;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpProxyCommissioningModeSmart(uint8_t options,
                                                                          uint16_t commissioningWindow,
                                                                          uint8_t channel)
{
  // docs-14-0563-08: "In the current version of the GP specification,
  // the Channel present sub-field SHALL always be set to 0b0 and
  // the Channel field SHALL NOT be present."
  uint16_t charCount = 0;

  options &= ~EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT;
  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                          | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_PROXY_COMMISSIONING_MODE_COMMAND_ID,
                                         "u",
                                         options);

  if (options & EMBER_AF_GP_PROXY_COMMISSIONING_MODE_EXIT_MODE_ON_COMMISSIONING_WINDOW_EXPIRATION) {
    emberAfPutInt16uInResp(commissioningWindow);
    charCount += 2;
  }

  if (options & EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT) {
    emberAfPutInt8uInResp(channel);
    charCount++;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpResponseSmart(uint8_t  options,
                                                            uint16_t tempMasterShortAddress,
                                                            uint8_t  tempMasterTxChannel,
                                                            uint32_t gpdSrcId,
                                                            uint8_t* gpdIeee,
                                                            uint8_t  endpoint,
                                                            uint8_t  gpdCommandId,
                                                            uint8_t gpdCommandPayloadLength,
                                                            uint8_t* gpdCommandPayload)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint16_t charCount = 0;
  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_RESPONSE_COMMAND_ID,
                                         "uvu",
                                         options,
                                         tempMasterShortAddress,
                                         tempMasterTxChannel);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    charCount += EUI64_SIZE;
    emberAfPutInt8uInResp(endpoint);
    charCount++;
  }

  emberAfPutInt8uInResp(gpdCommandId);
  emberAfPutInt8uInResp(gpdCommandPayloadLength);
  emberAfPutBlockInResp(gpdCommandPayload, gpdCommandPayloadLength);
  charCount += gpdCommandPayloadLength + 1;

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpSinkTableResponseSmart(uint8_t status,
                                                                     uint8_t totalNumberofNonEmptySinkTableEntries,
                                                                     uint8_t startIndex,
                                                                     uint8_t sinkTableEntriesCount,
                                                                     uint8_t *sinkTableEntries)
{
  uint16_t charCount = 0;

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), \
                                         ZCL_GREEN_POWER_CLUSTER_ID,             \
                                         ZCL_GP_SINK_TABLE_RESPONSE_COMMAND_ID,  \
                                         "uuuu",                                 \
                                         status,                                 \
                                         totalNumberofNonEmptySinkTableEntries,  \
                                         startIndex,                             \
                                         sinkTableEntriesCount);

  // FIXME: we need to define a type for tokTypeStackGpSinkTableEntry
  // emberAfPutBlockInResp(sinkTableEntry, entriesCount);
  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpProxyTableRequestSmart(uint8_t options,
                                                                     uint32_t gpdSrcId,
                                                                     uint8_t* gpdIeee,
                                                                     uint8_t endpoint,
                                                                     uint8_t index)
{
  EmberGpApplicationId appId = emberAfGreenPowerGetApplicationId(options);
  uint8_t requestType = (options & EMBER_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE) >> 3;
  uint16_t charCount = 0;
  if ((appId != EMBER_GP_APPLICATION_SOURCE_ID)
      && (appId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                          | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                         ZCL_GREEN_POWER_CLUSTER_ID,
                                         ZCL_GP_PROXY_TABLE_REQUEST_COMMAND_ID,
                                         "u",
                                         options);

  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfPutInt32uInResp(gpdSrcId);
    charCount += 4;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPutBlockInResp(gpdIeee, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    charCount += EUI64_SIZE + 1;
  }

  if (requestType == 0x01) {
    emberAfPutInt8uInResp(endpoint);
    charCount++;
  }

  return charCount;
}

uint16_t emberAfFillCommandGreenPowerClusterGpSinkCommissioningModeSmart(uint8_t options,
                                                                         uint16_t gpmAddrForSecurity,
                                                                         uint16_t gpmAddrForPairing,
                                                                         uint8_t sinkEndpoint)
{
  return emberAfFillCommandGreenPowerClusterGpSinkCommissioningMode(options,
                                                                    gpmAddrForSecurity,
                                                                    gpmAddrForPairing,
                                                                    sinkEndpoint);
}

/*
 * Comparing two GP address.
 *
 * return - true if addresses are the same. otherwise, false.
 *
 */
bool emberAfGreenPowerCommonGpAddrCompare(const EmberGpAddress * a1,
                                          const EmberGpAddress * a2)
{
#ifndef EZSP_HOST
  return emGpAddressMatch(a1, a2);
#else
  if (a1->applicationId == EMBER_GP_APPLICATION_SOURCE_ID
      && a2->applicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
    if (a1->id.sourceId == a2->id.sourceId) {
      return true;
    }
  } else if (a1->applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS
             && a2->applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    if (!MEMCOMPARE(a1->id.gpdIeeeAddress, a2->id.gpdIeeeAddress, EUI64_SIZE)) {
      if (a1->endpoint == a2->endpoint
          || a1->endpoint == 0xff
          || a2->endpoint == 0xff
          || a1->endpoint == 0x00
          || a2->endpoint == 0x00) {
        return true;
      }
    }
  }
  return false;
#endif
}

bool emGpMakeAddr(EmberGpAddress *addr,
                  EmberGpApplicationId appId,
                  EmberGpSourceId srcId,
                  uint8_t *gpdIeee,
                  uint8_t endpoint)
{
  addr->applicationId = appId;
  if (appId == EMBER_GP_APPLICATION_SOURCE_ID) {
    addr->id.sourceId = srcId;
  } else if (appId == EMBER_GP_APPLICATION_IEEE_ADDRESS
             && gpdIeee != NULL) {
    MEMCOPY(addr->id.gpdIeeeAddress, gpdIeee, EUI64_SIZE);
    addr->endpoint = endpoint;
  } else {
    return false;
  }
  return true;
}
