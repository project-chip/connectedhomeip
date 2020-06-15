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
 * @brief Routines for the Green Power Translation Table plugin.
 *******************************************************************************
   ******************************************************************************/

#include "enums.h"
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include EMBER_AF_API_GREEN_POWER_SERVER
#include EMBER_AF_API_GREEN_POWER_COMMON
#include EMBER_AF_API_GREEN_POWER_TRANSLATION_TABLE

#define isClusterInManufactureSpeceficRange(clusterId) ((0xFC00 <= clusterId) && (clusterId <= 0xFFFF))
#define isAttributeInManufactureSpecificRange(attributeId) ((0x5000 <= attributeId) && (attributeId <= 0xFFFF))

#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE
// Following GPDF to Zcl Command only applies for following ApplicationID
// EMBER_GP_APPLICATION_SOURCE_ID(0b000) and EMBER_GP_APPLICATION_IEEE_ADDRESS (0b010)
// NOTE: all mapped ZCL commands have frame control of ZCL_FRAME_CONTROL_CLIENT_TO_SERVER.
extern const EmberAfGreenPowerServerGpdSubTranslationTableEntry emberGpDefaultTranslationTable[];
extern uint16_t emGpDefaultTableSize;
static const EmberAfGreenPowerServerGpdSubTranslationTableEntry *getSystemDefinedDefaultTable(uint16_t *tableSize)
{
  *tableSize = emGpDefaultTableSize;
  return emberGpDefaultTranslationTable;
}
#endif

#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
EmberAfGreenPowerServerDefautGenericSwTranslation emberGpSwitchTranslationTable[] = {
  // default table for switch configuration :
  // switchType, nbOfIdentifiedContacts, nbOfTTEntriesNeeded, indicativeBitmask, gpdCommand, endpoint, zigbeeProfile, zigbeeCluster, zigbeeCommandId, payloadSrc, zclPayloadDefault
  // 1bp --> Toggle
  // b3, b0, b1, b0 TOGGLE
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 1, 1, 0x0F }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 2bp --> on off   b2, b0 OFF /  b3, b1 ON
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 2, 2, 0x0A }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 2, 2, 0x05 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 3bp --> dim+/stop dim-/stop toggle   b0 UP /b1 DOWN /b3 TOGGLE
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x08 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x02 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_DOWN } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_UP } } },
  // b0, b1 stop
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 1, 0x03 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 4bp --> on/off dim+/stop dim-/stop
  // b0 UP /b1 DOWN/ b2 OFF/b3 ON
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x08 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x04 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x02 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_DOWN } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_UP } } },
  // b0, b1 STOP
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 1, 0x03 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 5bp to 8bp --> pass to application
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 5, 1, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 5, 1, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 6, 1, 0x20 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 6, 1, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 8, 1, 0x80 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 8, 1, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 7, 1, 0x40 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_BUTTON_SWITCH_TYPE, 7, 1, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 1 rocker --> on/off
  //  b2, b0 OFF /  b3, b1 ON
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 2, 0x0A }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 2, 0x05 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 2 rocker --> on/off dim+/stop dim-/stop
  // b0 UP /b1 DOWN/ b2 OFF/b3 ON
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x08 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x04 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x02 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_DOWN } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x01 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, EMBER_ZCL_MOVE_MODE_UP } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 1, 0x03 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 3 to 4 rocker --> pass to application
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 3, 1, 0x20 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 3, 1, 0x20 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 4, 1, 0x80 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { EMBER_ZCL_GP_ROCKER_SWITCH_TYPE, 4, 1, 0x80 }, { true, EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
};

static const EmberAfGreenPowerServerDefautGenericSwTranslation*getSystemDefinedDefaultGenericSwitchTable(uint16_t *tableSize)
{
  *tableSize = (sizeof(emberGpSwitchTranslationTable) / sizeof(EmberAfGreenPowerServerDefautGenericSwTranslation));
  return &emberGpSwitchTranslationTable[0];
}
#endif

static const EmberAfGreenPowerServerGpdSubTranslationTableEntry *emGpDefaultTablePtr = NULL;
static const EmberAfGreenPowerServerDefautGenericSwTranslation *emGpDefaultGenericSwitchTablePtr = NULL;
static uint16_t emGpDefaultGenericSwitchTableSize = 0;
static EmGpCommandTranslationTable emGpTranslationTable = { 0 };
static EmberAfGreenPowerServerGpdSubTranslationTableEntry customizedTranslationTable[EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE] =  { 0 };
static EmberGpTranslationTableAdditionalInfoBlockField emGpAdditionalInfoTable = { 0 };

// Static functions
static uint8_t createCustomizedTranslationTableEntry(bool infoBlockPresent,
                                                     EmberGpAddress *gpdAddr,
                                                     uint8_t gpdCommandId,
                                                     uint8_t zbEndpoint,
                                                     uint16_t zigbeeProfile,
                                                     uint16_t zigbeeCluster,
                                                     uint8_t zigbeeCommandId,
                                                     uint8_t payloadLength,
                                                     uint8_t *payload,
                                                     uint8_t payloadSrc,
                                                     uint8_t *outIndex)
{
  uint8_t cTableIndex;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTable = emGpGetCustomizedTable();
  EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTableEntry = NULL;
  if (customizedTable == NULL) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  for (cTableIndex = 0; cTableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; cTableIndex++) {
    customizedTableEntry = &(customizedTable[cTableIndex]);
    if (customizedTableEntry->validEntry == false) {
      break;
    }
  }
  if (cTableIndex == EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }
  customizedTableEntry->gpdCommand = gpdCommandId;
  customizedTableEntry->endpoint = zbEndpoint;
  customizedTableEntry->zigbeeProfile = zigbeeProfile;
  customizedTableEntry->zigbeeCluster = zigbeeCluster;
  customizedTableEntry->zigbeeCommandId = zigbeeCommandId;
  if (payloadSrc) {
    customizedTableEntry->payloadSrc = payloadSrc;
  } else {
    customizedTableEntry->payloadSrc = EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA;
  }
  customizedTableEntry->zclPayloadDefault[0] = payloadLength;
  if (payloadLength > 0 && payloadLength < 0xFE
      &&  payloadLength < EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
    MEMCOPY(&customizedTableEntry->zclPayloadDefault[1], payload, payloadLength);
  }
  customizedTableEntry->validEntry = true;
  *outIndex = cTableIndex;
  emGpSetCustomizedTableEntry(cTableIndex);
  return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
}
static uint8_t compareTranslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                               EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *srcAddlInfoBlock,
                                                               EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *dstAddlInfoBlock)
{
  if (gpdCommand == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    if ((srcAddlInfoBlock->optionData.compactAttr.reportIdentifier == dstAddlInfoBlock->optionData.compactAttr.reportIdentifier)
        && (srcAddlInfoBlock->optionData.compactAttr.attrOffsetWithinReport == dstAddlInfoBlock->optionData.compactAttr.attrOffsetWithinReport)
        && (srcAddlInfoBlock->optionData.compactAttr.clusterID == dstAddlInfoBlock->optionData.compactAttr.clusterID)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeID == dstAddlInfoBlock->optionData.compactAttr.attributeID)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeDataType == dstAddlInfoBlock->optionData.compactAttr.attributeDataType)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeOptions == dstAddlInfoBlock->optionData.compactAttr.attributeOptions)
        && (srcAddlInfoBlock->optionData.compactAttr.manufacturerID == dstAddlInfoBlock->optionData.compactAttr.manufacturerID)) {
      return true;
    }
  } else if (gpdCommand == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
             || gpdCommand == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE) {
    if ((srcAddlInfoBlock->optionData.genericSwitch.contactStatus == dstAddlInfoBlock->optionData.genericSwitch.contactStatus)
        && (srcAddlInfoBlock->optionData.genericSwitch.contactBitmask == dstAddlInfoBlock->optionData.genericSwitch.contactBitmask)) {
      return true;
    }
  }
  return false;
}
static uint8_t deleteTanslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                             EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  uint8_t optionRecord = 0;
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = NULL;
  for (optionRecord = 0; optionRecord < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommand, addInfo, additionalInfoBlock)) {
      if (additionalInfoTable->validEntry[optionRecord]) {
        additionalInfoTable->validEntry[optionRecord]--;
        if (additionalInfoTable->validEntry[optionRecord] == 0) {
          MEMSET(addInfo, 0x00, sizeof(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField));
          additionalInfoTable->totlaNoOfEntries--;
        }
        emGpSetAdditionalInfoBlockTableEntry(optionRecord);
        return optionRecord;
      }
    }
  }
  return 0xFF;
}
static uint8_t createTanslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                             EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = NULL;
  for (uint8_t optionRecord = 0; optionRecord < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommand, addInfo, additionalInfoBlock)) {
      additionalInfoTable->validEntry[optionRecord]++;
      emGpSetAdditionalInfoBlockTableEntry(optionRecord);
      return optionRecord;
    }
  }
  for (uint8_t optionRecord = 0; optionRecord < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (additionalInfoTable->validEntry[optionRecord] == 0) {
      MEMCOPY(addInfo, additionalInfoBlock, sizeof(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField));
      additionalInfoTable->validEntry[optionRecord] += 1;
      additionalInfoTable->totlaNoOfEntries++;
      emGpSetAdditionalInfoBlockTableEntry(optionRecord);
      return optionRecord;
    }
  }
  return 0xFF;
}
static bool isCustomizedTableEntryReferenced(uint8_t index, uint8_t offset)
{
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  if ((emGptranslationtable != NULL) && (offset != 0xFF)) {
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; i++) {
      if ((emGptranslationtable->TableEntry[i].entry == CUSTOMIZED_TABLE_ENTRY)
          && (emGptranslationtable->TableEntry[i].offset == offset)
          && (index != i)) {
        return true;           //do not delete as reference is found for this customized table entry
      }
    }
  }
  return false;
}
static void deleteCustomizednTableEntry(uint8_t index)
{
  EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTable = emGpGetCustomizedTable();
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    MEMSET(&(customizedTable[index]),
           0x00,
           sizeof(EmberAfGreenPowerServerGpdSubTranslationTableEntry));
    emGpSetCustomizedTableEntry(index);
  }
}

