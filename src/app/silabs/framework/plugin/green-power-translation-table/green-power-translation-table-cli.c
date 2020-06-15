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
 * @brief CLI for the Green Power Translation Table plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include EMBER_AF_API_GREEN_POWER_COMMON
#include EMBER_AF_API_GREEN_POWER_TRANSLATION_TABLE

#define  GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN  36
#ifndef EMBER_AF_GENERATE_CLI
#error The Green Power Translation Table plugin is not compatible with the legacy CLI.
#endif

void emberAfPluginGreenPowerServerCliClearAdditionalInfoBlockTable(void)
{
  embGpClearAdditionalInfoBlockTable();
}

void emberAfPluginGreenPowerServerCliClearCustomizedTable(void)
{
  emGpClearCustomizedTable();
}

void emberAfPluginGreenPowerServerCliClearTranslationTranslationTable(void)
{
  emGpTransTableClearTranslationTable();
}

void emberAfPluginGreenPowerServerCliClearAdditionalInfoBlockTableEntry()
{
  uint8_t index = emberUnsignedCommandArgument(0);
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE
      && additionalInfoTable->validEntry[index]) {
    additionalInfoTable->validEntry[index]--;
    if (additionalInfoTable->validEntry[index] == 0) {
      MEMSET(&(additionalInfoTable->additionalInfoBlock[index]), 0x00, sizeof(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField));
      additionalInfoTable->totlaNoOfEntries--;
    }
    emGpSetAdditionalInfoBlockTableEntry(index);
  }
}

void emberAfPluginGreenPowerServerCliClearCustomizedTableEntry()
{
  uint8_t index = emberUnsignedCommandArgument(0);
  EmberAfGreenPowerServerGpdSubTranslationTableEntry* customizedTable = emGpGetCustomizedTable();
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    MEMSET(&(customizedTable[index]),
           0x00,
           sizeof(EmberAfGreenPowerServerGpdSubTranslationTableEntry));
    emGpSetCustomizedTableEntry(index);
  }
}

void emberAfPluginGreenPowerServerCliClearTranslationTableEntry(void)
{
  uint8_t index = emberUnsignedCommandArgument(0);
  EmGpCommandTranslationTable * translationtable = emGpTransTableGetTranslationTable();
  if ((index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) && (translationtable->totalNoOfEntries > 0)) {
    MEMSET(&translationtable->TableEntry[index], 0x00, sizeof(EmGpCommandTranslationTableEntry));
    translationtable->TableEntry[index].entry = NO_ENTRY;
    translationtable->TableEntry[index].offset = 0xFF;
    translationtable->totalNoOfEntries--;
    emGpSetTranslationTableEntry(index);
  }
}

void emberAfPluginGreenPowerServerCliSetAdditionalInfoBlockTableEntry(void)
{
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  uint8_t index = emberUnsignedCommandArgument(0);
  uint8_t gpdCommandId = emberUnsignedCommandArgument(1);
  uint8_t additionalInfoLength = emberUnsignedCommandArgument(2);
  uint8_t additionalInfoBlockIn[GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN];
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfoBlockOut;

  if (additionalInfoLength < GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN) {
    emberCopyStringArgument(3,
                            additionalInfoBlockIn,
                            additionalInfoLength,
                            false);
  }
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE) {
    emGpCopyAdditionalInfoBlockArrayToStructure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
    MEMCOPY(&additionalInfoTable->additionalInfoBlock[index],
            &additionalInfoBlockOut,
            sizeof(EmberGpTranslationTableAdditionalInfoBlockOptionRecordField));
    additionalInfoTable->validEntry[index]++;
    additionalInfoTable->totlaNoOfEntries++;
    emGpSetAdditionalInfoBlockTableEntry(index);
  }
}

