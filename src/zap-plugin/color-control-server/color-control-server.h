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

#ifndef ZCL_COLOR_CONTROL_SERVER_H
#define ZCL_COLOR_CONTROL_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Color Control plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasCurrentXValue;
  uint16_t currentXValue;
  bool hasCurrentYValue;
  uint16_t currentYValue;
  bool hasEnhancedCurrentHueValue;
  uint16_t enhancedCurrentHueValue;
  bool hasCurrentSaturationValue;
  uint8_t currentSaturationValue;
  bool hasColorLoopActiveValue;
  uint8_t colorLoopActiveValue;
  bool hasColorLoopDirectionValue;
  uint8_t colorLoopDirectionValue;
  bool hasColorLoopTimeValue;
  uint16_t colorLoopTimeValue;
} EmZclColorControlSceneSubTableEntry_t;

void emberZclColorControlServerSetColorControl(EmberZclEndpointId_t endpointId, bool value);

#endif // ZCL_COLOR_CONTROL_SERVER
