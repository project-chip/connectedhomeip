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

#ifndef _SILABS_GREEN_POWER_COMMON_H_
#define _SILABS_GREEN_POWER_COMMON_H_

#define EMBER_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_NEXT_ATTEMPT (0x0F)
#define EMBER_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_SECOND_NEXT_ATTEMPT (0xF0)
#define EMBER_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_SECOND_NEXT_ATTEMPT_OFFSET (4)

#define EMBER_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT           (0x01)
#define EMBER_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION (0xFC)

#define EMBER_AF_GP_SINK_TABLE_ENTRY_OPTIONS_MASK                      (0x03FF)

// bitmap of how the translation table is scanned when a gpd entry
// is search into it
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID                          (0x01)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID                      (0x02)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD                     (0x04)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT                     (0x08)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK           (0x10)

#define EMBER_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET  (9)

#define EMBER_AF_GP_GPP_GPD_LINK_RSSI         (0x3F)
#define EMBER_AF_GP_GPP_GPD_LINK_LINK_QUALITY (0xC0)
#define EMBER_AF_GP_GPP_GPD_LINK_LINK_QUALITY_OFFSET (6)

#define EMBER_AF_GP_PAIRING_CONFIGURATION_ACTIONS_MASK          (0x0F)
#define EMBER_AF_GP_PAIRING_CONFIGURATION_OPTION_MASK           (0x07FF)
#define EMBER_AF_GP_PAIRING_CONFIGURATION_OPTION_MASK           (0x07FF)

#define EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK (0x0F)
#define EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK (0xF0)
#define EMBER_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET (4)

#define EMBER_AF_GP_APPLICATION_INFORMATION_GPD_APPLICATION_DESCRIPTION_COMMAND_FOLLOWS (0x20)
#define EMBER_AF_GP_APPLICATION_INFORMATION_GPD_APPLICATION_DESCRIPTION_COMMAND_FOLLOWS_OFFSET (5)

#define EMBER_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_NB_OF_CONTACT   (0x0F)
#define EMBER_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_SWITCH_TYPE     (0x30)
#define EMBER_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_SWITCH_TYPE_OFFSET (4)

#define GP_ENDPOINT (242)

#define GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT (4)
#define GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT (6)
#define EMBER_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET (9)
#define EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET (12)

#define EMBER_AF_GREEN_POWER_SERVER_GPDF_SCENE_DERIVED_GROUP_ID (0xFFFF)

// TT, Option byte : additional information block present or not
#define emberAfGreenPowerTTGetAdditionalInfoBlockPresent(options) ((uint8_t) (options & 0x08))
// TT update cmd, Option byte :
#define emberAfGreenPowerTTUpdateGetAdditionalInfoBlockPresent(options) ((uint16_t) (options & 0x0100))
// Pairing config cmd, Option byte :
#define emberAfGreenPowerPairingConfigGetApplicationInfoPresent(options) ((uint16_t) (options & 0x0400))
#define emberAfGreenPowerPairingOptionsGetCommMode(options) (((options) & EMBER_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE) >> 5)
#define emberAfGreenPowerPairingOptionsGetAddSink(options) (((options) & EMBER_AF_GP_PAIRING_OPTION_ADD_SINK) >> 3)
#define emberAfGreenPowerGetApplicationId(options) ((EmberGpApplicationId) ((options) & 0x07))
#define emberAfGreenPowerGpdSeqNumCap(entry)      ((entry->options >> 8) & 0x01)
#define emberAfGreenPowerSecurityLevel(entry)     ((entry->options >> 9) & 0x03)
// If the value of this sub-field is 0b1, then the GPD is not expected to change
// its position during its operation in the network.
#define emberAfGreenPowerFixedDuringOperation(options) ((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> EMBER_AF_GP_PAIRING_OPTION_GPD_FIXED_OFFSET) & 0x01)
#define emberAfGreenPowerMobileCap(options)          (!((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> EMBER_AF_GP_PAIRING_OPTION_GPD_FIXED_OFFSET) & 0x01))
#define emberAfGreenPowerPortableCap(options)     ((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> 7) & 0x01)
#define emberAfGreenPowerSecurityKeyType(options) ((((options) & GP_PAIRING_OPTIONS_SECURITY_KEY_TYPE) >> 11) & 0x07)

