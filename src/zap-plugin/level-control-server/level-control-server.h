/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_LEVEL_CONTROL_SERVER_H
#define ZCL_LEVEL_CONTROL_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Level Control plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasCurrentLevelValue;
  uint8_t currentLevelValue;
} EmZclLevelControlSceneSubTableEntry_t;

void emberZclLevelControlServerSetOnOff(EmberZclEndpointId_t endpointId, bool value);

// TODO: This should be generated from types.xml the way it's already done for Zigbee
typedef enum {
  EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF             = 0x01,
  EMBER_ZCL_LEVEL_CONTROL_OPTIONS_COUPLE_COLOR_TEMP_TO_LEVEL = 0x02,
} EmberAfLevelControlOptions;

#endif // ZCL_LEVEL_CONTROL_SERVER_H
