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
 * @brief Definitions for the Smart Energy Registration plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT 3
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT

#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_INITIAL    (MILLISECOND_TICKS_PER_SECOND << 2)
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RETRY      (MILLISECOND_TICKS_PER_SECOND << 5)
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_BRIEF      (MILLISECOND_TICKS_PER_SECOND << 3)
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RESUME     (MILLISECOND_TICKS_PER_SECOND >> 1)
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_TRANSITION (MILLISECOND_TICKS_PER_SECOND >> 2)
#define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_NONE       0

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION

// ESI discovery and binding is only required if the device implements certain
// Smart Energy clusters.  If it doesn't implement these clusters (e.g., it is
// an ESI), the ESI discovery and binding process can be skipped altogether.  If
// discovery is required, it shall be repeated on a period of no more than once
// every three hours and no less than once every 24 hours, according to section
// 5.4.9.2 of 105638r09.
  #if (defined(ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT) \
  || defined(ZCL_USING_MESSAGING_CLUSTER_CLIENT)                      \
  || defined(ZCL_USING_PRICE_CLUSTER_CLIENT))
    #define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

// The plugin option for the discovery period is specified in hours and is
// converted here to milliseconds.  If rediscovery is not enabled, the delay
// is not defined, and the plugin won't schedule an event to rediscover ESIs.
    #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_REDISCOVERY
      #define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD \
  (EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_PERIOD * MILLISECOND_TICKS_PER_HOUR)
extern uint32_t emAfPluginSmartEnergyRegistrationDiscoveryPeriod;
    #endif
  #endif

// ReadAttributesResponse callback is used to determine the time source.
// It is only used if SE registration is enabled and then only if one of these
// two options are enabled.
  #if defined(EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED) \
  || defined(EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS)
    #define EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
void emAfPluginSmartEnergyRegistrationReadAttributesResponseCallback(uint8_t *buffer,
                                                                     uint16_t bufLen);
  #endif

#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION

uint8_t emAfPluginSmartEnergyRegistrationTrustCenterKeyEstablishmentEndpoint(void);
