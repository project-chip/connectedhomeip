/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_DEBUG_PRINT
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

#define MAX_FILE_SPECS 4

// ota-bootload-storage info
void otaBootloadStorageInfoCommand(void)
{
  ChipZclOtaBootloadStorageInfo_t info;
  ChipZclOtaBootloadFileSpec_t fileSpecs[MAX_FILE_SPECS];
  chipZclOtaBootloadStorageGetInfo(&info, fileSpecs, MAX_FILE_SPECS);
  chipAfAppPrintln("OTA Bootload Storage Info: maximumFileSize = %d, fileCount = %d",
                    info.maximumFileSize,
                    info.fileCount);
  for (size_t i = 0; i < info.fileCount; i++) {
    chipAfAppPrint("File %d:", i);

    ChipZclOtaBootloadStorageFileInfo_t fileInfo;
    ChipZclOtaBootloadStorageStatus_t storageStatus
      = chipZclOtaBootloadStorageFind(&fileSpecs[i], &fileInfo);
    if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
      chipAfAppPrintln(" ERROR");
    } else {
      chipAfAppPrintln(" m=0x%2X t=0x%2X v=0x%4X size = %d",
                        fileSpecs[i].manufacturerCode,
                        fileSpecs[i].type,
                        fileSpecs[i].version,
                        fileInfo.size);
    }
  }
}
