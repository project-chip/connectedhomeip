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

#pragma once

#include <app/common/gen/enums.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace clusters {

/** @brief
 *    Defines methods for implementing application-specific logic for the OTA Server Cluster.
 */
class OTAServerDelegate
{
public:
    // TODO: protocolsSupported should be list of OTADownloadProtocol enums, not uint8_t*
    virtual EmberAfStatus HandleQueryImage(uint16_t vendorId, uint16_t productId, uint16_t imageType, uint16_t hardwareVersion,
                                           uint32_t currentVersion, uint8_t * protocolsSupported, const chip::ByteSpan & location,
                                           bool clientCanConsent, const chip::ByteSpan & metadataForServer) = 0;

    virtual EmberAfStatus HandleApplyUpdateRequest(const chip::ByteSpan & updateToken, uint32_t newVersion) = 0;

    virtual EmberAfStatus HandleNotifyUpdateApplied(const chip::ByteSpan & updateToken, uint32_t currentVersion) = 0;

    virtual ~OTAServerDelegate() = default;
};

} // namespace clusters
} // namespace app
} // namespace chip
