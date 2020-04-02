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
#include EMBER_AF_API_DEBUG_PRINT
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

#define MAX_FILE_SPECS 4

// ota-bootload-storage info
void otaBootloadStorageInfoCommand(void)
{
  EmberZclOtaBootloadStorageInfo_t info;
  EmberZclOtaBootloadFileSpec_t fileSpecs[MAX_FILE_SPECS];
  emberZclOtaBootloadStorageGetInfo(&info, fileSpecs, MAX_FILE_SPECS);
  emberAfAppPrintln("OTA Bootload Storage Info: maximumFileSize = %d, fileCount = %d",
                    info.maximumFileSize,
                    info.fileCount);
  for (size_t i = 0; i < info.fileCount; i++) {
    emberAfAppPrint("File %d:", i);

    EmberZclOtaBootloadStorageFileInfo_t fileInfo;
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = emberZclOtaBootloadStorageFind(&fileSpecs[i], &fileInfo);
    if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
      emberAfAppPrintln(" ERROR");
    } else {
      emberAfAppPrintln(" m=0x%2X t=0x%2X v=0x%4X size = %d",
                        fileSpecs[i].manufacturerCode,
                        fileSpecs[i].type,
                        fileSpecs[i].version,
                        fileInfo.size);
    }
  }
}
