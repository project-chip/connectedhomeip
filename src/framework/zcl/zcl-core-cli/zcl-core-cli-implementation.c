/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_COMMAND_INTERPRETER2
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include "stack/framework/buffer-management.h"
#include "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#include "app/thread/plugin/zcl/zcl-core/zcl-core-well-known.h"

static ChipZclClusterSpec_t clusterSpec;
static ChipZclAttributeId_t attributeReadIds[32]; // TODO: find a good length for me!
static uint8_t attributeReadIdsCount = 0;
static uint8_t attributeWriteDataBuffer[CHIP_ZCL_ATTRIBUTE_MAX_SIZE];
static ChipZclAttributeWriteData_t attributeWriteData = {
  CHIP_ZCL_ATTRIBUTE_NULL,
  attributeWriteDataBuffer,
  0,
};

static ChipZclBindingEntry_t bindingEntry = {
  .destination.network.port = 0, // indicates not in use
};
static ChipZclBindingId_t bindingId = CHIP_ZCL_BINDING_NULL;

static void getClusterSpecArguments(uint8_t index,
                                    ChipZclClusterSpec_t *clusterSpec);

static void attributeWriteResponseHandler(ChipZclMessageStatus_t status,
                                          const ChipZclAttributeContext_t *context);
static void attributeReadResponseHandler(ChipZclMessageStatus_t status,
                                         const ChipZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength);
static void bindingResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclBindingContext_t *context,
                                   const ChipZclBindingEntry_t *entry);
static void discoveryResponseHandler(ChipCoapStatus status,
                                     ChipCoapCode code,
                                     ChipCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     ChipCoapResponseInfo *info);

// These functions lives in app/coap/coap.c.
const uint8_t *emGetCoapCodeName(ChipCoapCode type);
const uint8_t *emGetCoapContentFormatTypeName(ChipCoapContentFormatType type);
const uint8_t *emGetCoapStatusName(ChipCoapStatus status);

// ----------------------------------------------------------------------------
// Linkage with ZCL Core plugin
//
// Most of the ZCL Core plugin's CLI functionality has been moved to a separate plugin,
// ZCL Core CLI. This allows it to be excluded from builds for flash-space-challenged
// parts that have been suffering build failures. Excluding ZCL Core CLI plugin saves
// about 6500 bytes.
//
// ZCL Core plugin still retains implementation of the "zcl send" CLI command, which is
// needed not only by ZCL Core CLI commands, but also by CLI commands generated for the
// various ZCL cluster plugins (base, identify, etc.).
//
// Because CLI support is distributed between the two ZCL Core plugins, two direct calls
// from ZCL Core to ZCL Core CLI are implemented, one to clear internal ZCL CLI state,
// the other to coordinate execution of the "zcl send" command. These are conditionally
// wrapped by the CHIP_AF_PLUGIN_ZCL_CORE_CLI macro, and suitable stubs are substituted
// if the ZCL Core CLI plugin is not present. The two functions are:
//
// void chZclCoreCliPluginResetCliState(void);
// ChipStatus chZclCoreCliPluginSend(ChipZclDestination_t *destination);
//

// ----------------------------------------------------------------------------
// Commands

// zcl info
void chZclCliInfoCommand(void)
{
  uint8_t result[CHIP_ZCL_UID_STRING_SIZE];
  chZclUidToString(&chZclUid, CHIP_ZCL_UID_BITS, result);
  chipAfAppPrintln("uid: %s", result);
  chZclUidToBase64Url(&chZclUid, CHIP_ZCL_UID_BITS, result);
  chipAfAppPrintln("base64url: %s", result);
}

// zcl access-control mode
void chZclCliAccessControlModeCommand(void)
{
  chipAfAppPrint("access control: ");
  // We only allow access-control to be disabled in test mode
  // (in real apps access-control should always be enabled).
  extern chZclAccessControlMode_t chZclUseAccessControl;
  chZclUseAccessControl = chipUnsignedCommandArgument(0);
  chipAfAppPrintln((chZclUseAccessControl != 0)
                    ? "enabled"
                    : "disabled");
}

// zcl access-control print
void chZclCliAccessControlPrintCommand(void)
{
  #if !defined(CHIP_AF_PRINT_PLUGIN_ZCL_CORE)
  chipAfAppPrintln("[Debug printing for ZCL Core plugin should be enabled]");
  #endif
  chZclPrintAccessTokens();
}

