/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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
