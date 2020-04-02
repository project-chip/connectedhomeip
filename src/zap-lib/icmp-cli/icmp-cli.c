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
