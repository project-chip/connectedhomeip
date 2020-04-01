/***************************************************************************//**
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
