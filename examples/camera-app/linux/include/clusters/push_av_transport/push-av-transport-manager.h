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
#include "media-controller.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-server.h>
#include <protocols/interaction_model/StatusCode.h>
#include <transport/pushav-transport.h>

#include <unordered_map>

#include <optional>

#define MAX_PUSH_TRANSPORT_CONNECTION_ID 65535

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;

namespace Camera {

struct PushAvStream
{
    uint16_t id;
    TransportConfigurationStruct transportConfig;
};

class PushAvStreamTransportManager : public app::Clusters::PushAvStreamTransport::PushAvStreamTransportDelegate
{
public:
    PushAvStreamTransportManager() {};

    ~PushAvStreamTransportManager();

    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsDecodeableStruct & transportOptions,
                                                              TransportConfigurationStruct & outTransporConfiguration) override;

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) override;

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsDecodeableStruct & transportOptions) override;

    Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                           TransportStatusEnum transportStatus) override;

    Protocols::InteractionModel::Status ManuallyTriggerTransport(
        const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
        const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl) override;

    Protocols::InteractionModel::Status
    FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                  DataModel::List<const TransportConfigurationStruct> & outtransportConfigurations) override;

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                   const Optional<DataModel::Nullable<uint16_t>> & audioStreamId);

    void OnAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStruct> & currentConnections) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    void Init(MediaController * aMediaController);

private:
    MediaController * mMediaController = nullptr;

    std::unordered_map<uint16_t, std::unique_ptr<PushAVTransport>> mTransportMap;        // map for the transport objects
    std::unordered_map<uint16_t, TransportOptionsDecodeableStruct> mTransportOptionsMap; // map for the transport options
    std::unordered_map<uint16_t, TransportConfigurationStruct> mTransportConfigMap;      // map for the transport configurations
};

} // namespace Camera
