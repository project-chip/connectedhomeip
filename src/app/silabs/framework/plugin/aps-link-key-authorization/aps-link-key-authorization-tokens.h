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
 * @brief APIs for the APS Link Key Authorization feature, which sets APS
 *        encryption requirement when interfacing defined clusters.
 *******************************************************************************
   ******************************************************************************/

#define CREATOR_APS_LINK_KEY_AUTH_ENABLED  (0x8726)
#define NVM3KEY_APS_LINK_KEY_AUTH_ENABLED (NVM3KEY_DOMAIN_ZIGBEE | 0x8726)

#define CREATOR_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT  (0x8727)
#define NVM3KEY_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT (NVM3KEY_DOMAIN_ZIGBEE | 0x8727)

#define CREATOR_APS_LINK_KEY_AUTH_EXEMPT_LIST (0x8728)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_APS_LINK_KEY_AUTH_EXEMPT_LIST (NVM3KEY_DOMAIN_ZIGBEE | 0x4100)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif //DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_BASIC_TOKEN(APS_LINK_KEY_AUTH_ENABLED, uint8_t, 0x00)
DEFINE_BASIC_TOKEN(APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT, uint8_t, 0x00)
DEFINE_INDEXED_TOKEN(APS_LINK_KEY_AUTH_EXEMPT_LIST,
                     uint16_t,
                     EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS,
                     { 0x00 })
#endif //DEFINETOKENS
