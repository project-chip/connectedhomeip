/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Tokens for the LWIP plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef DEFINETYPES

typedef struct {
  // bit 0 = reserved
  // bit 1 = Use DHCP (1) or Use Static (0)
  uint32_t flags;

  uint32_t hostOrderIpv4Address;
  uint32_t hostOrderIpv4Netmask;
  uint32_t hostOrderIpv4Gateway;
} EmberAfPluginLwipIpv4TokenStruct;

#define EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_MASK   0x0002
#define EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_VALUE  0x0002

#endif // DEFINETYPES

#ifdef DEFINETOKENS

// We set the MSB to indicate it is an Ember Token (not a customer defined token)
#define CREATOR_AFV2_PLUGIN_LWIP_IPV4 0xF333

#define NVM3KEY_AFV2_PLUGIN_LWIP_IPV4 (NVM3KEY_DOMAIN_ZIGBEE | 0xF333)

DEFINE_BASIC_TOKEN(AFV2_PLUGIN_LWIP_IPV4,
                   EmberAfPluginLwipIpv4TokenStruct,
                   { 0xFFFFFFFF })

#endif // DEFINETOKENS
