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
/****************************************************************************
 * @file
 * @brief Definitions for the Reporting plugin.
 *******************************************************************************
 ******************************************************************************/

// The default reporting will generate a table that is mandatory
// but user may still allocate some table for adding more reporting over
// the air or by cli as part of reporting plugin.
#ifndef REPORT_TABLE_SIZE
#if defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE
#define REPORT_TABLE_SIZE (EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE + EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#else
#define REPORT_TABLE_SIZE (EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#endif
#endif

typedef struct
{
    uint32_t lastReportTimeMs;
    EmberAfDifferenceType lastReportValue;
    bool reportableChange;
} EmAfPluginReportVolatileData;
extern EmAfPluginReportVolatileData emAfPluginReportVolatileData[];
EmberAfStatus emberAfPluginReportingConfigureReportedAttribute(const EmberAfPluginReportingEntry * newEntry);
void emAfPluginReportingGetEntry(uint8_t index, EmberAfPluginReportingEntry * result);
void emAfPluginReportingSetEntry(uint8_t index, EmberAfPluginReportingEntry * value);
uint8_t emAfPluginReportingAddEntry(EmberAfPluginReportingEntry * newEntry);
EmberStatus emAfPluginReportingRemoveEntry(uint8_t index);
bool emAfPluginReportingDoEntriesMatch(const EmberAfPluginReportingEntry * const entry1,
                                       const EmberAfPluginReportingEntry * const entry2);
uint8_t emAfPluginReportingConditionallyAddReportingEntry(EmberAfPluginReportingEntry * newEntry);
void emberAfPluginReportingLoadReportingConfigDefaults(void);
bool emberAfPluginReportingGetReportingConfigDefaults(EmberAfPluginReportingEntry * defaultConfiguration);
