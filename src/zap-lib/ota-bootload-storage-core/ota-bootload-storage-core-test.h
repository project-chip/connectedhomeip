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

#ifndef OTA_BOOTLOAD_STORAGE_CORE_TEST_H
#define OTA_BOOTLOAD_STORAGE_CORE_TEST_H

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE

void deletionTimePassesCallback(void);
void runOtaBootloadStorageCoreTest(bool runMultipleFilesTest);

extern const EmberZclOtaBootloadFileSpec_t myFileSpec;

#endif // OTA_BOOTLOAD_STORAGE_CORE_TEST_H