static uint8_t addTranslationTableEntryByIndex(uint8_t incomingReqType,
                                               uint8_t index,
                                               bool infoBlockPresent,
                                               EmberGpAddress *gpdAddr,
                                               uint8_t gpdCommandId,
                                               uint8_t zbEndpoint,
                                               uint16_t zigbeeProfile,
                                               uint16_t zigbeeCluster,
                                               uint8_t zigbeeCommandId,
                                               uint8_t payloadLength,
                                               uint8_t *payload,
                                               uint8_t payloadSrc,
                                               uint8_t additionalInfoLength,
                                               EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  uint8_t outIndex = 0xFF;
  uint8_t entryType = NO_ENTRY;
  for (entryType = CUSTOMIZED_TABLE_ENTRY; entryType > NO_ENTRY; entryType--) {
    // First search for the matching entry in the Customized table. If not found then search in the Default Table.
    ret = emGpFindMatchingGenericTranslationTableEntry(entryType,
                                                       incomingReqType,
                                                       0xFF,
                                                       infoBlockPresent,
                                                       gpdCommandId,
                                                       zigbeeProfile,
                                                       zigbeeCluster,
                                                       zigbeeCommandId,
                                                       payloadLength,
                                                       payload,
                                                       &outIndex);
    if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
      break;
    }
  }
  if ((outIndex == 0xFF) && (entryType == NO_ENTRY)) {
    ret = createCustomizedTranslationTableEntry(infoBlockPresent,
                                                gpdAddr,
                                                gpdCommandId,
                                                zbEndpoint,
                                                zigbeeProfile,
                                                zigbeeCluster,
                                                zigbeeCommandId,
                                                payloadLength,
                                                payload,
                                                payloadSrc,
                                                &outIndex);

    if (ret != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
      return ret;
    } else {
      entryType = CUSTOMIZED_TABLE_ENTRY;
    }
  }
  if (outIndex != 0xFF) {
    if (emGptranslationtable->TableEntry[index].entry == NO_ENTRY) {
      //Do not increment total number of entries while replacing Table entry
      emGptranslationtable->totalNoOfEntries++;
      MEMSET(&emGptranslationtable->TableEntry[index], 0x00, sizeof(EmGpCommandTranslationTableEntry));
      emGptranslationtable->TableEntry[index].offset = 0xFF;
      emGptranslationtable->TableEntry[index].additionalInfoOffset = 0xFF;
    }
    if (infoBlockPresent) {
      uint8_t addInfoOffset = emGptranslationtable->TableEntry[index].additionalInfoOffset;
      if ((emGptranslationtable->TableEntry[index].entry != NO_ENTRY) && (addInfoOffset != 0xFF)) {
        EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
        EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
        uint8_t status = deleteTanslationTableAdditionalInfoBlockEntry(gpdCommandId, addInfo);
        if (status != 0xFF) {
          emGptranslationtable->TableEntry[index].additionalInfoOffset = 0xFF;
        }
      }
      emGptranslationtable->TableEntry[index].additionalInfoOffset = createTanslationTableAdditionalInfoBlockEntry(gpdCommandId, additionalInfoBlock);
    }
    if ((entryType == CUSTOMIZED_TABLE_ENTRY)
        && (emGptranslationtable->TableEntry[index].offset != 0xFF)
        && (emGptranslationtable->TableEntry[index].offset != outIndex)) {
      if (!isCustomizedTableEntryReferenced(index, emGptranslationtable->TableEntry[index].offset)) {
        deleteCustomizednTableEntry(emGptranslationtable->TableEntry[index].offset);
        emGptranslationtable->TableEntry[index].offset = 0xFF;
      }
    }

    emGptranslationtable->TableEntry[index].infoBlockPresent = infoBlockPresent;
    emGptranslationtable->TableEntry[index].gpApplicationId = gpdAddr->applicationId;
    MEMCOPY(&(emGptranslationtable->TableEntry[index].gpAddr), gpdAddr, sizeof(EmberGpAddress));
    emGptranslationtable->TableEntry[index].gpdCommand = gpdCommandId;
    emGptranslationtable->TableEntry[index].zbEndpoint = zbEndpoint;
    emGptranslationtable->TableEntry[index].offset = outIndex;
    emGptranslationtable->TableEntry[index].entry = entryType;
    emGpSetTranslationTableEntry(index);
    ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return ret;
}

static uint8_t addPairedDeviceToTranslationTable(uint8_t incomingReqType,
                                                 bool infoBlockPresent,
                                                 EmberGpAddress *gpdAddr,
                                                 uint8_t gpdCommandId,
                                                 uint8_t zbEndpoint,
                                                 uint16_t zigbeeProfile,
                                                 uint16_t zigbeeCluster,
                                                 uint8_t zigbeeCommandId,
                                                 uint8_t payloadLength,
                                                 uint8_t *payload,
                                                 uint8_t payloadSrc,
                                                 uint8_t additionalInfoLength,
                                                 EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock,
                                                 uint8_t *outNewTTEntryIndex)
{
  int tableIndex;
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  if ((emGptranslationtable->totalNoOfEntries) >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }

  for (tableIndex = 0; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
    if (emGptranslationtable->TableEntry[tableIndex].entry == NO_ENTRY ) {
      break;
    }
  }

  if (tableIndex == EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }

  ret = addTranslationTableEntryByIndex(incomingReqType,
                                        tableIndex,
                                        infoBlockPresent,
                                        gpdAddr,
                                        gpdCommandId,
                                        zbEndpoint,
                                        zigbeeProfile,
                                        zigbeeCluster,
                                        zigbeeCommandId,
                                        payloadLength,
                                        payload,
                                        payloadSrc,
                                        additionalInfoLength,
                                        additionalInfoBlock);
  if (ret != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
    MEMSET(&emGptranslationtable->TableEntry[tableIndex], 0x00, sizeof(EmGpCommandTranslationTableEntry));
    emGptranslationtable->TableEntry[tableIndex].offset = 0xFF;
    emGptranslationtable->TableEntry[tableIndex].additionalInfoOffset = 0xFF;
    emGptranslationtable->TableEntry[tableIndex].entry = NO_ENTRY;
  } else {
    *outNewTTEntryIndex = tableIndex;
  }
  return ret;
}

static void updateSwitchCurrentContact(EmberGpAddress *gpdAddr)
{
  GpCommDataSaved *commissioningGpd = emberAfGreenPowerServerFindCommissioningGpdInstance(gpdAddr);
  // The following is to ensure the rocker gets both the entries in a pair even if one of the bit is set.
  if (commissioningGpd != NULL
      && commissioningGpd->switchInformationStruct.switchType == EMBER_ZCL_GP_ROCKER_SWITCH_TYPE) {
    // Realign the current contacts as a pair of bits.
    // If one bit of the bit pairs (b7,b6),(b5,b4),(b3,b2) or (b1,b0), is set then set the bit pair
    for (uint8_t i = 0; i < commissioningGpd->switchInformationStruct.nbOfContacts; i += 2) {
      if (commissioningGpd->switchInformationStruct.currentContact & (0x03 << i)) {
        commissioningGpd->switchInformationStruct.currentContact |= (0x03 << i);
      }
    }
  }
}

void emberAfPluginGreenPowerTranslationTableStackStatusCallback(EmberStatus status)
{
  emberAfGreenPowerClusterPrintln("Green Power Translation Table Stack Status Callback status = %x", status);
  if (status == EMBER_NETWORK_DOWN
      && emberStackIsPerformingRejoin() == FALSE) {
    // Clear the additional info, translation table and sink table in order.
    embGpClearAdditionalInfoBlockTable();
    emGpClearCustomizedTable();
    emGpTransTableClearTranslationTable();
  }
}

void emberAfPluginGreenPowerTranslationTableInitCallback(void)
{
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  // On device initialization, Read Translation Table / Customized Table
  // and the Aditional Info Block Table from the persistent memory.

  //emGpGetCustomizedTable();
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; i++) {
    halCommonGetIndexedToken(&customizedTranslationTable[i], TOKEN_CUSTOMIZED_TABLE, i);
  }

  //emGpTransTableGetTranslationTable();
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; i++) {
    halCommonGetIndexedToken(&emGpTranslationTable.TableEntry[i], TOKEN_TRANSLATION_TABLE, i);
  }
  halCommonGetToken(&emGpTranslationTable.totalNoOfEntries, TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES);

  //emGpGetAdditionalInfoTable();
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    halCommonGetIndexedToken(&emGpAdditionalInfoTable.additionalInfoBlock[i], TOKEN_ADDITIONALINFO_TABLE, i);
    halCommonGetIndexedToken(&emGpAdditionalInfoTable.validEntry[i], TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES, i);
  }
  halCommonGetToken(&emGpAdditionalInfoTable.totlaNoOfEntries, TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES);
#endif
  emberAfPluginTableInitCallback();
}

// Get the entire translation table
EmGpCommandTranslationTable *emGpTransTableGetTranslationTable(void)
{
  return &emGpTranslationTable;
}

// Clears the entire translation table
void emGpTransTableClearTranslationTable(void)
{
  MEMSET(&emGpTranslationTable, 0x00, sizeof(EmGpCommandTranslationTable));
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; i++) {
    emGpTranslationTable.TableEntry[i].offset = 0xFF;
    emGpTranslationTable.TableEntry[i].additionalInfoOffset = 0xFF;
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
    halCommonSetIndexedToken(TOKEN_TRANSLATION_TABLE, i, &emGpTranslationTable.TableEntry[i]);
#endif
  }
  emGpTranslationTable.totalNoOfEntries = 0;
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  halCommonSetToken(TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES, &emGpTranslationTable.totalNoOfEntries);
#endif
}
// Set the translation table entry at index
void emGpSetTranslationTableEntry(uint8_t index)
{
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
    halCommonSetToken(TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES, &emGpTranslationTable.totalNoOfEntries);
    halCommonSetIndexedToken(TOKEN_TRANSLATION_TABLE, index, &emGpTranslationTable.TableEntry[index]);
  }
#endif
}
// Get the entire customized table
EmberAfGreenPowerServerGpdSubTranslationTableEntry *emGpGetCustomizedTable(void)
{
  if (EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    return &customizedTranslationTable[0];
  } else {
    return NULL;
  }
}
// Clears the entire customized table
void emGpClearCustomizedTable(void)
{
  MEMSET(customizedTranslationTable,
         0x00,
         sizeof(EmberAfGreenPowerServerGpdSubTranslationTableEntry)
         * EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE);
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; i++) {
    halCommonSetIndexedToken(TOKEN_CUSTOMIZED_TABLE, i, &customizedTranslationTable[i]);
  }
#endif
}
// Set the custmized table entry at index
void emGpSetCustomizedTableEntry(uint8_t index)
{
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    halCommonSetIndexedToken(TOKEN_CUSTOMIZED_TABLE, index, &customizedTranslationTable[index]);
  }
#endif
}
// Get the entire  additional info block table
EmberGpTranslationTableAdditionalInfoBlockField *emGpGetAdditionalInfoTable(void)
{
  return &emGpAdditionalInfoTable;
}
// Clears the entire additional info block table
void embGpClearAdditionalInfoBlockTable(void)
{
  MEMSET(&emGpAdditionalInfoTable, 0x00, sizeof(EmberGpTranslationTableAdditionalInfoBlockField));
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    halCommonSetIndexedToken(TOKEN_ADDITIONALINFO_TABLE, i, &(emGpAdditionalInfoTable.additionalInfoBlock[i]));
    halCommonSetIndexedToken(TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES, i, &(emGpAdditionalInfoTable.validEntry[i]));
  }
  halCommonSetToken(TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES, &(emGpAdditionalInfoTable.totlaNoOfEntries));
