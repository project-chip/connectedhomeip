/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginOtaBootloadClientPolicyPrint(...)
  #define chipAfPluginOtaBootloadClientPolicyPrintln(...)
  #define chipAfPluginOtaBootloadClientPolicyFlush()
  #define chipAfPluginOtaBootloadClientPolicyDebugExec(x)
  #define chipAfPluginOtaBootloadClientPolicyPrintBuffer(buffer, len, withSpace)
  #define chipAfPluginOtaBootloadClientPolicyPrintString(buffer)
#endif
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CLIENT

enum {
  CHIP_ZCL_OTA_STATIC_IP   = 0,
  CHIP_ZCL_OTA_DNS_LOOKUP  = 1,
  CHIP_ZCL_OTA_DISCOVER    = 2
};

// These values are communicated to the user through the description of the
// plugin options for this plugin.
#define INVALID_PORT 0xFFFF
#define INVALID_ENDPOINT 0xFF

uint8_t const serverName[] = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_NAME;

#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_DELETE_EXISTING_IMAGES_BEFORE_DOWNLOAD
static bool deletingImage = false;

static void storageDeleteCallback(ChipZclOtaBootloadStorageStatus_t storageStatus)
{
  assert(deletingImage);
  chipAfPluginOtaBootloadClientPolicyPrintln("DeleteImage COMPLETE s=0x%1X",
                                              storageStatus);
  deletingImage = false;
}
#endif

bool handleSetAttributeStatus(char *attribName, ChipZclStatus_t status)
{
  if (status == CHIP_SUCCESS) {
    return false;
  }

  if (status == CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE) {
    chipAfPluginOtaBootloadClientPolicyPrintln("%s attribute not enabled", attribName);
  } else {
    chipAfPluginOtaBootloadClientPolicyPrintln("Can't set %s attribute: status=0x%x", attribName, status);
  }
  return true;
}

bool chipZclOtaBootloadClientSetVersionInfoCallback()
{
  bool ret = false;
  const ChipZclEndpointId_t endpoint = 1;
  const ChipZclClusterSpec_t chipZclClusterBootloadClientSpec = {
    CHIP_ZCL_ROLE_CLIENT,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_OTA_BOOTLOAD,
  };
  uint32_t currentFileVersion = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_VERSION;
  ret |= handleSetAttributeStatus("Current File Version",
                                  chipZclWriteAttribute(endpoint,
                                                         &chipZclClusterBootloadClientSpec,
                                                         CHIP_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_CURRENT_FILE_VERSION,
                                                         &currentFileVersion,
                                                         sizeof(currentFileVersion)));
  uint16_t manufacturerId = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_MANUFACTURER_CODE;
  ret |= handleSetAttributeStatus("Manufacturer ID",
                                  chipZclWriteAttribute(endpoint,
                                                         &chipZclClusterBootloadClientSpec,
                                                         CHIP_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_MANUFACTURER_ID,
                                                         &manufacturerId,
                                                         sizeof(manufacturerId)));
  uint16_t imageType = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_TYPE;
  ret |= handleSetAttributeStatus("Image Type",
                                  chipZclWriteAttribute(endpoint,
                                                         &chipZclClusterBootloadClientSpec,
                                                         CHIP_ZCL_CLUSTER_OTA_BOOTLOAD_CLIENT_ATTRIBUTE_IMAGE_TYPE_ID,
                                                         &imageType,
                                                         sizeof(imageType)));
  chipAfPluginOtaBootloadClientPolicyPrintln("Firmware information: %2x-%2x-%4x", manufacturerId, imageType, currentFileVersion);
  return ret;
}

bool chipZclOtaBootloadClientServerHasStaticAddressCallback(ChipZclOtaBootloadClientServerInfo_t *serverInfo)
{
  #define O(x) CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != CHIP_ZCL_OTA_STATIC_IP) {
    return false;
  }

  ChipIpv6Address address = { O(ADDRESS_PARAMETER) };
  ChipZclUid_t uid = { O(UID_PARAMETER) };

  if ( ((!chipZclOtaBootloadIsWildcard(address.bytes, sizeof(address.bytes)))
        || (!chipZclOtaBootloadIsWildcard(uid.bytes, sizeof(uid.bytes))))
       && (O(PORT_PARAMETER) != 0xFFFF)
       && (O(ENDPOINT_PARAMETER) != 0xFF)) {
    serverInfo->scheme = O(SCHEME_PARAMETER);
    serverInfo->name = NULL;
    serverInfo->nameLength = 0;
    serverInfo->address = address;
    serverInfo->port = O(PORT_PARAMETER);
    serverInfo->uid = uid;
    serverInfo->endpointId = O(ENDPOINT_PARAMETER);
    return true;
  }
  #undef O
  return false;
}

