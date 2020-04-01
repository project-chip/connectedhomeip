/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
  #include <sys/socket.h>
  #include "stack/ip/host/host-listener-table.h"
#endif

extern uint8_t emMacExtendedId[];

const uint8_t * invalidString = "invalid";

static const char * const networkStatusNames[] = {
  "no network",
  "saved network",
  "joining",
  "joined attached",
  "joined no parent",
  "joined attaching"
};

static const uint8_t *networkStatusString(uint8_t networkStatus)
{
  return (networkStatus < COUNTOF(networkStatusNames)
          ? (const uint8_t*)networkStatusNames[networkStatus]
          : invalidString);
}

static const char * const nodeTypeNames[] = {
  "unknown",
  "invalid",
  "router",
  "end device",
  "sleepy end device",
  "minimal end device",
  "invalid",
  "commissioner"
};

static const uint8_t *nodeTypeString(uint8_t nodeType)
{
  return (nodeType < COUNTOF(nodeTypeNames)
          ? (const uint8_t*)nodeTypeNames[nodeType]
          : invalidString);
}

// info
void networkManagementInfoCommand(void)
{
  EmberNetworkStatus status = emberNetworkStatus();
  emberAfAppPrintln("network status: 0x%x - %p", status, networkStatusString(status));

  emberAfAppPrint("eui64: ");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(emberEui64()));
  emberAfAppPrintln("");

  emberAfAppPrint("macExtendedId: ");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64((EmberEui64*)emMacExtendedId));
  emberAfAppPrintln("");

  if (status != EMBER_NO_NETWORK) {
    EmberNetworkParameters parameters = { { 0 } };
    emberGetNetworkParameters(&parameters);

    uint8_t networkId[EMBER_NETWORK_ID_SIZE + 1] = { 0 };
    MEMCOPY(networkId, parameters.networkId, EMBER_NETWORK_ID_SIZE);
    emberAfAppPrintln("network id: %s", networkId);
    emberAfAppPrintln("node type: 0x%x - %p", parameters.nodeType, nodeTypeString(parameters.nodeType));

    emberAfAppPrint("extended pan id: ");
    emberAfAppDebugExec(emberAfPrintExtendedPanId(parameters.extendedPanId));
    emberAfAppPrintln("");

    emberAfAppPrintln("pan id: 0x%2x", parameters.panId);
    emberAfAppPrintln("channel: %u", parameters.channel);
    emberAfAppPrintln("radio tx power: %d dBm", parameters.radioTxPower);

    EmberIpv6Address ulaPrefix = { { 0 } };
    MEMCOPY(&ulaPrefix, &parameters.ulaPrefix, sizeof(EmberIpv6Prefix));
    emberAfAppPrint("ula prefix: ");
    emberAfAppDebugExec(emberAfPrintIpv6Prefix(&ulaPrefix, 64));
    emberAfAppPrintln("");

    for (uint8_t i = 0; i < EMBER_MAX_IPV6_ADDRESS_COUNT; i++) {
      EmberIpv6Address address;
      if (emberGetLocalIpAddress(i, &address)) {
        if (i > 2) {
          emberAfAppPrint("GUA: ");
        } else if (i == 2) {
          emberAfAppPrint("DUA: ");
        } else if (i == 1) {
          emberAfAppPrint("link-local: ");
        } else {
          emberAfAppPrint("mesh-local: ");
        }
        emberAfAppDebugExec(emberAfPrintIpv6Address(&address));
        emberAfAppPrintln("");
      }
    }
  }
}

#include "stack/include/thread-debug.h"

void networkManagementStackInfoCommand(void)
{
#if (defined (EMBER_TEST) || defined (QA_THREAD_TEST))
  emberGetNodeStatus();
#else
  emberAfAppPrintln("Stack debug info not supported.");
#endif
}

void emberGetNodeStatusReturn(EmberStatus status, const EmberNetworkDebugParameters *parameters)
{
  if (status == EMBER_SUCCESS) {
    bool amLeader = (parameters->nodeId == parameters->leaderNodeId);
    emberAfAppPrintln("Network up%s.", (amLeader ? " (Leader)" : ""));
    emberAfAppPrintln("my node id: 0x%2x", parameters->nodeId);
    emberAfAppPrintln("leader node id: 0x%2x", parameters->leaderNodeId);
    emberAfAppPrintln("parent node id: 0x%2x", parameters->parentId);
    emberAfAppPrintln("my rip id: %u", parameters->ripId);
    emberAfAppPrintln("parent rip id: %u", parameters->parentRipId);
    emberAfAppPrintln("my fragment id: %x-%x-%x-%x-%x",
                      parameters->networkFragmentIdentifier[0],
                      parameters->networkFragmentIdentifier[1],
                      parameters->networkFragmentIdentifier[2],
                      parameters->networkFragmentIdentifier[3],
                      parameters->networkFragmentIdentifier[4]);
    emberAfAppPrintln("my frame counter: %d", parameters->networkFrameCounter);
  } else {
    emberAfAppPrintln("Failed with status: 0x%x", status);
  }
}

