/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>

#include <map>
#include <vector>

namespace chip {
namespace app {

/**
 * Camera device type (0x0142) for a platform with no camera hardware attached.
 *
 * Exposes the mandatory Camera AV Stream Management and WebRTC Transport Provider
 * server clusters. Every delegate callback only logs and keeps the minimal
 * bookkeeping required for the clusters to stay consistent; no media is produced.
 */
class LoggingCamera : public SingleEndpoint,
                      public Clusters::CameraAvStreamManagement::CameraAVStreamManagementDelegate,
                      public Clusters::WebRTCTransportProvider::Delegate
{
    using VideoStreamStruct     = Clusters::CameraAvStreamManagement::VideoStreamStruct;
    using AudioStreamStruct     = Clusters::CameraAvStreamManagement::AudioStreamStruct;
    using VideoResolutionStruct = Clusters::CameraAvStreamManagement::VideoResolutionStruct;
    using WebRTCSessionStruct   = Clusters::WebRTCTransportProvider::WebRTCSessionStruct;
    using ICECandidateStruct    = Clusters::WebRTCTransportProvider::ICECandidateStruct;
    using WebRTCEndReasonEnum   = Clusters::WebRTCTransportProvider::WebRTCEndReasonEnum;
    using StreamUsageEnum       = Clusters::Globals::StreamUsageEnum;

public:
    LoggingCamera();
    ~LoggingCamera() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    /// Applies attribute defaults and loads persisted stream allocations.
    /// Must be called once the data model provider has started the clusters (i.e. after Server::Init()).
    CHIP_ERROR InitClusters();

    // CameraAVStreamManagementDelegate
    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override;
    void OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream,
                                Clusters::CameraAvStreamManagement::StreamAllocationAction action) override;
    Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                          const Optional<bool> osdEnabled) override;
    Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) override;
    Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override;
    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override;
    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                               uint16_t & outStreamID) override;
    Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                             const Optional<bool> osdEnabled) override;
    Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) override;
    void OnStreamUsagePrioritiesChanged() override;
    void OnAttributeChanged(AttributeId attributeId) override;
    Protocols::InteractionModel::Status
    CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID, const VideoResolutionStruct & resolution,
                    Clusters::CameraAvStreamManagement::ImageSnapshot & outImageSnapshot) override;
    CHIP_ERROR PersistentAttributesLoadedCallback() override;
    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const override;
    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const override;

    // WebRTCTransportProvider::Delegate
    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                  bool & outDeferredOffer) override;
    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override;
    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override;
    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override;
    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override;
    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<std::vector<uint16_t>> & videoStreams,
                                   Optional<std::vector<uint16_t>> & audioStreams) override;
    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override;
    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override;
    CHIP_ERROR ValidateVideoStreams(const std::vector<uint16_t> & videoStreams) override;
    CHIP_ERROR ValidateAudioStreams(const std::vector<uint16_t> & audioStreams) override;
    CHIP_ERROR IsStreamUsageSupported(StreamUsageEnum streamUsage) override;
    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override;
    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override;
    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override;
    bool HasAllocatedVideoStreams() override;
    bool HasAllocatedAudioStreams() override;
    CHIP_ERROR ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength) override;
    CHIP_ERROR IsUTCTimeNull(bool & isNull) override;

private:
    // Storage backing the non-owning stream lists of a WebRTCSessionStruct.
    struct SessionStreams
    {
        std::vector<uint16_t> videoStreams;
        std::vector<uint16_t> audioStreams;
    };

    void FillSession(const OfferRequestArgs & args, WebRTCSessionStruct & outSession);
    void ReleaseSessionStreams(uint16_t sessionId);

    LazyRegisteredServerCluster<Clusters::CameraAvStreamManagement::CameraAVStreamManagementCluster> mAvStreamManagementCluster;
    LazyRegisteredServerCluster<Clusters::WebRTCTransportProvider::WebRTCTransportProviderCluster> mWebRTCTransportProviderCluster;
    std::map<uint16_t, SessionStreams> mSessionStreams;
};

} // namespace app
} // namespace chip