bool chipZclOtaBootloadClientServerHasDnsNameCallback(ChipZclOtaBootloadClientServerInfo_t *serverInfo)
{
  #define O(x) CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != CHIP_ZCL_OTA_DNS_LOOKUP) {
    return false;
  }

  ChipIpv6Address address = { O(ADDRESS_PARAMETER) };
  ChipZclUid_t uid = { O(UID_PARAMETER) };

  if ( (O(PORT_PARAMETER) != 0xFFFF) && (!chipZclOtaBootloadIsWildcard(uid.bytes, sizeof(uid.bytes))) && (O(ENDPOINT_PARAMETER) != 0xFF)) {
    serverInfo->scheme = O(SCHEME_PARAMETER);
    serverInfo->name = serverName;
    serverInfo->nameLength = O(NAME_LENGTH);
    serverInfo->address = address;
    serverInfo->port = O(PORT_PARAMETER);
    serverInfo->uid = uid;
    serverInfo->endpointId = O(ENDPOINT_PARAMETER);
    return true;
  }
  #undef O
  return false;
}

bool chipZclOtaBootloadClientServerHasDiscByClusterId(const ChipZclClusterSpec_t *clusterSpec, ChipCoapResponseHandler responseHandler)
{
  #define O(x) CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if (O(LOOKUP_TYPE) != CHIP_ZCL_OTA_DISCOVER) {
    return false;
  }

  #undef O
  return chipZclDiscByClusterId(clusterSpec, responseHandler);
}

static void printFileSpec(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                          bool newline)
{
  chipAfPluginOtaBootloadClientPolicyPrint(" m=0x%2X t=0x%2X v=0x%4X",
                                            fileSpec->manufacturerCode,
                                            fileSpec->type,
                                            fileSpec->version);
  if (newline) {
    chipAfPluginOtaBootloadClientPolicyPrintln("");
  }
}

#ifdef CHIP_AF_PRINT_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY
typedef struct {
  ChipZclScheme_t scheme;
  const uint8_t * const name;
} SchemeNameInfo_t;
static const SchemeNameInfo_t schemeNameInfo[] = {
  { CHIP_ZCL_SCHEME_COAP, (const uint8_t *)"COAP", },
  { CHIP_ZCL_SCHEME_COAPS, (const uint8_t *)"COAPS", },
};
static const uint8_t *getSchemeName(ChipZclScheme_t scheme)
{
  for (size_t i = 0; i < COUNTOF(schemeNameInfo); i++) {
    if (schemeNameInfo[i].scheme == scheme) {
      return schemeNameInfo[i].name;
    }
  }
  return (const uint8_t *)"?????";
}
#endif

bool chipZclOtaBootloadClientExpectSecureOta()
{
  #define O(x) CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  ChipZclUid_t uid = { O(UID_PARAMETER) };
  bool accept = (O(SCHEME_PARAMETER) == CHIP_ZCL_SCHEME_COAPS
                 || !chipZclOtaBootloadIsWildcard(uid.bytes, sizeof(uid.bytes)));
  #undef O
  return accept;
}