// network-management form <channel:1> <power:1> <node type:1> [<network id:0--16> [<ula prefix>]]
void networkManagementFormCommand(void)
{
  // Default commissioner passphrase for default PSKc.
  static const uint8_t commissionerKey[] = "2JMRYXP7";

  // Set default PSKc: Required by the harness.
  if (emberNetworkStatus() == EMBER_NO_NETWORK) {
    emberAllowNativeCommissioner(true);
    emberAfAppPrintln("Setting default PSKc using passphrase: \"%s\"",
                      commissionerKey);
    emberSetCommissionerKey(commissionerKey, sizeof(commissionerKey) - 1);
  }

  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t channelMask = (channel == 0
                          ? EMBER_ALL_802_15_4_CHANNELS_MASK
                          : BIT32(channel));

  EmberNetworkParameters parameters = { { 0 } };
  uint16_t options = EMBER_USE_DEFAULTS;

  parameters.radioTxPower = (int8_t)emberSignedCommandArgument(1);
  options |= EMBER_TX_POWER_OPTION;

  parameters.nodeType = (EmberNodeType)emberUnsignedCommandArgument(2);
  options |= EMBER_NODE_TYPE_OPTION;

  if (emberCommandArgumentCount() > 3) {
    uint8_t networkIdLength;
    uint8_t *networkId = emberStringCommandArgument(3, &networkIdLength);
    if (networkIdLength > EMBER_NETWORK_ID_SIZE) {
      emberAfAppPrintln("%p: %p", "ERR", "invalid network id");
      return;
    } else if (networkIdLength != 0) {
      MEMCOPY(parameters.networkId, networkId, networkIdLength);
      options |= EMBER_NETWORK_ID_OPTION;
    }
  }

  if (emberCommandArgumentCount() > 4) {
    EmberIpv6Address dst;
    uint8_t prefixBits;
    if (emberGetIpv6PrefixArgument(4, &dst, &prefixBits) && prefixBits == 64) {
      MEMCOPY(&parameters.ulaPrefix, &dst, sizeof(EmberIpv6Prefix));
    } else {
      emberAfAppPrintln("%p: %p", "ERR", "invalid ula prefix");
      return;
    }
    options |= EMBER_ULA_PREFIX_OPTION;
  }

  emberFormNetwork(&parameters, options, channelMask);
}

// network-management join <channel:1> <power:1> <node type:1> <network id:0--16> <extended pan id:8> <pan id:2> <join key:0--16>
void networkManagementJoinCommand(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t channelMask = (channel == 0
                          ? EMBER_ALL_802_15_4_CHANNELS_MASK
                          : BIT32(channel));

  EmberNetworkParameters parameters = { { 0 } };
  uint16_t options = EMBER_USE_DEFAULTS;

  parameters.radioTxPower = (int8_t)emberSignedCommandArgument(1);
  options |= EMBER_TX_POWER_OPTION;

  parameters.nodeType = (EmberNodeType)emberUnsignedCommandArgument(2);
  options |= EMBER_NODE_TYPE_OPTION;

  uint8_t networkIdLength;
  uint8_t *networkId = emberStringCommandArgument(3, &networkIdLength);
  if (networkIdLength > EMBER_NETWORK_ID_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid network id");
    return;
  } else if (networkIdLength != 0) {
    MEMCOPY(parameters.networkId, networkId, networkIdLength);
    options |= EMBER_NETWORK_ID_OPTION;
  }

  emberGetExtendedPanIdArgument(4, parameters.extendedPanId);
  uint8_t zeros[EXTENDED_PAN_ID_SIZE] = { 0 };
  if (MEMCOMPARE(zeros, parameters.extendedPanId, EXTENDED_PAN_ID_SIZE) != 0) {
    options |= EMBER_EXTENDED_PAN_ID_OPTION;
  }

  parameters.panId = (uint16_t)emberUnsignedCommandArgument(5);
  if (parameters.panId != 0xFFFF) {
    options |= EMBER_PAN_ID_OPTION;
  }

  uint8_t *joinKey = emberStringCommandArgument(6, &parameters.joinKeyLength);
  if (parameters.joinKeyLength > EMBER_JOIN_KEY_MAX_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid join key");
    return;
  } else if (parameters.joinKeyLength != 0) {
    MEMCOPY(parameters.joinKey, joinKey, parameters.joinKeyLength);
    options |= EMBER_JOIN_KEY_OPTION;
  }

  emberJoinNetwork(&parameters, options, channelMask);
}

