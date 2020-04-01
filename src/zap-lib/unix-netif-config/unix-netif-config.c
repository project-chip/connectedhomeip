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
#include EMBER_AF_API_DEBUG_PRINT
#include EMBER_AF_API_COMMAND_INTERPRETER2

#include "stack/ip/ip-address.h"
#include "stack/ip/host/host-global-address-table.h"
#include "stack/ip/host/host-listener-table.h"
#include "stack/ip/host/thread-interface.h"

#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

typedef enum {
  EXISTS,
  GET_LL,
  GET_ML,
} AddressAction;

static EmberStatus udpListenCoap(const EmberIpv6Address *address);
static bool setNetif(const char *name, size_t length);
static void setLlAddress(const EmberIpv6Address *address);
static void setMlAddress(const EmberIpv6Address *address);
static bool addGlobalAddress(EmberIpv6Address *address);
static bool addressActionForUnixInterface(EmberIpv6Address *address,
                                          AddressAction action);
static bool processAddressAction(EmberIpv6Address *address,
                                 AddressAction action,
                                 struct sockaddr *ifaddr);

void emberAfPluginUnixNetifConfigInitCallback(void)
{
  setNetif(EMBER_AF_PLUGIN_UNIX_NETIF_CONFIG_DEFAULT_NETIF,
           EMBER_AF_PLUGIN_UNIX_NETIF_CONFIG_DEFAULT_NETIF_LENGTH);
}

// -----------------------------------------------------------------------------
// CLI

// unix-netif get
void unixNetifGetCommand(void)
{
  emberAfAppPrintln("%s %s", "emUnixThreadInterface", emUnixThreadInterface);
}

// unix-netif set <netif>
void unixNetifSetCommand(void)
{
  uint8_t length;
  char *name = (char *)emberStringCommandArgument(0, &length);
  emberAfAppPrintln("%s 0x%x",
                    "set",
                    (setNetif(name, length) ? EMBER_SUCCESS : EMBER_ERR_FATAL));
}

// unix-netif address add <address>
void unixNetifAddressAddCommand(void)
{
  EmberStatus status;
  EmberIpv6Address address;
  if (!emberGetIpv6AddressArgument(0, &address)) {
    emberAfAppPrintln("unix-netif add: Bad argument");
    status = EMBER_BAD_ARGUMENT;
  } else if (addressActionForUnixInterface(&address, EXISTS)) {
    emberAfAppPrintln("unix-netif add: Address exists");
    status = EMBER_ERR_FATAL;
  } else if (emIsDefaultGlobalPrefix(address.bytes)) {
    // If this address is within our network segment, then we try to claim it as
    // our default address (or "mesh" local or "realm" local). We also listen
    // for CoAP traffic on it.
    setMlAddress(&address);
    status = udpListenCoap(&address);
  } else {
    // Else, add it to the global address table and listen for CoAP traffic.
    if (addGlobalAddress(&address)) {
      status = udpListenCoap(&address);
    } else {
      emberAfAppPrintln("unix-netif add: Can't add global address");
      status = EMBER_ERR_FATAL;
    }
  }
  emberAfAppPrintln("%s 0x%x", "add", status);
}

// unix-netif address clear
void unixNetifAddressClearCommand(void)
{
  emberInitializeHostGlobalAddressTable();
  emberInitializeListeners();
}

// unix-netif address print
void unixNetifAddressPrintCommand(void)
{
  EmberIpv6Address address;
  uint8_t ipv6AddrStr[EMBER_IPV6_ADDRESS_STRING_SIZE];

  emberAfAppPrintln("ADDRESSES");

  // Link local.
  extern uint8_t emMacExtendedId[];
  MEMSET(&address, 0x00, sizeof(address));
  emStoreLongFe8Address(emMacExtendedId, address.bytes);
  assert(emberIpv6AddressToString(&address, ipv6AddrStr, sizeof(ipv6AddrStr)));
  emberAfAppPrintln("ll %s", ipv6AddrStr);

  // "Mesh" local.
  MEMSET(&address, 0x00, sizeof(address));
  if (!emStoreLocalMl64(address.bytes)) {
    strcpy((char *)ipv6AddrStr, "???");
  } else {
    assert(emberIpv6AddressToString(&address, ipv6AddrStr, sizeof(ipv6AddrStr)));
  }
  emberAfAppPrintln("ml %s", ipv6AddrStr);

  // Global.
  for (size_t i = 0;; i++) {
    GlobalAddressEntry *entry = emberGetHostGlobalAddress(i);
    if (entry == NULL) {
      break;
    }
    assert(emberIpv6AddressToString((const EmberIpv6Address *)entry->address,
                                    ipv6AddrStr,
                                    sizeof(ipv6AddrStr)));
    emberAfAppPrintln("global %d %s", i, ipv6AddrStr);
  }

  emberAfAppPrintln("LISTENERS");
  const HostListener *listener = emberGetHostListeners();
  for (size_t i = 0; listener->socket != INVALID_SOCKET; i++, listener++) {
    assert(emberIpv6AddressToString((const EmberIpv6Address *)listener->sourceAddress,
                                    ipv6AddrStr,
                                    sizeof(ipv6AddrStr)));
    emberAfAppPrintln("%d p=%u a=%s", i, listener->port, ipv6AddrStr);
  }
}

