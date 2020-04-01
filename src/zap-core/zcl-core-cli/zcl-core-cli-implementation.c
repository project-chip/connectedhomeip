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
#include "stack/framework/buffer-management.h"
#include "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#include "app/thread/plugin/zcl/zcl-core/zcl-core-well-known.h"

static EmberZclClusterSpec_t clusterSpec;
static EmberZclAttributeId_t attributeReadIds[32]; // TODO: find a good length for me!
static uint8_t attributeReadIdsCount = 0;
static uint8_t attributeWriteDataBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
static EmberZclAttributeWriteData_t attributeWriteData = {
  EMBER_ZCL_ATTRIBUTE_NULL,
  attributeWriteDataBuffer,
  0,
};

static EmberZclBindingEntry_t bindingEntry = {
  .destination.network.port = 0, // indicates not in use
};
static EmberZclBindingId_t bindingId = EMBER_ZCL_BINDING_NULL;

static void getClusterSpecArguments(uint8_t index,
                                    EmberZclClusterSpec_t *clusterSpec);

static void attributeWriteResponseHandler(EmberZclMessageStatus_t status,
                                          const EmberZclAttributeContext_t *context);
static void attributeReadResponseHandler(EmberZclMessageStatus_t status,
                                         const EmberZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength);
static void bindingResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclBindingContext_t *context,
                                   const EmberZclBindingEntry_t *entry);
static void discoveryResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info);

// These functions lives in app/coap/coap.c.
const uint8_t *emGetCoapCodeName(EmberCoapCode type);
const uint8_t *emGetCoapContentFormatTypeName(EmberCoapContentFormatType type);
const uint8_t *emGetCoapStatusName(EmberCoapStatus status);

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
// wrapped by the EMBER_AF_PLUGIN_ZCL_CORE_CLI macro, and suitable stubs are substituted
// if the ZCL Core CLI plugin is not present. The two functions are:
//
// void emZclCoreCliPluginResetCliState(void);
// EmberStatus emZclCoreCliPluginSend(EmberZclDestination_t *destination);
//

// ----------------------------------------------------------------------------
// Commands

// zcl info
void emZclCliInfoCommand(void)
{
  uint8_t result[EMBER_ZCL_UID_STRING_SIZE];
  emZclUidToString(&emZclUid, EMBER_ZCL_UID_BITS, result);
  emberAfAppPrintln("uid: %s", result);
  emZclUidToBase64Url(&emZclUid, EMBER_ZCL_UID_BITS, result);
  emberAfAppPrintln("base64url: %s", result);
}

// zcl access-control mode
void emZclCliAccessControlModeCommand(void)
{
  emberAfAppPrint("access control: ");
  // We only allow access-control to be disabled in test mode
  // (in real apps access-control should always be enabled).
  extern emZclAccessControlMode_t emZclUseAccessControl;
  emZclUseAccessControl = emberUnsignedCommandArgument(0);
  emberAfAppPrintln((emZclUseAccessControl != 0)
                    ? "enabled"
                    : "disabled");
}

// zcl access-control print
void emZclCliAccessControlPrintCommand(void)
{
  #if !defined(EMBER_AF_PRINT_PLUGIN_ZCL_CORE)
  emberAfAppPrintln("[Debug printing for ZCL Core plugin should be enabled]");
  #endif
  emZclPrintAccessTokens();
}

