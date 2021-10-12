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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <lib/support/BufferReader.h>

#include "ota-requestor-delegate.h"
#include "ota-requestor.h"

using chip::app::clusters::OTARequestorDelegate;

OTARequestorDelegate * gDelegate = nullptr;

/**
 * @brief OTA Software Update Requestor Cluster AnnounceOtaProvider Command callback (from client)
 */
bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath, chip::EndpointId endpoint,
    chip::ByteSpan providerLocation, uint16_t vendorId, uint8_t announcementReason, chip::ByteSpan metadataForNode,
    chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & fields)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    if (gDelegate == nullptr)
    {
        ChipLogError(Zcl, "No OTARequestorDelegate set");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_COMMAND);
        return true;
    }

    // This is temporary code that will be changed once the definiton of AnnounceOTAProvider is updated to no longer use ByteSpan
    // for the serverLocation field. Note: NodeID is stored as little-endian but in bytes, where the first byte in the ByteSpan is
    // also the most significant byte in the ID.
    chip::NodeId providerId = 0;
    uint8_t temp            = 0;
    chip::Encoding::LittleEndian::Reader bufReader(providerLocation);
    for (size_t i = 0; i < sizeof(chip::NodeId); i++)
    {
        err = bufReader.Read8(&temp).StatusCode();
        providerId += (static_cast<uint64_t>(temp) << ((sizeof(chip::NodeId) - 1 - i) * 8));
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "failed to parse providerLocation as uint64_t: %s", chip::ErrorStr(bufReader.StatusCode()));
        return false;
    }

    status = gDelegate->HandleAnnounceOTAProvider(commandObj, providerId, vendorId, announcementReason, metadataForNode);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

namespace chip {
namespace app {
namespace clusters {

void OTARequestor::SetDelegate(OTARequestorDelegate * delegate)
{
    gDelegate = delegate;
}

} // namespace clusters
} // namespace app
} // namespace chip
