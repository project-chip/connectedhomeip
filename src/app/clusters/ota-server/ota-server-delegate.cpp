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

/*
 * @file
 *   This file provides example implementations for OTAServerDelegate methods.
 *   Developers can modify this file to fit the needs of the application or implement their own handlers.
 */

#include <app/util/af.h>
#include <gen/cluster-id.h>
#include <gen/command-id.h>
#include <gen/enums.h>

#include "ota-server-delegate.h"

#ifdef OTA_SERVER_CLUSTER_EXAMPLE_IMPL

namespace chip {
namespace app {
namespace clusters {

// TODO: these handlers need to be filled in with example OTA Server logic.
EmberAfStatus OTAServerDelegate::HandleQueryImage(uint16_t vendorId, uint16_t productId, uint16_t imageType,
                                                  uint16_t hardwareVersion, uint32_t currentVersion, uint8_t * protocolsSupported,
                                                  const chip::ByteSpan & location, bool clientCanConsent,
                                                  const chip::ByteSpan & metadataForServer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_UNSUP_COMMAND;
    return status;
}

EmberAfStatus OTAServerDelegate::HandleApplyUpdateRequest(const chip::ByteSpan & updateToken, uint32_t newVersion)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_UNSUP_COMMAND;
    return status;
}

EmberAfStatus OTAServerDelegate::HandleNotifyUpdateApplied(const chip::ByteSpan & updateToken, uint32_t currentVersion)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_UNSUP_COMMAND;
    return status;
}

} // namespace clusters
} // namespace app
} // namespace chip

#endif // OTA_SERVER_CLUSTER_EXAMPLE_IMPL
