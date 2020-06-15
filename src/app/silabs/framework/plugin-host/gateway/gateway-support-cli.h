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
 * @brief Gateway specific behavior for a host application.
 *  In this case we assume our application is running on
 *   a PC with Unix library support, connected to an NCP via serial uart.
 *******************************************************************************
   ******************************************************************************/

extern EmberCommandEntry emberAfPluginGatewayCommands[];

#if defined(GATEWAY_APP)
  #define EMBER_AF_PLUGIN_GATEWAY_COMMANDS               \
  emberCommandEntrySubMenu("gateway",                    \
                           emberAfPluginGatewayCommands, \
                           "Commands for the Linux host application"),

#else
  #define EMBER_AF_PLUGIN_GATEWAY_COMMANDS

#endif