// zcl attribute print
void emZclCliAttributePrintCommand(void)
{
  emberAfAppPrintln("");
  emberAfAppPrintln(" EE | R | MMMM | CCCC | AAAA | KKKK | Data ");
  emberAfAppPrintln("----+---+------+------+------+------+------");
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    for (size_t j = 0; j < EM_ZCL_ATTRIBUTE_COUNT; j++) {
      uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
      if (emZclReadAttributeEntry(emZclEndpointTable[i].endpointId,
                                  &emZclAttributeTable[j],
                                  buffer,
                                  sizeof(buffer))
          == EMBER_ZCL_STATUS_SUCCESS) {
        halResetWatchdog();
        emberAfAppPrint(" %x | %c | %2x | %2x | %2x | %2x | ",
                        emZclEndpointTable[i].endpointId,
                        ((emZclAttributeTable[j].clusterSpec->role
                          == EMBER_ZCL_ROLE_CLIENT)
                         ? 'c'
                         : 's'),
                        emZclAttributeTable[j].clusterSpec->manufacturerCode,
                        emZclAttributeTable[j].clusterSpec->id,
                        emZclAttributeTable[j].attributeId,
                        emZclAttributeTable[j].mask);
        emberAfAppPrintBuffer(buffer, emZclAttributeTable[j].size, false);
        emberAfAppPrintln("");
      }
    }
  }
}

// zcl attribute reset <endpoint id:1>
void emZclCliAttributeResetCommand(void)
{
  EmberZclEndpointId_t endpointId
    = (EmberZclEndpointId_t)emberUnsignedCommandArgument(0);
  emberZclResetAttributes(endpointId);
}

// zcl attribute read <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2>
void emZclCliAttributeReadCommand(void)
{
  EmberZclEndpointId_t endpointId
    = (EmberZclEndpointId_t)emberUnsignedCommandArgument(0);
  EmberZclClusterSpec_t clusterSpec;
  getClusterSpecArguments(1, &clusterSpec);
  EmberZclAttributeId_t attributeId
    = (EmberZclAttributeId_t)emberUnsignedCommandArgument(4);

  const EmZclAttributeEntry_t *attribute
    = emZclFindAttribute(&clusterSpec, attributeId, false); // exclude remote
  uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
  EmberZclStatus_t status = emZclReadAttributeEntry(endpointId,
                                                    attribute,
                                                    buffer,
                                                    sizeof(buffer));
  emberAfAppPrint("%s 0x%x", "read", status);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfAppPrint(": ");
    // SUCCESS implies that 'attribute' is non-NULL.
    size_t size = emZclAttributeSize(attribute, buffer);
    emberAfAppPrintBuffer(buffer, size, false);
  }
  emberAfAppPrintln("");
}

// zcl attribute write <endpoint id:1> <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> <data:n>
void emZclCliAttributeWriteCommand(void)
{
  EmberZclEndpointId_t endpointId
    = (EmberZclEndpointId_t)emberUnsignedCommandArgument(0);
  EmberZclClusterSpec_t clusterSpec;
  getClusterSpecArguments(1, &clusterSpec);
  EmberZclAttributeId_t attributeId
    = (EmberZclAttributeId_t)emberUnsignedCommandArgument(4);
  uint8_t bufferLength;
  uint8_t *buffer = emberStringCommandArgument(5, &bufferLength);

  EmberZclStatus_t status = emberZclWriteAttribute(endpointId,
                                                   &clusterSpec,
                                                   attributeId,
                                                   buffer,
                                                   bufferLength);
  emberAfAppPrintln("%s 0x%x", "write", status);
}

// zcl attribute remote read <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> [<attribute id:2> ...]
void emZclCliAttributeRemoteReadCommand(void)
{
  emZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);

  uint8_t commandArgumentCount = emberCommandArgumentCount();
  for (uint8_t i = 3; i < commandArgumentCount; i++) {
    attributeReadIds[attributeReadIdsCount++]
      = (EmberZclAttributeId_t)emberUnsignedCommandArgument(i);
  }

  emberAfAppPrintln("%s 0x%x", "read", EMBER_ZCL_STATUS_SUCCESS);
}

