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

#ifndef OTA_BOOTLOAD_SERVER_TEST_H
#define OTA_BOOTLOAD_SERVER_TEST_H

#include "../ota-bootload-core/ota-bootload-core-test.h"

extern EmberEventControl emZclOtaBootloadServerImageNotifyEventControl;
void emZclOtaBootloadServerImageNotifyEventHandler(void);

void emZclOtaBootloadServerNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason);

bool emberZclOtaBootloadServerGetImageNotifyInfoCallback(EmberIpv6Address *address,
                                                         EmberZclOtaBootloadFileSpec_t *fileSpec);
EmberZclStatus_t emberZclOtaBootloadServerGetNextImageCallback(const EmberIpv6Address *source,
                                                               const EmberZclOtaBootloadFileSpec_t *currentFileSpec,
                                                               EmberZclOtaBootloadFileSpec_t *nextFileSpec);
uint32_t emberZclOtaBootloadServerUpgradeEndRequestCallback(const EmberIpv6Address *source,
                                                            const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                            EmberZclStatus_t status);

void emZclOtaBootloadServerDownloadHandler(EmberCoapCode code,
                                           uint8_t *uri,
                                           EmberCoapReadOptions *options,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const EmberCoapRequestInfo *info);

#endif // OTA_BOOTLOAD_SERVER_TEST_H
