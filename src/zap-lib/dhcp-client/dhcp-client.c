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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#ifndef ALIAS
  #define ALIAS(x) x
#endif

void ALIAS(emberDhcpServerChangeHandler)(const uint8_t * prefix,
                                         uint8_t prefixLengthInBits,
                                         bool available)
{
  if (available) {
    emberAfCorePrint("Requesting DHCP address in ");
    emberAfCoreDebugExec(emberAfPrintIpv6Prefix((const EmberIpv6Address *)prefix,
                                                prefixLengthInBits));
    emberAfCorePrintln(" prefix");
    emberRequestDhcpAddress(prefix, prefixLengthInBits);
  }
}

void ALIAS(emberRequestDhcpAddressReturn)(EmberStatus status,
                                          const uint8_t * prefix,
                                          uint8_t prefixLengthInBits)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrint("ERR: Requesting DHCP address in ");
    emberAfCoreDebugExec(emberAfPrintIpv6Prefix((const EmberIpv6Address *)prefix,
                                                prefixLengthInBits));
    emberAfCorePrintln(" prefix failed: 0x%x", status);
  }
}
