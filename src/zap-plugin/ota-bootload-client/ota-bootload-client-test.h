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

#ifndef OTA_BOOTLOAD_CLIENT_TEST_H
#define OTA_BOOTLOAD_CLIENT_TEST_H

#include "../ota-bootload-core/ota-bootload-core-test.h"

extern EmberEventControl emZclOtaBootloadClientEventControl;
void emZclOtaBootloadClientEventHandler(void);

void emZclOtaBootloadClientInitCallback(void);
void emZclOtaBootloadClientNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason);

bool emberZclOtaBootloadClientSetVersionInfoCallback();
bool emberZclOtaBootloadClientExpectSecureOta(void);
bool emberZclDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec, EmberCoapResponseHandler responseHandler);
EmberStatus emberDnsLookup(const uint8_t *domainName,
                           uint8_t domainNameLength,
                           const uint8_t *prefix64,
                           EmberDnsResponseHandler responseHandler,
                           uint8_t *appData,
                           uint16_t appDataLength);
bool emberZclOtaBootloadClientServerHasStaticAddressCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo);
bool emberZclOtaBootloadClientServerHasDnsNameCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo);
bool emberZclOtaBootloadClientServerHasDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec,
                                                       EmberCoapResponseHandler responseHandler);
bool emberZclOtaBootloadClientServerDiscoveredCallback(const EmberZclOtaBootloadClientServerInfo_t *serverInfo);
bool emberZclOtaBootloadClientGetQueryNextImageParametersCallback(EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  EmberZclOtaBootloadHardwareVersion_t *hardwareVersion);
bool emberZclOtaBootloadClientStartDownloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                    bool existingFile);
EmberZclStatus_t emberZclOtaBootloadClientDownloadCompleteCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                   EmberZclStatus_t status);
void emberZclOtaBootloadClientPreBootloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec);

#endif // OTA_BOOTLOAD_CLIENT_TEST_H
