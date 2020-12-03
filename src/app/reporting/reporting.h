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

#pragma once

#include <app/util/af-types.h>
#include <app/util/types_stub.h>
#include <stdbool.h>
#include <stdint.h>

// The default reporting will generate a table that is mandatory
// but user may still allocate some table for adding more reporting over
// the air or by cli as part of reporting plugin.

#ifndef EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE
#define EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE 5
#endif // EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE

#ifndef REPORT_TABLE_SIZE
#if defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE
#define REPORT_TABLE_SIZE (EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE + EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#else
#define REPORT_TABLE_SIZE (EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#endif
#endif

typedef struct
{
    // If the size of lastReportTimeMs changes (see
    // https://github.com/project-chip/connectedhomeip/issues/3590) adjust the
    // casts on assigning to it.
    uint32_t lastReportTimeMs;
    EmberAfDifferenceType lastReportValue;
    bool reportableChange;
} EmAfPluginReportVolatileData;
extern EmAfPluginReportVolatileData emAfPluginReportVolatileData[];
void emberAfPluginReportingInitCallback(void);
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

/** @brief Configure Reporting Command
 *
 * This function is called by the application framework when a Configure
 * Reporting command is received from an external device.  The Configure
 * Reporting command contains a series of attribute reporting configuration
 * records.  The application should return true if the message was processed or
 * false if it was not.
 *
 * @param cmd   Ver.: always
 */
bool emberAfConfigureReportingCommandCallback(const EmberAfClusterCommand * cmd);

/** @brief Read Reporting Configuration Command
 *
 * This function is called by the application framework when a Read Reporting
 * Configuration command is received from an external device.  The application
 * should return true if the message was processed or false if it was not.
 *
 * @param cmd   Ver.: always
 */
bool emberAfReadReportingConfigurationCommandCallback(const EmberAfClusterCommand * cmd);

/** @brief Clear Report Table
 *
 * This function is called by the framework when the application should clear
 * the report table.
 *
 */
EmberStatus emberAfClearReportTableCallback(void);

/** @brief Configure Reporting Response
 *
 * This function is called by the application framework when a Configure
 * Reporting Response command is received from an external device.  The
 * application should return true if the message was processed or false if it
 * was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of attribute status records.  Ver.:
 * always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfConfigureReportingResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

/** @brief Read Reporting Configuration Response
 *
 * This function is called by the application framework when a Read Reporting
 * Configuration Response command is received from an external device.  The
 * application should return true if the message was processed or false if it
 * was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of attribute reporting configuration
 * records.  Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReadReportingConfigurationResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

/** @brief Reporting Attribute Change
 *
 * This function is called by the framework when an attribute managed by the
 * framework changes.  The application should call this function when an
 * externally-managed attribute changes.  The application should use the change
 * notification to inform its reporting decisions.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param type   Ver.: always
 * @param data   Ver.: always
 */
void emberAfReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             uint8_t mask, uint16_t manufacturerCode, EmberAfAttributeType type, uint8_t * data);