void emberAfPluginGreenPowerServerCliSetCustomizedTableEntry(void)
{
  EmberAfGreenPowerServerGpdSubTranslationTableEntry customizedTableEntryIn;
  uint8_t index = emberUnsignedCommandArgument(0);
  customizedTableEntryIn.validEntry = true;
  customizedTableEntryIn.gpdCommand    = emberUnsignedCommandArgument(1);
  customizedTableEntryIn.endpoint    = emberUnsignedCommandArgument(2);
  customizedTableEntryIn.zigbeeProfile = emberUnsignedCommandArgument(3);
  customizedTableEntryIn.zigbeeCluster = emberUnsignedCommandArgument(4);
  customizedTableEntryIn.serverClient  = emberUnsignedCommandArgument(5);
  customizedTableEntryIn.zigbeeCommandId = emberUnsignedCommandArgument(6);
  customizedTableEntryIn.payloadSrc    = emberUnsignedCommandArgument(7);
  uint8_t payloadLength = emberUnsignedCommandArgument(8);

  if (payloadLength < EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
    emberCopyStringArgument(9,
                            customizedTableEntryIn.zclPayloadDefault,
                            EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN,
                            false);
  }
  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    EmberAfGreenPowerServerGpdSubTranslationTableEntry* customizedTable = emGpGetCustomizedTable();
    MEMCOPY(&(customizedTable[index]),
            &customizedTableEntryIn,
            sizeof(EmberAfGreenPowerServerGpdSubTranslationTableEntry));
    emGpSetCustomizedTableEntry(index);
  }
}

void emberAfPluginGreenPowerServerCliSetTranslationTable(void)
{
  //assign default values wherever possible since CLI has a limitation on number of arguments
  uint8_t retval;
  uint8_t index = emberUnsignedCommandArgument(0);
  uint16_t options = emberUnsignedCommandArgument(1);
  bool infoBlockPresent = ((options >> 0x03) & 0x01);
  uint8_t gpApplicationId = (options & 0x07);
  uint32_t  gpdSrcId = 0;
  EmberEUI64 gpdIeeeAddr;
  uint8_t gpdEndpoint = 1;
  uint8_t zbEndpoint = emberUnsignedCommandArgument(5);
  uint8_t gpdCommandId = emberUnsignedCommandArgument(6);
  uint16_t zigbeeProfile = emberUnsignedCommandArgument(7);
  uint16_t zigbeeCluster = emberUnsignedCommandArgument(8);
  uint8_t  zigbeeCommandId = emberUnsignedCommandArgument(9);
  uint8_t payloadSrc  = EMBER_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED;//emberUnsignedCommandArgument(10);
  uint8_t payloadLength = emberUnsignedCommandArgument(10);
  uint8_t payload[EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN];
  uint8_t additionalInfoLength = emberUnsignedCommandArgument(12);
  uint8_t additionalInfoBlockIn[GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN];
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField additionalInfoBlockOut;

  if (index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE) {
    if (gpApplicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
      emberCopyBigEndianEui64Argument(2, gpdIeeeAddr);
      gpdEndpoint = emberUnsignedCommandArgument(3);
    } else if (gpApplicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
      gpdSrcId = emberUnsignedCommandArgument(4);
    }
    if (payloadLength < EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
      emberCopyStringArgument(11,
                              payload,
                              EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN,
                              false);
    }

    if ( infoBlockPresent) {
      if (additionalInfoLength < GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN) {
        emberCopyStringArgument(13,
                                additionalInfoBlockIn,
                                additionalInfoLength,
                                false);
      }
      emGpCopyAdditionalInfoBlockArrayToStructure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
    }
    EmberGpAddress gpdAddr;
    if (!emGpMakeAddr(&gpdAddr, gpApplicationId, gpdSrcId, gpdIeeeAddr, gpdEndpoint)) {
      emberAfCorePrintln("GPD addr Error");
      return;
    }
    retval = emGpTransTableAddTranslationTableEntryUpdateCommand(index,
                                                                 infoBlockPresent,
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
      if (retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY) {
        emberAfCorePrintln("Entry @Index [%d] is not empty, trying replace", index);
        retval = emGpTransTableReplaceTranslationTableEntryUpdateCommand(index,
                                                                         infoBlockPresent,
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
        emberAfCorePrintln("Replace Ststus  = %d ", retval);
      } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
        emberAfCorePrintln("Parameter does not match @Index [%d]", index);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE); //send failure notification immediately
      }
    }
  } else {
    emberAfCorePrintln("invalid Translation Table index!");
  }
}

