/***************************************************************************//**
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

void ALIAS(emberIncomingIcmpHandler)(Ipv6Header * ipHeader)
{
  emberAfCorePrint("ICMP RX:");

  emberAfCorePrint(" s=");
  emberAfCoreDebugExec(
    emberAfPrintIpv6Address((const EmberIpv6Address *)ipHeader->source)
    );

  emberAfCorePrint(" d=");
  emberAfCoreDebugExec(
    emberAfPrintIpv6Address((const EmberIpv6Address *)ipHeader->destination)
    );

  emberAfCorePrint(" ");
  switch (ipHeader->transportProtocol) {
    case IPV6_NEXT_HEADER_UDP:
      emberAfCorePrint("UDP sp=%u dp=%u",
                       ipHeader->sourcePort,
                       ipHeader->destinationPort);
      break;
    case IPV6_NEXT_HEADER_ICMPV6:
      emberAfCorePrint((ipHeader->icmpType == ICMP_ECHO_REPLY
                        ? "ICMP ECHO_REPLY"
                        : (ipHeader->icmpType == ICMP_ECHO_REQUEST
                           ? "ICMP ECHO_REQUEST"
                           : "ICMP t=%d c=%d")),
                       ipHeader->icmpType,
                       ipHeader->icmpCode);
      break;
  }

  emberAfCorePrint(" [");
  emberAfCorePrintBuffer(ipHeader->transportPayload,
                         ipHeader->transportPayloadLength,
                         false);
  emberAfCorePrintln("]");
}