bool chipZclOtaBootloadClientServerDiscoveredCallback(const ChipZclOtaBootloadClientServerInfo_t *serverInfo)
{
  chipAfPluginOtaBootloadClientPolicyPrint("ServerDiscovered");
  chipAfPluginOtaBootloadClientPolicyPrint(" s=%s a=", (const char *)getSchemeName(serverInfo->scheme));
  chipAfPluginOtaBootloadClientPolicyDebugExec(chipAfPrintIpv6Address(&serverInfo->address));
  chipAfPluginOtaBootloadClientPolicyPrint(" p=%d u=", serverInfo->port);
  chipAfPluginOtaBootloadClientPolicyPrintBuffer(serverInfo->uid.bytes, CHIP_ZCL_UID_SIZE, true); // withSpace?
  chipAfPluginOtaBootloadClientPolicyPrint(" e=%d", serverInfo->endpointId);
  #define O(x) CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_SERVER_ ## x
  if ((serverInfo->scheme != O(SCHEME_PARAMETER)) && (O(SCHEME_PARAMETER) == CHIP_ZCL_SCHEME_COAPS)) {
    // reject only if we want COAPS but got COAP
    chipAfPluginOtaBootloadClientPolicyPrintln(" reject: Scheme");
    return false;
  }
  ChipIpv6Address address = { O(ADDRESS_PARAMETER) };
  if ((MEMCOMPARE(serverInfo->address.bytes, address.bytes, sizeof(serverInfo->address.bytes)) != 0)
      && !chipZclOtaBootloadIsWildcard(address.bytes, sizeof(serverInfo->address.bytes))) {
    chipAfPluginOtaBootloadClientPolicyPrintln(" reject: Address");
    return false;
  }
  if ((serverInfo->port != O(PORT_PARAMETER)) && (O(PORT_PARAMETER) != 0xFFFF)) {
    chipAfPluginOtaBootloadClientPolicyPrintln(" reject: Port");
    return false;
  }
  ChipZclUid_t uid = { O(UID_PARAMETER) };
  if ((MEMCOMPARE(serverInfo->uid.bytes, uid.bytes, sizeof(serverInfo->uid.bytes)) != 0)
      && !chipZclOtaBootloadIsWildcard(uid.bytes, sizeof(serverInfo->uid.bytes))) {
    chipAfPluginOtaBootloadClientPolicyPrintln(" reject: Uid");
    return false;
  }
  if ((serverInfo->endpointId != O(ENDPOINT_PARAMETER)) && (O(ENDPOINT_PARAMETER) != 0xFF)) {
    chipAfPluginOtaBootloadClientPolicyPrintln(" reject: Endpoint");
    return false;
  }
  #undef O

  chipAfPluginOtaBootloadClientPolicyPrintln(" accept");

  return true;
}

bool chipZclOtaBootloadClientGetQueryNextImageParametersCallback(ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  ChipZclOtaBootloadHardwareVersion_t *hardwareVersion)
{
  fileSpec->manufacturerCode
    = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_MANUFACTURER_CODE;
  fileSpec->type
    = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_TYPE;
  fileSpec->version
    = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_IMAGE_VERSION;
  *hardwareVersion
    = CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_CURRENT_HARDWARE_VERSION;

  chipAfPluginOtaBootloadClientPolicyPrint("GetQueryNextImageParameters");
  printFileSpec(fileSpec, false); // newline?
  chipAfPluginOtaBootloadClientPolicyPrintln(" h=0x%2X", *hardwareVersion);

  return true;
}

bool chipZclOtaBootloadClientStartDownloadCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                    bool existingFile)
{
  bool startDownload = true;

  chipAfPluginOtaBootloadClientPolicyPrint("StartDownload e=%s",
                                            (existingFile ? "true" : "false"));
  printFileSpec(fileSpec, true); // newline?

#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY_DELETE_EXISTING_IMAGES_BEFORE_DOWNLOAD
  // To make sure that there is nothing in storage that would prevent us from
  // writing a new downloaded OTA file, we delete all files here (by passing the
  // special "null" file spec value below). The existingFile bool parameter will
  // tell us if there is a file in storage matching the fileSpec parameter, but
  // it still may be the case that there are other files (with different file
  // specs) in storage. We should update the existingFile parameter to be more
  // helpful.
  ChipZclOtaBootloadStorageInfo_t info;
  chipZclOtaBootloadStorageGetInfo(&info, NULL, 0);
  if (info.fileCount > 0 && !deletingImage) {
    deletingImage = true;
    #ifdef CHIP_AF_PRINT_PLUGIN_OTA_BOOTLOAD_CLIENT_POLICY
    ChipZclOtaBootloadStorageStatus_t storageStatus =
    #endif
    chipZclOtaBootloadStorageDelete(&chipZclOtaBootloadFileSpecNull,
                                     storageDeleteCallback);
    chipAfPluginOtaBootloadClientPolicyPrintln("DeleteImage START s=0x%1X",
                                                storageStatus);
  }
  startDownload = !deletingImage;
#endif

  return startDownload;
}

ChipZclStatus_t chipZclOtaBootloadClientDownloadCompleteCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                   ChipZclStatus_t status)
{
  chipAfPluginOtaBootloadClientPolicyPrint("DownloadComplete");
  printFileSpec(fileSpec, false); // newline?
  #ifdef UNIX_HOST
  chipAfPluginOtaBootloadClientPolicyPrintln(" s=0x%X", status);
  #else
  chipAfPluginOtaBootloadClientPolicyPrintln(" s=0x%1X", status);
  #endif
  return status;
}

void chipZclOtaBootloadClientPreBootloadCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  chipAfPluginOtaBootloadClientPolicyPrint("PreBootload");
  printFileSpec(fileSpec, true); // newline?
}
