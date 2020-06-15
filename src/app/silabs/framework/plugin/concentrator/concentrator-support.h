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
 * @brief APIs and defines for the Concentrator plugin.
 *******************************************************************************
   ******************************************************************************/

extern uint8_t emAfRouteErrorCount;
extern uint8_t emAfDeliveryFailureCount;

extern EmberEventControl emberAfPluginConcentratorUpdateEventControl;

#define LOW_RAM_CONCENTRATOR  EMBER_LOW_RAM_CONCENTRATOR
#define HIGH_RAM_CONCENTRATOR EMBER_HIGH_RAM_CONCENTRATOR

#define emAfConcentratorStartDiscovery emberAfPluginConcentratorQueueDiscovery
void emAfConcentratorStopDiscovery(void);

uint32_t emberAfPluginConcentratorQueueDiscovery(void);
void emberAfPluginConcentratorStopDiscovery(void);

// These values are defined by appbuilder.
#define NONE (0x00)
#define FULL (0x01)
enum {
  EMBER_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_NONE = NONE,
  EMBER_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL = FULL,

  EMBER_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_MAX = EMBER_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_FULL,
};
typedef uint8_t EmberAfPluginConcentratorRouterBehavior;

extern EmberAfPluginConcentratorRouterBehavior emAfPluginConcentratorRouterBehavior;
#define emberAfPluginConcentratorGetRouterBehavior() \
  (emAfPluginConcentratorRouterBehavior)
#define emberAfPluginConcentratorSetRouterBehavior(behavior) \
  do { emAfPluginConcentratorRouterBehavior = behavior; } while (0);