// network-management commission <preferred channel:1> <fallback channel mask:4> <network id:0--16> <ula prefix> <extended pan id:8> <key:16> [<pan id:2> [<key sequence:4>]]
void networkManagementCommissionCommand(void)
{
  uint8_t preferredChannel = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t fallbackChannelMask = emberUnsignedCommandArgument(1);

  uint8_t networkIdLength;
  uint8_t *networkId = emberStringCommandArgument(2, &networkIdLength);
  if (networkIdLength > EMBER_NETWORK_ID_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid network id");
    return;
  }

  EmberIpv6Address dst;
  uint8_t prefixBits;
  EmberIpv6Prefix ulaPrefix;
  if (emberGetIpv6PrefixArgument(3, &dst, &prefixBits) && prefixBits == 64) {
    MEMCOPY(&ulaPrefix, &dst, sizeof(EmberIpv6Prefix));
  } else {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ula prefix");
    return;
  }

  uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];
  emberGetExtendedPanIdArgument(4, extendedPanId);

  uint8_t contentsLength;
  uint8_t *contents = emberStringCommandArgument(5, &contentsLength);
  if (contentsLength != EMBER_ENCRYPTION_KEY_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid key");
    return;
  }
  EmberKeyData key;
  MEMCOPY(key.contents, contents, EMBER_ENCRYPTION_KEY_SIZE);

  EmberPanId panId = 0xFFFF;
  uint32_t keySequence = 0;
  if (emberCommandArgumentCount() > 6) {
    panId = (EmberPanId)emberUnsignedCommandArgument(6);
    if (emberCommandArgumentCount() > 7) {
      keySequence = emberUnsignedCommandArgument(7);
    }
  }

  emberCommissionNetwork(preferredChannel,
                         fallbackChannelMask,
                         networkId,
                         networkIdLength,
                         panId,
                         ulaPrefix.bytes,
                         extendedPanId,
                         &key,
                         keySequence);
}

// network-management join-commissioned <power:1> <node type:1> [<require connectivity:1>]
void networkManagementJoinCommissionedCommand(void)
{
  int8_t radioTxPower = (int8_t)emberSignedCommandArgument(0);
  EmberNodeType nodeType = (EmberNodeType)emberUnsignedCommandArgument(1);
  bool requireConnectivity = (emberCommandArgumentCount() > 2
                              && (bool)emberUnsignedCommandArgument(2));
  emberJoinCommissioned(radioTxPower, nodeType, requireConnectivity);
}

// network-management set-master-key <master key:16>
void networkManagementSetMasterKeyCommand(void)
{
  uint8_t contentsLength;
  uint8_t *contents = emberStringCommandArgument(0, &contentsLength);
  if (contentsLength != EMBER_ENCRYPTION_KEY_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid master key");
    return;
  }
  EmberKeyData masterKey;
  MEMCOPY(masterKey.contents, contents, EMBER_ENCRYPTION_KEY_SIZE);

  EmberSecurityParameters parameters = { 0 };
  parameters.networkKey = &masterKey;

  emberSetSecurityParameters(&parameters, EMBER_NETWORK_KEY_OPTION);
}

// network-management set-join-key <join key:1--32> [<eui64:8>]
void networkManagementSetJoinKeyCommand(void)
{
  uint8_t joinKeyLength;
  uint8_t *joinKey = emberStringCommandArgument(0, &joinKeyLength);
  if (joinKeyLength > EMBER_JOIN_KEY_MAX_SIZE) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid join key");
    return;
  }

  bool hasEui64 = (emberCommandArgumentCount() > 1);
  EmberEui64 eui64;
  if (hasEui64) {
    emberGetEui64Argument(1, &eui64);
  }

  emberSetJoinKey((hasEui64 ? &eui64 : NULL), joinKey, joinKeyLength);
}

// network-management commissioning start <commissioner id>
void networkManagementCommissioningStartCommand(void)
{
  uint8_t commissionerIdLength;
  uint8_t *commissionerId = emberStringCommandArgument(0,
                                                       &commissionerIdLength);
  emberBecomeCommissioner(commissionerId, commissionerIdLength);
}