// zcl attribute print
void chZclCliAttributePrintCommand(void)
{
  chipAfAppPrintln("");
  chipAfAppPrintln(" EE | R | MMMM | CCCC | AAAA | KKKK | Data ");
  chipAfAppPrintln("----+---+------+------+------+------+------");
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    for (size_t j = 0; j < CH_ZCL_ATTRIBUTE_COUNT; j++) {
      uint8_t buffer[CHIP_ZCL_ATTRIBUTE_MAX_SIZE];
      if (chZclReadAttributeEntry(chZclEndpointTable[i].endpointId,
                                  &chZclAttributeTable[j],
                                  buffer,
                                  sizeof(buffer))
          == CHIP_ZCL_STATUS_SUCCESS) {
        halResetWatchdog();
        chipAfAppPrint(" %x | %c | %2x | %2x | %2x | %2x | ",
                        chZclEndpointTable[i].endpointId,
                        ((chZclAttributeTable[j].clusterSpec->role
                          == CHIP_ZCL_ROLE_CLIENT)
                         ? 'c'
                         : 's'),
                        chZclAttributeTable[j].clusterSpec->manufacturerCode,
                        chZclAttributeTable[j].clusterSpec->id,
                        chZclAttributeTable[j].attributeId,
                        chZclAttributeTable[j].mask);
        chipAfAppPrintBuffer(buffer, chZclAttributeTable[j].size, false);
        chipAfAppPrintln("");
      }
    }
  }
}

// zcl attribute reset <endpoint id:1>
void chZclCliAttributeResetCommand(void)
{
  ChipZclEndpointId_t endpointId
    = (ChipZclEndpointId_t)chipUnsignedCommandArgument(0);
  chipZclResetAttributes(endpointId);
}

// zcl attribute read <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2>
void chZclCliAttributeReadCommand(void)
{
  ChipZclEndpointId_t endpointId
    = (ChipZclEndpointId_t)chipUnsignedCommandArgument(0);
  ChipZclClusterSpec_t clusterSpec;
  getClusterSpecArguments(1, &clusterSpec);
  ChipZclAttributeId_t attributeId
    = (ChipZclAttributeId_t)chipUnsignedCommandArgument(4);

  const ChZclAttributeEntry_t *attribute
    = chZclFindAttribute(&clusterSpec, attributeId, false); // exclude remote
  uint8_t buffer[CHIP_ZCL_ATTRIBUTE_MAX_SIZE];
  ChipZclStatus_t status = chZclReadAttributeEntry(endpointId,
                                                    attribute,
                                                    buffer,
                                                    sizeof(buffer));
  chipAfAppPrint("%s 0x%x", "read", status);
  if (status == CHIP_ZCL_STATUS_SUCCESS) {
    chipAfAppPrint(": ");
    // SUCCESS implies that 'attribute' is non-NULL.
    size_t size = chZclAttributeSize(attribute, buffer);
    chipAfAppPrintBuffer(buffer, size, false);
  }
  chipAfAppPrintln("");
}

// zcl attribute write <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> <data:n>
void chZclCliAttributeWriteCommand(void)
{
  ChipZclEndpointId_t endpointId
    = (ChipZclEndpointId_t)chipUnsignedCommandArgument(0);
  ChipZclClusterSpec_t clusterSpec;
  getClusterSpecArguments(1, &clusterSpec);
  ChipZclAttributeId_t attributeId
    = (ChipZclAttributeId_t)chipUnsignedCommandArgument(4);
  uint8_t bufferLength;
  uint8_t *buffer = chipStringCommandArgument(5, &bufferLength);

  ChipZclStatus_t status = chipZclWriteAttribute(endpointId,
                                                   &clusterSpec,
                                                   attributeId,
                                                   buffer,
                                                   bufferLength);
  chipAfAppPrintln("%s 0x%x", "write", status);
}

// zcl attribute remote read <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> [<attribute id:2> ...]
void chZclCliAttributeRemoteReadCommand(void)
{
  chZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);

  uint8_t commandArgumentCount = chipCommandArgumentCount();
  for (uint8_t i = 3; i < commandArgumentCount; i++) {
    attributeReadIds[attributeReadIdsCount++]
      = (ChipZclAttributeId_t)chipUnsignedCommandArgument(i);
  }

  chipAfAppPrintln("%s 0x%x", "read", CHIP_ZCL_STATUS_SUCCESS);
}