// -----------------------------------------------------------------------------
// Utilities

static EmberStatus udpListenCoap(const EmberIpv6Address *address)
{
  EmberStatus status = emberUdpListen(EMBER_COAP_PORT, address->bytes);
  if (status == EMBER_SUCCESS) {
    status = emberUdpListen(EMBER_COAP_SECURE_PORT, address->bytes);
  }
  return status;
}

static bool setNetif(const char *name, size_t length)
{
  static char ifname[IF_NAMESIZE] = { 0 };
  emUnixThreadInterface = ifname;
  MEMMOVE(ifname, name, length);

  // Here, we get the link local address for our native interface. The interface
  // should always have this address by default. This also confirms that the
  // interface exists.
  EmberIpv6Address llAddress;
  if (!addressActionForUnixInterface(&llAddress, GET_LL)) {
    emberAfPluginUnixNetifConfigPrintln("Unable to get link local address");
    return false;
  }
  setLlAddress(&llAddress);

  // Now that we know this new interface is legit, clear all the address stuff
  // from the previous interface.
  unixNetifAddressClearCommand();

  // Now, we try to get a default address to use as the "mesh" local address
  // in the thread world or the "realm" local address in the macro-IP world.
  // We need this guy to send stuff to different nodes our network segment.
  EmberIpv6Address mlAddress;
  if (!addressActionForUnixInterface(&mlAddress, GET_ML)) {
    emberAfPluginUnixNetifConfigPrintln("Unable to get mesh local address");
    return false;
  }
  setMlAddress(&mlAddress);

  // Listen to CoAP packets on the link local address as well as the "mesh"
  // local address.
  if (udpListenCoap(&llAddress) != EMBER_SUCCESS
      || udpListenCoap(&mlAddress) != EMBER_SUCCESS
      || emberIcmpListen(llAddress.bytes) != EMBER_SUCCESS
      || emberIcmpListen(mlAddress.bytes) != EMBER_SUCCESS) {
    emberAfPluginUnixNetifConfigPrintln("Unable to set link or mesh local address");
    return false;
  }

  // Listen to a bunch of multicast addresses. These are used a variety
  // of places. For example, the ff33 one is used in currently used in
  // the zcl-core-ez-mode.c code. The first three are nice to have around.
  extern const Bytes16 emFf02AllNodesMulticastAddress; // all ll nodes
  extern const Bytes16 emFf03AllNodesMulticastAddress; // all ml nodes
  extern const Bytes16 emFf05AllNodesMulticastAddress; // all global nodes
  extern       Bytes16 emFf32AllThreadNodesMulticastAddress;
  extern       Bytes16 emFf33AllThreadNodesMulticastAddress;
  const Bytes16 multicastAddresses[] = {
    emFf02AllNodesMulticastAddress,
    emFf03AllNodesMulticastAddress,
    emFf05AllNodesMulticastAddress,
    emFf32AllThreadNodesMulticastAddress,
    emFf33AllThreadNodesMulticastAddress,
  };
  for (size_t i = 0; i < COUNTOF(multicastAddresses); i++) {
    if (emberUdpListen(EMBER_COAP_PORT, multicastAddresses[i].contents)
        != EMBER_SUCCESS) {
      emberAfPluginUnixNetifConfigPrint("Unable to listen on address: ");
      emberAfPluginUnixNetifConfigDebugExec(emberAfPrintIpv6Address(&multicastAddresses[i]));
      emberAfPluginUnixNetifConfigPrintln("");
      return false;
    }
  }

  // ZCLIP must listen on AllCoAPNodes address with scopes 3, 4 and 5
  extern const Bytes16 emFf05AllCoapNodesMulticastAddress; // All CoAP Nodes
  Bytes16 allCoapNodes = emFf05AllCoapNodesMulticastAddress;
  for (int8s i = 0x03; i <= 0x05; i++) {
    allCoapNodes.contents[1] = i;
    if (emberUdpListen(EMBER_COAP_PORT, allCoapNodes.contents) != EMBER_SUCCESS) {
      emberAfPluginUnixNetifConfigPrint("Unable to listen on address: ");
      emberAfPluginUnixNetifConfigDebugExec(emberAfPrintIpv6Address(&multicastAddresses[i]));
      emberAfPluginUnixNetifConfigPrintln("");
      return false;
    }
  }

  // Broadcast group always exists, register listen on it at startup
  extern const EmberIpv6Address broadcastGroupAddress;
  if (emberUdpListen(EMBER_COAP_PORT, broadcastGroupAddress.bytes) != EMBER_SUCCESS) {
    emberAfPluginUnixNetifConfigPrint("Unable to listen on address: ");
    emberAfPluginUnixNetifConfigDebugExec(emberAfPrintIpv6Address(&broadcastGroupAddress));
    emberAfPluginUnixNetifConfigPrintln("");
    return false;
  }

  return true;
}

