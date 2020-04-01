/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

// These values are communicated to the user through the plugin option
// descriptions.
#define INVALID_MANUFACTURER_CODE 0xFFFF
#define INVALID_TYPE 0xFFFF
#define INVALID_VERSION 0xFFFFFFFF

// This is super duper arbitrary. It should be updated as needed.
#define MAX_FILE_SPEC_COUNT 16

static void printFileSpec(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                          bool newline)
{
  chipAfCorePrint(" m=0x%2X t=0x%2X v=0x%4X",
                   fileSpec->manufacturerCode,
                   fileSpec->type,
                   fileSpec->version);
  if (newline) {
    chipAfCorePrintln("");
  }
}

static bool useNextVersion(ChipZclOtaBootloadFileVersion_t currentVersion,
                           ChipZclOtaBootloadFileVersion_t nextVersion)
{
  return (false
#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_UPGRADES
          || (currentVersion < nextVersion)
#endif
#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_DOWNGRADES
          || (currentVersion > nextVersion)
#endif
#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_REINSTALLS
          || (currentVersion == nextVersion)
#endif
          );
}

bool chipZclOtaBootloadServerGetImageNotifyInfoCallback(ChipIpv6Address *address,
                                                         ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  chipAfCorePrint("OTA Bootload Server Policy: GetImageNotifyInfo");
  chipAfCorePrint(" a=");
  chipAfCoreDebugExec(chipAfPrintIpv6Address(address));
  printFileSpec(fileSpec, false); // newline?

  ChipZclOtaBootloadStorageFileInfo_t storageFileInfo;
  ChipZclOtaBootloadStorageStatus_t storageStatus
    = chipZclOtaBootloadStorageFind(fileSpec, &storageFileInfo);
  if (storageStatus == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    chipAfCorePrintln(" found");
    return true;
  }

  chipAfCorePrintln(" missing");
  return false;
}

ChipZclStatus_t chipZclOtaBootloadServerGetNextImageCallback(const ChipIpv6Address *source,
                                                               const ChipZclOtaBootloadFileSpec_t *currentFileSpec,
                                                               ChipZclOtaBootloadFileSpec_t *nextFileSpec)
{
  ChipZclStatus_t status = CHIP_ZCL_STATUS_NO_IMAGE_AVAILABLE;
  ChipZclOtaBootloadStorageInfo_t storageInfo;
  ChipZclOtaBootloadFileSpec_t fileSpecs[MAX_FILE_SPEC_COUNT];
  chipZclOtaBootloadStorageGetInfo(&storageInfo, fileSpecs, MAX_FILE_SPEC_COUNT);
  assert(storageInfo.fileCount <  MAX_FILE_SPEC_COUNT);
  for (size_t i = 0; i < storageInfo.fileCount; i++) {
    if (fileSpecs[i].manufacturerCode == currentFileSpec->manufacturerCode
        && (fileSpecs[i].type == currentFileSpec->type
            || fileSpecs[i].type == CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD
            || currentFileSpec->type == CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD)
        && useNextVersion(currentFileSpec->version, fileSpecs[i].version)) {
      if ((status == CHIP_ZCL_STATUS_NO_IMAGE_AVAILABLE) || (nextFileSpec->version < fileSpecs[i].version)) {
        *nextFileSpec = fileSpecs[i];
        status = CHIP_ZCL_STATUS_SUCCESS;
      }
    }
  }

  chipAfCorePrint("OTA Bootload Server Policy: GetNextImageCallback");
  chipAfCorePrint(" a=");
  chipAfCoreDebugExec(chipAfPrintIpv6Address(source));
  printFileSpec(currentFileSpec, false); // newline?
  printFileSpec(nextFileSpec, false); // newline?
  chipAfCorePrintln(" s=0x%X", status);

  return status;
}

uint32_t chipZclOtaBootloadServerUpgradeEndRequestCallback(const ChipIpv6Address *source,
                                                            const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                            ChipZclStatus_t status)
{
  uint32_t upgradeTime = 0;

  chipAfCorePrint("OTA Bootload Server Policy: UpgradeEndRequest");
  chipAfCorePrint(" a=");
  chipAfCoreDebugExec(chipAfPrintIpv6Address(source));
  printFileSpec(fileSpec, false); // newline?
  chipAfCorePrintln(" s=0x%X t=%d", status, upgradeTime);

  // TODO: implement me.
  return upgradeTime;
}