// zcl attribute remote write <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> <data:n>
void chZclCliAttributeRemoteWriteCommand(void)
{
  chZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);
  attributeWriteData.attributeId
    = (ChipZclAttributeId_t)chipUnsignedCommandArgument(3);
  uint8_t bufferLength;
  uint8_t *buffer = chipStringCommandArgument(4, &bufferLength);
  if (bufferLength <= sizeof(attributeWriteDataBuffer)) {
    MEMCOPY(attributeWriteDataBuffer, buffer, bufferLength);
    attributeWriteData.bufferLength = bufferLength;
    chipAfAppPrintln("%s 0x%x", "write", CHIP_ZCL_STATUS_SUCCESS);
  } else {
    chipAfAppPrintln("%s 0x%x", "write", CHIP_ZCL_STATUS_INSUFFICIENT_SPACE);
  }
}

// zcl binding clear
void chZclCliBindingClearCommand(void)
{
  chipZclRemoveAllBindings();
}

// zcl binding print
void chZclCliBindingPrintCommand(void)
{
  chipAfAppPrintln(" II | EE | R | MMMM | CCCC | RR |                       R                        ");
  chipAfAppPrintln("----+----+---+------+------+----+------------------------------------------------");
  for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
    ChipZclBindingEntry_t entry;
    if (chipZclGetBinding(i, &entry)) {
      chipAfAppPrint(" %x | %x | %c | %2x | %2x | %x | ",
                      i,
                      entry.endpointId,
                      (entry.clusterSpec.role == CHIP_ZCL_ROLE_CLIENT
                       ? 'c'
                       : 's'),
                      entry.clusterSpec.manufacturerCode,
                      entry.clusterSpec.id,
                      entry.reportingConfigurationId);
      uint8_t uri[CHIP_ZCL_URI_MAX_LENGTH];
      ChipZclDestination_t destination;
      chZclReadDestinationFromBinding(&entry, &destination);
      chZclDestinationToUri(&destination, uri);
      chipAfAppPrintln("%s", uri);
    }
  }
}

// zcl binding add <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1>
// zcl binding set <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1> <binding id:1>
// zcl binding remote add          <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1>
// zcl binding remote update       <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1> <binding id:1>
//
// This doesn't actually do anything if isRemote is true.
static void bindingAddUpdateCommand(bool isRemote)
{
  chZclCliResetCliState();
  bool isAdd = chipStringCommandArgument(-1, NULL)[0] == 'a';
  uint8_t i = isRemote ? 0 : 1; // base index into arguments

  getClusterSpecArguments(i, &bindingEntry.clusterSpec);
  if (!isRemote) {
    bindingEntry.endpointId = (ChipZclEndpointId_t)chipUnsignedCommandArgument(0);
  }
  bindingEntry.destination.network.scheme
    = ((bool)chipUnsignedCommandArgument(i + 3)
       ? CHIP_ZCL_SCHEME_COAPS
       : CHIP_ZCL_SCHEME_COAP);

  if (!chipGetIpv6AddressArgument(i + 4, &bindingEntry.destination.network.data.address)) {
    chipAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }
  bindingEntry.destination.network.type = CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS;
  bindingEntry.destination.network.port = (uint16_t)chipUnsignedCommandArgument(i + 5);

  ChipZclEndpointId_t tempEndpointId = (ChipZclEndpointId_t)chipUnsignedCommandArgument(i + 6);
  ChipZclGroupId_t tempGroupId = (ChipZclGroupId_t)chipUnsignedCommandArgument(i + 7);
  if (tempGroupId != CHIP_ZCL_GROUP_NULL) {
    bindingEntry.destination.application.data.groupId = tempGroupId;
    bindingEntry.destination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    bindingEntry.destination.application.data.endpointId = tempEndpointId;
    bindingEntry.destination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  bindingEntry.reportingConfigurationId = (ChipZclReportingConfigurationId_t)chipUnsignedCommandArgument(i + 8);

  bool success = true;
  const char *name = "add";

  if (isRemote) {
    if (isAdd) {
      bindingId = CHIP_ZCL_BINDING_NULL;
    } else {
      name = "update";
      bindingId = (ChipZclBindingId_t)chipUnsignedCommandArgument(9);
    }
  } else if (isAdd) {
    bindingId = chipZclAddBinding(&bindingEntry);
    success = bindingId != CHIP_ZCL_BINDING_NULL;
  } else {
    bindingId = (ChipZclBindingId_t)chipUnsignedCommandArgument(10);
    success = chipZclSetBinding(bindingId, &bindingEntry);
    name = "set";
  }
  chipAfAppPrintln("%s 0x%x",
                    name,
                    (success
                     ? CHIP_ZCL_STATUS_SUCCESS
                     : CHIP_ZCL_STATUS_FAILURE));
}

void chZclCliBindingAddSetCommand(void)
{
  bindingAddUpdateCommand(false);
}

// zcl binding remove <binding id:1>
void chZclCliBindingRemoveCommand(void)
{
  chZclCliResetCliState();

  bindingId = (ChipZclBindingId_t)chipUnsignedCommandArgument(0);
  bool success = chipZclRemoveBinding(bindingId);
  chipAfAppPrintln("%s 0x%x",
                    "remove",
                    (success
                     ? CHIP_ZCL_STATUS_SUCCESS
                     : CHIP_ZCL_STATUS_FAILURE));
}

// zcl binding remote add    <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1>
// zcl binding remote update <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1> <binding id:1>
void chZclCliBindingRemoteAddUpdateCommand(void)
{
  bindingAddUpdateCommand(true);
}

// zcl binding remote remove <role:1> <manufacturer code:2> <cluster id:2> <binding id:1>
void chZclCliBindingRemoteRemoveCommand(void)
{
  chZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);
  bindingId = (ChipZclBindingId_t)chipUnsignedCommandArgument(3);
  chipAfAppPrintln("%s 0x%x", "remove", CHIP_ZCL_STATUS_SUCCESS);
}

