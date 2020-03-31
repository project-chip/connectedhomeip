/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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