#endif
}
// Set the additional info block entry at index
void emGpSetAdditionalInfoBlockTableEntry(uint8_t index)
{
#if defined(EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE) {
    halCommonSetIndexedToken(TOKEN_ADDITIONALINFO_TABLE, index, &(emGpAdditionalInfoTable.additionalInfoBlock[index]));
    halCommonSetIndexedToken(TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES, index, &(emGpAdditionalInfoTable.validEntry[index]));
    halCommonSetToken(TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES, &(emGpAdditionalInfoTable.totlaNoOfEntries));
  }
#endif
}

// This function, validates and changes the option to fit the check if success
static bool manufactureIdValidation(uint8_t *attributeOptions,
                                    uint16_t manufacturerID,
                                    uint16_t clusterId,
                                    uint16_t attributeId)
{
  // Check if the cluster or attribute is in range of Manufacture Specific range
  if (((*attributeOptions) & 0x02)
      && (isClusterInManufactureSpeceficRange(clusterId)
          || isAttributeInManufactureSpecificRange(attributeId))) {
    // Check if the manufactureId is supported in sink for the attribute/cluster
    if (manufacturerID != EMBER_AF_MANUFACTURER_CODE) {
      // manufacturId present but does not match the manufactureCode in sink
      // TODO : call emberAfLocateAttributeMetadata to locate if any such
      // attribute of a cluster is present in sink or not.
      return false;
    } else {
      return true;
    }
  }
  // Neither the cluster not attribute is manufacture specific, hence ensure the
  // manufacture specific flag in option;
  *attributeOptions &= ~(0x02);
  return true;
}

static void addReportstoTranslationTable(EmberGpAddress *gpdAddr,
                                         uint8_t gpdCommandId,
                                         uint8_t endpoint)
{
  // none entry match, create TT entries for New GPD
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfo = { 0 };
  uint8_t additionalInfoReportLen = 0;
  uint8_t additionalInfoRecordLen = 0;
  uint8_t additionalInfoAttrLen = 0;
  uint8_t noOfattributes = 0;
  uint8_t length = 0;
  uint8_t reportLength = 0;
  uint8_t recordRemainingLength = 0;
  uint8_t attributeRemainingLength = 0;
  uint8_t outIndex = 0xFF;

  uint8_t index = 0;
  const uint8_t *report = NULL;
  const GpCommDataSaved *gpdCommDataPtr = emberAfGreenPowerServerFindCommissioningGpdInstance(gpdAddr);

  if (gpdCommDataPtr == NULL) {
    return;
  }
  // Populate all the reports as option records of the additional block
  for (index = 0; index < gpdCommDataPtr->numberOfReports; index++) {
    report = emGpFindReportId(index,
                              gpdCommDataPtr->numberOfReports,
                              gpdCommDataPtr->reportsStorage);
    if (report != NULL) {
      length = 0;
      reportLength = 0;
      additionalInfoReportLen = 0;

      additionalInfo.optionData.compactAttr.reportIdentifier = report[length]; //reportId
      length += 1; //reportId
      additionalInfoReportLen++;
      if (report[length] & EMBER_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT) {
        length += 3; // option and 2 byte timeout
      } else {
        length += 1; // just the option byte
      }
      recordRemainingLength = report[length];
      length += 1; // length
      additionalInfoReportLen++;

      reportLength = length; // length
      do {
        emberAfPrintBuffer(EMBER_AF_PRINT_CORE, &report[length - 4], (recordRemainingLength + 4), true);
        emberAfGreenPowerClusterPrintln("");
        additionalInfoRecordLen = 0;
        uint8_t tempAttributeOption = ((report[length] & 0x1F) >> 3); // attribute option
        noOfattributes = ( (report[length] & 0x07) + 1);
        length += 1; //data point options
        additionalInfoRecordLen++;
        additionalInfo.optionData.compactAttr.clusterID = report[length] + ((uint16_t)report[length + 1] << 8);
        length += 2; // clusterid
        additionalInfoRecordLen += 2;
        if (tempAttributeOption & 0x02) { // ManufactureId present
          additionalInfo.optionData.compactAttr.manufacturerID = report[length] + ((uint16_t)report[length + 1] << 8);
          length += 2;
        }
        additionalInfoRecordLen = (additionalInfoReportLen + additionalInfoRecordLen);
        for (uint8_t attrRecordIndex = 0; attrRecordIndex < noOfattributes; attrRecordIndex++) {
          additionalInfo.optionData.compactAttr.attributeOptions = tempAttributeOption;
          additionalInfoAttrLen = 0;
          additionalInfo.optionData.compactAttr.attributeID = report[length] + ((uint16_t)report[length + 1] << 8);
          length += 2;
          additionalInfoAttrLen += 2;
          additionalInfo.optionData.compactAttr.attributeDataType = report[length];
          length += 1;
          additionalInfoAttrLen++;
          uint8_t reportDescAttributeOption = report[length];
          length += 1;
          attributeRemainingLength = (reportDescAttributeOption & 0x0F) + 1;
          if (reportDescAttributeOption & 0x10) {
            if ((attributeRemainingLength) && (recordRemainingLength > (length - reportLength))) {
              additionalInfo.optionData.compactAttr.attrOffsetWithinReport = report[length];
              length += 1;
              attributeRemainingLength--;
            }
          } else {
            additionalInfo.optionData.compactAttr.attrOffsetWithinReport = 0x00;
          }
          additionalInfoAttrLen++;
          if (reportDescAttributeOption & 0x20) {
            if ((attributeRemainingLength) && (recordRemainingLength > (length - reportLength))) {
              length += attributeRemainingLength;
            }
          }
          outIndex = 0xFF;
          if (reportDescAttributeOption & 0x10) { // Reported bit set
            if (emGpEndpointAndClusterIdValidation(endpoint,
                                                   !(additionalInfo.optionData.compactAttr.attributeOptions & 0x01), // Mask for server/Client
                                                   additionalInfo.optionData.compactAttr.clusterID)
                && manufactureIdValidation(&(additionalInfo.optionData.compactAttr.attributeOptions),
                                           additionalInfo.optionData.compactAttr.manufacturerID,
                                           additionalInfo.optionData.compactAttr.clusterID,
                                           additionalInfo.optionData.compactAttr.attributeID)) {
              if ((additionalInfo.optionData.compactAttr.attributeOptions) & 0x02) {
                additionalInfoRecordLen += 2;
              }
              additionalInfo.optionSelector = (additionalInfoRecordLen + additionalInfoAttrLen - additionalInfoReportLen);//(endRecord - startRecord - 1);
              emberAfGreenPowerClusterPrintln(" AdditionalInfo.optionSelector : %d additionalInfoReportdLen = %d additionalInfoRecordLen = %d additionalInfoAttrLen",
                                              additionalInfo.optionSelector,
                                              additionalInfoReportLen,
                                              additionalInfoRecordLen,
                                              additionalInfoAttrLen);
              additionalInfo.totalLengthOfAddInfoBlock = (additionalInfoRecordLen + additionalInfoAttrLen);

              uint8_t status = 0xFF;
              uint8_t outIndex = 0xFF;
              status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                        | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                        | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT
                                                                        | GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK),//uint8_t levelOfScan,
                                                                       true,
                                                                       gpdAddr,
                                                                       EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING,
                                                                       endpoint,
                                                                       NULL,
                                                                       &additionalInfo,
                                                                       &outIndex,
                                                                       0);
              if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
                status = addPairedDeviceToTranslationTable(ADD_PAIRED_DEVICE,
                                                           true,
                                                           gpdAddr,
                                                           EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING,
                                                           endpoint,
                                                           0,
                                                           additionalInfo.optionData.compactAttr.clusterID,
                                                           0,
                                                           0,
                                                           NULL,
                                                           0,
                                                           additionalInfo.totalLengthOfAddInfoBlock,
                                                           &additionalInfo,
                                                           &outIndex);
                emberAfGreenPowerClusterPrintln("Status %d attrRecord index = %d ", status, attrRecordIndex);
              }
            }
          }
        }
        emberAfGreenPowerClusterPrintln("record index = %d ", index);
      } while (recordRemainingLength > (length - reportLength));
    }
  }
}

// Gets switch configuration from configured switches in switch translation table
static const EmberAfGreenPowerServerDefautGenericSwTranslation *getSwConfig(uint8_t gpdCommandId,
                                                                            uint8_t switchType,
                                                                            uint8_t noOfPairedBits,
                                                                            uint8_t currentContactStatus)
{
  const EmberAfGreenPowerServerDefautGenericSwTranslation *genericSwitchTable = emGpDefaultGenericSwitchTablePtr;
  if (genericSwitchTable == NULL) {
    return NULL;
  }
  for (int i = 0; i < emGpDefaultTableSize; i++) {
    if (switchType == genericSwitchTable[i].SwitchType.switchType
        && (gpdCommandId == genericSwitchTable[i].genericSwitchDefaultTableEntry.gpdCommand)
        && noOfPairedBits == genericSwitchTable[i].SwitchType.nbOfIdentifiedContacts) {
      if (currentContactStatus & genericSwitchTable[i].SwitchType.indicativeBitmask) {
        return &(genericSwitchTable[i]);
      }
    }
  }
  return NULL;
}

// Prepares an additional info block from the switch configuration and received
// contact status
static void swInfoToAdditionalInfo(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfo,
                                   uint8_t currentContact)
{
  additionalInfo->totalLengthOfAddInfoBlock = 3;
  additionalInfo->optionSelector = 1;
  additionalInfo->optionData.genericSwitch.contactBitmask = currentContact;
  additionalInfo->optionData.genericSwitch.contactStatus = currentContact;
}

static void ignoreAnyBitSetHigherThanNoOfContacts(uint8_t noOfContacts, uint8_t *currentContact)
{
  uint8_t BitMask = 0;
  for (uint8_t iContact = 0; iContact < noOfContacts; iContact++) {
    BitMask |=  (0x01 << iContact);
  }
  (*currentContact) &= BitMask;
}

static uint8_t calculatePairedBits(uint8_t noOfContacts, uint8_t *savedContact)
{
  uint8_t noOfPairedBits = 0;
  ignoreAnyBitSetHigherThanNoOfContacts(noOfContacts,
                                        savedContact);

  for (uint8_t iContact = 0; iContact < noOfContacts; iContact++) {
    if ((*savedContact) & (0x01 << iContact)) {
      noOfPairedBits++;
    }
  }
  return noOfPairedBits;
}

