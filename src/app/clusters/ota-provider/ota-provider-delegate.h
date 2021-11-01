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

#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {

// TODO: Should this be OTAProvider::Delegate?
/** @brief
 *    Defines methods for implementing application-specific logic for the OTA Provider Cluster.
 */
class OTAProviderDelegate
{
public:
    // TODO(#8605): protocolsSupported should be list of OTADownloadProtocol enums, not uint8_t
    virtual EmberAfStatus HandleQueryImage(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, uint16_t vendorId,
                                           uint16_t productId, uint32_t softwareVersion, uint8_t protocolsSupported,
                                           const Optional<uint16_t> & hardwareVersion, const Optional<CharSpan> & location,
                                           const Optional<bool> & requestorCanConsent,
                                           const Optional<ByteSpan> & metadataForProvider) = 0;

    virtual EmberAfStatus HandleApplyUpdateRequest(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                                   const chip::ByteSpan & updateToken, uint32_t newVersion) = 0;

    virtual EmberAfStatus HandleNotifyUpdateApplied(const chip::ByteSpan & updateToken, uint32_t softwareVersion) = 0;

    virtual ~OTAProviderDelegate() = default;
};

} // namespace Clusters
} // namespace app
} // namespace chip