// zcl attribute remote write <role:1> <manufacturer code:2> <cluster id:2> <attribute id:2> <data:n>
void emZclCliAttributeRemoteWriteCommand(void)
{
  emZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);
  attributeWriteData.attributeId
    = (EmberZclAttributeId_t)emberUnsignedCommandArgument(3);
  uint8_t bufferLength;
  uint8_t *buffer = emberStringCommandArgument(4, &bufferLength);
  if (bufferLength <= sizeof(attributeWriteDataBuffer)) {
    MEMCOPY(attributeWriteDataBuffer, buffer, bufferLength);
    attributeWriteData.bufferLength = bufferLength;
    emberAfAppPrintln("%s 0x%x", "write", EMBER_ZCL_STATUS_SUCCESS);
  } else {
    emberAfAppPrintln("%s 0x%x", "write", EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
  }
}

// zcl binding clear
void emZclCliBindingClearCommand(void)
{
  emberZclRemoveAllBindings();
}

// zcl binding print
void emZclCliBindingPrintCommand(void)
{
  emberAfAppPrintln(" II | EE | R | MMMM | CCCC | RR |                       R                        ");
  emberAfAppPrintln("----+----+---+------+------+----+------------------------------------------------");
  for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    EmberZclBindingEntry_t entry;
    if (emberZclGetBinding(i, &entry)) {
      emberAfAppPrint(" %x | %x | %c | %2x | %2x | %x | ",
                      i,
                      entry.endpointId,
                      (entry.clusterSpec.role == EMBER_ZCL_ROLE_CLIENT
                       ? 'c'
                       : 's'),
                      entry.clusterSpec.manufacturerCode,
                      entry.clusterSpec.id,
                      entry.reportingConfigurationId);
      uint8_t uri[EMBER_ZCL_URI_MAX_LENGTH];
      EmberZclDestination_t destination;
      emZclReadDestinationFromBinding(&entry, &destination);
      emZclDestinationToUri(&destination, uri);
      emberAfAppPrintln("%s", uri);
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
  emZclCliResetCliState();
  bool isAdd = emberStringCommandArgument(-1, NULL)[0] == 'a';
  uint8_t i = isRemote ? 0 : 1; // base index into arguments

  getClusterSpecArguments(i, &bindingEntry.clusterSpec);
  if (!isRemote) {
    bindingEntry.endpointId = (EmberZclEndpointId_t)emberUnsignedCommandArgument(0);
  }
  bindingEntry.destination.network.scheme
    = ((bool)emberUnsignedCommandArgument(i + 3)
       ? EMBER_ZCL_SCHEME_COAPS
       : EMBER_ZCL_SCHEME_COAP);

  if (!emberGetIpv6AddressArgument(i + 4, &bindingEntry.destination.network.data.address)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }
  bindingEntry.destination.network.type = EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS;
  bindingEntry.destination.network.port = (uint16_t)emberUnsignedCommandArgument(i + 5);

  EmberZclEndpointId_t tempEndpointId = (EmberZclEndpointId_t)emberUnsignedCommandArgument(i + 6);
  EmberZclGroupId_t tempGroupId = (EmberZclGroupId_t)emberUnsignedCommandArgument(i + 7);
  if (tempGroupId != EMBER_ZCL_GROUP_NULL) {
    bindingEntry.destination.application.data.groupId = tempGroupId;
    bindingEntry.destination.application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    bindingEntry.destination.application.data.endpointId = tempEndpointId;
    bindingEntry.destination.application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  bindingEntry.reportingConfigurationId = (EmberZclReportingConfigurationId_t)emberUnsignedCommandArgument(i + 8);

  bool success = true;
  const char *name = "add";

  if (isRemote) {
    if (isAdd) {
      bindingId = EMBER_ZCL_BINDING_NULL;
    } else {
      name = "update";
      bindingId = (EmberZclBindingId_t)emberUnsignedCommandArgument(9);
    }
  } else if (isAdd) {
    bindingId = emberZclAddBinding(&bindingEntry);
    success = bindingId != EMBER_ZCL_BINDING_NULL;
  } else {
    bindingId = (EmberZclBindingId_t)emberUnsignedCommandArgument(10);
    success = emberZclSetBinding(bindingId, &bindingEntry);
    name = "set";
  }
  emberAfAppPrintln("%s 0x%x",
                    name,
                    (success
                     ? EMBER_ZCL_STATUS_SUCCESS
                     : EMBER_ZCL_STATUS_FAILURE));
}

void emZclCliBindingAddSetCommand(void)
{
  bindingAddUpdateCommand(false);
}

// zcl binding remove <binding id:1>
void emZclCliBindingRemoveCommand(void)
{
  emZclCliResetCliState();

  bindingId = (EmberZclBindingId_t)emberUnsignedCommandArgument(0);
  bool success = emberZclRemoveBinding(bindingId);
  emberAfAppPrintln("%s 0x%x",
                    "remove",
                    (success
                     ? EMBER_ZCL_STATUS_SUCCESS
                     : EMBER_ZCL_STATUS_FAILURE));
}

// zcl binding remote add    <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1>
// zcl binding remote update <role:1> <manufacturer code:2> <cluster id:2> <secure:1> <destination address> <destination port:2> <destination endpoint id:1> <destination group id:2> <reporting configuration id:1> <binding id:1>
void emZclCliBindingRemoteAddUpdateCommand(void)
{
  bindingAddUpdateCommand(true);
}

// zcl binding remote remove <role:1> <manufacturer code:2> <cluster id:2> <binding id:1>
void emZclCliBindingRemoteRemoveCommand(void)
{
  emZclCliResetCliState();

  getClusterSpecArguments(0, &clusterSpec);
  bindingId = (EmberZclBindingId_t)emberUnsignedCommandArgument(3);
  emberAfAppPrintln("%s 0x%x", "remove", EMBER_ZCL_STATUS_SUCCESS);
}

EmberStatus emZclCoreCliPluginSend(EmberZclDestination_t *destination)
{
  EmberStatus status;

  if (attributeReadIdsCount != 0) {
    status = emberZclSendAttributeRead(destination,
                                       &clusterSpec,
                                       attributeReadIds,
                                       attributeReadIdsCount,
                                       attributeReadResponseHandler);
  } else if (attributeWriteData.attributeId != EMBER_ZCL_ATTRIBUTE_NULL) {
    status = emberZclSendAttributeWrite(destination,
                                        &clusterSpec,
                                        &attributeWriteData,
                                        1, // attribute write data count
                                        attributeWriteResponseHandler);
  } else if (bindingEntry.destination.network.port != 0
             && bindingId == EMBER_ZCL_BINDING_NULL) {
    status = emberZclSendAddBinding(destination,
                                    &bindingEntry,
                                    bindingResponseHandler);
  } else if (bindingEntry.destination.network.port != 0
             && bindingId != EMBER_ZCL_BINDING_NULL) {
    status = emberZclSendUpdateBinding(destination,
                                       &bindingEntry,
                                       bindingId,
                                       bindingResponseHandler);
  } else if (bindingEntry.destination.network.port == 0
             && bindingId != EMBER_ZCL_BINDING_NULL) {
    status = emberZclSendRemoveBinding(destination,
                                       &clusterSpec,
                                       bindingId,
                                       bindingResponseHandler);
  } else {
    status = EMBER_INVALID_CALL;
  }

  return status;
}

// zcl ez-mode start <role:1> <manufacturer code:2> <cluster id:2>
void emZclCliEzModeStartCommand(void)
{
  EmberStatus status = emberZclStartEzMode();
  emberAfAppPrintln("%s 0x%x", "start", status);
}

// zcl discovery cluster <role:1> <manufacturer code:2> <cluster id:2>
void emZclCliDiscByClusterIdCommand(void)
{
  EmberZclClusterSpec_t spec;
  getClusterSpecArguments(0, &spec);
  emberZclDiscByClusterId(&spec, discoveryResponseHandler);
}

// zcl discovery endpoint <endpoint:1> <deviceId:2> / query to ze=urn:zcl:d.$DEVICE_ID.$ENDPOINT
void emZclCliDiscByEndpointCommand(void)
{
  EmberZclEndpointId_t endpoint = (EmberZclEndpointId_t) emberUnsignedCommandArgument(0);
  EmberZclDeviceId_t deviceId = (EmberZclDeviceId_t) emberUnsignedCommandArgument(1);
  emberZclDiscByEndpoint(endpoint, deviceId, discoveryResponseHandler);
}

// zcl discovery device-type <device type:2>
void emZclCliDiscByDeviceTypeCommand(void)
{
  EmberZclDeviceId_t deviceId = (EmberZclDeviceId_t) emberUnsignedCommandArgument(0);
  emberZclDiscByDeviceId(deviceId, discoveryResponseHandler);
}

static void discoveryResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info)
{
  emberAfAppPrint("Discovery CLI:");
  emberAfAppPrint(" %s", emGetCoapStatusName(status));

  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    emberAfAppPrint(" %s", emGetCoapCodeName(code));
    if (payloadLength != 0) {
      uint32_t valueLoc;
      EmberCoapContentFormatType contentFormat
        = (emberReadIntegerOption(options,
                                  EMBER_COAP_OPTION_CONTENT_FORMAT,
                                  &valueLoc)
           ? (EmberCoapContentFormatType)valueLoc
           : EMBER_COAP_CONTENT_FORMAT_NONE);
      emberAfAppPrint(" f=%s p=",
                      emGetCoapContentFormatTypeName(contentFormat));
      if (contentFormat == EMBER_COAP_CONTENT_FORMAT_TEXT_PLAIN
          || contentFormat == EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT) {
        emberAfAppDebugExec(emberAfPrintCharacters(EMBER_AF_PRINT_APP,
                                                   payload,
                                                   payloadLength));
      } else {
        emberAfAppPrintBuffer(payload, payloadLength, false);
      }
    }
  }

  emberAfAppPrintln("");
}