ChipStatus chZclCoreCliPluginSend(ChipZclDestination_t *destination)
{
  ChipStatus status;

  if (attributeReadIdsCount != 0) {
    status = chipZclSendAttributeRead(destination,
                                       &clusterSpec,
                                       attributeReadIds,
                                       attributeReadIdsCount,
                                       attributeReadResponseHandler);
  } else if (attributeWriteData.attributeId != CHIP_ZCL_ATTRIBUTE_NULL) {
    status = chipZclSendAttributeWrite(destination,
                                        &clusterSpec,
                                        &attributeWriteData,
                                        1, // attribute write data count
                                        attributeWriteResponseHandler);
  } else if (bindingEntry.destination.network.port != 0
             && bindingId == CHIP_ZCL_BINDING_NULL) {
    status = chipZclSendAddBinding(destination,
                                    &bindingEntry,
                                    bindingResponseHandler);
  } else if (bindingEntry.destination.network.port != 0
             && bindingId != CHIP_ZCL_BINDING_NULL) {
    status = chipZclSendUpdateBinding(destination,
                                       &bindingEntry,
                                       bindingId,
                                       bindingResponseHandler);
  } else if (bindingEntry.destination.network.port == 0
             && bindingId != CHIP_ZCL_BINDING_NULL) {
    status = chipZclSendRemoveBinding(destination,
                                       &clusterSpec,
                                       bindingId,
                                       bindingResponseHandler);
  } else {
    status = CHIP_INVALID_CALL;
  }

  return status;
}

// zcl ez-mode start <role:1> <manufacturer code:2> <cluster id:2>
void chZclCliEzModeStartCommand(void)
{
  ChipStatus status = chipZclStartEzMode();
  chipAfAppPrintln("%s 0x%x", "start", status);
}

// zcl discovery cluster <role:1> <manufacturer code:2> <cluster id:2>
void chZclCliDiscByClusterIdCommand(void)
{
  ChipZclClusterSpec_t spec;
  getClusterSpecArguments(0, &spec);
  chipZclDiscByClusterId(&spec, discoveryResponseHandler);
}

// zcl discovery endpoint <endpoint:1> <deviceId:2> / query to ze=urn:zcl:d.$DEVICE_ID.$ENDPOINT
void chZclCliDiscByEndpointCommand(void)
{
  ChipZclEndpointId_t endpoint = (ChipZclEndpointId_t) chipUnsignedCommandArgument(0);
  ChipZclDeviceId_t deviceId = (ChipZclDeviceId_t) chipUnsignedCommandArgument(1);
  chipZclDiscByEndpoint(endpoint, deviceId, discoveryResponseHandler);
}

