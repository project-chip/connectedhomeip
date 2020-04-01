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
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

// icmp listen <address>
void icmpListenCommand(void)
{
  EmberIpv6Address address;
  if (!emberGetIpv6AddressArgument(0, &address)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  EmberStatus status = emberIcmpListen(address.bytes);
  emberAfAppPrintln("%p 0x%x", "listen", status);
}

// icmp ping <destination> [<id:2> <sequence:2> <length:2> <hop limit:2>]
void icmpPingCommand(void)
{
  EmberIpv6Address destination;
  if (!emberGetIpv6AddressArgument(0, &destination)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  uint16_t id = 0xABCD;
  uint16_t sequence = 0x1234;
  uint16_t length = 0;
  uint16_t hopLimit = 0;
  if (emberCommandArgumentCount() > 1) {
    if (emberCommandArgumentCount() != 5) {
      emberAfAppPrintln("%p: %p", "ERR", "expected <id:2> <sequence:2> <length:2> <hop limit:2>");
      return;
    }
    id = (uint16_t)emberUnsignedCommandArgument(1);
    sequence = (uint16_t)emberUnsignedCommandArgument(2);
    length = (uint16_t)emberUnsignedCommandArgument(3);
    hopLimit = (uint16_t)emberUnsignedCommandArgument(4);
  }

  bool success = emberIpPing(destination.bytes, id, sequence, length, hopLimit);
  emberAfAppPrintln("%p 0x%x", "ping",
                    (success ? EMBER_SUCCESS : EMBER_ERR_FATAL));
}