// zcl discovery uid <uid>
void emZclCliDiscByUidCommand(void)
{
  uint8_t *string = emberStringCommandArgument(0, NULL);
  emZclDiscByUidString(string, discoveryResponseHandler);
}

// zcl discovery resource-version <version:2>
void emZclCliDiscByResourceVersionCommand(void)
{
  EmberZclClusterRevision_t version = (EmberZclClusterRevision_t) emberUnsignedCommandArgument(0);
  emberZclDiscByResourceVersion(version, discoveryResponseHandler);
}

// zcl discovery cluster-version <version:2>
void emZclCliDiscByClusterRevisionCommand(void)
{
  EmberZclClusterRevision_t version = (EmberZclClusterRevision_t) emberUnsignedCommandArgument(0);
  emberZclDiscByClusterRev(version, discoveryResponseHandler);
}

// zcl discovery mode <mode:1>
// mode - 0: single query (default)
//        1: multiple query

void emZclCliDiscSetModeCommand(void)
{
  EmberZclDiscoveryRequestMode mode = (EmberZclDiscoveryRequestMode) emberUnsignedCommandArgument(0);
  bool status = emberZclDiscSetMode(mode);
  emberAfAppPrintln("%s 0x%x", "mode", status);
}

// zcl discovery init
void emZclCliDiscInitCommand(void)
{
  emberZclDiscInit();
}

