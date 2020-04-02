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
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_COMMAND_INTERPRETER2
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_CORE_RESOURCE_DIRECTORY

#define CLI_COMMAND_STRUCT_LENGTH 64

typedef struct {
  const EmberZclClusterSpec_t *clusterSpec;
  EmberZclCommandId_t commandId;
  const ZclipStructSpec *structSpec;

  uint8_t payloadStruct[CLI_COMMAND_STRUCT_LENGTH];
  char commandStrings[EMBER_COMMAND_BUFFER_LENGTH];

  EmZclCliRequestCommandFunction function;
} CurrentRequestCommand;
static CurrentRequestCommand currentRequestCommand = {
  .function = NULL,
};

static void commandResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclCommandContext_t *context,
                                   const void *response);
static void commandPrintInfoExtended(const EmberZclDestination_t *destination,
                                     const EmberZclClusterSpec_t *clusterSpec,
                                     EmberZclCommandId_t commandId,
                                     const ZclipStructSpec *structSpec,
                                     const uint8_t *theStruct,
                                     const char *prefix);
static void commandReallyPrintInfoExtended(const EmberZclApplicationDestination_t *destination,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclCommandId_t commandId,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const char *prefix);

static EmberStatus addressWithBinding (EmberZclDestination_t *address);
static EmberStatus addressWithEndpoint(EmberZclDestination_t *address);
static EmberStatus addressWithGroup   (EmberZclDestination_t *address);

// These functions lives in app/coap/coap.c.
const uint8_t *emGetCoapCodeName(EmberCoapCode type);
const uint8_t *emGetCoapContentFormatTypeName(EmberCoapContentFormatType type);
const uint8_t *emGetCoapStatusName(EmberCoapStatus status);

// ----------------------------------------------------------------------------
// Linkage with ZCL Core CLI plugin
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
// if the ZCL Core CLI plugin is not present.

#if defined(EMBER_AF_PLUGIN_ZCL_CORE_CLI)
// Prototypes for provided functions.
void emZclCoreCliPluginResetCliState(void);
EmberStatus emZclCoreCliPluginSend(EmberZclDestination_t *destination);
#else
// Macro substitutions for plugin functions.
#define emZclCoreCliPluginResetCliState()
#define emZclCoreCliPluginSend(x) (EMBER_INVALID_CALL)
#endif

// ----------------------------------------------------------------------------
// Commands

// zcl send binding  <binding id:1>
// zcl send endpoint <destination type:1> <destination> <endpoint id:1>
// zcl send group    <address> <group id:2>
void emZclCliSendCommand(void)
{
  EmberStatus status;
  uint8_t *payloadStruct;

  EmberZclDestination_t destination = { { 0 } };
  destination.network.flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
  switch (emberStringCommandArgument(-1, NULL)[0]) {
    case 'b': // send to binding
      status = addressWithBinding(&destination);
      break;
    case 'e': // send to endpoint
      status = addressWithEndpoint(&destination);
      break;
    case 'g': // send to group
      status = addressWithGroup(&destination);
      break;
    default:
      assert(false); // uh...
      return;
  }

  if (status != EMBER_SUCCESS) {
    goto done;
  }

  // First delegate to ZCL Core CLI plugin for "send" action, then check for local "send"
  // action if it declines (returns EMBER_INVALID_CALL).
  status = emZclCoreCliPluginSend(&destination);
  if (status == EMBER_INVALID_CALL) {
    if (currentRequestCommand.function != NULL) {
      // If the struct spec is NULL, then there is no payload, so we need to
      // set our payload pointer to NULL.
      payloadStruct = (currentRequestCommand.structSpec == NULL
                       ? NULL
                       : currentRequestCommand.payloadStruct);
      status = (*currentRequestCommand.function)(&destination,
                                                 payloadStruct,
                                                 commandResponseHandler);
    }
  }

  done:
  emberAfAppPrint("%s 0x%x", "send", status);
  if (currentRequestCommand.function != NULL) {
    commandPrintInfoExtended(&destination,
                             currentRequestCommand.clusterSpec,
                             currentRequestCommand.commandId,
                             currentRequestCommand.structSpec,
                             currentRequestCommand.payloadStruct,
                             " ");
  } else {
    emberAfAppPrintln("");
  }

  emZclCliResetCliState();
}