static void countSwitchEntries(EmberGpAddress *gpdAddr, uint8_t *savedContact)
{
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = NULL;
  for ( uint8_t tableIndex = 0; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
    if (emGptranslationtable->TableEntry[tableIndex].entry == NO_ENTRY) {
      continue;
    }
    if (emberAfGreenPowerCommonGpAddrCompare(&emGptranslationtable->TableEntry[tableIndex].gpAddr, gpdAddr)) {
      if (emGptranslationtable->TableEntry[tableIndex].additionalInfoOffset != 0xFF) {
        addInfo = &(additionalInfoTable->additionalInfoBlock[emGptranslationtable->TableEntry[tableIndex].additionalInfoOffset]);
        *savedContact |= addInfo->optionData.genericSwitch.contactStatus;
      }
    }
  }
}
static uint8_t deleteTranslationTableEntryByIndex(uint8_t index,
                                                  bool infoBlockPresent,
                                                  uint8_t gpdCommandId,
                                                  uint16_t zigbeeProfile,
                                                  uint16_t zigbeeCluster,
                                                  uint8_t zigbeeCommandId,
                                                  uint8_t payloadLength,
                                                  uint8_t *payload,
                                                  uint8_t additionalInfoLength,
                                                  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  uint8_t status = 0xFF;
  uint8_t outIndex = 0xFF;
  ret = emGpFindMatchingGenericTranslationTableEntry(emGptranslationtable->TableEntry[index].entry,
                                                     DELETE_PAIRED_DEVICE,
                                                     emGptranslationtable->TableEntry[index].offset,
                                                     infoBlockPresent,
                                                     gpdCommandId,
                                                     zigbeeProfile,
                                                     zigbeeCluster,
                                                     zigbeeCommandId,
                                                     payloadLength,
                                                     payload,
                                                     &outIndex);
  if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS && outIndex != 0xFF) {
    EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
    uint8_t addInfoOffset = emGptranslationtable->TableEntry[index].additionalInfoOffset;
    if (addInfoOffset != 0xFF) {
      status = deleteTanslationTableAdditionalInfoBlockEntry(gpdCommandId, &(additionalInfoTable->additionalInfoBlock[addInfoOffset]));
      if (status != 0xFF) {
        emGptranslationtable->TableEntry[index].additionalInfoOffset = 0xFF;
      }
    }
    if ((emGptranslationtable->TableEntry[index].entry == CUSTOMIZED_TABLE_ENTRY) && (emGptranslationtable->TableEntry[index].offset != 0xFF)) {
      if (!isCustomizedTableEntryReferenced(index, emGptranslationtable->TableEntry[index].offset)) {
        deleteCustomizednTableEntry(emGptranslationtable->TableEntry[index].offset);
      }
    }
    MEMSET(&emGptranslationtable->TableEntry[index], 0x00, sizeof(EmGpCommandTranslationTableEntry));
    emGptranslationtable->TableEntry[index].entry = NO_ENTRY;
    emGptranslationtable->TableEntry[index].offset = 0xFF;
    emGptranslationtable->TableEntry[index].additionalInfoOffset = 0xFF;
    emGptranslationtable->totalNoOfEntries--;
    emGpSetTranslationTableEntry(index);
    ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return ret;
}

static void addSwitchToTranslationTable(EmberGpAddress *gpdAddr,
                                        uint8_t gpdCommandId,
                                        uint8_t endpoint)
{
  uint8_t savedContact = 0;
  uint8_t bitShift = 0;
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfo = { 0 };
  const GpCommDataSaved *gpdCommDataPtr = emberAfGreenPowerServerFindCommissioningGpdInstance(gpdAddr);

  if (gpdCommDataPtr == NULL) {
    return;
  }
  uint8_t alignSwitchType = gpdCommDataPtr->switchInformationStruct.switchType;
  // look at the switch type and supported no of contacts to find how many
  //commands and what are the commands need to be added in TT.
  const EmberAfGreenPowerServerDefautGenericSwTranslation *swConfig;
  // Find the first entry of the configuration, that will tell how many entries
  // to follow.
  //
  if (gpdCommDataPtr->switchInformationStruct.switchType == EMBER_ZCL_GP_UNKNOWN_SWITCH_TYPE) {
    alignSwitchType = EMBER_ZCL_GP_BUTTON_SWITCH_TYPE;
  } else {
    alignSwitchType = gpdCommDataPtr->switchInformationStruct.switchType;
    updateSwitchCurrentContact(gpdAddr);
  }

  countSwitchEntries(gpdAddr, &savedContact);
  if (savedContact == (savedContact | gpdCommDataPtr->switchInformationStruct.currentContact)) {
    return; //Ignore the contact if it is already commissioned
  }
  if (savedContact) {   // check if the switch already commissioned!
    for (bitShift = 0; bitShift < gpdCommDataPtr->switchInformationStruct.nbOfContacts; bitShift++) {
      uint8_t contactStatus = savedContact & (0x01 << bitShift);
      if (contactStatus) {
        EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfo = { 0 };
        swInfoToAdditionalInfo(&additionalInfo, contactStatus);
        uint8_t outIndex = 0xFF;
        uint8_t status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                          | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                          | GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK),//uint8_t levelOfScan,
                                                                         true,//bool infoBlockPresent,
                                                                         gpdAddr,
                                                                         gpdCommandId,
                                                                         0,//uint8_t zbEndpoint,
                                                                         0,
                                                                         &additionalInfo,
                                                                         &outIndex,
                                                                         0);
        if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
            && outIndex != 0xFF) {
          deleteTranslationTableEntryByIndex(outIndex,
                                             true,
                                             gpdCommandId,
                                             0,
                                             0,
                                             0,
                                             0x00,
                                             NULL,
                                             0x00,
                                             NULL);
        }
      }
    }
  }
  savedContact |= gpdCommDataPtr->switchInformationStruct.currentContact;
  uint8_t pairedBits = calculatePairedBits(gpdCommDataPtr->switchInformationStruct.nbOfContacts, &savedContact);
  //addpaired
  for (bitShift = 0; bitShift < gpdCommDataPtr->switchInformationStruct.nbOfContacts; bitShift++) {
    uint8_t contactStatus = (savedContact & (0x01 << bitShift));
    swConfig = getSwConfig(gpdCommandId, alignSwitchType, pairedBits, contactStatus);
    if (NULL != swConfig) {
      MEMSET(&additionalInfo, 0x00, sizeof(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField));
      swInfoToAdditionalInfo(&additionalInfo, contactStatus);
      uint8_t outIndex = 0xFF;
      uint8_t status;
      const EmberAfGreenPowerServerGpdSubTranslationTableEntry *entry;
      entry = &(swConfig->genericSwitchDefaultTableEntry);
      status = addPairedDeviceToTranslationTable(TRANSLATION_TABLE_UPDATE,
                                                 true,
                                                 gpdAddr,
                                                 entry->gpdCommand,
                                                 endpoint,
                                                 entry->zigbeeProfile,
                                                 entry->zigbeeCluster,
                                                 entry->zigbeeCommandId,
                                                 entry->zclPayloadDefault[0],
                                                 (uint8_t *)&(entry->zclPayloadDefault[1]),
                                                 entry->payloadSrc,
                                                 additionalInfo.totalLengthOfAddInfoBlock,
                                                 &additionalInfo,
                                                 &outIndex);
    }
  }
}

// This function Copies the addiitional information block structure to an array to be
// used for the Translation Table request/update request reponses.
uint16_t emGpCopyAdditionalInfoBlockArrayToStructure(uint8_t *additionalInfoBlockIn,
                                                     EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlockOut,
                                                     uint8_t gpdCommandId)
{
  uint8_t charCount = 0;
  uint8_t *additionalInfoBlockInPtr = additionalInfoBlockIn;
  uint8_t recordIndex = 0;
  uint8_t optionSelector = 0;
  uint8_t attributeoptions = 0;
  uint8_t totalLengthOfAddInfoBlockCnt = 0;
  emberAfGreenPowerClusterPrintln("GP SERVER - STORE ADDITIONAL INFOROMATION BLOCK into a structure");

  // copy GPD Additional information block length
  additionalInfoBlockOut->totalLengthOfAddInfoBlock = emberAfGetInt8u(additionalInfoBlockInPtr, 0, 1);
  additionalInfoBlockInPtr += sizeof(uint8_t);
  totalLengthOfAddInfoBlockCnt = additionalInfoBlockOut->totalLengthOfAddInfoBlock;

  if ((additionalInfoBlockOut->totalLengthOfAddInfoBlock) != 0x00) {
    optionSelector = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlock);
    additionalInfoBlockInPtr += sizeof(uint8_t);
    totalLengthOfAddInfoBlockCnt -= 1;

    if ( ((optionSelector & 0x0F) != 0) && (totalLengthOfAddInfoBlockCnt > 0)) {
      // copy  option selector field
      additionalInfoBlockOut->optionSelector = optionSelector;

      if (gpdCommandId  == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING ) {
        additionalInfoBlockOut->optionData.compactAttr.reportIdentifier = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.compactAttr.attrOffsetWithinReport = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.compactAttr.clusterID = emberAfGetInt16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint16_t);
        totalLengthOfAddInfoBlockCnt -= 2;

        additionalInfoBlockOut->optionData.compactAttr.attributeID = emberAfGetInt16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint16_t);
        totalLengthOfAddInfoBlockCnt -= 2;

        additionalInfoBlockOut->optionData.compactAttr.attributeDataType = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        attributeoptions = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;
        //additionalInfoBlockOut->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent  = ( (attributeoptions >> 1) & 0x01);
        //additionalInfoBlockOut->optionRecord[recordIndex].optionData.compactAttr.clientServer = (attributeoptions & 0x01);
        additionalInfoBlockOut->optionData.compactAttr.attributeOptions  = attributeoptions;
        if ( additionalInfoBlockOut->optionData.compactAttr.attributeOptions & 0x02 ) {
          additionalInfoBlockOut->optionData.compactAttr.manufacturerID = emberAfGetInt16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
          additionalInfoBlockInPtr += sizeof(uint16_t);
          totalLengthOfAddInfoBlockCnt -= 1;
        }
        recordIndex++;
      } else {
        additionalInfoBlockOut->optionData.genericSwitch.contactStatus = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.genericSwitch.contactBitmask = emberAfGetInt8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;
      }
    }
  }
  charCount = (uint16_t)(additionalInfoBlockInPtr - additionalInfoBlockIn);
  if (additionalInfoBlockOut->totalLengthOfAddInfoBlock != (charCount - 1)) {
    emberAfGreenPowerClusterPrintln("[%s:%d] Error in Addiotional Information Block", __FUNCTION__, __LINE__);
  }
  return charCount;
}