// zcl discovery send
void emZclCliDiscSendCommand(void)
{
  emberAfAppPrintln("%s 0x%x", "send", emberZclDiscSend(discoveryResponseHandler));
}

// zcl discovery accept <accept:2>
// accept - 40: link-format
//          65064: link-format+cbor (default)
void emZclCliDiscSetAcceptCommand(void)
{
  EmberCoapContentFormatType accept = (EmberCoapContentFormatType) emberUnsignedCommandArgument(0);
  bool status = emZclDiscSetAccept(accept);
  emberAfAppPrintln("%s 0x%x", "accept", status);
}

// zcl cache clear
void emZclCliCacheClearCommand(void)
{
  emZclCacheRemoveAll();
}

// EmZclCacheScanPredicate to print each cache entry.
typedef struct {
  bool dotdotdot; // true - truncate length for more compact display
  bool base64Url; // true - base64url format instead of hex text
} CachePrintCriteria_t;
static bool printCacheEntry(const void *criteria,
                            const EmZclCacheEntry_t *entry)
{
  bool dotdotdot = ((CachePrintCriteria_t *)criteria)->dotdotdot;
  bool base64Url = ((CachePrintCriteria_t *)criteria)->base64Url;
  uint8_t *pad = (uint8_t *)"";

  const EmberZclUid_t *uid = &entry->key;
  const EmberIpv6Address *address = &entry->value;
  emberAfAppPrint(" 0x%2x |", entry->index);
  uint8_t result[EMBER_ZCL_UID_STRING_SIZE];
  if (base64Url) {
    emZclUidToBase64Url(uid, EMBER_ZCL_UID_BITS, result);
    if (!dotdotdot) {
      // If not truncated, base64url needs padding to fill the field.
      pad = (uint8_t *)"                     ";
    }
  } else {
    emZclUidToString(uid, EMBER_ZCL_UID_BITS, result);
  }

  if (dotdotdot) {
    result[30] = '.';
    result[31] = '.';
    result[32] = '.';
    result[33] = '\0';
  }

  emberAfAppPrint(" uid: %s%s", result, pad);

  emberAfAppPrint(" | ipv6: ");
  emberAfAppDebugExec(emberAfPrintIpv6Address(address));
  emberAfAppPrintln("");

  return false; // "not a match" -> continue through all cache entries
}

// zcl cache add uid <uid> ipv6 <ipv6address>
void emZclCliCacheAddCommand(void)
{
  EmberZclUid_t key;
  EmberIpv6Address value;
  EmZclCacheIndex_t index;

  if (!emZclCliGetUidArgument(0, &key)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid uid");
    return;
  }

  if (!emberGetIpv6AddressArgument(1, &value)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid ip");
    return;
  }

  bool success = emZclCacheAdd(&key, &value, &index);
  emberAfAppPrint("%s 0x%x",
                  "added",
                  (success
                   ? EMBER_ZCL_STATUS_SUCCESS
                   : EMBER_ZCL_STATUS_FAILURE));
  if (success) {
    emberAfAppPrintln(" index 0x%2x", index);
  } else {
    emberAfAppPrintln("");
  }
}

// zcl cache print
void emZclCliCachePrintCommand(void)
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
    .dotdotdot = (0 == emberUnsignedCommandArgument(0)),
    .base64Url = (1 == emberUnsignedCommandArgument(1)),
  };
  emberAfAppPrintln("");
  emberAfAppPrint(" Index  | Identifier                             ");
  if (!criteria.dotdotdot) {
    emberAfAppPrint("                               ");
  }
  emberAfAppPrintln("| Address");
  emberAfAppPrint("--------+----------------------------------------");
  if (!criteria.dotdotdot) {
    emberAfAppPrint("-------------------------------");
  }
  emberAfAppPrintln("+-----------------------------------------------");
  emZclCacheScan(&criteria, printCacheEntry);
}

// zcl cache remove <index:2>
void emZclCliCacheRemoveCommand(void)
{
  EmZclCacheIndex_t index
    = (EmZclCacheIndex_t)emberUnsignedCommandArgument(0);
  bool success = emZclCacheRemoveByIndex(index);
  emberAfAppPrintln("%s 0x%x",
                    "remove",
                    (success
                     ? EMBER_ZCL_STATUS_SUCCESS
                     : EMBER_ZCL_STATUS_FAILURE));
}

// zcl cbor force-definite-len <force:1>
// force - 0: allow indefinite length CBOR encoding
//         1: force all CBOR encoding to use definite length format (default)
void emZclCliCborForceDefiniteLenCommand(void)
{
  bool force = (emberUnsignedCommandArgument(0) != 0);
  emCborSetForceDefiniteLengthEncoding(force);
  emberAfAppPrintln("force %s", force ? "enabled" : "disabled");
}

