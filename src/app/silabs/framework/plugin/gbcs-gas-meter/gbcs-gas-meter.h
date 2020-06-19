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
/***************************************************************************//**
 * @file
 * @brief APIs and defines for the GBCS Gas Meter plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef GBCS_GAS_METER_H_
#define GBCS_GAS_METER_H_

// Printing macros for plugin: GBCS Gas Meter
#define emberAfPluginGbcsGasMeterPrint(...)    emberAfAppPrint(__VA_ARGS__)
#define emberAfPluginGbcsGasMeterPrintln(...)  emberAfAppPrintln(__VA_ARGS__)
#define emberAfPluginGbcsGasMeterDebugExec(x)  emberAfAppDebugExec(x)
#define emberAfPluginGbcsGasMeterPrintBuffer(buffer, len, withSpace) emberAfAppPrintBuffer(buffer, len, withSpace)

/** @brief Reports attributes.
 *
 * If the mirror is ready to receive attribute reports and a report isn't already
 * in progress, this function will kick off the task of reporting
 * all attributes to the mirror.
 */
void emberAfPluginGbcsGasMeterReportAttributes(void);

#endif /* GBCS_GAS_METER_H_ */