static uint16_t copyTranslationTableEntryToBuffer(uint8_t entryIndex,
                                                  EmberAfGreenPowerServerGpdSubTranslationTableEntry *translationTableEntry,
                                                  uint8_t *tempDatabuffer,
                                                  uint8_t tempDatabufferLen)
{
  uint8_t charCount = 0;
  uint8_t *bufPtr = tempDatabuffer;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  emberAfGreenPowerClusterPrintln("GP SERVER - STORE TRANSLATION TABLE ENTRY into a buffer");
  // no more ID field

  // copy 32bit or 64bit address field
  if (emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfCopyInt32u(bufPtr, 0, emGptranslationtable->TableEntry[entryIndex].gpAddr.id.sourceId);
    bufPtr += sizeof(uint32_t);
  } else if (emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    MEMMOVE(bufPtr, emGptranslationtable->TableEntry[entryIndex].gpAddr.id.gpdIeeeAddress, EUI64_SIZE);
    bufPtr += EUI64_SIZE;

    // copy ieee endpoint field
    emberAfCopyInt8u(bufPtr, 0, emGptranslationtable->TableEntry[entryIndex].gpAddr.endpoint);
    bufPtr += sizeof(uint8_t);
  }

  // copy GPD command ID field
  emberAfCopyInt8u(bufPtr, 0, (uint8_t)(translationTableEntry->gpdCommand));
  bufPtr += sizeof(uint8_t);

  // copy zbEndpoint field
  emberAfCopyInt8u(bufPtr, 0, (uint8_t)(translationTableEntry->endpoint));
  bufPtr += sizeof(uint8_t);

  // copy profile field
  emberAfCopyInt16u(bufPtr, 0, (uint16_t)(translationTableEntry->zigbeeProfile));
  bufPtr += sizeof(uint16_t);

  // copy cluster field
  emberAfCopyInt16u(bufPtr, 0, (uint16_t)(translationTableEntry->zigbeeCluster));
  bufPtr += sizeof(uint16_t);

  // copy Zigbee Command ID field
  emberAfCopyInt8u(bufPtr, 0, (uint8_t)(translationTableEntry->zigbeeCommandId));
  bufPtr += sizeof(uint8_t);

  uint8_t zclPayloadLen = translationTableEntry->zclPayloadDefault[0];
  // copy Zigbee payload length ID field
  emberAfCopyInt8u(bufPtr, 0, (uint8_t)(zclPayloadLen));
  bufPtr += sizeof(uint8_t);

  // copy Zigbee payload - first byte is a length and rest is a payload
  if (zclPayloadLen > 0 && zclPayloadLen < 0xFE) {
    MEMMOVE(bufPtr, &(translationTableEntry->zclPayloadDefault[1]), zclPayloadLen);
    bufPtr += zclPayloadLen;
  }

  if (emGptranslationtable->TableEntry[entryIndex].infoBlockPresent == TRUE) {
    EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
    uint8_t additionalInfoOffset = emGptranslationtable->TableEntry[entryIndex].additionalInfoOffset;
    EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = &(additionalInfoTable->additionalInfoBlock[additionalInfoOffset]);
    charCount = emCopyAdditionalInfoBlockStructureToArray(translationTableEntry->gpdCommand,
                                                          addInfo,
                                                          bufPtr);
    if (charCount) {
      bufPtr += charCount;
    }
  }
  charCount = (uint16_t)(bufPtr - tempDatabuffer);
  return charCount;
}

uint8_t emGpTransTableFindMatchingTranslationTableEntry(uint8_t levelOfScan,
                                                        bool infoBlockPresent,
                                                        EmberGpAddress *gpAddr,
                                                        uint8_t gpdCommandId,
                                                        uint8_t zbEndpoint,
                                                        uint8_t *gpdCmdPayload,
                                                        EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfoBlock,
                                                        uint8_t *outIndex,
                                                        uint8_t startIndex)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  uint8_t scanLevelMatchFound = 0xFF;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry const *genericTranslationTable = NULL;  //this pointer either point to default table or customied table
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  if (!emGptranslationtable->totalNoOfEntries) {
    return GP_TRANSLATION_TABLE_STATUS_EMPTY;
  }
  if (startIndex >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  for ( uint8_t tableIndex = startIndex; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
    scanLevelMatchFound = 0;
    if (emGptranslationtable->TableEntry[tableIndex].entry == NO_ENTRY) {
      continue;
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID) {
      if (emberAfGreenPowerCommonGpAddrCompare(&emGptranslationtable->TableEntry[tableIndex].gpAddr, gpAddr)) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID;
      }
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID) {
      if (emGptranslationtable->TableEntry[tableIndex].gpdCommand == gpdCommandId) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID;
      }
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT) {
      if (emGptranslationtable->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        EmberAfGreenPowerServerGpdSubTranslationTableEntry const *defaultTable = emGpGetDefaultTable();
        if (defaultTable == NULL) {
          return GP_TRANSLATION_TABLE_STATUS_FAILED;
        }
        genericTranslationTable = &(defaultTable[emGptranslationtable->TableEntry[tableIndex].offset]);
      } else if (emGptranslationtable->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTable = emGpGetCustomizedTable();
        if (customizedTable == NULL) {
          return GP_TRANSLATION_TABLE_STATUS_FAILED;
        }
        genericTranslationTable = &(customizedTable[emGptranslationtable->TableEntry[tableIndex].offset]);
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      if (genericTranslationTable->endpoint == EMBER_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
      }
      if (emGptranslationtable->TableEntry[tableIndex].zbEndpoint == zbEndpoint) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
      }
    }
    if (emGptranslationtable->TableEntry[tableIndex].infoBlockPresent == true) {
      uint8_t additionalInfoOffset = emGptranslationtable->TableEntry[tableIndex].additionalInfoOffset;
      if (additionalInfoOffset == 0xFF) {
        continue;
      }
      EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
      EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = &(additionalInfoTable->additionalInfoBlock[additionalInfoOffset]);

      if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK) {
        if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommandId, addInfo, addInfoBlock)) {
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK;
        }
      }
      if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD) {
        if (gpdCmdPayload == NULL) {
          continue;
        }
        if ((gpdCommandId == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING)
            && (addInfo->optionData.compactAttr.reportIdentifier  == gpdCmdPayload[0])) {
          // end of choice1 and choice2
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD;
        } else if ((gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
                    || gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE)
                   && (addInfo->optionData.genericSwitch.contactStatus == (addInfo->optionData.genericSwitch.contactBitmask & gpdCmdPayload[0]))) {
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD;
        }
      }
    }  //end of InfoBlockPresent
    if (levelOfScan == scanLevelMatchFound) {
      *outIndex = tableIndex;
      ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
      break;
    } else {
      continue;
    }
  }
  return ret;
}

uint8_t emGpTransTableDeletePairedDevicefromTranslationTableEntry(EmberGpAddress *gpdAddr)
{
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  if (!emGptranslationtable->totalNoOfEntries) {
    return GP_TRANSLATION_TABLE_STATUS_EMPTY;
  }
  uint8_t entryIndex;
  do {
    entryIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
    uint8_t status = emGpTransTableFindMatchingTranslationTableEntry(GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID,
                                                                     false,
                                                                     gpdAddr,
                                                                     0x00,
                                                                     0x00,
                                                                     NULL,
                                                                     NULL,
                                                                     &entryIndex,
                                                                     0);
    if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
        && (entryIndex != 0xFF)) {
      EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
      uint8_t addInfoOffset = emGptranslationtable->TableEntry[entryIndex].additionalInfoOffset;
      EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
      if (addInfoOffset != 0xFF) {
        status = deleteTanslationTableAdditionalInfoBlockEntry(emGptranslationtable->TableEntry[entryIndex].gpdCommand,
                                                               addInfo);
        if (status != 0xFF) {
          emGptranslationtable->TableEntry[entryIndex].additionalInfoOffset = 0xFF;
        }
      }
      if ((emGptranslationtable->TableEntry[entryIndex].entry == CUSTOMIZED_TABLE_ENTRY) && (emGptranslationtable->TableEntry[entryIndex].offset != 0xFF)) {
        if (!isCustomizedTableEntryReferenced(entryIndex, emGptranslationtable->TableEntry[entryIndex].offset)) {
          deleteCustomizednTableEntry(emGptranslationtable->TableEntry[entryIndex].offset);
        }
      }
      MEMSET(&emGptranslationtable->TableEntry[entryIndex], 0x00, sizeof(EmGpCommandTranslationTableEntry));
      emGptranslationtable->TableEntry[entryIndex].entry = NO_ENTRY;
      emGptranslationtable->TableEntry[entryIndex].offset = 0xFF;
      emGptranslationtable->TableEntry[entryIndex].additionalInfoOffset = 0xFF;
      emGptranslationtable->totalNoOfEntries--;
      emGpSetTranslationTableEntry(entryIndex);
    }
  } while (entryIndex != 0xFF);
  return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
}