// as the proxy table response status present in enum.h, this is for the sink
typedef enum {
  EMBER_ZCL_GP_GPD_GPP_LINK_QUALITY_POOR       = 0x00,
  EMBER_ZCL_GP_GPD_GPP_LINK_QUALITY_MODERATE   = 0x01,
  EMBER_ZCL_GP_GPD_GPP_LINK_QUALITY_HIGH       = 0x02,
  EMBER_ZCL_GP_GPD_GPP_LINK_QUALITY_EXCELLENT  = 0x03,
}EmberAfGPGpdGppLinkQuality;

// new enum for enum.h
typedef enum {
  EMBER_ZCL_GP_UNKNOWN_SWITCH_TYPE        = 0x00,
  EMBER_ZCL_GP_BUTTON_SWITCH_TYPE         = 0x01,
  EMBER_ZCL_GP_ROCKER_SWITCH_TYPE         = 0x02,
  EMBER_ZCL_GP_RESERVED_SWITCH_TYPE       = 0x03,
} EmberAfGPGenericSwSwitchType;

//enum: EmberAfGpGpdDeviceId
typedef enum {
  EMBER_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_ONE_STATE_SWITCH   = 0x00,
  EMBER_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_TWO_STATE_SWITCH   = 0x01,
  EMBER_GP_DEVICE_ID_GPD_ON_OFF_SWITCH                     = 0x02,
  EMBER_GP_DEVICE_ID_GPD_LEVEL_CONTROL_SWITCH              = 0x03,
  EMBER_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH              = 0x04,
  EMBER_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_ONE_STATE_SWITCH = 0x05,
  EMBER_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_TWO_STATE_SWITCH = 0x06,
  EMBER_GP_DEVICE_ID_GPD_GENERIC_SWITCH                    = 0x07,
  EMBER_GP_DEVICE_ID_GPD_COLOR_DIMMER_SWITCH               = 0x10,
  EMBER_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH               = 0x11,
  EMBER_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH           = 0x12,
  EMBER_GP_DEVICE_ID_GPD_DOOR_LOCK_CONTROLLER_SWITCH       = 0x20,
  EMBER_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH         = 0x30,
  EMBER_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH            = 0x31,
  EMBER_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH                = 0x32,
  EMBER_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR         = 0x33,
  EMBER_GP_DEVICE_ID_GPD_SCENCES                           = 0xFE,
  EMBER_GP_DEVICE_ID_GPD_UNDEFINED                         = 0xFE,
} EmberAfGpDeviceIdGpd;

uint16_t emberAfFillCommandGreenPowerClusterGpNotificationSmart(uint16_t options,
                                                                uint32_t gpdSrcId,
                                                                uint8_t* gpdIeee,
                                                                uint8_t  gpdEndpoint,
                                                                uint32_t gpdSecurityFrameCounter,
                                                                uint8_t  gpdCommandId,
                                                                uint8_t gpdCommandPayloadLength,
                                                                const uint8_t* gpdCommandPayload,
                                                                uint16_t gppShortAddress,
                                                                uint8_t  gppDistance);

uint16_t emberAfFillCommandGreenPowerClusterGpPairingSearchSmart(uint16_t options,
                                                                 uint32_t gpdSrcId,
                                                                 uint8_t* gpdIeee,
                                                                 uint8_t endpoint);

uint16_t emberAfFillCommandGreenPowerClusterGpTunnelingStopSmart(uint8_t options,
                                                                 uint32_t gpdSrcId,
                                                                 uint8_t* gpdIeee,
                                                                 uint8_t endpoint,
                                                                 uint32_t gpdSecurityFrameCounter,
                                                                 uint16_t gppShortAddress,
                                                                 int8_t gppDistance);

