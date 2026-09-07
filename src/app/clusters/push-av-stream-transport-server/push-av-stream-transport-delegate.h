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
#include <app/clusters/tls-certificate-management-server/TLSCertificateManagementCluster.h>
#include <app/clusters/tls-client-management-server/TLSClientManagementCluster.h>
#include <functional>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

// Forward declarations
class PushAvStreamTransportServer;

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

    /**
     * @brief Handles stream transport allocation with the provided transport configuration option.
     *
     * @param transportOptions The configuration options of the transport to be allocated
     * @param connectionID The connectionID to allocate
     * @param AccessingFabricIndex The FabricIndex of the associated Fabric
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
                          const uint16_t connectionID, FabricIndex accessingFabricIndex) = 0;

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
     * the Push AV Stream Transport server. The allocated buffers are cleared and reassigned to modified
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
     * @brief Validates the provided StreamUsage.
     *
     * @param streamUsage The StreamUsage to validate
     * @return true if StreamUsage is present in the StreamUsagePriorities list, false otherwise
     */
    virtual bool ValidateStreamUsage(PushAvStreamTransport::StreamUsageEnum streamUsage) = 0;

    /**
     * @brief Validates the provided Segment Duration.
     *
     * @param segmentDuration The Segment Duration to validate
     * @param videoStreamId   The video stream to be validated against
     * @return true if Segment Duration is multiple of KeyFrameInterval for the provided videoStreamId, false otherwise
     */
    virtual bool ValidateSegmentDuration(uint16_t segmentDuration,
                                         const Optional<DataModel::Nullable<uint16_t>> & videoStreamId) = 0;

    /**
     * @brief Validates the provided Max Pre Roll Length.
     *
     * @param maxPreRollLength Max Pre Roll length to validate
     * @param videoStreamId The video stream ID to be validated against
     * @return true if the Max pre-roll length is greater than or equal to KeyFrameInterval for the provided videoStreamId, false
       otherwise
     */
    virtual bool ValidateMaxPreRollLength(uint16_t maxPreRollLength, const DataModel::Nullable<uint16_t> & videoStreamId) = 0;

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
     * @brief Sets the video stream for Push AV usage corresponding to videoStreamID,
     * if it is valid and is allocated.
     *
     * @param videoStreamId Identifier for the requested video stream
     * @return Status::Success if allocated video stream exists and is set for PushAV usage;
     *         Status::InvalidStream if no allocated video stream with videoStreamID exists
     */
    virtual Protocols::InteractionModel::Status SetVideoStream(uint16_t videoStreamId) = 0;

    /**
     * @brief Sets the audio stream for Push AV usage corresponding to audioStreamID,
     * if it is valid and is allocated.
     *
     * @param audioStreamId Identifier for the requested audio stream
     * @return Status::Success if allocated audio stream exists and is set for PushAV usage;
     *         Status::InvalidStream if no allocated audio stream with audioStreamID exists
     */
    virtual Protocols::InteractionModel::Status SetAudioStream(uint16_t audioStreamId) = 0;

    /**
     * @brief Validates that the zone corresponding to zoneId exists.
     *
     * @param zoneId Identifier for the requested zone
     * @return Status::Success if zone exists;
     *         Status::InvalidZone if no zone with zoneId exists
     */
    virtual Protocols::InteractionModel::Status ValidateZoneId(uint16_t zoneId) = 0;

    /**
     * @brief Validates size of motion zone List.
     *
     * @param zoneListSize Size of the motion zone list
     * @return true if the motion zone list size is less than or equal to the defined maximum, false otherwise
     */
    virtual bool ValidateMotionZoneListSize(size_t zoneListSize) = 0;

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

    /**
     * @brief Sets TLS certificates for secure push transport connections.
     *
     * @param clientCertEntry Reference to buffered client certificate entry
     * @param rootCertEntry Reference to buffered root certificate entry
     */
    virtual void SetTLSCerts(Tls::CertificateTable::BufferedClientCert & clientCertEntry,
                             Tls::CertificateTable::BufferedRootCert & rootCertEntry) = 0;

    /**
     * @brief Verifies whether Hard privacy mode is active on the device as set against the stream management instance
     *
     * @param isActive boolean that is set by the delegate indicating privacy status, True is active
     * @return CHIP_ERROR indicating success or failure
     */
    virtual CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) = 0;

    /**
     * @brief Verifies whether Soft Recording privacy mode is active on the device as set against the stream management instance
     *
     * @param isActive boolean that is set by the delegate indicating privacy status, True is active
     * @return CHIP_ERROR indicating success or failure
     */
    virtual CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) = 0;

    /**
     * @brief Verifies whether Soft Livestream privacy mode is active on the device as set against the stream management instance
     *
     * @param isActive boolean that is set by the delegate indicating privacy status, True is active
     * @return CHIP_ERROR indicating success or failure
     */
    virtual CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) = 0;

    /**
     * @brief Gets the current CMAF session number for the specified connection.
     *
     * @param connectionID The connection ID for which to get the session number
     * @param sessionNumber Output parameter for the current session number
     * @return true if a session number is available, false otherwise
     */
    virtual bool GetCMAFSessionNumber(const uint16_t connectionID, uint64_t & sessionNumber) = 0;

    /**
     * @brief Sets the PushAvStreamTransportServer instance for the delegate.
     *
     * This method is called by the PushAvStreamTransportServer to provide
     * the delegate with a pointer to the server instance. This allows the
     * delegate to interact with the server, for example, to generate events.
     *
     * @param server A pointer to the PushAvStreamTransportServer instance.
     */
    virtual void SetPushAvStreamTransportServer(PushAvStreamTransportServer * server) = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip
