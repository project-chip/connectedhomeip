/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTARequestorEventHandler.h>
#include <app/clusters/ota-requestor/OTARequestorEventHandlerRegistry.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip::app::Clusters {

class OtaRequestorCluster : public DefaultServerCluster, public OTARequestorEventHandler
{
public:
    OtaRequestorCluster(EndpointId endpointId, OTARequestorInterface & otaRequestor);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    void Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    void OnStateTransition(OtaSoftwareUpdateRequestor::UpdateStateEnum previousState,
                           OtaSoftwareUpdateRequestor::UpdateStateEnum newState,
                           OtaSoftwareUpdateRequestor::ChangeReasonEnum reason,
                           DataModel::Nullable<uint32_t> const & targetSoftwareVersion) override;

    void OnVersionApplied(uint32_t softwareVersion, uint16_t productId) override;

    void OnDownloadError(uint32_t softwareVersion, uint64_t bytesDownloaded,
                         DataModel::Nullable<uint8_t> progressPercent,
                         DataModel::Nullable<int64_t> platformCode) override;

private:
    OTARequestorEventHandlerRegistration mEventHandlerRegistration;
    OTARequestorInterface & mOtaRequestor;
};

}  // namespace chip::app::Clusters
