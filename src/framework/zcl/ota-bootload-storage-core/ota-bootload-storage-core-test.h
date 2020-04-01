/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef OTA_BOOTLOAD_STORAGE_CORE_TEST_H
#define OTA_BOOTLOAD_STORAGE_CORE_TEST_H

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE

void deletionTimePassesCallback(void);
void runOtaBootloadStorageCoreTest(bool runMultipleFilesTest);

extern const ChipZclOtaBootloadFileSpec_t myFileSpec;

#endif // OTA_BOOTLOAD_STORAGE_CORE_TEST_H