// ----------------------------------------------------------------------------
// Utilities

void emZclCliSetCurrentRequestCommand(const EmberZclClusterSpec_t *clusterSpec,
                                      EmberZclCommandId_t commandId,
                                      const ZclipStructSpec *structSpec,
                                      EmZclCliRequestCommandFunction function,
                                      const char *cliFormat)
{
  emZclCliResetCliState();

  ZclipStructData structData;
  ZclipFieldData fieldData;
  uint8_t *finger;
  uint8_t index = 0;
  char *string = currentRequestCommand.commandStrings;

  currentRequestCommand.clusterSpec = clusterSpec;
  currentRequestCommand.commandId   = commandId;
  currentRequestCommand.structSpec  = structSpec;
  currentRequestCommand.function    = function;

  if (!*cliFormat) {
    goto done;
  }

  if (!emExpandZclipStructData(currentRequestCommand.structSpec, &structData)) {
    emberAfAppPrint("Struct data init failed.");
    goto done;
  }

  for (; *cliFormat; cliFormat++, index++) {
    assert(!emZclipFieldDataFinished(&structData));
    emGetNextZclipFieldData(&structData, &fieldData);
    finger = currentRequestCommand.payloadStruct + fieldData.valueOffset;
    switch (*cliFormat) {
      case '*':
        assert(0); // TODO: handle me.
        break;

      case 's':
        *((int8_t *)finger) = emberSignedCommandArgument(index) & 0x000000FF;
        break;
      case 'r':
        *((int16_t *)finger) = emberSignedCommandArgument(index) & 0x0000FFFF;
        break;
      case 'q':
        *((int32_t *)finger) = emberSignedCommandArgument(index);
        break;
      case 'u':
        *((uint8_t *)finger) = emberUnsignedCommandArgument(index) & 0x000000FF;
        break;
      case 'v':
        *((uint16_t *)finger) = emberUnsignedCommandArgument(index) & 0x0000FFFF;
        break;
      case 'w':
        *((uint32_t *)finger) = emberUnsignedCommandArgument(index);
        break;

      case 'b': {
        // We assume that "b" means a EmberZclStringType_t substructure.
        uint8_t length;
        uint8_t *tmp = emberStringCommandArgument(index, &length);
        EmberZclStringType_t *ezst = (EmberZclStringType_t *)finger;
        ezst->length = length;
        ezst->ptr = (uint8_t *)string;
        finger += sizeof(EmberZclStringType_t);
        MEMMOVE(string, tmp, length);
        string += length;
        break;
      }

      default:
        assert(0); // we should never get to a bad format char
    }

    assert(finger - currentRequestCommand.payloadStruct
           <= sizeof(currentRequestCommand.payloadStruct));
  } // end of for loop

  done:
  commandPrintInfoExtended(NULL, // destination address
                           currentRequestCommand.clusterSpec,
                           currentRequestCommand.commandId,
                           currentRequestCommand.structSpec,
                           currentRequestCommand.payloadStruct,
                           "buffer ");
}

bool emZclCliGetUidArgument(uint8_t index, EmberZclUid_t *uid)
{
  uint8_t uidlen = 0;
  uint8_t *uidarg = emberStringCommandArgument(index, &uidlen);
  if (uidarg != NULL && uidlen == sizeof(EmberZclUid_t)) {
    MEMCOPY(uid, uidarg, sizeof(EmberZclUid_t));
    return true;
  }
  return false;
}

void emZclCliResetCliState(void)
{
  emZclCoreCliPluginResetCliState();
  currentRequestCommand.function = NULL;
}