uint8_t emGpTransTableGetTranslationTableEntry(uint8_t entryIndex,
                                               EmberAfGreenPowerServerGpdSubTranslationTableEntry *TranslationTableEntry)
{
  EmGpCommandTranslationTable const *emGptranslationtable = emGpTransTableGetTranslationTable();
  EmberAfGreenPowerServerGpdSubTranslationTableEntry const *genericTranslationTable = NULL;
  if (TranslationTableEntry == NULL || (entryIndex >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE)) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }

  if (entryIndex != 0xFF ) {
    if (emGptranslationtable->TableEntry[entryIndex].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    MEMSET(TranslationTableEntry, 0, sizeof(EmberAfGreenPowerServerGpdSubTranslationTableEntry));
    TranslationTableEntry->endpoint = emGptranslationtable->TableEntry[entryIndex].zbEndpoint;
    TranslationTableEntry->gpdCommand = emGptranslationtable->TableEntry[entryIndex].gpdCommand;
    if (emGptranslationtable->TableEntry[entryIndex].entry == DEFAULT_TABLE_ENTRY) {
      EmberAfGreenPowerServerGpdSubTranslationTableEntry const *defaultTable = emGpGetDefaultTable();
      if (defaultTable == NULL) {
        return EMBER_ERR_FATAL;
      }
      genericTranslationTable = &(defaultTable[emGptranslationtable->TableEntry[entryIndex].offset]);
    } else if (emGptranslationtable->TableEntry[entryIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
      EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTable = emGpGetCustomizedTable();
      if (customizedTable == NULL) {
        return EMBER_ERR_FATAL;
      }
      genericTranslationTable = &(customizedTable[emGptranslationtable->TableEntry[entryIndex].offset]);
    }
    if (genericTranslationTable != NULL) {
      TranslationTableEntry->validEntry = genericTranslationTable->validEntry;
      TranslationTableEntry->zigbeeProfile = genericTranslationTable->zigbeeProfile;
      TranslationTableEntry->zigbeeCluster = genericTranslationTable->zigbeeCluster;
      TranslationTableEntry->zigbeeCommandId = genericTranslationTable->zigbeeCommandId;
      MEMCOPY(&(TranslationTableEntry->payloadSrc),
              &(genericTranslationTable->payloadSrc),
              sizeof(uint8_t));
      MEMCOPY(&(TranslationTableEntry->zclPayloadDefault),
              &(genericTranslationTable->zclPayloadDefault),
              sizeof(EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN));
      return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
    } else {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
  }
  return GP_TRANSLATION_TABLE_STATUS_FAILED;
}

uint8_t emGpTransTableAddTranslationTableEntryUpdateCommand(uint8_t index,
                                                            bool infoBlockPresent,
                                                            EmberGpAddress *gpdAddr,
                                                            uint8_t gpdCommandId,
                                                            uint8_t zbEndpoint,
                                                            uint16_t zigbeeProfile,
                                                            uint16_t zigbeeCluster,
                                                            uint8_t zigbeeCommandId,
                                                            uint8_t payloadLength,
                                                            uint8_t *payload,
                                                            uint8_t payloadSrc,
                                                            uint8_t additionalInfoLength,
                                                            EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  bool ret = FALSE;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  if (index == 0xFF) {
    //If index is 0xff, the sink SHALL choose any free entry
    uint8_t newTTEntryIndex;
    ret = addPairedDeviceToTranslationTable(TRANSLATION_TABLE_UPDATE,
                                            infoBlockPresent,
                                            gpdAddr,
                                            gpdCommandId,
                                            zbEndpoint,
                                            zigbeeProfile,
                                            zigbeeCluster,
                                            zigbeeCommandId,
                                            payloadLength,
                                            payload,
                                            payloadSrc,
                                            additionalInfoLength,
                                            additionalInfoBlock,
                                            &newTTEntryIndex);
  } else {
    if (index >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (emGptranslationtable->TableEntry[index].entry != NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY;
    } else {
      ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                            index,
                                            infoBlockPresent,
                                            gpdAddr,
                                            gpdCommandId,
                                            zbEndpoint,
                                            zigbeeProfile,
                                            zigbeeCluster,
                                            zigbeeCommandId,
                                            payloadLength,
                                            payload,
                                            payloadSrc,
                                            additionalInfoLength,
                                            additionalInfoBlock);
    }
  }
  return ret;
}

uint8_t emGpTransTableReplaceTranslationTableEntryUpdateCommand(uint8_t index,
                                                                bool infoBlockPresent,
                                                                EmberGpAddress *gpdAddr,
                                                                uint8_t gpdCommandId,
                                                                uint8_t zbEndpoint,
                                                                uint16_t zigbeeProfile,
                                                                uint16_t zigbeeCluster,
                                                                uint8_t zigbeeCommandId,
                                                                uint8_t payloadLength,
                                                                uint8_t *payload,
                                                                uint8_t payloadSrc,
                                                                uint8_t additionalInfoLength,
                                                                EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  bool found = FALSE;
  int tableIndex = 0;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  if (index != 0xFF ) {
    if (index >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (emGptranslationtable->TableEntry[index].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY;
    }
    ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                          index,
                                          infoBlockPresent,
                                          gpdAddr,
                                          gpdCommandId,
                                          zbEndpoint,
                                          zigbeeProfile,
                                          zigbeeCluster,
                                          zigbeeCommandId,
                                          payloadLength,
                                          payload,
                                          payloadSrc,
                                          additionalInfoLength,
                                          additionalInfoBlock);
  } else {
    const EmberAfGreenPowerServerGpdSubTranslationTableEntry *genericTranslationTable = NULL;
    /*If index is 0xff, the sink replacing any number of translation entry(s) for the same
       (GPD ID, GPD Endpoint, GPD CommandID, EndPoint, Profile, Cluster) quintuple by the supplied number of entries*/

    for (tableIndex = 0; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
      if (!emGptranslationtable->TableEntry[tableIndex].entry) {
        continue;
      }
      if (emGptranslationtable->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        genericTranslationTable = emGpGetCustomizedTable();
      } else if (emGptranslationtable->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        genericTranslationTable = emGpGetDefaultTable();
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      genericTranslationTable = &genericTranslationTable[emGptranslationtable->TableEntry[tableIndex].offset];
      if ((emberAfGreenPowerCommonGpAddrCompare(&emGptranslationtable->TableEntry[tableIndex].gpAddr, gpdAddr)
           && (uint8_t)emGptranslationtable->TableEntry[tableIndex].gpdCommand == (uint8_t)gpdCommandId)
          && (emGptranslationtable->TableEntry[tableIndex].zbEndpoint == zbEndpoint)
          && (genericTranslationTable->zigbeeProfile == zigbeeProfile)
          && (genericTranslationTable->zigbeeCluster == zigbeeCluster)) {
        ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                              tableIndex,
                                              infoBlockPresent,
                                              gpdAddr,
                                              gpdCommandId,
                                              zbEndpoint,
                                              zigbeeProfile,
                                              zigbeeCluster,
                                              zigbeeCommandId,
                                              payloadLength,
                                              payload,
                                              payloadSrc,
                                              additionalInfoLength,
                                              additionalInfoBlock);
        if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
          found = TRUE;
        }
      }
    }
    if (!found) { //If Cannot replace, add new entry to the table
      for (tableIndex = 0; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
        if (emGptranslationtable->TableEntry[tableIndex].entry) {
          continue;
        }
        ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                              tableIndex,
                                              infoBlockPresent,
                                              gpdAddr,
                                              gpdCommandId,
                                              zbEndpoint,
                                              zigbeeProfile,
                                              zigbeeCluster,
                                              zigbeeCommandId,
                                              payloadLength,
                                              payload,
                                              payloadSrc,
                                              additionalInfoLength,
                                              additionalInfoBlock);
        break;
      }
    }
  }
  return ret;
}

static uint8_t removeTranslationTableEntryUpdateCommand(uint8_t index,
                                                        bool infoBlockPresent,
                                                        EmberGpAddress *gpdAddr,
                                                        uint8_t gpdCommandId,
                                                        uint8_t zbEndpoint,
                                                        uint16_t zigbeeProfile,
                                                        uint16_t zigbeeCluster,
                                                        uint8_t zigbeeCommandId,
                                                        uint8_t payloadLength,
                                                        uint8_t *payload,
                                                        uint8_t payloadSrc,
                                                        uint8_t additionalInfoLength,
                                                        EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *additionalInfoBlock)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  int tableIndex = 0;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();

  if (index != 0xFF ) {
    if (index >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (emGptranslationtable->TableEntry[index].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY;
    }
    ret = deleteTranslationTableEntryByIndex(index,
                                             infoBlockPresent,
                                             gpdCommandId,
                                             zigbeeProfile,
                                             zigbeeCluster,
                                             zigbeeCommandId,
                                             0x00,
                                             NULL,
                                             0x00,
                                             NULL);
  } else {
    const EmberAfGreenPowerServerGpdSubTranslationTableEntry *genericTranslationTable = NULL;
    // If index is 0xff, the sink removes any number of translation entry(s) for the same
    // (GPD ID, GPD Endpoint, GPD CommandID, EndPoint, Profile, Cluster) quintuple by the supplied number of entries
    for (tableIndex = 0; tableIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; tableIndex++) {
      if (emGptranslationtable->TableEntry[tableIndex].entry == NO_ENTRY ) {
        continue;
      }
      if (emGptranslationtable->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        genericTranslationTable = emGpGetCustomizedTable();
      } else if (emGptranslationtable->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        genericTranslationTable = emGpGetDefaultTable();
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      genericTranslationTable = &genericTranslationTable[emGptranslationtable->TableEntry[tableIndex].offset];
      if (emberAfGreenPowerCommonGpAddrCompare(&emGptranslationtable->TableEntry[tableIndex].gpAddr, gpdAddr)
          && (emGptranslationtable->TableEntry[tableIndex].gpdCommand == gpdCommandId)
          && (emGptranslationtable->TableEntry[tableIndex].zbEndpoint == zbEndpoint)
          && (genericTranslationTable->zigbeeProfile == zigbeeProfile)
          && (genericTranslationTable->zigbeeCluster == zigbeeCluster)) {
        ret = deleteTranslationTableEntryByIndex(tableIndex,
                                                 infoBlockPresent,
                                                 gpdCommandId,
                                                 zigbeeProfile,
                                                 zigbeeCluster,
                                                 zigbeeCommandId,
                                                 0x00,
                                                 NULL,
                                                 0x00,
                                                 NULL);
      }
    }
  }
  return ret;
}

void emGpRemoveGpdEndpointFromTranslationTable(EmberGpAddress *gpdAddr, uint8_t zbEndpoint)
{
  uint8_t outIndex = 0xFF;
  uint8_t startIndex = 0x00;
  EmGpCommandTranslationTable *translationTable = emGpTransTableGetTranslationTable();
  emberAfGreenPowerClusterPrintln("removeGpdEndpointFromTranslationTable zbEndpoint %d ", zbEndpoint);

  // by default : remove one particular GPD from this combo edp
  uint8_t levelOfScanMask = (GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT);
  if ((gpdAddr == NULL)
      || ((gpdAddr->applicationId == EMBER_GP_APPLICATION_SOURCE_ID
           && gpdAddr->id.sourceId == GP_ADDR_SRC_ID_WILDCARD)
          && ((gpdAddr->applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS)
              && (emberAfMemoryByteCompare(gpdAddr->id.gpdIeeeAddress, EUI64_SIZE, 0xFF))))) {
    // WildcardId thus remove AllGpd from this combo edp
    levelOfScanMask = GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
  }

  do {
    outIndex = 0xFF;
    uint8_t status = emGpTransTableFindMatchingTranslationTableEntry(levelOfScanMask,
                                                                     false,//bool infoBlockPresent,
                                                                     gpdAddr,
                                                                     0,
                                                                     zbEndpoint,//uint8_t zbEndpoint,
                                                                     0,
                                                                     NULL,
                                                                     &outIndex,
                                                                     startIndex);
    if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
        && outIndex != 0xFF) {
      deleteTranslationTableEntryByIndex(outIndex,
                                         true,
                                         translationTable->TableEntry[outIndex].gpdCommand,
                                         0,
                                         0,
                                         0,
                                         0x00,
                                         NULL,
                                         0x00,
                                         NULL);
      startIndex = outIndex + 1;
    }
  } while (outIndex != 0xFF);
}

