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
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-server.h>
#include <transport/pushav-transport.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

#define MAX_PUSH_TRANSPORT_CONNECTION_ID 65535

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;

using MetadataOptionsStruct                   = Structs::MetadataOptionsStruct::Type;
using CMAFContainerOptionsStruct              = Structs::CMAFContainerOptionsStruct::Type;
using ContainerOptionsStruct                  = Structs::ContainerOptionsStruct::Type;
using TransportZoneOptionsStruct              = Structs::TransportZoneOptionsStruct::Type;
using TransportTriggerOptionsStruct           = Structs::TransportTriggerOptionsStruct::Type;
using TransportMotionTriggerTimeControlStruct = Structs::TransportMotionTriggerTimeControlStruct::Type;
using TransportOptionsStruct                  = Structs::TransportOptionsStruct::DecodableType;
using TransportConfigurationStruct            = Structs::TransportConfigurationStruct::Type;

namespace Camera {

class PushAVTransportManager : public app::Clusters::PushAvStreamTransport::PushAvStreamTransportDelegate
{
public:
    PushAVTransportManager() { std::fill_n(Transports, MAX_PUSH_TRANSPORT_CONNECTION_ID, nullptr); }

    ~PushAVTransportManager() {}

    Protocols::InteractionModel::Status AllocatePushTransport(uint16_t connectionID,
                                                              const TransportOptionsStruct & transportOptions,
                                                              TransportStatusEnum & outTransportStatus) override;

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) override;

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStruct & outTransportOptions) override;

    Protocols::InteractionModel::Status SetTransportStatus(const uint16_t connectionID,
                                                           TransportStatusEnum transportStatus) override;

    Protocols::InteractionModel::Status
    ManuallyTriggerTransport(const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
                             const TransportMotionTriggerTimeControlStruct & timeControl) override;

    Protocols::InteractionModel::Status
    FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                  DataModel::List<TransportConfigurationStruct> & outtransportConfigurations) override;

    void OnAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR LoadCurrentConnections(std::vector<uint16_t> & currentConnections) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

private:
    PushAVTransport * Transports[MAX_PUSH_TRANSPORT_CONNECTION_ID];                         // map for the transport objects
    std::optional<TransportOptionsStruct> ConnectionsMap[MAX_PUSH_TRANSPORT_CONNECTION_ID]; // map for the transport options
    std::vector<uint16_t> mCurrentConnections;
};

} // namespace Camera