static void commandResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclCommandContext_t *context,
                                   const void *responsePayload)
{
  EmberZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  emberAfAppPrint("response %s %s",
                  emZclGetMessageStatusName(status),
                  emGetCoapCodeName(context->code));
  commandReallyPrintInfoExtended(&destination,
                                 context->clusterSpec,
                                 context->commandId,
                                 context->payload,
                                 context->payloadLength,
                                 " ");
}

static void commandPrintInfoExtended(const EmberZclDestination_t *destination,
                                     const EmberZclClusterSpec_t *clusterSpec,
                                     EmberZclCommandId_t commandId,
                                     const ZclipStructSpec *structSpec,
                                     const uint8_t *theStruct,
                                     const char *prefix)
{
  uint8_t cbor[EM_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t cborLen;

  if (structSpec != NULL) {
    cborLen = emCborEncodeOneStruct(cbor, sizeof(cbor), structSpec, theStruct);
  } else {
    cborLen = 0;
  }

  commandReallyPrintInfoExtended((destination == NULL
                                  ? NULL
                                  : &destination->application),
                                 clusterSpec,
                                 commandId,
                                 cbor,
                                 cborLen,
                                 prefix);
}

static void commandReallyPrintInfoExtended(const EmberZclApplicationDestination_t *destination,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclCommandId_t commandId,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const char *prefix)
{
  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];

  emZclCommandIdToUriPath(destination, clusterSpec, commandId, uriPath);
  emberAfAppPrint("%su=%s p=", prefix, uriPath);
  emberAfAppPrintBuffer(payload, payloadLength, false);
  emberAfAppPrintln("");
}

static EmberStatus addressWithBinding(EmberZclDestination_t *destination)
{
  EmberZclBindingId_t id = (EmberZclBindingId_t)emberUnsignedCommandArgument(0);
  EmberZclBindingEntry_t entry;

  if (emberZclGetBinding(id, &entry)) {
    emZclReadDestinationFromBinding(&entry, destination);
    return EMBER_SUCCESS;
  } else {
    return EMBER_INVALID_BINDING_INDEX;
  }
}

static EmberStatus addressWithEndpoint(EmberZclDestination_t *destination)
{
  uint8_t addressType = (uint8_t)emberUnsignedCommandArgument(0);
  switch (addressType) {
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
      if (!emberGetIpv6AddressArgument(1, &destination->network.address)) {
        return EMBER_BAD_ARGUMENT;
      }
      destination->network.flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      break;
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_UID:
      if (!emZclCliGetUidArgument(1, &destination->network.uid)) {
        return EMBER_BAD_ARGUMENT;
      }
      destination->network.flags = EMBER_ZCL_HAVE_UID_FLAG;
      break;
    default:
      return EMBER_BAD_ARGUMENT;
  }

  EmberZclEndpointId_t endpointId
    = (EmberZclEndpointId_t)emberUnsignedCommandArgument(2);
  destination->application.data.endpointId = endpointId;
  destination->application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;

  return EMBER_SUCCESS;
}

static EmberStatus addressWithGroup(EmberZclDestination_t *destination)
{
  if (!emberGetIpv6AddressArgument(0, &destination->network.address)) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberZclGroupId_t groupId
    = (EmberZclGroupId_t)emberUnsignedCommandArgument(1);
  destination->application.data.groupId = groupId;
  destination->application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;

  return EMBER_SUCCESS;
}

// CLI: zcl register [<IPv6Address> <port>]
void emZclResourceDirectoryRegisterCommand(void)
{
  EmberIpv6Address resourceDirectoryIp;
  emberGetIpv6AddressArgument(0, &resourceDirectoryIp);

  uint16_t resourceDirectoryPort = (uint16_t)emberUnsignedCommandArgument(1);

  emberAfPluginResourceDirectoryClientRegister(&resourceDirectoryIp, resourceDirectoryPort);
}