static uint8_t findTTEntriesByStartIndex(EmberGpAddress *addr,
                                         uint8_t gpdCommandId,
                                         uint8_t *gpdCommandPayload,
                                         uint8_t *outIndex,
                                         uint8_t startIndex)
{
  uint8_t status = GP_TRANSLATION_TABLE_STATUS_FAILED;
  if (gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
      || gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE
      || gpdCommandId == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    uint8_t *payload = NULL;
    // In incomming GPDF has following interpretation for the command payload
    // gpdCommandPayload [0] = length (has special meaning 0xFF)
    // gpdCommandPayload [1] = data start
    if (gpdCommandPayload != NULL
        && gpdCommandPayload[0]
        && gpdCommandPayload[0] != GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_UNSPECIFIED_LENGTH
        && gpdCommandPayload[0] != GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_TT_DERIVED_LENGTH) {
      payload = &gpdCommandPayload[1];
    }
    // this is special case as there may be multiple configuration for the same
    // switch and command - hence the payload which is the currentStaus of contact
    // need to match as well.
    status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                              | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                              | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD),
                                                             true,
                                                             addr,
                                                             gpdCommandId,
                                                             0,
                                                             payload,
                                                             NULL,
                                                             outIndex,
                                                             startIndex);
  } else {
    status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                              | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID),
                                                             false,
                                                             addr,
                                                             gpdCommandId,
                                                             0,
                                                             NULL,
                                                             NULL,
                                                             outIndex,
                                                             startIndex);
    if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS
        || *outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX) {
      if (gpdCommandId == EMBER_ZCL_GP_GPDF_ATTRIBUTE_REPORTING
          || gpdCommandId == EMBER_ZCL_GP_GPDF_MFR_SP_ATTR_RPTG
          || gpdCommandId == EMBER_ZCL_GP_GPDF_MULTI_CLUSTER_RPTG
          || gpdCommandId == EMBER_ZCL_GP_GPDF_MFR_SP_MULTI_CLUSTER_RPTG) {
        status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                  | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID),
                                                                 false,
                                                                 addr,
                                                                 EMBER_ZCL_GP_GPDF_ANY_GPD_SENSOR_CMD,
                                                                 0,
                                                                 NULL,
                                                                 NULL,
                                                                 outIndex,
                                                                 startIndex);
      }
    }
  }
  return status;
}

static EmberStatus filloutCommandAndForward(EmberGpAddress *addr,
                                            uint8_t gpdCommandId,
                                            uint8_t *gpdCommandPayload,
                                            EmGpCommandTranslationTableEntry *tableEntry)
{
  if (tableEntry->entry != NO_ENTRY) {
    EmberAfGreenPowerServerGpdSubTranslationTableEntry const *genericTranslationTable = NULL;
    if (tableEntry->entry == DEFAULT_TABLE_ENTRY) {
      EmberAfGreenPowerServerGpdSubTranslationTableEntry const *defaultTable = emGpGetDefaultTable();
      genericTranslationTable = &(defaultTable[tableEntry->offset]);
      if (defaultTable == NULL) {
        return EMBER_ERR_FATAL;
      }
    } else if (tableEntry->entry == CUSTOMIZED_TABLE_ENTRY) {
      EmberAfGreenPowerServerGpdSubTranslationTableEntry *customizedTable = emGpGetCustomizedTable();
      if (customizedTable == NULL) {
        return EMBER_ERR_FATAL;
      }
      genericTranslationTable = &(customizedTable[tableEntry->offset]);
    }
    EmberGpTranslationTableAdditionalInfoBlockOptionRecordField *addInfo = NULL;
    if (tableEntry->infoBlockPresent) {
      EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
      addInfo = &(additionalInfoTable->additionalInfoBlock[tableEntry->additionalInfoOffset]);
      if (addInfo == NULL) {
        return EMBER_ERR_FATAL;
      }
    }
    // Pass the frame to Application
    if (genericTranslationTable->endpoint == EMBER_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
      emberAfGreenPowerClusterPassFrameWithoutTranslationCallback(addr,
                                                                  gpdCommandId,
                                                                  gpdCommandPayload);
      return EMBER_SUCCESS;
    }

    if (gpdCommandId == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
      if (addInfo == NULL) {
        return EMBER_ERR_FATAL;
      }
      uint8_t dataSize = emberAfGetDataSize(addInfo->optionData.compactAttr.attributeDataType);
      emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND                                                              \
                                 | ((addInfo->optionData.compactAttr.attributeOptions & 0x01)                    \
                                    ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER)), \
                                addInfo->optionData.compactAttr.clusterID,                                       \
                                genericTranslationTable->zigbeeCommandId,                                        \
                                "");
      emberAfAppendToExternalBuffer((uint8_t *)(&addInfo->optionData.compactAttr.attributeID), 2);
      emberAfAppendToExternalBuffer((uint8_t *)(&addInfo->optionData.compactAttr.attributeDataType), 1);
      emberAfAppendToExternalBuffer((uint8_t *)(gpdCommandPayload       \
                                                + CAR_DATA_POINT_OFFSET \
                                                + addInfo->optionData.compactAttr.attrOffsetWithinReport), dataSize);
      emberAfSetCommandEndpoints(tableEntry->zbEndpoint,
                                 tableEntry->zbEndpoint);
      return emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, emberAfGetNodeId());
    } else {
      return emGpForwardGpdToMappedEndpoint(addr,
                                            gpdCommandId,
                                            gpdCommandPayload,
                                            genericTranslationTable,
                                            tableEntry->zbEndpoint);
    }
  }
  return EMBER_ERR_FATAL;
}

void emGpForwardGpdCommandBasedOnTranslationTable(EmberGpAddress *addr,
                                                  uint8_t gpdCommandId,
                                                  uint8_t *gpdCommandPayload)
{
  // Get translation table head
  EmGpCommandTranslationTable *translationTable = emGpTransTableGetTranslationTable();
  if (translationTable == NULL) {
    // Error or No TT suported
    return;
  }
  // Loop through the TT entries and forward the commands to the endpoints that are commissionined
  uint8_t startIndex = 0;
  while (true) {
    uint8_t outIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
    uint8_t status = GP_TRANSLATION_TABLE_STATUS_FAILED;
    status = findTTEntriesByStartIndex(addr,
                                       gpdCommandId,
                                       gpdCommandPayload,
                                       &outIndex,
                                       startIndex);
    if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS
        || outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX) {
      // Error or end of TT
      return;
    }
    filloutCommandAndForward(addr,
                             gpdCommandId,
                             gpdCommandPayload,
                             &translationTable->TableEntry[outIndex]);
    startIndex = outIndex + 1;
  }
}

bool emberAfGreenPowerClusterGpTranslationTableUpdateCallback(uint16_t options,
                                                              uint32_t gpdSrcId,
                                                              uint8_t *gpdIeee,
                                                              uint8_t gpdEndpoint,
                                                              uint8_t *translations)
{
  uint8_t retval = 0;
  uint16_t payloadOffset = 0;
  uint8_t gpApplicationId = (options & 0x0007);
  EmberGpAddress gpdAddr;
  if (!emGpMakeAddr(&gpdAddr, gpApplicationId, gpdSrcId, gpdIeee, gpdEndpoint)) {
    return false;
  }
  uint8_t action = ((options >> 3) & 0x0003); //3..4bits
  uint8_t noOfTranslations = ((options >> 5) & 0x0003); //5..7 bits
  bool additionalInfoBlockPresent = ((options >> 8) & 0x0001); //8 bits
  uint8_t index = 0;
  uint8_t gpdCommandId = 0;
  uint8_t zbEndpoint = 0;
  uint16_t zigbeeProfile;
  uint16_t zigbeeCluster = 0;
  uint8_t zigbeeCommandId = 0;
  uint8_t payloadLength = 0;
  uint8_t payload[EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN] = { 0 };
  uint8_t payloadSrc = 0;
  uint8_t additionalInfoLength = 0;
  uint8_t *additionalInfoBlockIn = NULL;
  uint8_t *translationsEntryPtr = translations;
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfoBlockOut;
  for (uint8_t i = 0; i < noOfTranslations; i++) {
    index = (uint8_t)emberAfGetInt8u(translationsEntryPtr, payloadOffset, payloadOffset + 1); //1byte
    payloadOffset += 1u;
    gpdCommandId = (uint8_t)emberAfGetInt8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    zbEndpoint = (uint8_t)emberAfGetInt8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    zigbeeProfile = (uint16_t)emberAfGetInt16u(translationsEntryPtr, payloadOffset, (payloadOffset + 2)); //2byte
    payloadOffset += 2u;
    zigbeeCluster = (uint16_t)emberAfGetInt16u(translationsEntryPtr, payloadOffset, (payloadOffset + 2)); //2byte
    payloadOffset += 2u;
    zigbeeCommandId = (uint8_t)emberAfGetInt8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    payloadLength = (uint8_t)emberAfGetInt8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    if (payloadLength > 0) {
      //Do not copy the payload when payloadLength is 0xFF 0r 0xFE
      if ( payloadLength <= EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
        MEMCOPY(payload, &translationsEntryPtr[payloadOffset], payloadLength);
        payloadOffset += payloadLength;
      }
    }
    if (additionalInfoBlockPresent) {
      additionalInfoBlockIn = &translationsEntryPtr[payloadOffset];
      retval = emGpCopyAdditionalInfoBlockArrayToStructure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
      additionalInfoLength = retval;
      payloadOffset += retval;
    }

    index = 0xFF; //In the current version of the specification, the Index field SHALL always be set to 0xff
                  //upon transmis-sion and SHALL always be ignored on reception
    if ( action == 0x00) {
      retval = emGpTransTableAddTranslationTableEntryUpdateCommand(index,
                                                                   additionalInfoBlockPresent,
                                                                   &gpdAddr,
                                                                   gpdCommandId,
                                                                   zbEndpoint,
                                                                   zigbeeProfile,
                                                                   zigbeeCluster,
                                                                   zigbeeCommandId,
                                                                   payloadLength,
                                                                   payload,
                                                                   payloadSrc,
                                                                   additionalInfoLength,
                                                                   &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if ( retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY ) {
          emberAfGreenPowerClusterPrintln("Entry @Index [%d] is not empty", index);
        } else if ( retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          emberAfGreenPowerClusterPrintln("Parameter does not match @Index [%d]", index);
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE); //send failure notification immediately
        }
      }
    } else if (action == 0x01) {
      retval = emGpTransTableReplaceTranslationTableEntryUpdateCommand(index,
                                                                       additionalInfoBlockPresent,
                                                                       &gpdAddr,
                                                                       gpdCommandId,
                                                                       zbEndpoint,
                                                                       zigbeeProfile,
                                                                       zigbeeCluster,
                                                                       zigbeeCommandId,
                                                                       payloadLength,
                                                                       payload,
                                                                       payloadSrc,
                                                                       additionalInfoLength,
                                                                       &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if ( retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY) {
          emberAfGreenPowerClusterPrintln("Entry @Index [%d] is empty", index);
        } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          emberAfGreenPowerClusterPrintln("Parameter does not match @Index [%d]", index);
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        }
      }
    } else if (action == 0x02) {
      retval = removeTranslationTableEntryUpdateCommand(index,
                                                        additionalInfoBlockPresent,
                                                        &gpdAddr,
                                                        gpdCommandId,
                                                        zbEndpoint,
                                                        zigbeeProfile,
                                                        zigbeeCluster,
                                                        zigbeeCommandId,
                                                        payloadLength,
                                                        payload,
                                                        payloadSrc,
                                                        additionalInfoLength,
                                                        &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if (retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY) {
          emberAfGreenPowerClusterPrintln("Entry @Index [%d] is empty", index);
        } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          emberAfGreenPowerClusterPrintln("Parameter does not match @Index [%d]", index);
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        }
      }
    } else {
      goto kickout;
    }
    translationsEntryPtr += payloadOffset; //If successful move the pointer to the next translation
    payloadOffset = 0;
  }
  kickout: return true;
}

