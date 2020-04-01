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