// ----------------------------------------------------------------------------
// Utilities
static void getClusterSpecArguments(uint8_t index,
                                    EmberZclClusterSpec_t *clusterSpec)
{
  clusterSpec->role = (EmberZclRole_t)emberUnsignedCommandArgument(index);
  clusterSpec->manufacturerCode
    = (EmberZclManufacturerCode_t)emberUnsignedCommandArgument(index + 1);
  clusterSpec->id
    = (EmberZclClusterId_t)emberUnsignedCommandArgument(index + 2);
}

// This function provides only partial reset of CLI state, which is distributed between this
// plugin and the ZCL Core plugin. This function should not be invoked directly. Instead, invoke
// emZclCliResetCliState (a ZCL Core plugin function), which will reset its CLI state and call
// this function to do the same in this plugin.
void emZclCoreCliPluginResetCliState(void)
{
  attributeReadIdsCount = 0;
  attributeWriteData.attributeId = EMBER_ZCL_ATTRIBUTE_NULL;
  bindingEntry.destination.network.port = 0;
  bindingId = EMBER_ZCL_BINDING_NULL;
}

static void attributeReadResponseHandler(EmberZclMessageStatus_t status,
                                         const EmberZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength)
{
  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  EmberZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  emZclAttributeIdToUriPath(&destination,
                            context->clusterSpec,
                            context->attributeId,
                            uriPath);
  emberAfAppPrint("response %s %s u=%s ",
                  emZclGetMessageStatusName(status),
                  emGetCoapCodeName(context->code),
                  uriPath);
  if (context->status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfAppPrint("v=");
    emberAfAppPrintBuffer(buffer, bufferLength, false); // attribute value
  } else {
    emberAfAppPrint("s=%x", context->status); // failure status
  }
  emberAfAppPrintln("");
}

static void attributeWriteResponseHandler(EmberZclMessageStatus_t status,
                                          const EmberZclAttributeContext_t *context)
{
  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  EmberZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  EmberZclAttributeId_t attrId = EMBER_ZCL_ATTRIBUTE_NULL;
  EmberZclStatus_t attrStatus = EMBER_ZCL_STATUS_SUCCESS;

  // The write response payload contains a map only if not
  // all attribute writes were successful.
  if (emCborDecodePeek(context->state, NULL) == CBOR_MAP) {
    // In the case of a cli write response the map will consist of a single
    // {attrId:{"s":status}} entry indicating the write fail status.
    uint8_t key[2]; // 's' plus a NUL
    if (!emCborDecodeMap(context->state)
        || !emCborDecodeValue(context->state,
                              EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(attrId),
                              (uint8_t *)&attrId)
        || !emCborDecodeMap(context->state)
        || !emCborDecodeValue(context->state,
                              EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(key),
                              key)
        || key[0] != 's'
        || !emCborDecodeValue(context->state,
                              EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(attrStatus),
                              (uint8_t *)&attrStatus)) {
      emberAfAppPrintln("failed to decode write response failure status");
      return;
    }
  } else if (emCborNonEmpty(context->state)) {  // Success response must carry empty payload
    emberAfAppPrintln("failed to decode write response success status");
    return;
  }

  emZclAttributeIdToUriPath(&destination,
                            context->clusterSpec,
                            attrId,
                            uriPath);
  emberAfAppPrintln("response %s %s u=%s s=0x%x",
                    emZclGetMessageStatusName(status),
                    emGetCoapCodeName(context->code),
                    uriPath,
                    attrStatus);
}

static void bindingResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclBindingContext_t *context,
                                   const EmberZclBindingEntry_t *entry)
{
  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  EmberZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  emZclBindingIdToUriPath(&destination,
                          context->clusterSpec,
                          context->bindingId,
                          uriPath);
  emberAfAppPrintln("response %s %s u=%s",
                    emZclGetMessageStatusName(status),
                    emGetCoapCodeName(context->code),
                    uriPath);
}
