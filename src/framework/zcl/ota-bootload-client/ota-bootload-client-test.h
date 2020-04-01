/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef OTA_BOOTLOAD_CLIENT_TEST_H
#define OTA_BOOTLOAD_CLIENT_TEST_H

#include "../ota-bootload-core/ota-bootload-core-test.h"

extern ChipEventControl chZclOtaBootloadClientEventControl;
void chZclOtaBootloadClientEventHandler(void);

void chZclOtaBootloadClientInitCallback(void);
void chZclOtaBootloadClientNetworkStatusCallback(ChipNetworkStatus newNetworkStatus,
                                                 ChipNetworkStatus oldNetworkStatus,
                                                 ChipJoinFailureReason reason);

bool chipZclOtaBootloadClientSetVersionInfoCallback();
bool chipZclOtaBootloadClientExpectSecureOta(void);
bool chipZclDiscByClusterId(const ChipZclClusterSpec_t *clusterSpec, ChipCoapResponseHandler responseHandler);
ChipStatus chipDnsLookup(const uint8_t *domainName,
                           uint8_t domainNameLength,
                           const uint8_t *prefix64,
                           ChipDnsResponseHandler responseHandler,
                           uint8_t *appData,
                           uint16_t appDataLength);
bool chipZclOtaBootloadClientServerHasStaticAddressCallback(ChipZclOtaBootloadClientServerInfo_t *serverInfo);
bool chipZclOtaBootloadClientServerHasDnsNameCallback(ChipZclOtaBootloadClientServerInfo_t *serverInfo);
bool chipZclOtaBootloadClientServerHasDiscByClusterId(const ChipZclClusterSpec_t *clusterSpec,
                                                       ChipCoapResponseHandler responseHandler);
bool chipZclOtaBootloadClientServerDiscoveredCallback(const ChipZclOtaBootloadClientServerInfo_t *serverInfo);
bool chipZclOtaBootloadClientGetQueryNextImageParametersCallback(ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  ChipZclOtaBootloadHardwareVersion_t *hardwareVersion);
bool chipZclOtaBootloadClientStartDownloadCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                    bool existingFile);
ChipZclStatus_t chipZclOtaBootloadClientDownloadCompleteCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                   ChipZclStatus_t status);
void chipZclOtaBootloadClientPreBootloadCallback(const ChipZclOtaBootloadFileSpec_t *fileSpec);

#endif // OTA_BOOTLOAD_CLIENT_TEST_H