// zcl discovery device-type <device type:2>
void chZclCliDiscByDeviceTypeCommand(void)
{
  ChipZclDeviceId_t deviceId = (ChipZclDeviceId_t) chipUnsignedCommandArgument(0);
  chipZclDiscByDeviceId(deviceId, discoveryResponseHandler);
}

static void discoveryResponseHandler(ChipCoapStatus status,
                                     ChipCoapCode code,
                                     ChipCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     ChipCoapResponseInfo *info)
{
  chipAfAppPrint("Discovery CLI:");
  chipAfAppPrint(" %s", emGetCoapStatusName(status));

  if (status == CHIP_COAP_MESSAGE_RESPONSE) {
    chipAfAppPrint(" %s", emGetCoapCodeName(code));
    if (payloadLength != 0) {
      uint32_t valueLoc;
      ChipCoapContentFormatType contentFormat
        = (chipReadIntegerOption(options,
                                  CHIP_COAP_OPTION_CONTENT_FORMAT,
                                  &valueLoc)
           ? (ChipCoapContentFormatType)valueLoc
           : CHIP_COAP_CONTENT_FORMAT_NONE);
      chipAfAppPrint(" f=%s p=",
                      emGetCoapContentFormatTypeName(contentFormat));
      if (contentFormat == CHIP_COAP_CONTENT_FORMAT_TEXT_PLAIN
          || contentFormat == CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT) {
        chipAfAppDebugExec(chipAfPrintCharacters(CHIP_AF_PRINT_APP,
                                                   payload,
                                                   payloadLength));
      } else {
        chipAfAppPrintBuffer(payload, payloadLength, false);
      }
    }
  }

  chipAfAppPrintln("");
}

// zcl discovery uid <uid>
void chZclCliDiscByUidCommand(void)
{
  uint8_t *string = chipStringCommandArgument(0, NULL);
  chZclDiscByUidString(string, discoveryResponseHandler);
}

// zcl discovery resource-version <version:2>
void chZclCliDiscByResourceVersionCommand(void)
{
  ChipZclClusterRevision_t version = (ChipZclClusterRevision_t) chipUnsignedCommandArgument(0);
  chipZclDiscByResourceVersion(version, discoveryResponseHandler);
}

// zcl discovery cluster-version <version:2>
void chZclCliDiscByClusterRevisionCommand(void)
{
  ChipZclClusterRevision_t version = (ChipZclClusterRevision_t) chipUnsignedCommandArgument(0);
  chipZclDiscByClusterRev(version, discoveryResponseHandler);
}

// zcl discovery mode <mode:1>
// mode - 0: single query (default)
//        1: multiple query

void chZclCliDiscSetModeCommand(void)
{
  ChipZclDiscoveryRequestMode mode = (ChipZclDiscoveryRequestMode) chipUnsignedCommandArgument(0);
  bool status = chipZclDiscSetMode(mode);
  chipAfAppPrintln("%s 0x%x", "mode", status);
}

// zcl discovery init
void chZclCliDiscInitCommand(void)
{
  chipZclDiscInit();
}

// zcl discovery send
void chZclCliDiscSendCommand(void)
{
  chipAfAppPrintln("%s 0x%x", "send", chipZclDiscSend(discoveryResponseHandler));
}

// zcl discovery accept <accept:2>
// accept - 40: link-format
//          65064: link-format+cbor (default)
void chZclCliDiscSetAcceptCommand(void)
{
  ChipCoapContentFormatType accept = (ChipCoapContentFormatType) chipUnsignedCommandArgument(0);
  bool status = chZclDiscSetAccept(accept);
  chipAfAppPrintln("%s 0x%x", "accept", status);
}

// zcl cache clear
void chZclCliCacheClearCommand(void)
{
  chZclCacheRemoveAll();
}

