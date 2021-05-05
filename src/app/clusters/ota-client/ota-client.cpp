/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
 */

#include <app/Command.h>
#include <app/util/af.h>
#include <gen/att-storage.h>
#include <gen/attribute-id.h>
#include <gen/attribute-type.h>
#include <gen/callback.h>
#include <gen/cluster-id.h>
#include <gen/command-id.h>
#include <gen/enums.h>

#include <support/Span.h>

using namespace chip;

// TODO: all callbacks need implementations

/**
 * @brief OTA Software Update Client Cluster AnnounceOtaServer Command callback
 * @param serverLocation
 * @param vendorId
 * @param announcementReason
 * @param metadataForNode
 */

bool emberAfOtaSoftwareUpdateClientClusterAnnounceOtaServerCallback(chip::app::Command * commandObj, chip::ByteSpan serverLocation,
                                                                    uint16_t vendorId, uint8_t announcementReason,
                                                                    chip::ByteSpan metadataForNode)
{
    ChipLogDetail(Zcl, "OTA Client Cluster received QueryImageResponse");
    EmberAfStatus status = EMBER_ZCL_STATUS_UNSUP_COMMAND;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief OTA Software Update Server Cluster QueryImageResponse Command callback
 * @param status
 * @param delayedActionTime
 * @param imageURI
 * @param softwareVersion
 * @param updateToken
 * @param userConsentNeeded
 * @param metadataForClient
 */

bool emberAfOtaSoftwareUpdateServerClusterQueryImageResponseCallback(chip::app::Command * commandObj, uint8_t status,
                                                                     uint32_t delayedActionTime, uint8_t * imageURI,
                                                                     uint32_t softwareVersion, chip::ByteSpan updateToken,
                                                                     uint8_t userConsentNeeded, chip::ByteSpan metadataForClient)
{
    ChipLogDetail(Zcl, "OTA Server Cluster (client) received QueryImageResponse");
    return true;
}