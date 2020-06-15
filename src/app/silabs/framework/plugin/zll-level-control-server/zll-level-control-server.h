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
 * @brief Definitions for the ZLL Level Control Server plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL 0x01
#define EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL 0xFE

EmberAfStatus emberAfPluginZllLevelControlServerMoveToLevelWithOnOffZllExtensions(const EmberAfClusterCommand *cmd);
bool emberAfPluginZllLevelControlServerIgnoreMoveToLevelMoveStepStop(uint8_t endpoint, uint8_t commandId);