// ChZclCacheScanPredicate to print each cache entry.
typedef struct {
  bool dotdotdot; // true - truncate length for more compact display
  bool base64Url; // true - base64url format instead of hex text
} CachePrintCriteria_t;
static bool printCacheEntry(const void *criteria,
                            const ChZclCacheEntry_t *entry)
{
  bool dotdotdot = ((CachePrintCriteria_t *)criteria)->dotdotdot;
  bool base64Url = ((CachePrintCriteria_t *)criteria)->base64Url;
  uint8_t *pad = (uint8_t *)"";

  const ChipZclUid_t *uid = &entry->key;
  const ChipIpv6Address *address = &entry->value;
  chipAfAppPrint(" 0x%2x |", entry->index);
  uint8_t result[CHIP_ZCL_UID_STRING_SIZE];
  if (base64Url) {
    chZclUidToBase64Url(uid, CHIP_ZCL_UID_BITS, result);
    if (!dotdotdot) {
      // If not truncated, base64url needs padding to fill the field.
      pad = (uint8_t *)"                     ";
    }
  } else {
    chZclUidToString(uid, CHIP_ZCL_UID_BITS, result);
  }

  if (dotdotdot) {
    result[30] = '.';
    result[31] = '.';
    result[32] = '.';
    result[33] = '\0';
  }

  chipAfAppPrint(" uid: %s%s", result, pad);

  chipAfAppPrint(" | ipv6: ");
  chipAfAppDebugExec(chipAfPrintIpv6Address(address));
  chipAfAppPrintln("");

  return false; // "not a match" -> continue through all cache entries
}

