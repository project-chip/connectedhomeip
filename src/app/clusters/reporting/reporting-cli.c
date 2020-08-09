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
 * @brief CLI for the Reporting plugin.
 *******************************************************************************
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/util/serial/command-interpreter2.h"
#include "reporting.h"

void emAfPluginReportingCliPrint(void);
void emAfPluginReportingCliClear(void);
void emAfPluginReportingCliRemove(void); // "remov" to avoid a conflict with "remove" in stdio
void emAfPluginReportingCliAdd(void);
void emAfPluginReportingCliClearLastReportTime(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginReportingCommands[] = {
    emberCommandEntryAction("print", emAfPluginReportingCliPrint, "", "Print the reporting table"),
    emberCommandEntryAction("clear", emAfPluginReportingCliClear, "", "Clear the reporting tabel"),
    emberCommandEntryAction("remove", emAfPluginReportingCliRemove, "u", "Remove an entry from the reporting table"),
    emberCommandEntryAction("add", emAfPluginReportingCliAdd, "uvvuvvw", "Add an entry to the reporting table"),
    emberCommandEntryAction("clear-last-report-time", emAfPluginReportingCliClearLastReportTime, "",
                            "Clear last report time of attributes."),
    emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin reporting print
void emAfPluginReportingCliPrint(void)
{
    uint8_t i;
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        EmberAfPluginReportingEntry entry;
        emAfPluginReportingGetEntry(i, &entry);
        emberAfReportingPrint("%x:", i);
        if (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        {
            emberAfReportingPrint("ep %x clus %2x attr %2x svr %c", entry.endpoint, entry.clusterId, entry.attributeId,
                                  (entry.mask == CLUSTER_MASK_SERVER ? 'y' : 'n'));
            if (entry.manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE)
            {
                emberAfReportingPrint(" mfg %x", entry.manufacturerCode);
            }
            if (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
            {
                emberAfReportingPrint(" report min %2x max %2x rpt-chg %4x", entry.data.reported.minInterval,
                                      entry.data.reported.maxInterval, entry.data.reported.reportableChange);
                emberAfReportingFlush();
            }
            else
            {
                emberAfReportingPrint(" receive from %2x ep %x timeout %2x", entry.data.received.source,
                                      entry.data.received.endpoint, entry.data.received.timeout);
            }
        }
        emberAfReportingPrintln("");
        emberAfReportingFlush();
    }
}

// plugin reporting clear
void emAfPluginReportingCliClear(void)
{
    EmberStatus status = emberAfClearReportTableCallback();
    emberAfReportingPrintln("%p 0x%x", "clear", status);
}

// plugin reporting remove <index:1>
void emAfPluginReportingCliRemove(void)
{
    EmberStatus status = emAfPluginReportingRemoveEntry((uint8_t) emberUnsignedCommandArgument(0));
    emberAfReportingPrintln("%p 0x%x", "remove", status);
}

// plugin reporting add <endpoint:1> <cluster id:2> <attribute id:2> ...
// ... <mask:1> <min interval:2> <max interval:2> <reportable change:4>
void emAfPluginReportingCliAdd(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    EmberAfPluginReportingEntry entry;
    entry.endpoint                  = (uint8_t) emberUnsignedCommandArgument(0);
    entry.clusterId                 = (EmberAfClusterId) emberUnsignedCommandArgument(1);
    entry.attributeId               = (EmberAfAttributeId) emberUnsignedCommandArgument(2);
    entry.mask                      = (uint8_t)(emberUnsignedCommandArgument(3) == 0 ? CLUSTER_MASK_CLIENT : CLUSTER_MASK_SERVER);
    entry.manufacturerCode          = EMBER_AF_NULL_MANUFACTURER_CODE;
    entry.data.reported.minInterval = (uint16_t) emberUnsignedCommandArgument(4);
    entry.data.reported.maxInterval = (uint16_t) emberUnsignedCommandArgument(5);
    entry.data.reported.reportableChange = emberUnsignedCommandArgument(6);
    UNUSED_VAR(status);
    status = emberAfPluginReportingConfigureReportedAttribute(&entry);

    emberAfReportingPrintln("%p 0x%x", "add", status);
}

// plugin reporting add clear-last-report-time
void emAfPluginReportingCliClearLastReportTime(void)
{
    uint8_t i;
    for (i = 0; i < REPORT_TABLE_SIZE; i++)
    {
        emAfPluginReportVolatileData[i].lastReportTimeMs = halCommonGetInt32uMillisecondTick();
    }
    emberAfReportingPrintln("clearing last report time of all attributes");
}