void emGpPrintAdditionalInfoBlock(uint8_t gpdCommand, uint8_t addInfoOffset)
{
  EmberGpTranslationTableAdditionalInfoBlockField *additionalInfoTable = emGpGetAdditionalInfoTable();
  EmberGpTranslationTableAdditionalInfoBlockOptionRecordField * addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
  if (gpdCommand == EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    emberAfCorePrintln("%d %d %d %d 0x%2X 0x%2X %d %d 0x%2X",
                       addInfo->totalLengthOfAddInfoBlock,
                       addInfo->optionSelector,
                       addInfo->optionData.compactAttr.reportIdentifier,
                       addInfo->optionData.compactAttr.attrOffsetWithinReport,
                       addInfo->optionData.compactAttr.clusterID,
                       addInfo->optionData.compactAttr.attributeID,
                       addInfo->optionData.compactAttr.attributeDataType,
                       addInfo->optionData.compactAttr.attributeOptions,
                       addInfo->optionData.compactAttr.manufacturerID);
  } else {
    emberAfCorePrintln("%d %d 0x%2X 0x%2X",
                       addInfo->totalLengthOfAddInfoBlock,
                       addInfo->optionSelector,
                       addInfo->optionData.genericSwitch.contactBitmask,
                       addInfo->optionData.genericSwitch.contactStatus);
  }
}

void emberAfPluginGreenPowerServerCliAdditionalInfoBlockTablePrint(void)
{
  uint8_t gpdCommandId = EMBER_ZCL_GP_GPDF_8BITS_VECTOR_PRESS;
  EmberGpTranslationTableAdditionalInfoBlockField *addInfoTable = emGpGetAdditionalInfoTable();
  for (int i = 0; i < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    if (addInfoTable->additionalInfoBlock[i].totalLengthOfAddInfoBlock > 3) {
      gpdCommandId = EMBER_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING;
    }
    emGpPrintAdditionalInfoBlock(gpdCommandId, i);
  }
}
void emberAfPluginGreenPowerServerCliCustomizedTablePrint(void)
{
  EmberAfGreenPowerServerGpdSubTranslationTableEntry  * entry = NULL;
  // print customized table.
  emberAfCorePrintln("Print Sink Proxy Server Customized Translation Table:");
  emberAfCorePrintln("index:\tvalid\topt\tgpEp\tieee\t\t\tsrcId\t\tep\tgpdCommandId\tprofile\tcluster\tcmdId\tpayloadType\tpayload");
  uint8_t index;
  uint8_t len;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry* customizedTable = emGpGetCustomizedTable();
  for (index = 0; index < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; index++) {
    entry = &customizedTable[index];
    emberAfCorePrint("%d  0x%X\t\t  0x%2X\t  0x%2X\t  0x%X\t",
                     entry->endpoint,
                     entry->gpdCommand,
                     entry->zigbeeProfile,
                     entry->zigbeeCluster,
                     entry->zigbeeCommandId);
    emberAfCorePrint("0x%X", entry->payloadSrc);

    len = emberAfStringLength(entry->zclPayloadDefault);
    if (len >= EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
      len = EMBER_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN - 1;
    }
    emberAfPrintBuffer(EMBER_AF_PRINT_CORE, entry->zclPayloadDefault, len + 1, false);

    emberAfCorePrint(" \t");
    emberAfCorePrintln("");
  }
}