// zcl cache add uid <uid> ipv6 <ipv6address>
void chZclCliCacheAddCommand(void)
{
  ChipZclUid_t key;
  ChipIpv6Address value;
  ChZclCacheIndex_t index;

  if (!chZclCliGetUidArgument(0, &key)) {
    chipAfAppPrintln("%p: %p", "ERR", "invalid uid");
    return;
  }

  if (!chipGetIpv6AddressArgument(1, &value)) {
    chipAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  bool success = chZclCacheAdd(&key, &value, &index);
  chipAfAppPrint("%s 0x%x",
                  "added",
                  (success
                   ? CHIP_ZCL_STATUS_SUCCESS
                   : CHIP_ZCL_STATUS_FAILURE));
  if (success) {
    chipAfAppPrintln(" index 0x%2x", index);
  } else {
    chipAfAppPrintln("");
  }
}

// zcl cache print
void chZclCliCachePrintCommand(void)
{
//     zcl cache print 1 0 (Full-length UID, hex format)
//     Index  | Identifier                                                            | Address
//     -------+-----------------------------------------------------------------------+-----------------------------------------------
//     0xIIII | hex: 00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff | ipv6: 0000:1111:2222:3333:4444:5555:6666:7777
//
//     zcl cache print 1 1 (Full-length UID, base64url format)
//     Index  | Identifier                                                            | Address
//     -------+-----------------------------------------------------------------------+-----------------------------------------------
//     0xIIII | b64: HIQWERTY_01234567-loqwerty_98765432_-123456                      | ipv6: 0000:1111:2222:3333:4444:5555:6666:7777
//
//     zcl cache print 0 0 (Abbreviated UID, hex format format; i.e., dotdotdot)
//     Index  | Identifier                             | Address
//     -------+----------------------------------------+-----------------------------------------------
//     0xIIII | hex: 00112233445566778899aabbccddee... | ipv6: 0000:1111:2222:3333:4444:5555:6666:7777
//
//     zcl cache print 0 1 (Abbreviated UID, hex format format; i.e., dotdotdot)
//     Index  | Identifier                             | Address
//     -------+----------------------------------------+-----------------------------------------------
//     0xIIII | b64: HIQWERTY_01234567-loqwerty_987... | ipv6: 0000:1111:2222:3333:4444:5555:6666:7777

  CachePrintCriteria_t criteria = {
    .dotdotdot = (0 == chipUnsignedCommandArgument(0)),
    .base64Url = (1 == chipUnsignedCommandArgument(1)),
  };
  chipAfAppPrintln("");
  chipAfAppPrint(" Index  | Identifier                             ");
  if (!criteria.dotdotdot) {
    chipAfAppPrint("                               ");
  }
  chipAfAppPrintln("| Address");
  chipAfAppPrint("--------+----------------------------------------");
  if (!criteria.dotdotdot) {
    chipAfAppPrint("-------------------------------");
  }
  chipAfAppPrintln("+-----------------------------------------------");
  chZclCacheScan(&criteria, printCacheEntry);
}

// zcl cache remove <index:2>
void chZclCliCacheRemoveCommand(void)
{
  ChZclCacheIndex_t index
    = (ChZclCacheIndex_t)chipUnsignedCommandArgument(0);
  bool success = chZclCacheRemoveByIndex(index);
  chipAfAppPrintln("%s 0x%x",
                    "remove",
                    (success
                     ? CHIP_ZCL_STATUS_SUCCESS
                     : CHIP_ZCL_STATUS_FAILURE));
}

// zcl cbor force-definite-len <force:1>
// force - 0: allow indefinite length CBOR encoding
//         1: force all CBOR encoding to use definite length format (default)
void chZclCliCborForceDefiniteLenCommand(void)
{
  bool force = (chipUnsignedCommandArgument(0) != 0);
  emCborSetForceDefiniteLengthEncoding(force);
  chipAfAppPrintln("force %s", force ? "enabled" : "disabled");
}

// ----------------------------------------------------------------------------
// Utilities
static void getClusterSpecArguments(uint8_t index,
                                    ChipZclClusterSpec_t *clusterSpec)
{
  clusterSpec->role = (ChipZclRole_t)chipUnsignedCommandArgument(index);
  clusterSpec->manufacturerCode
    = (ChipZclManufacturerCode_t)chipUnsignedCommandArgument(index + 1);
  clusterSpec->id
    = (ChipZclClusterId_t)chipUnsignedCommandArgument(index + 2);
}

// This function provides only partial reset of CLI state, which is distributed between this
// plugin and the ZCL Core plugin. This function should not be invoked directly. Instead, invoke
// chZclCliResetCliState (a ZCL Core plugin function), which will reset its CLI state and call
// this function to do the same in this plugin.
void chZclCoreCliPluginResetCliState(void)
{
  attributeReadIdsCount = 0;
  attributeWriteData.attributeId = CHIP_ZCL_ATTRIBUTE_NULL;
  bindingEntry.destination.network.port = 0;
  bindingId = CHIP_ZCL_BINDING_NULL;
}

static void attributeReadResponseHandler(ChipZclMessageStatus_t status,
                                         const ChipZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength)
{
  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
  ChipZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  chZclAttributeIdToUriPath(&destination,
                            context->clusterSpec,
                            context->attributeId,
                            uriPath);
  chipAfAppPrint("response %s %s u=%s ",
                  chZclGetMessageStatusName(status),
                  emGetCoapCodeName(context->code),
                  uriPath);
  if (context->status == CHIP_ZCL_STATUS_SUCCESS) {
    chipAfAppPrint("v=");
    chipAfAppPrintBuffer(buffer, bufferLength, false); // attribute value
  } else {
    chipAfAppPrint("s=%x", context->status); // failure status
  }
  chipAfAppPrintln("");
}

static void attributeWriteResponseHandler(ChipZclMessageStatus_t status,
                                          const ChipZclAttributeContext_t *context)
{
  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
  ChipZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  ChipZclAttributeId_t attrId = CHIP_ZCL_ATTRIBUTE_NULL;
  ChipZclStatus_t attrStatus = CHIP_ZCL_STATUS_SUCCESS;

  // The write response payload contains a map only if not
  // all attribute writes were successful.
  if (emCborDecodePeek(context->state, NULL) == CBOR_MAP) {
    // In the case of a cli write response the map will consist of a single
    // {attrId:{"s":status}} entry indicating the write fail status.
    uint8_t key[2]; // 's' plus a NUL
    if (!emCborDecodeMap(context->state)
        || !emCborDecodeValue(context->state,
                              CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(attrId),
                              (uint8_t *)&attrId)
        || !emCborDecodeMap(context->state)
        || !emCborDecodeValue(context->state,
                              CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(key),
                              key)
        || key[0] != 's'
        || !emCborDecodeValue(context->state,
                              CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(attrStatus),
                              (uint8_t *)&attrStatus)) {
      chipAfAppPrintln("failed to decode write response failure status");
      return;
    }
  } else if (emCborNonEmpty(context->state)) {  // Success response must carry empty payload
    chipAfAppPrintln("failed to decode write response success status");
    return;
  }

  chZclAttributeIdToUriPath(&destination,
                            context->clusterSpec,
                            attrId,
                            uriPath);
  chipAfAppPrintln("response %s %s u=%s s=0x%x",
                    chZclGetMessageStatusName(status),
                    emGetCoapCodeName(context->code),
                    uriPath,
                    attrStatus);
}

static void bindingResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclBindingContext_t *context,
                                   const ChipZclBindingEntry_t *entry)
{
  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
  ChipZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  chZclBindingIdToUriPath(&destination,
                          context->clusterSpec,
                          context->bindingId,
                          uriPath);
  chipAfAppPrintln("response %s %s u=%s",
                    chZclGetMessageStatusName(status),
                    emGetCoapCodeName(context->code),
                    uriPath);
}
