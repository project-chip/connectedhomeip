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
 * @brief CLI commands for sending various messages.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_ZCL_CLI_H
#define SILABS_ZCL_CLI_H

#if !defined(EMBER_AF_GENERATE_CLI)
void emAfCliSendCommand(void);
void emAfCliSendUsingMulticastBindingCommand(void);
void emAfCliBsendCommand(void);
void emAfCliReadCommand(void);
void emAfCliWriteCommand(void);
void emAfCliTimesyncCommand(void);
void emAfCliRawCommand(void);
void emAfCliAddReportEntryCommand(void);
#endif

void zclSimpleCommand(uint8_t frameControl,
                      uint16_t clusterId,
                      uint8_t commandId);

extern EmberCommandEntry keysCommands[];
extern EmberCommandEntry interpanCommands[];
extern EmberCommandEntry printCommands[];
extern EmberCommandEntry zclCommands[];
extern EmberCommandEntry certificationCommands[];

#define zclSimpleClientCommand(clusterId, commandId)                                  \
  zclSimpleCommand(ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER, \
                   (clusterId),                                                       \
                   (commandId))

#define zclSimpleServerCommand(clusterId, commandId)                                  \
  zclSimpleCommand(ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT, \
                   (clusterId),                                                       \
                   (commandId))

#endif // SILABS_ZCL_CLI_H