bool emberAfGreenPowerClusterGpTranslationTableRequestCallback(uint8_t startIndex)
{
  uint8_t retval;
  EmGpCommandTranslationTable *emGptranslationtable = emGpTransTableGetTranslationTable();
  uint8_t entryIndex = 0;
  uint8_t options = 0;

  emberAfGreenPowerClusterPrintln("Got translation table request with index %x emGptranslationtable->totalNoOfEntries = %d",
                                  startIndex,
                                  emGptranslationtable->totalNoOfEntries);
  // only respond to unicast messages.
  if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST) {
    emberAfGreenPowerClusterPrintln("Not unicast");
    goto kickout;
  }

  // the device SHALL check if it implements a Translation Table.
  if (EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE == 0) {
    emberAfGreenPowerClusterPrintln("Unsup cluster command");
    goto kickout;
  }

  if (emberAfCurrentEndpoint() != GP_ENDPOINT) {
    emberAfGreenPowerClusterPrintln("Drop frame due to unknown endpoint: %X", emberAfCurrentEndpoint());
    return false;
  }
  if ((emGptranslationtable->totalNoOfEntries == 0)
      || (startIndex >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE)) {
    // "index" is already 0xFF if search by ID
    // or already set to the value from "SinkTableRequest" triggered frame in case it is search by INDEX
    emberAfFillCommandGreenPowerClusterGpTranslationTableResponse(EMBER_ZCL_GP_TRANSLATION_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                  0x00, //options
                                                                  emGptranslationtable->totalNoOfEntries, //totalNoOfEntries
                                                                  0x00, //startIndex
                                                                  0x00, //entryCount
                                                                  NULL,
                                                                  0);
    emberAfSendResponse();
    goto kickout;
  } else {
    for (entryIndex = startIndex; entryIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; entryIndex++) {
      if (!emGptranslationtable->TableEntry[entryIndex].entry) {
        continue;
      } else {
        break;
      }
    }
    if (entryIndex >= EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
      emberAfFillCommandGreenPowerClusterGpTranslationTableResponse(EMBER_ZCL_GP_TRANSLATION_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                    0x00, //options
                                                                    emGptranslationtable->totalNoOfEntries, //totalNoOfEntries
                                                                    0x00, //startIndex
                                                                    0x00, //entryCount
                                                                    NULL,
                                                                    0);
      emberAfSendResponse();
      goto kickout;
    }
    uint16_t entriesCount = 0;
    uint8_t tempDatabuffer[EMBER_AF_RESPONSE_BUFFER_LEN];
    uint16_t tempDataLength = 0;
    uint8_t entryCountOffset = (appResponseLength + 7);
    EmberAfGreenPowerServerGpdSubTranslationTableEntry TranslationTableEntry = { 0 };

    for (entryIndex = startIndex; entryIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; entryIndex++) {
      if (!emGptranslationtable->TableEntry[entryIndex].entry) {
        continue;
      }
      emberAfGreenPowerClusterPrintln("Send the response with translation table entries -- entryIndex = %d", entryIndex);
      retval  = emGpTransTableGetTranslationTableEntry(entryIndex, &TranslationTableEntry);
      if (retval == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if (!entriesCount) {
          //Fill the options with first translation entry found after startIndex
          options = (emGptranslationtable->TableEntry[entryIndex].infoBlockPresent << 0x03 | emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId);
        }
        if ( options == ((emGptranslationtable->TableEntry[entryIndex].infoBlockPresent << 0x03)
                         | (emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId))) {
          // Copy to a temp buffer and add if there is space
          tempDataLength = copyTranslationTableEntryToBuffer(entryIndex, &TranslationTableEntry, tempDatabuffer, EMBER_AF_RESPONSE_BUFFER_LEN);
          // If space add to buffer
          if ((sizeof(appResponseData) - (entryCountOffset + 1)) >= (appResponseLength + tempDataLength)) {
            if (!entriesCount) {
              emberAfFillCommandGreenPowerClusterGpTranslationTableResponse(EMBER_ZCL_GP_SINK_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                            options, //options
                                                                            emGptranslationtable->totalNoOfEntries, //totalNoOfEntries
                                                                            startIndex, //startIndex
                                                                            0x00, //entryCount
                                                                            NULL,
                                                                            0);
            }
            MEMMOVE(&appResponseData[appResponseLength], tempDatabuffer, tempDataLength);
            appResponseLength +=  tempDataLength;
            entriesCount++;
          } else {
            break;
          }
        }
      }
    }
    if (entriesCount == 0) {
      emberAfGreenPowerClusterPrintln("INSUFFICIENT SPACE"); //send Insufficient space message if not even one entry could fit in the response
      emberAfFillCommandGreenPowerClusterGpTranslationTableResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE,
                                                                    0x00, //options
                                                                    emGptranslationtable->totalNoOfEntries, //totalNoOfEntries
                                                                    0x00, //startIndex
                                                                    0x00, //entryCount
                                                                    NULL,
                                                                    0);
      emberAfSendResponse();
      goto kickout;
    } else {
      //Insert the number of entries actually included @ entryCountOffset
      appResponseData[entryCountOffset] = entriesCount;
      EmberStatus status = emberAfSendResponse();
      if (status == EMBER_MESSAGE_TOO_LONG) {
        emberAfFillCommandGreenPowerClusterGpTranslationTableResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE,
                                                                      0x00, //options
                                                                      emGptranslationtable->totalNoOfEntries, //totalNoOfEntries
                                                                      0x00, //startIndex
                                                                      0x00, //entryCount
                                                                      NULL,
                                                                      0);
        emberAfSendResponse();
      }
    }
  }           //end of else
  kickout: return true;
}

uint8_t emGpFindMatchingGenericTranslationTableEntry(uint8_t entryType,
                                                     uint8_t incomingReqType,
                                                     uint8_t offset,
                                                     bool infoBlockPresent,
                                                     uint8_t gpdCommandId,
                                                     uint16_t zigbeeProfile,
                                                     uint16_t zigbeeCluster,
                                                     uint8_t zigbeeCommandId,
                                                     uint8_t payloadLength,
                                                     uint8_t *payload,
                                                     uint8_t *outIndex)
{
  uint8_t tableSize = 0xFF;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry const *genericTable = NULL;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry const *genericTableEntry = NULL;

  if (entryType == CUSTOMIZED_TABLE_ENTRY) {
    genericTable = emGpGetCustomizedTable();
    tableSize = EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE;
  } else if (entryType == DEFAULT_TABLE_ENTRY) {
    genericTable = emGpGetDefaultTable();
    tableSize = emGpDefaultTableSize;
  }

  if (genericTable == NULL || tableSize == 0xFF) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  if (offset == 0xFF) {
    //If the offset is 0xFF then search entire Customized Table to get the matching entry
    for (uint8_t index = 0; (index < tableSize); index++) {
      genericTableEntry = &(genericTable[index]);
      if (genericTableEntry->validEntry != true) {
        continue;
      }
      if ((genericTableEntry->gpdCommand == gpdCommandId)
          && ((incomingReqType == ADD_PAIRED_DEVICE)
              || ((incomingReqType == TRANSLATION_TABLE_UPDATE)
                  && (genericTableEntry->zigbeeProfile == zigbeeProfile)
                  && (genericTableEntry->zigbeeCluster == zigbeeCluster)
                  && (genericTableEntry->zigbeeCommandId == zigbeeCommandId)
                  && (genericTableEntry->zclPayloadDefault[0] == payloadLength)))) {
        *outIndex = index;
        break;
      }
    }           // end of for
  } else if (offset < tableSize) {
    genericTableEntry = &(genericTable[offset]);
    //If the offset is not 0xFF then look for matching entry in customized Table[offset]
    if (genericTableEntry->validEntry == true) {
      if ((genericTableEntry->gpdCommand == gpdCommandId)
          && ((incomingReqType == ADD_PAIRED_DEVICE)
              || ((incomingReqType == TRANSLATION_TABLE_UPDATE)
                  && (genericTableEntry->zigbeeProfile == zigbeeProfile)
                  && (genericTableEntry->zigbeeCluster == zigbeeCluster)))) {
        *outIndex = offset;
      }
    }
  }

  if (*outIndex != 0xFF) {
    return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return GP_TRANSLATION_TABLE_STATUS_FAILED;
}

void emberAfPluginTableInitCallback(void)
{
#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE
  // Get user/system defined default table
  emGpDefaultTablePtr  = getSystemDefinedDefaultTable(&emGpDefaultTableSize);
#endif //EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_TRANSLATION_TABLE
#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
  emGpDefaultGenericSwitchTablePtr = getSystemDefinedDefaultGenericSwitchTable(&emGpDefaultGenericSwitchTableSize);
#endif //EMBER_AF_PLUGIN_GREEN_POWER_SERVER_USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
}

//After the pairing session, manage the customized translation table
void emGpPairingDoneThusSetCustomizedTranslationTable(EmberGpAddress *gpdAddr,
                                                      uint8_t gpdCommandId,
                                                      uint8_t endpoint)
{
  // If switch command and recievd within the commissioning time
  if ((gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
       || gpdCommandId == EMBER_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE)
      && emberEventControlGetActive(emberAfPluginGreenPowerServerGenericSwitchCommissioningTimeoutEventControl)) {
    addSwitchToTranslationTable(gpdAddr,
                                gpdCommandId,
                                endpoint);
    return;
  }
  // Multisensor or CAR
  if (gpdCommandId == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    addReportstoTranslationTable(gpdAddr,
                                 gpdCommandId,
                                 endpoint);
    return;
  }
  uint8_t outIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
  uint8_t status = emGpTransTableFindMatchingTranslationTableEntry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                    | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                    | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT),
                                                                   false,
                                                                   gpdAddr,
                                                                   gpdCommandId,
                                                                   endpoint,
                                                                   NULL,
                                                                   NULL,
                                                                   &outIndex,
                                                                   0);
  if (outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX
      && status != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
    uint8_t newTTEntryIndex = 0xFF;
    addPairedDeviceToTranslationTable(ADD_PAIRED_DEVICE,
                                      false,
                                      gpdAddr,
                                      gpdCommandId,
                                      endpoint,
                                      0,
                                      0,
                                      0,
                                      0,
                                      NULL,
                                      0,
                                      0,
                                      NULL,
                                      &newTTEntryIndex);
  }
}

// Get the default table entry
const EmberAfGreenPowerServerGpdSubTranslationTableEntry *emGpGetDefaultTable(void)
{
  return emGpDefaultTablePtr;
}
