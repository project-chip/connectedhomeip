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

#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-cluster.h>
#include <app/util/config.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

struct PushAvStream
{
    uint16_t id;
    TransportOptionsStruct transportOptions;
    TransportStatusEnum transportStatus;
    PushAvStreamTransportStatusEnum connectionStatus;
};

/**
 * The application delegate to define the options & implement commands.
 */
class PushAvStreamTransportManager : public PushAvStreamTransportDelegate
{
public:
    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                              const uint16_t connectionID);
    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID);
    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStorage transportOptions);
    Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                           TransportStatusEnum transportStatus);

    Protocols::InteractionModel::Status
    ManuallyTriggerTransport(const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
                             const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl);

    bool ValidateUrl(std::string url);

    Protocols::InteractionModel::Status ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                                               const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                               const Optional<DataModel::Nullable<uint16_t>> & audioStreamId);
    Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId);

    Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId);

    Protocols::InteractionModel::Status ValidateVideoStream(uint16_t videoStreamId);

    Protocols::InteractionModel::Status ValidateAudioStream(uint16_t audioStreamId);

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID);

    void OnAttributeChanged(AttributeId attributeId);
    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections);
    CHIP_ERROR PersistentAttributesLoadedCallback();

    void Init();
    PushAvStreamTransportManager() = default;

    ~PushAvStreamTransportManager() = default;

private:
    std::vector<PushAvStream> pushavStreams;
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
