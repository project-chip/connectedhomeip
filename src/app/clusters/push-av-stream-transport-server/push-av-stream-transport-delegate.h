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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Defines interfaces for implementing application-specific logic for the PushAvStreamTransport Delegate.
 *
 * This class provides interfaces for command handling and loading of allocated streams.
 */
class PushAvStreamTransportDelegate
{
public:
    PushAvStreamTransportDelegate() = default;

    virtual ~PushAvStreamTransportDelegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    /**
     * @brief Handles stream transport allocation with the provided transport configuration option.
     *
     * @param transportOptions The configuration options of the transport to be allocated
     * @param connectionID The connectionID to allocate
     * @return Success if allocation is successful and a PushTransportConnectionID was produced;
     *         otherwise, the command is rejected with Failure
     *
     * The delegate processes the following transport options:
     * - URL: Validates the URL
     * - StreamUsage, VideoStreamID, AudioStreamID: For selection of Stream
     *
     * Allocates the transport and maps it to the connectionID.
     * On Success, TransportConfigurationStruct is sent as response by the server.
     */
    virtual Protocols::InteractionModel::Status
    AllocatePushTransport(const PushAvStreamTransport::Structs::TransportOptionsStruct::Type & transportOptions,
                          const uint16_t connectionID) = 0;

    /**
     * @brief Handles stream transport deallocation for the provided connectionID.
     *
     * @param connectionID The connectionID to deallocate
     * @return Success if transport deallocation is successful;
     *         BUSY if the transport is currently uploading
     */
    virtual Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) = 0;

    /**
     * @brief Handles stream transport modification.
     *
     * @param connectionID The connectionID of the stream transport to modify
     * @param transportOptions The Transport Options to modify
     * @return Success if stream transport modification is successful;
     *         Failure if modification fails
     *
     * @note The buffers storing URL, Trigger Options, Motion Zones, Container Options are owned by
     * the PushAVStreamTransport Server. The allocated buffers are cleared and reassigned to modified
     * transportOptions on success of ModifyPushTransport and deallocated on success of DeallocatePushTransport.
     */
    virtual Protocols::InteractionModel::Status
    ModifyPushTransport(const uint16_t connectionID, const PushAvStreamTransport::TransportOptionsStorage transportOptions) = 0;

    /**
     * @brief Handles stream transport status modification.
     *
     * @param connectionIDList List of connectionIDs for which new transport status to apply
     * @param transportStatus Updated status of the connection(s)
     * @return Success if stream transport status is successfully set;
     *         Failure if status modification fails
     *
     * Behavior based on transportStatus:
     * - Inactive(1):
     *   - Disables transmissions
     *   - Removes queued items
     *   - Cancels active uploads
     *   - Emits GeneratePushTransportEndEvent
     * - Active(0):
     *   - Enables transmissions
     *   - For Continuous trigger type: begins transmission immediately
     *   - For Command/Motion trigger: begins if trigger is active and within Time Control Bounds
     *   - Emits GeneratePushTransportBeginEvent
     */
    virtual Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                                   PushAvStreamTransport::TransportStatusEnum transportStatus) = 0;

    /**
     * @brief Requests manual start of the specified push transport.
     *
     * @param connectionID The connectionID of the stream transport to trigger
     * @param activationReason Information about why the transport was started/stopped
     * @param timeControl Configuration to control triggered transport lifecycle
     * @return Success if stream transport trigger is successful;
     *         Failure if trigger fails
     *
     * @note The server handles PushTransportBegin event emission on success.
     * The delegate should emit PushTransportEnd Event using GeneratePushTransportEndEvent()
     * when timeControl values indicate end of transmission.
     */
    virtual Protocols::InteractionModel::Status ManuallyTriggerTransport(
        const uint16_t connectionID, PushAvStreamTransport::TriggerActivationReasonEnum activationReason,
        const Optional<PushAvStreamTransport::Structs::TransportMotionTriggerTimeControlStruct::Type> & timeControl) = 0;

    /**
     * @brief Validates the provided URL.
     *
     * @param url The URL to validate
     * @return true if URL is valid, false otherwise
     */
    virtual bool ValidateUrl(std::string url) = 0;

    /**
     * @brief Validates bandwidth requirements against camera's resource management.
     *
     * @param streamUsage The desired usage type for the stream (e.g. live view, recording)
     * @param videoStreamId Optional identifier for the requested video stream
     * @param audioStreamId Optional identifier for the requested audio stream
     * @return Status::Success if stream usage is valid;
     *         Status::ResourceExhausted if resources are insufficient
     *
     * Ensures the requested stream usage is allowed given current allocation of
     * camera resources (CPU, memory, network bandwidth).
     */
    virtual Protocols::InteractionModel::Status
    ValidateBandwidthLimit(PushAvStreamTransport::StreamUsageEnum streamUsage,
                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    /**
     * @brief Assigns existing Video Stream based on camera's resource management and stream priority policies.
     *
     * @param streamUsage The desired usage type for the stream
     * @param videoStreamId Identifier for the requested video stream
     * @return Status::Success and selected videoStreamID if successful;
     *         Status::InvalidStream if no allocated VideoStream exists
     */
    virtual Protocols::InteractionModel::Status SelectVideoStream(PushAvStreamTransport::StreamUsageEnum streamUsage,
                                                                  uint16_t & videoStreamId) = 0;

    /**
     * @brief Assigns existing Audio Stream based on camera's resource management and stream priority policies.
     *
     * @param streamUsage The desired usage type for the stream
     * @param audioStreamId Identifier for the requested audio stream
     * @return Status::Success and selected audioStreamID if successful;
     *         Status::InvalidStream if no allocated AudioStream exists
     */
    virtual Protocols::InteractionModel::Status SelectAudioStream(PushAvStreamTransport::StreamUsageEnum streamUsage,
                                                                  uint16_t & audioStreamId) = 0;

    /**
     * @brief Validates that the video stream corresponding to videoStreamID is allocated.
     *
     * @param videoStreamId Identifier for the requested video stream
     * @return Status::Success if allocated video stream exists;
     *         Status::InvalidStream if no allocated video stream with videoStreamID exists
     */
    virtual Protocols::InteractionModel::Status ValidateVideoStream(uint16_t videoStreamId) = 0;

    /**
     * @brief Validates that the audio stream corresponding to audioStreamID is allocated.
     *
     * @param audioStreamId Identifier for the requested audio stream
     * @return Status::Success if allocated audio stream exists;
     *         Status::InvalidStream if no allocated audio stream with audioStreamID exists
     */
    virtual Protocols::InteractionModel::Status ValidateAudioStream(uint16_t audioStreamId) = 0;

    /**
     * @brief Gets the status of the transport.
     *
     * @param connectionID The connectionID of the stream transport to check status
     * @return busy if transport is uploading, idle otherwise
     */
    virtual PushAvStreamTransport::PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID) = 0;

    /**
     * @brief Delegate callback for notifying change in an attribute.
     *
     * @param attributeId The ID of the attribute that changed
     */
    virtual void OnAttributeChanged(AttributeId attributeId) = 0;

    /**
     * @brief Loads pre-allocated transport connections into the cluster server list.
     *
     * @param currentConnections Vector to store loaded transport configurations
     * @return CHIP_ERROR indicating success or failure
     *
     * The delegate application is responsible for creating and persisting connections
     * based on Allocation commands. These connections' context information is loaded
     * into the cluster server list at initialization for serving attribute reads.
     * The list can be updated via Add/Remove functions for respective transport connections.
     *
     * @note Required buffers are managed by TransportConfigurationStorage;
     * the delegate function must populate the vector correctly.
     */
    virtual CHIP_ERROR
    LoadCurrentConnections(std::vector<PushAvStreamTransport::TransportConfigurationStorage> & currentConnections) = 0;

    /**
     * @brief Callback after persistent attributes managed by the Cluster are loaded from Storage.
     *
     * @return CHIP_ERROR indicating success or failure
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;

protected:
    EndpointId mEndpointId = kInvalidEndpointId;
};
} // namespace Clusters
} // namespace app
} // namespace chip
