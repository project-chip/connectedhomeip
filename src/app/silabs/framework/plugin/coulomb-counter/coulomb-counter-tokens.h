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
 * @brief Tokens for the Coulomb Counter plugin.
 ******************************************************************************/

#ifdef DEFINETOKENS

// We set the MSB to indicate it is an Ember Token (not a customer defined token)
#define CREATOR_COULOMB_COUNTER_PLUGIN_ENERGY_USAGE 0x8041
#define NVM3KEY_COULOMB_COUNTER_PLUGIN_ENERGY_USAGE (NVM3KEY_DOMAIN_ZIGBEE | 0x8041)

DEFINE_BASIC_TOKEN(COULOMB_COUNTER_PLUGIN_ENERGY_USAGE,
                   uint32_t,
                   0x00000000)

#endif // DEFINETOKENS
