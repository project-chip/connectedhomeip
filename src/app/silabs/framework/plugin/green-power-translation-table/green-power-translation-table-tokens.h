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
 * @brief Token defines for the Green Power Translation Table plugin.
 *******************************************************************************
   ******************************************************************************/

// Green Power Translation Table Tokens
#ifdef EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS

#define CREATOR_TRANSLATION_TABLE     (0x8729)
#define CREATOR_CUSTOMIZED_TABLE      (0x872A)
#define CREATOR_ADDITIONALINFO_TABLE  (0x872B)
#define CREATOR_ADDITIONALINFO_TABLE_VALID_ENTRIES  (0x872C)
#define CREATOR_TRANSLATION_TABLE_TOTAL_ENTRIES     (0x872D)
#define CREATOR_ADDITIONALINFO_TABLE_TOTAL_ENTRIES  (0x872E)

// NVM3 indexed tokens need special consideration for key number assigment
// Ref :
//     Section 3.1.2  Special Considerations for Indexed Tokens
//  of an1135-using-third-generation-nonvolatile-memory.pdf

// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_TRANSLATION_TABLE    (NVM3KEY_DOMAIN_ZIGBEE | 0x8729)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_CUSTOMIZED_TABLE     (NVM3KEY_DOMAIN_ZIGBEE | 0x87A9)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ADDITIONALINFO_TABLE (NVM3KEY_DOMAIN_ZIGBEE | 0x8829)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ADDITIONALINFO_TABLE_VALID_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x88A9)

// Basic Tokens
#define NVM3KEY_TRANSLATION_TABLE_TOTAL_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x8929)
#define NVM3KEY_ADDITIONALINFO_TABLE_TOTAL_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x892A)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#include "green-power-translation-table.h"
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// Define the actual token storage information here

#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE
#define EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE (20)
#endif

#ifndef EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE
#define EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE 30
#endif

#define EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE

DEFINE_INDEXED_TOKEN(TRANSLATION_TABLE,
                     EmGpCommandTranslationTableEntry,
                     EMBER_AF_PLUGIN_GREEN_POWER_SERVER_TRANSLATION_TABLE_SIZE,
                     { 0x00, 0x00, 0x00, 0x00, 0x00, { 0x00000000 }, 0x00, 0x00 })
DEFINE_INDEXED_TOKEN(CUSTOMIZED_TABLE,
                     EmberAfGreenPowerServerGpdSubTranslationTableEntry,
                     EMBER_AF_PLUGIN_GREEN_POWER_SERVER_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE,
                     { 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x00, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } })
DEFINE_INDEXED_TOKEN(ADDITIONALINFO_TABLE,
                     EmberGpTranslationTableAdditionalInfoBlockOptionRecordField,
                     EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE,
                     { 0x00, 0x00, { 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x0000 } })
DEFINE_INDEXED_TOKEN(ADDITIONALINFO_TABLE_VALID_ENTRIES,
                     uint8_t,
                     EMBER_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE,
                     { 0x00 })
DEFINE_BASIC_TOKEN(TRANSLATION_TABLE_TOTAL_ENTRIES, uint8_t, 0x00)
DEFINE_BASIC_TOKEN(ADDITIONALINFO_TABLE_TOTAL_ENTRIES, uint8_t, 0x00)
#endif //DEFINETOKENS
#endif //EMBER_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS
