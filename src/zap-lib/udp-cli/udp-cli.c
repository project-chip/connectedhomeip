/***************************************************************************//**
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

// udp listen <port:2> <address>
void udpListenCommand(void)
{
  uint16_t port = (uint16_t)emberUnsignedCommandArgument(0);

  EmberIpv6Address address;
  if (!emberGetIpv6AddressArgument(1, &address)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  EmberStatus status = emberUdpListen(port, address.bytes);
  emberAfAppPrintln("%p 0x%x", "listen", status);
}

// udp send <destination> <source port:2> <destination port:2> <payload> <hoplimit>
void udpSendCommand(void)
{
  EmberIpv6Address destination;
  if (!emberGetIpv6AddressArgument(0, &destination)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  uint16_t sourcePort = (uint16_t)emberUnsignedCommandArgument(1);
  uint16_t destinationPort = (uint16_t)emberUnsignedCommandArgument(2);

  uint8_t payloadLength;
  uint8_t *payload = emberStringCommandArgument(3, &payloadLength);

  EmberUdpOptions options;
  EmberStatus status;

  if (emberCommandArgumentCount() == 5) {
    options.hopLimit = emberUnsignedCommandArgument(4);
    status = emberSendUdpWithOptions(destination.bytes,
                                     sourcePort,
                                     destinationPort,
                                     payload,
                                     payloadLength,
                                     &options);
  } else {
    status = emberSendUdp(destination.bytes,
                          sourcePort,
                          destinationPort,
                          payload,
                          payloadLength);
  }
  emberAfAppPrintln("%p 0x%x", "send", status);
}