void emberAfPluginGreenPowerServerCliTranslationTablePrint(void)
{
  uint8_t status;
  EmberAfGreenPowerServerGpdSubTranslationTableEntry TranslationTableEntry;
  EmGpCommandTranslationTable * emGptranslationtable = emGpTransTableGetTranslationTable();
  if (!(emGptranslationtable->totalNoOfEntries)) {
    emberAfCorePrintln("translation table is Empty");
  } else {
    emberAfCorePrintln("\nIndex Valid AppId Ieee                Ep gpdSrc   gdpCmd zEp zProf zCluster zCmd EType Info ");
    for (uint8_t entryIndex = 0; entryIndex < EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE; entryIndex++ ) {
      if (emGptranslationtable->TableEntry[entryIndex].entry == NO_ENTRY) {
        continue;
      }
      status = emGpTransTableGetTranslationTableEntry(entryIndex, &TranslationTableEntry);
      if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        emberAfCorePrint("%x    ", entryIndex);
        emberAfCorePrint("%x    ", TranslationTableEntry.validEntry);
        emberAfCorePrint("%x    ", emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId);
        if ((emGptranslationtable->TableEntry[entryIndex].gpAddr.applicationId & EMBER_AF_GP_NOTIFICATION_OPTION_APPLICATION_ID) == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
          emberAfPrintBigEndianEui64(emGptranslationtable->TableEntry[entryIndex].gpAddr.id.gpdIeeeAddress);
          emberAfCorePrint(" ");
          emberAfCorePrint("%x ", emGptranslationtable->TableEntry[entryIndex].gpAddr.endpoint);
          emberAfCorePrint("         ");
        } else {
          emberAfCorePrint("                       ");
          emberAfCorePrint("%4x ", emGptranslationtable->TableEntry[entryIndex].gpAddr.id.sourceId);
        }
        emberAfCorePrint("%x     ", TranslationTableEntry.gpdCommand);
        emberAfCorePrint("%x  ", TranslationTableEntry.endpoint);
        emberAfCorePrint("%2x  ", TranslationTableEntry.zigbeeProfile);
        emberAfCorePrint("%2x     ", TranslationTableEntry.zigbeeCluster);
        emberAfCorePrint("%x   ", TranslationTableEntry.zigbeeCommandId);
        emberAfCorePrint("%x    ", emGptranslationtable->TableEntry[entryIndex].entry);
        emberAfCorePrint("%x   ", emGptranslationtable->TableEntry[entryIndex].infoBlockPresent);
        if (emGptranslationtable->TableEntry[entryIndex].infoBlockPresent == true) {
          emGpPrintAdditionalInfoBlock(TranslationTableEntry.gpdCommand, emGptranslationtable->TableEntry[entryIndex].additionalInfoOffset);
        }
        emberAfCorePrint("\n");
      } else {
        emberAfCorePrintln("[%s] emGpTransTableGetTranslationTable FAILED", __FUNCTION__);
      }
    }
    emberAfCorePrint("\n");
  }
}

void emberAfPluginGreenPowerServerCliRemoveGpdEndpoint(void)
{
  //assign default values wherever possible since CLI has a limitation on number of arguments
  uint8_t gpApplicationId = emberUnsignedCommandArgument(0);
  uint32_t  gpdSrcId = 0;
  EmberEUI64 gpdIeeeAddr;
  uint8_t gpdEndpoint = 1;
  uint8_t zbEndpoint = emberUnsignedCommandArgument(4);

  if (gpApplicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
    gpdSrcId = emberUnsignedCommandArgument(1);
  } else if (gpApplicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberCopyBigEndianEui64Argument(2, gpdIeeeAddr);
    gpdEndpoint = emberUnsignedCommandArgument(3);
  }

  EmberGpAddress gpdAddr;
  if (!emGpMakeAddr(&gpdAddr, gpApplicationId, gpdSrcId, gpdIeeeAddr, gpdEndpoint)) {
    emberAfGreenPowerClusterPrintln("GPD addr Error");
    return;
  }
  emberAfGreenPowerClusterPrint("%x    ", gpApplicationId);
  if ((gpApplicationId & EMBER_AF_GP_NOTIFICATION_OPTION_APPLICATION_ID) == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfPrintBigEndianEui64(gpdAddr.id.gpdIeeeAddress);
    emberAfGreenPowerClusterPrint(" ");
    emberAfGreenPowerClusterPrint("%x ", gpdAddr.endpoint);
    emberAfGreenPowerClusterPrint("         ");
  } else {
    emberAfGreenPowerClusterPrint("                       ");
    emberAfGreenPowerClusterPrint("%4x ", gpdAddr.id.sourceId);
  }
  emberAfGreenPowerClusterPrintln("%x  ", zbEndpoint);
  emGpRemoveGpdEndpointFromTranslationTable(&gpdAddr, zbEndpoint);
}
