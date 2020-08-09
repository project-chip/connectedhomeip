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
 * @brief Tokens for the Reporting plugin.
 *******************************************************************************
 ******************************************************************************/

#define CREATOR_REPORT_TABLE (0x8725)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_REPORT_TABLE (NVM3KEY_DOMAIN_ZIGBEE | 0x4000)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif // DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

// Following is for backward compatibility.
// The default reporting will generate a table that is mandatory
// but user may still allocate some table for adding more reporting over
// the air or by cli as part of reporting plugin.
#if defined EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE
#define REPORT_TABLE_SIZE (EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE + EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#else
#define REPORT_TABLE_SIZE (EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE)
#endif

DEFINE_INDEXED_TOKEN(REPORT_TABLE, EmberAfPluginReportingEntry, REPORT_TABLE_SIZE,
                     { EMBER_ZCL_REPORTING_DIRECTION_REPORTED, EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID })

#endif // DEFINETOKENS
