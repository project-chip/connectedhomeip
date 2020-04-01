/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef OTA_BOOTLOAD_SERVER_TEST_H
#define OTA_BOOTLOAD_SERVER_TEST_H

#include "../ota-bootload-core/ota-bootload-core-test.h"

extern ChipEventControl chZclOtaBootloadServerImageNotifyEventControl;
void chZclOtaBootloadServerImageNotifyEventHandler(void);

void chZclOtaBootloadServerNetworkStatusCallback(ChipNetworkStatus newNetworkStatus,
                                                 ChipNetworkStatus oldNetworkStatus,
                                                 ChipJoinFailureReason reason);

bool chipZclOtaBootloadServerGetImageNotifyInfoCallback(ChipIpv6Address *address,
                                                         ChipZclOtaBootloadFileSpec_t *fileSpec);
ChipZclStatus_t chipZclOtaBootloadServerGetNextImageCallback(const ChipIpv6Address *source,
                                                               const ChipZclOtaBootloadFileSpec_t *currentFileSpec,
                                                               ChipZclOtaBootloadFileSpec_t *nextFileSpec);
uint32_t chipZclOtaBootloadServerUpgradeEndRequestCallback(const ChipIpv6Address *source,
                                                            const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                            ChipZclStatus_t status);

void chZclOtaBootloadServerDownloadHandler(ChipCoapCode code,
                                           uint8_t *uri,
                                           ChipCoapReadOptions *options,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const ChipCoapRequestInfo *info);

#endif // OTA_BOOTLOAD_SERVER_TEST_H