static void setLlAddress(const EmberIpv6Address *address)
{
  // The lower layers use this variable to determine the source address that
  // we use to send link local fe80 data. We should be sending this kind of
  // data from our default interface's fe80 link local address. We communicate
  // that identifier to the stubbed stack using the emMacExtendedId global.
  extern uint8_t emMacExtendedId[];
  emInterfaceIdToLongId(address->bytes + 8, emMacExtendedId);
}

static void setMlAddress(const EmberIpv6Address *address)
{
  // These two globals combine to make the "mesh" local address in thread
  // speak, or the "default" or "realm" address in the macro-IP world. We
  // will use these two pieces of data to form our source address when we
  // are sending data to other nodes on our network segment.
  extern uint8_t emHostUlaPrefix[];
  MEMMOVE(emHostUlaPrefix, address->bytes, 8);
  emSetDefaultGlobalPrefix(emHostUlaPrefix);
  extern uint8_t emMeshLocalIdentifier[];
  MEMMOVE(emMeshLocalIdentifier, address->bytes + 8, 8);
}

static bool addGlobalAddress(EmberIpv6Address *address)
{
  extern uint8_t emMacExtendedId[];
  EmberIpv6Address fe80Address, defaultAddress;
  emStoreLongFe8Address(emMacExtendedId, fe80Address.bytes);
  emStoreLocalMl64(defaultAddress.bytes);
  return ((MEMCOMPARE(fe80Address.bytes, address->bytes, sizeof(address->bytes))
           == 0)
          || (MEMCOMPARE(defaultAddress.bytes, address->bytes, sizeof(address->bytes))
              == 0)
          || (emberFindHostGlobalAddress(address->bytes, 16)
              != NULL)
          || (emberAddHostGlobalAddress(address,
                                        12345,    // preferredLifetime
                                        12345,    // validLifetime
                                        0)     // localConfigurationFlags
              == EMBER_SUCCESS));
}

static bool addressActionForUnixInterface(EmberIpv6Address *address,
                                          AddressAction action)
{
  struct ifaddrs *interfaceList = NULL;
  if (getifaddrs(&interfaceList) < 0) {
    perror("getifaddrs");
    return false;
  }

  struct ifaddrs *interface = interfaceList;
  for (; interface != NULL; interface = interface->ifa_next) {
    if (strcmp(interface->ifa_name, emUnixThreadInterface) == 0
        && interface->ifa_addr != NULL
        && interface->ifa_addr->sa_family == AF_INET6
        && processAddressAction(address, action, interface->ifa_addr)) {
      break;
    }
  }

  freeifaddrs(interfaceList);
  return (interface != NULL);
}

static bool processAddressAction(EmberIpv6Address *address,
                                 AddressAction action,
                                 struct sockaddr *ifaddr)
{
  struct sockaddr_in6 *ifaddrin6 = (struct sockaddr_in6 *)ifaddr;
  const uint8_t *ifAddressBytes = (const uint8_t *)ifaddrin6->sin6_addr.s6_addr;

  switch (action) {
    case EXISTS:
      return (emIsFf01MulticastAddress(address->bytes)
              || emIsFf02MulticastAddress(address->bytes)
              || emIsFf03MulticastAddress(address->bytes)
              || emIsFf32MulticastAddress(address->bytes)
              || emIsFf33MulticastAddress(address->bytes)
              || !MEMCOMPARE(address->bytes,
                             ifAddressBytes,
                             sizeof(address->bytes)));

    case GET_LL:
    case GET_ML:
      MEMMOVE(address->bytes, ifAddressBytes, sizeof(address->bytes));
      return ((action == GET_LL) == (emIsFe8Address(ifAddressBytes)));

    default:
      assert(0);
  }

  return false;
}