uint16_t emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(uint16_t options,
                                                                             uint32_t gpdSrcId,
                                                                             uint8_t* gpdIeee,
                                                                             uint8_t endpoint,
                                                                             uint8_t sequenceNumber,
                                                                             EmberGpSecurityLevel gpdfSecurityLevel,
                                                                             EmberGpSecurityFrameCounter gpdSecurityFrameCounter,
                                                                             uint8_t gpdCommandId,
                                                                             uint8_t gpdCommandPayloadLength,
                                                                             const uint8_t *gpdCommandPayload,
                                                                             EmberNodeId gppShortAddress,
                                                                             uint8_t gppLink,
                                                                             EmberGpMic mic);

uint16_t emberAfFillCommandGreenPowerClusterGpTranslationTableUpdateSmart(uint16_t options,
                                                                          uint32_t gpdSrcId,
                                                                          uint8_t* gpdIeee,
                                                                          uint8_t endpoint,
                                                                          uint8_t translationsLen,
                                                                          GpTranslationTableUpdateTranslation* translations,
                                                                          EmberGpTranslationTableAdditionalInfoBlockOptionRecordField* additionnalInfoBlock);

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
                                                                        uint8_t* reportDescriptor);

uint16_t emberAfFillCommandGreenPowerClusterGpSinkTableRequestSmart(uint8_t options,
                                                                    uint32_t gpdSrcId,
                                                                    uint8_t* gpdIeee,
                                                                    uint8_t endpoint,
                                                                    uint8_t index);

uint16_t emberAfFillCommandGreenPowerClusterGpProxyTableResponseSmart(uint8_t status,
                                                                      uint8_t totalNumberOfNonEmptyProxyTableEntries,
                                                                      uint8_t startIndex,
                                                                      uint8_t entriesCount,
                                                                      uint8_t* proxyTableEntries);

uint32_t emberAfFillCommandGreenPowerClusterGpNotificationResponseSmart(uint8_t options,
                                                                        uint32_t gpdSrcId,
                                                                        uint8_t* gpdIeee,
                                                                        uint8_t endpoint,
                                                                        uint32_t gpdSecurityFrameCounter);

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
                                                           uint8_t groupcastRadius);

uint16_t emberAfFillCommandGreenPowerClusterGpProxyCommissioningModeSmart(uint8_t options,
                                                                          uint16_t commissioningWindow,
                                                                          uint8_t channel);

uint16_t emberAfFillCommandGreenPowerClusterGpResponseSmart(uint8_t  options,
                                                            uint16_t tempMasterShortAddress,
                                                            uint8_t  tempMasterTxChannel,
                                                            uint32_t gpdSrcId,
                                                            uint8_t* gpdIeee,
                                                            uint8_t  endpoint,
                                                            uint8_t  gpdCommandId,
                                                            uint8_t gpdCommandPayloadLength,
                                                            uint8_t* gpdCommandPayload);

uint16_t emberAfFillCommandGreenPowerClusterGpSinkTableResponseSmart(uint8_t status,
                                                                     uint8_t totalNumberofNonEmptySinkTableEntries,
                                                                     uint8_t startIndex,
                                                                     uint8_t sinkTableEntriesCount,
                                                                     uint8_t *sinkTableEntries);

uint16_t emberAfFillCommandGreenPowerClusterGpProxyTableRequestSmart(uint8_t options,
                                                                     uint32_t gpdSrcId,
                                                                     uint8_t* gpdIeee,
                                                                     uint8_t endpoint,
                                                                     uint8_t index);

uint16_t emberAfFillCommandGreenPowerClusterGpSinkCommissioningModeSmart(uint8_t options,
                                                                         uint16_t gpmAddrForSecurity,
                                                                         uint16_t gpmAddrForPairing,
                                                                         uint8_t sinkEndpoint);

bool emberAfGreenPowerCommonGpAddrCompare(const EmberGpAddress * a1,
                                          const EmberGpAddress * a2);

EmberNodeId emGpdAlias(EmberGpAddress *addr);

bool emGpMakeAddr(EmberGpAddress *addr,
                  EmberGpApplicationId appId,
                  EmberGpSourceId srcId,
                  uint8_t *gpdIeee,
                  uint8_t endpoint);
uint16_t emCopyAdditionalInfoBlockStructureToArray(uint8_t commandId,
                                                   EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlockIn,
                                                   uint8_t *additionalInfoBlockOut);
#endif //_GREEN_POWER_COMMON_H_
