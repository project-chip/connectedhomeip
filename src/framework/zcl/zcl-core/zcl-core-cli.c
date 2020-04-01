/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BUFFER_MANAGEMENT
#include CHIP_AF_API_COMMAND_INTERPRETER2
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_CORE_RESOURCE_DIRECTORY

#define CLI_COMMAND_STRUCT_LENGTH 64

typedef struct {
  const ChipZclClusterSpec_t *clusterSpec;
  ChipZclCommandId_t commandId;
  const ZclipStructSpec *structSpec;

  uint8_t payloadStruct[CLI_COMMAND_STRUCT_LENGTH];
  char commandStrings[CHIP_COMMAND_BUFFER_LENGTH];

  ChZclCliRequestCommandFunction function;
} CurrentRequestCommand;
static CurrentRequestCommand currentRequestCommand = {
  .function = NULL,
};

static void commandResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclCommandContext_t *context,
                                   const void *response);
static void commandPrintInfoExtended(const ChipZclDestination_t *destination,
                                     const ChipZclClusterSpec_t *clusterSpec,
                                     ChipZclCommandId_t commandId,
                                     const ZclipStructSpec *structSpec,
                                     const uint8_t *theStruct,
                                     const char *prefix);
static void commandReallyPrintInfoExtended(const ChipZclApplicationDestination_t *destination,
                                           const ChipZclClusterSpec_t *clusterSpec,
                                           ChipZclCommandId_t commandId,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const char *prefix);

static ChipStatus addressWithBinding (ChipZclDestination_t *address);
static ChipStatus addressWithEndpoint(ChipZclDestination_t *address);
static ChipStatus addressWithGroup   (ChipZclDestination_t *address);

// These functions lives in app/coap/coap.c.
const uint8_t *emGetCoapCodeName(ChipCoapCode type);
const uint8_t *emGetCoapContentFormatTypeName(ChipCoapContentFormatType type);
const uint8_t *emGetCoapStatusName(ChipCoapStatus status);

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
// wrapped by the CHIP_AF_PLUGIN_ZCL_CORE_CLI macro, and suitable stubs are substituted
// if the ZCL Core CLI plugin is not present.

#if defined(CHIP_AF_PLUGIN_ZCL_CORE_CLI)
// Prototypes for provided functions.
void chZclCoreCliPluginResetCliState(void);
ChipStatus chZclCoreCliPluginSend(ChipZclDestination_t *destination);
#else
// Macro substitutions for plugin functions.
#define chZclCoreCliPluginResetCliState()
#define chZclCoreCliPluginSend(x) (CHIP_INVALID_CALL)
#endif

// ----------------------------------------------------------------------------
// Commands