// network-management set-joining-mode <mode:1> <length:1>
void networkManagementSetJoiningModeCommand(void)
{
  EmberJoiningMode mode = (EmberJoiningMode)emberUnsignedCommandArgument(0);
  uint8_t length = (uint8_t)emberUnsignedCommandArgument(1);
  emberSetJoiningMode(mode, length);
}

// network-management steering add <eui64:8>
void networkManagementSteeringAddCommand(void)
{
  EmberEui64 eui64;
  emberGetEui64Argument(0, &eui64);
  emberAddSteeringEui64(&eui64);
}

// network-management scan active [<channel:1> [<duration:1>]]
// network-management scan energy [<channel:1> [<duration:1>]]
void networkManagementScanCommand(void)
{
  EmberNetworkScanType scanType = ((emberStringCommandArgument(-1, NULL)[0]
                                    == 'a')
                                   ? EMBER_ACTIVE_SCAN
                                   : EMBER_ENERGY_SCAN);

  uint8_t channel = (emberCommandArgumentCount() > 0
                     ? (uint8_t)emberUnsignedCommandArgument(0)
                     : 0);
  uint32_t channelMask = (channel == 0
                          ? EMBER_ALL_802_15_4_CHANNELS_MASK
                          : BIT32(channel));

  uint8_t duration = (emberCommandArgumentCount() > 1
                      ? (uint8_t)emberUnsignedCommandArgument(1)
                      : DEFAULT_SCAN_DURATION);

  emberStartScan(scanType, channelMask, duration);
}

// network-management gateway <border router flags:2> <is stable:1> <prefix> <domain id:1> <preferred lifetime:4> <valid lifetime:4>
void networkManagementConfigureGatewayCommand(void)
{
  uint16_t borderRouterFlags = (uint16_t)emberUnsignedCommandArgument(0);
  bool isStable = (bool)emberUnsignedCommandArgument(1);
  uint8_t prefixLength;
  EmberIpv6Address prefix;
  uint8_t prefixBits = 0;

  emberStringCommandArgument(2, &prefixLength);
  if (prefixLength != 0
      && !emberGetIpv6PrefixArgument(2, &prefix, &prefixBits)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid prefix");
    return;
  }

  uint8_t domainId = (uint8_t)emberUnsignedCommandArgument(3);

  uint32_t preferredLifetimeS = emberUnsignedCommandArgument(4);
  uint32_t validLifetimeS = emberUnsignedCommandArgument(5);

  emberConfigureGateway(borderRouterFlags,
                        isStable,
                        (prefixLength == 0 ? NULL : prefix.bytes),
                        prefixBits,
                        domainId,
                        preferredLifetimeS,
                        validLifetimeS);
}

// network-management global-addresses [prefix]
void networkManagementGetGlobalAddressesCommand(void)
{
  if (emberCommandArgumentCount() > 0) {
    EmberIpv6Address prefix = { { 0 } };
    uint8_t prefixBits;
    if (!emberGetIpv6PrefixArgument(0, &prefix, &prefixBits)) {
      emberAfAppPrintln("%p: %p", "ERR", "invalid prefix");
      return;
    }
    emberGetGlobalAddresses(prefix.bytes, prefixBits);
  } else {
    emberGetGlobalAddresses(NULL, 0);
  }
}

void networkManagementListenersCommand(void)
{
#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
  const HostListener *listeners = emberGetHostListeners();
  if (listeners != NULL) {
    for (size_t i = 0; i < EMBER_HOST_LISTENER_TABLE_SIZE; i++) {
      if (listeners[i].socket != INVALID_SOCKET) {
        emberAfAppPrint("%s: ",
                        (listeners[i].type == SOCK_DGRAM ? "UDP" : "ICMP"));
        emberAfAppDebugExec(emberAfPrintIpv6Address((const EmberIpv6Address *)listeners[i].sourceAddress));
        emberAfAppPrintln(" port %u", listeners[i].port);
      }
    }
  }
#endif
}

void networkManagementMulticastSubscribeCommand(void)
{
  EmberIpv6Address multicastGroup;
  if (!emberGetIpv6AddressArgument(0, &multicastGroup)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  if (multicastGroup.bytes[0] != 0xFF) {
    emberAfAppPrintln("%p: %p", "ERR", "not a multicast ip");
    return;
  }

  emberRequestMulticastListener(&multicastGroup);
}
