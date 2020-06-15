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
 * @brief CLI commands for sending ZDO messages.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_ZDO_CLI_H
#define SILABS_ZDO_CLI_H

extern EmberCommandEntry zdoCommands[];

#if defined(EMBER_TEST) && !defined(EMBER_AF_ENABLE_TX_ZDO)
  #define EMBER_AF_ENABLE_TX_ZDO
#endif

#ifdef EMBER_AF_ENABLE_TX_ZDO
  #define ZDO_COMMANDS \
  { "zdo", NULL, (const char *)zdoCommands },
#else
  #define ZDO_COMMANDS
#endif

#endif // SILABS_ZDO_CLI_H