// zcl send binding  <binding id:1>
// zcl send endpoint <destination type:1> <destination> <endpoint id:1>
// zcl send group    <address> <group id:2>
void chZclCliSendCommand(void)
{
  ChipStatus status;
  uint8_t *payloadStruct;

  ChipZclDestination_t destination = { { 0 } };
  destination.network.flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
  switch (chipStringCommandArgument(-1, NULL)[0]) {
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

  if (status != CHIP_SUCCESS) {
    goto done;
  }

  // First delegate to ZCL Core CLI plugin for "send" action, then check for local "send"
  // action if it declines (returns CHIP_INVALID_CALL).
  status = chZclCoreCliPluginSend(&destination);
  if (status == CHIP_INVALID_CALL) {
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
  chipAfAppPrint("%s 0x%x", "send", status);
  if (currentRequestCommand.function != NULL) {
    commandPrintInfoExtended(&destination,
                             currentRequestCommand.clusterSpec,
                             currentRequestCommand.commandId,
                             currentRequestCommand.structSpec,
                             currentRequestCommand.payloadStruct,
                             " ");
  } else {
    chipAfAppPrintln("");
  }

  chZclCliResetCliState();
}

// ----------------------------------------------------------------------------
// Utilities

void chZclCliSetCurrentRequestCommand(const ChipZclClusterSpec_t *clusterSpec,
                                      ChipZclCommandId_t commandId,
                                      const ZclipStructSpec *structSpec,
                                      ChZclCliRequestCommandFunction function,
                                      const char *cliFormat)
{
  chZclCliResetCliState();

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
    chipAfAppPrint("Struct data init failed.");
    goto done;
  }

  for (; *cliFormat; cliFormat++, index++) {
    assert(!chZclipFieldDataFinished(&structData));
    emGetNextZclipFieldData(&structData, &fieldData);
    finger = currentRequestCommand.payloadStruct + fieldData.valueOffset;
    switch (*cliFormat) {
      case '*':
        assert(0); // TODO: handle me.
        break;

      case 's':
        *((int8_t *)finger) = chipSignedCommandArgument(index) & 0x000000FF;
        break;
      case 'r':
        *((int16_t *)finger) = chipSignedCommandArgument(index) & 0x0000FFFF;
        break;
      case 'q':
        *((int32_t *)finger) = chipSignedCommandArgument(index);
        break;
      case 'u':
        *((uint8_t *)finger) = chipUnsignedCommandArgument(index) & 0x000000FF;
        break;
      case 'v':
        *((uint16_t *)finger) = chipUnsignedCommandArgument(index) & 0x0000FFFF;
        break;
      case 'w':
        *((uint32_t *)finger) = chipUnsignedCommandArgument(index);
        break;

      case 'b': {
        // We assume that "b" means a ChipZclStringType_t substructure.
        uint8_t length;
        uint8_t *tmp = chipStringCommandArgument(index, &length);
        ChipZclStringType_t *ezst = (ChipZclStringType_t *)finger;
        ezst->length = length;
        ezst->ptr = (uint8_t *)string;
        finger += sizeof(ChipZclStringType_t);
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

bool chZclCliGetUidArgument(uint8_t index, ChipZclUid_t *uid)
{
  uint8_t uidlen = 0;
  uint8_t *uidarg = chipStringCommandArgument(index, &uidlen);
  if (uidarg != NULL && uidlen == sizeof(ChipZclUid_t)) {
    MEMCOPY(uid, uidarg, sizeof(ChipZclUid_t));
    return true;
  }
  return false;
}

void chZclCliResetCliState(void)
{
  chZclCoreCliPluginResetCliState();
  currentRequestCommand.function = NULL;
}

static void commandResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclCommandContext_t *context,
                                   const void *responsePayload)
{
  ChipZclApplicationDestination_t destination = { { 0 } };
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    destination.data.groupId = context->groupId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
  } else {
    destination.data.endpointId = context->endpointId;
    destination.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
  }

  chipAfAppPrint("response %s %s",
                  chZclGetMessageStatusName(status),
                  emGetCoapCodeName(context->code));
  commandReallyPrintInfoExtended(&destination,
                                 context->clusterSpec,
                                 context->commandId,
                                 context->payload,
                                 context->payloadLength,
                                 " ");
}

static void commandPrintInfoExtended(const ChipZclDestination_t *destination,
                                     const ChipZclClusterSpec_t *clusterSpec,
                                     ChipZclCommandId_t commandId,
                                     const ZclipStructSpec *structSpec,
                                     const uint8_t *theStruct,
                                     const char *prefix)
{
  uint8_t cbor[CH_ZCL_MAX_PAYLOAD_SIZE];
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

static void commandReallyPrintInfoExtended(const ChipZclApplicationDestination_t *destination,
                                           const ChipZclClusterSpec_t *clusterSpec,
                                           ChipZclCommandId_t commandId,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const char *prefix)
{
  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];

  chZclCommandIdToUriPath(destination, clusterSpec, commandId, uriPath);
  chipAfAppPrint("%su=%s p=", prefix, uriPath);
  chipAfAppPrintBuffer(payload, payloadLength, false);
  chipAfAppPrintln("");
}

static ChipStatus addressWithBinding(ChipZclDestination_t *destination)
{
  ChipZclBindingId_t id = (ChipZclBindingId_t)chipUnsignedCommandArgument(0);
  ChipZclBindingEntry_t entry;

  if (chipZclGetBinding(id, &entry)) {
    chZclReadDestinationFromBinding(&entry, destination);
    return CHIP_SUCCESS;
  } else {
    return CHIP_INVALID_BINDING_INDEX;
  }
}

static ChipStatus addressWithEndpoint(ChipZclDestination_t *destination)
{
  uint8_t addressType = (uint8_t)chipUnsignedCommandArgument(0);
  switch (addressType) {
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
      if (!chipGetIpv6AddressArgument(1, &destination->network.address)) {
        return CHIP_BAD_ARGUMENT;
      }
      destination->network.flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      break;
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_UID:
      if (!chZclCliGetUidArgument(1, &destination->network.uid)) {
        return CHIP_BAD_ARGUMENT;
      }
      destination->network.flags = CHIP_ZCL_HAVE_UID_FLAG;
      break;
    default:
      return CHIP_BAD_ARGUMENT;
  }

  ChipZclEndpointId_t endpointId
    = (ChipZclEndpointId_t)chipUnsignedCommandArgument(2);
  destination->application.data.endpointId = endpointId;
  destination->application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;

  return CHIP_SUCCESS;
}

static ChipStatus addressWithGroup(ChipZclDestination_t *destination)
{
  if (!chipGetIpv6AddressArgument(0, &destination->network.address)) {
    return CHIP_BAD_ARGUMENT;
  }

  ChipZclGroupId_t groupId
    = (ChipZclGroupId_t)chipUnsignedCommandArgument(1);
  destination->application.data.groupId = groupId;
  destination->application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;

  return CHIP_SUCCESS;
}

// CLI: zcl register [<IPv6Address> <port>]
void chZclResourceDirectoryRegisterCommand(void)
{
  ChipIpv6Address resourceDirectoryIp;
  chipGetIpv6AddressArgument(0, &resourceDirectoryIp);

  uint16_t resourceDirectoryPort = (uint16_t)chipUnsignedCommandArgument(1);

  chipAfPluginResourceDirectoryClientRegister(&resourceDirectoryIp, resourceDirectoryPort);
}
