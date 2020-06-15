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
 * @brief APIs and defines for the LWIP plugin.
 *******************************************************************************
   ******************************************************************************/

void emberAfPluginLwipSetIpv4AddressCommand(void);
void emberAfPluginLwipStatusCommand(void);
void emberAfPluginLwipSetMacAddressCommand(void);
void emberAfPluginLwipIfconfigCommand(void);
void emberAfPluginLwipIfupCommand(void);
void emberAfPluginLwipIfdownCommand(void);
void emberAfPluginLwipPrintCommand(void);
void emberAfPluginLwipTickCallback(void);

bool emberAfPluginLwipIsDhcpEnabled(void);
bool emberAfPluginLwipIsDnsEnabled(void);
