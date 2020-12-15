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
 * @brief Default configuration for the Reporting
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#include "reporting.h"
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/common.h>
#include <stdbool.h>
#include <stdint.h>

#define REPORT_FAILED 0xFF

#if (defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE &&                                                            \
     0 != EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE)
const EmberAfPluginReportingEntry generatedReportingConfigDefaults[] = EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS;
#endif

// Load Default reporting configuration from generated table
void emberAfPluginReportingLoadReportingConfigDefaults(void)
{
#if (defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE &&                                                            \
     0 != EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE)
    static_assert(EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE <= EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE,
                  "Not enough slots to hold EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS");
    for (int i = 0; i < EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE; i++)
    {
        emAfPluginReportingConditionallyAddReportingEntry((EmberAfPluginReportingEntry *) &generatedReportingConfigDefaults[i]);
    }
#endif
}

/** @brief Configured
 *
 * This callback is called by the Reporting plugin to get the default reporting
 * configuration values from user if there is no default value available within
 * af generated default reporting configuration tabel. The application need to
 * write to the minInterval, maxInterval and reportable change in the passed
 * IO pointer in the arguement while handleing this callback, then application
 * shall return true if it has provided the default values or else false for
 * reporting plugin to further handleing.
 *
 * @param entry   Ver.: always
 */
bool emberAfPluginReportingGetDefaultReportingConfigCallback(EmberAfPluginReportingEntry * entry)
{
    // Change the values as appropriate for the application.
    entry->data.reported.minInterval      = 1;
    entry->data.reported.maxInterval      = 0xFFFE;
    entry->data.reported.reportableChange = 1;
    entry->direction                      = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
    return true;
}

// Get default reporting values - returns true if there is default value
// avilable either in the table or a call back to application
bool emberAfPluginReportingGetReportingConfigDefaults(EmberAfPluginReportingEntry * defaultConfiguration)
{
    // NULL error check - return false.
    if (NULL == defaultConfiguration)
    {
        return false;
    }
    // When there is a table table available - search and read the values,
    // if default config value found, retrun true to the caller to use it
#if ((defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE) &&                                                          \
     (0 != EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE))
    for (int i = 0; i < EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE; i++)
    {
        // All of the serach parameters match then copy the default config.
        if ((defaultConfiguration->endpoint == generatedReportingConfigDefaults[i].endpoint) &&
            (defaultConfiguration->clusterId == generatedReportingConfigDefaults[i].clusterId) &&
            (defaultConfiguration->attributeId == generatedReportingConfigDefaults[i].attributeId) &&
            (defaultConfiguration->mask == generatedReportingConfigDefaults[i].mask) &&
            (defaultConfiguration->manufacturerCode == generatedReportingConfigDefaults[i].manufacturerCode))
        {
            defaultConfiguration->direction = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
            defaultConfiguration->data      = generatedReportingConfigDefaults[i].data;
            return true;
        }
    }
#endif
    // At this point - There is no entry in the generated deafult table,
    // so the application need to be called to get default reporting values
    // The application should provide the default values because , in BDB an
    // implemented reportable attribute will reset its reporting configuration
    // when receives a special case of minInterval and maxInterval for which
    // function gets called.
    if (emberAfPluginReportingGetDefaultReportingConfigCallback(defaultConfiguration))
    {
        return true;
    }
    return false;
}
