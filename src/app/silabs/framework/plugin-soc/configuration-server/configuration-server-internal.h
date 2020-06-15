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
 * @brief Internal definitions for the Configuration Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_CONFIGURATION_SERVER_INTERNAL_H
#define SILABS_CONFIGURATION_SERVER_INTERNAL_H

/** @brief Reads data from the tokens using the creator.
 *
 * Local method for reading the configurable tokens from their respective
 * creators. Note:  as this is used by the cluster command, data needs to be
 * in the format required for the local command.
 *
 */
void emAfPluginConfigurationServerReadTokenDataFromCreator(uint16_t creator, uint8_t *data);

#endif
