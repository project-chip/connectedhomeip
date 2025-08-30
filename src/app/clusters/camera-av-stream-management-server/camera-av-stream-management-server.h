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
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/StatusResponse.h>
#include <app/reporting/reporting.h>

#include <app/SafeAttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TypeTraits.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

using VideoStreamStruct            = Structs::VideoStreamStruct::Type;
using AudioStreamStruct            = Structs::AudioStreamStruct::Type;
using SnapshotStreamStruct         = Structs::SnapshotStreamStruct::Type;
using AudioCapabilitiesStruct      = Structs::AudioCapabilitiesStruct::Type;
using VideoSensorParamsStruct      = Structs::VideoSensorParamsStruct::Type;
using SnapshotCapabilitiesStruct   = Structs::SnapshotCapabilitiesStruct::Type;
using VideoResolutionStruct        = Structs::VideoResolutionStruct::Type;
using RateDistortionTradeOffStruct = Structs::RateDistortionTradeOffPointsStruct::Type;
using StreamUsageEnum              = Globals::StreamUsageEnum;

constexpr uint8_t kMaxSpeakerLevel              = 254;
constexpr uint8_t kMaxMicrophoneLevel           = 254;
constexpr uint16_t kMaxImageRotationDegrees     = 359;
constexpr uint8_t kMaxChannelCount              = 8;
constexpr uint8_t kMaxImageQualityMetric        = 100;
constexpr uint16_t kMaxKeyFrameIntervalMaxValue = 65500;
// Conservative room for other fields (resolution + codec) in
// capture snapshot response. TODO: Make a tighter bound.
constexpr size_t kMaxSnapshotImageSize = kMaxLargeSecureSduLengthBytes - 100;

constexpr size_t kViewportStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t));

// The number of possible values of StreamUsageEnum.
constexpr size_t kNumOfStreamUsageTypes = 4;

// StreamUsageEnum + Anonymous tag (1 byte).
// Assumes min-size encoding (1 byte) for the integer.
constexpr size_t kStreamUsageTlvSize = sizeof(Globals::StreamUsageEnum) + 1;

// 1 control byte + end-of-array marker
constexpr size_t kArrayTlvOverhead = 2;

constexpr size_t kStreamUsagePrioritiesTlvSize = kArrayTlvOverhead + kStreamUsageTlvSize * kNumOfStreamUsageTypes;

enum class StreamAllocationAction
{
    kNewAllocation, // Fresh stream allocation - always start
    kModification,  // Existing stream with parameter changes - restart if active
    kReuse          // Reusing existing stream without changes - no action needed
};

class CameraAVStreamMgmtServer;

// ImageSnapshot response data for a CaptureSnapshot command.
struct ImageSnapshot
{
    std::vector<uint8_t> data;      // Buffer to hold the image data
    VideoResolutionStruct imageRes; // Image resolution
    ImageCodecEnum imageCodec;      // Image codec used
};

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the CameraAvStreamManagement Cluster.
 *  Specifically, it defines interfaces for the command handling and loading of the allocated streams.
 */
class CameraAVStreamMgmtDelegate
{
public:
    CameraAVStreamMgmtDelegate() = default;

    virtual ~CameraAVStreamMgmtDelegate() = default;

    /**
     *   @brief Handle Command Delegate for Video stream allocation with the provided parameter list.
     *
     *   @param[in]  allocateArgs   Structure with parameters for video stream allocation.
     *                              The videoStreamID and referenceCount fields in the struct
     *                              must be ignored by the callee.
     *
     *   @param[out] outStreamID    Indicates the ID of the allocated Video Stream.
     *
     *   @return Success if the allocation is successful and a VideoStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                                    uint16_t & outStreamID) = 0;

    /**
     *   @brief Called after the server has finalized video stream allocation and narrowed parameters.
     *          This is where the actual video stream should be started using the final allocated parameters.
     *
     *   @param allocatedStream   The finalized video stream with narrowed parameters from the server.
     *   @param action           Action indicating how to handle the stream: new allocation, modification, or reuse.
     */
    virtual void OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream modification.
     *
     *   @param streamID           Indicates the streamID of the video stream to modify.
     *
     *   @param waterMarkEnabled   Indicates  whether a watermark can be applied on the video stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @param osdEnabled         Indicates  whether the on-screen display can be applied on the video stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @return Success if the stream modification is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                                  const Optional<bool> osdEnabled) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream deallocation for the
     *   provided streamID.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success       Stream shall be de-allocated, if found.
     *
     */
    virtual Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream allocation.
     *
     *   @param[in]  allocateArgs   Structure with parameters for audio stream allocation.
     *                              The audioStreamID and referenceCount fields in the struct
     *                              must be ignored by the callee.
     *
     *   @param[out] outStreamID    Indicates the ID of the allocated Audio Stream.
     *
     *   @return Success if the allocation is successful and an AudioStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                                    uint16_t & outStreamID) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream deallocation.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success       Stream shall be de-allocated, if found.
     */
    virtual Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream allocation.
     *
     *   @param[in]  allocateArgs   Structure with parameters for snapshot stream allocation.
     *                              The snapshotStreamID and referenceCount fields in the struct
     *                              must be ignored by the callee.
     *
     *   @param[out] outStreamID    Indicates the ID of the allocated Audio Stream.
     *
     *
     *   @return Success if the allocation is successful and a SnapshotStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                                       uint16_t & outStreamID) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream modification.
     *
     *   @param streamID           Indicates the streamID of the snapshot stream to modify.
     *
     *   @param waterMarkEnabled   Indicates  whether a watermark can be applied on the snapshot stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @param osdEnabled         Indicates  whether the on-screen display can be applied on the snapshot stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @return Success if the stream modification is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                                     const Optional<bool> osdEnabled) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream deallocation.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success       Stream shall be de-allocated, if found.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Command Delegate for notifying change in StreamPriorities.
     *
     */
    virtual void OnStreamUsagePrioritiesChanged() = 0;

    /**
     *   @brief Delegate callback for notifying change in an attribute.
     *
     */
    virtual void OnAttributeChanged(AttributeId attributeId) = 0;

    /**
     *   @brief Handle Command Delegate for CaptureSnapshot.
     *
     *   @param streamID          Indicates the streamID representing the shapshot stream.
     *
     *   @param videoResolution   Indicates the preferred resolution of the snapshot image.
     *
     *   @return Success if the processing of the Command is successful; otherwise, the command SHALL be rejected with an
     * appropriate error.
     */
    virtual Protocols::InteractionModel::Status CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID,
                                                                const VideoResolutionStruct & resolution,
                                                                ImageSnapshot & outImageSnapshot) = 0;

    /**
     *  Delegate functions to load the allocated video, audio, and snapshot streams.
     *  The delegate application is responsible for creating and persisting
     *  these streams (based on the Allocation commands). These Load APIs would be
     *  used to load the pre-allocated stream context information into the cluster server list,
     *  at initialization.
     *  Once loaded, the cluster server would be serving Reads on these
     *  attributes. The list is updatable via the Add/Remove functions for the
     *  respective streams.
     */
    virtual CHIP_ERROR LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams) = 0;

    virtual CHIP_ERROR LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams) = 0;

    virtual CHIP_ERROR LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;

    /**
     * @brief Called by transports when they start using the corresponding audio and video streams.
     *
     */
    virtual CHIP_ERROR OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) = 0;

    /**
     * @brief Called by transports when they release the corresponding audio and video streams.
     *
     */
    virtual CHIP_ERROR OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) = 0;

private:
    friend class CameraAVStreamMgmtServer;

    CameraAVStreamMgmtServer * mCameraAVStreamMgmtServer = nullptr;

    /**
     * This method is used by the SDK to ensure the delegate points to the server instance it's associated with.
     * When a server instance is created or destroyed, this method will be called to set and clear, respectively,
     * the pointer to the server instance.
     *
     * @param aCameraAVStreamMgmtServer A pointer to the CameraAVStreamMgmtServer object related to this delegate object.
     */
    void SetCameraAVStreamMgmtServer(CameraAVStreamMgmtServer * aCameraAVStreamMgmtServer)
    {
        mCameraAVStreamMgmtServer = aCameraAVStreamMgmtServer;
    }

protected:
    CameraAVStreamMgmtServer * GetCameraAVStreamMgmtServer() const { return mCameraAVStreamMgmtServer; }
};

enum class OptionalAttribute : uint32_t
{
    kHardPrivacyModeOn     = 0x0001,
    kNightVisionIllum      = 0x0002,
    kMicrophoneAGCEnabled  = 0x0004,
    kImageRotation         = 0x0008,
    kImageFlipHorizontal   = 0x0010,
    kImageFlipVertical     = 0x0020,
    kStatusLightEnabled    = 0x0040,
    kStatusLightBrightness = 0x0080,
};

class CameraAVStreamMgmtServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * @brief Creates a Camera AV Stream Management cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     *
     * @param aDelegate                         A pointer to the delegate to be used by this server.
     *                                          Note: the caller must ensure that the delegate lives throughout the instance's
     *                                          lifetime.
     *
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures                         The bitflags value that identifies which features are supported by this instance.
     * @param aOptionalAttrs                    The bitflags value that identifies the optional attributes supported by this
     *                                          instance.
     * @param aMaxConcurrentEncoders            The maximum number of video encoders supported by camera.
     * @param aMaxEncodedPixelRate              The maximum data rate (encoded pixels/sec) supported by camera.
     * @param aVideoSensorParams                The set of video sensor parameters for the camera.
     * @param aNightVisionUsesInfrared          Indicates whether nightvision mode does or does not use infrared
     * @param aMinViewPort                      Indicates minimum resolution (width/height) in pixels allowed for camera viewport.
     * @param aRateDistortionTradeOffPoints     Indicates the list of rate distortion trade-off points for supported hardware
     *                                          encoders.
     * @param aMaxContentBufferSize             The maximum size of the content buffer containing data for all streams, including
     *                                          pre-roll.
     * @param aMicrophoneCapabilities           Indicates the audio capabilities of the speaker in terms of the codec used,
     *                                          supported sample rates and the number of channels.
     * @param aSpkrCapabilities                 Indicates the audio capabilities of the speaker in terms of the codec used,
     *                                          supported sample rates and the number of channels.
     * @param aTwoWayTalkSupport                Indicates the type of two-way talk support the device has, e.g., half-duplex,
     *                                          full-duplex, etc.
     * @param aSnapshotCapabilities             Indicates the set of supported snapshot capabilities by the device, e.g., the image
     *                                          codec, the resolution and the maximum frame rate.
     * @param aMaxNetworkBandwidth              Indicates the maximum network bandwidth (in mbps) that the device would consume
     * @param aSupportedStreamUsages            Indicates the possible stream types available
     * @param aStreamUsagePriorities            Indicates the priority ranking of the available streams
     * for the transmission of its media streams.
     *
     */
    CameraAVStreamMgmtServer(CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                             const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxConcurrentEncoders,
                             uint32_t aMaxEncodedPixelRate, const VideoSensorParamsStruct & aVideoSensorParams,
                             bool aNightVisionUsesInfrared, const VideoResolutionStruct & aMinViewPort,
                             const std::vector<RateDistortionTradeOffStruct> & aRateDistortionTradeOffPoints,
                             uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
                             const AudioCapabilitiesStruct & aSpkrCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
                             const std::vector<SnapshotCapabilitiesStruct> & aSnapshotCapabilities, uint32_t aMaxNetworkBandwidth,
                             const std::vector<Globals::StreamUsageEnum> & aSupportedStreamUsages,
                             const std::vector<Globals::StreamUsageEnum> & aStreamUsagePriorities);

    ~CameraAVStreamMgmtServer() override;

    /**
     * @brief Initialise the Camera AV Stream Management server instance.
     * This function must be called after defining an CameraAVStreamMgmtServer class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will return CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    bool HasFeature(Feature feature) const;

    bool SupportsOptAttr(OptionalAttribute aOptionalAttr) const;

    bool IsLocalVideoRecordingEnabled() const;

    // Attribute Setters
    CHIP_ERROR SetCurrentFrameRate(uint16_t aCurrentFrameRate);

    CHIP_ERROR SetHDRModeEnabled(bool aHDRModeEnabled);

    CHIP_ERROR SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled);

    CHIP_ERROR SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled);

    CHIP_ERROR SetHardPrivacyModeOn(bool aHardPrivacyModeOn);

    CHIP_ERROR SetNightVisionUsesInfrared(bool aNightVisionUsesInfrared);

    CHIP_ERROR SetNightVision(TriStateAutoEnum aNightVision);

    CHIP_ERROR SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum);

    CHIP_ERROR SetViewport(const Globals::Structs::ViewportStruct::Type & aViewport);

    CHIP_ERROR SetSpeakerMuted(bool aSpeakerMuted);

    CHIP_ERROR SetSpeakerVolumeLevel(uint8_t aSpeakerVolumeLevel);

    CHIP_ERROR SetSpeakerMaxLevel(uint8_t aSpeakerMaxLevel);

    CHIP_ERROR SetSpeakerMinLevel(uint8_t aSpeakerMinLevel);

    CHIP_ERROR SetMicrophoneMuted(bool aMicrophoneMuted);

    CHIP_ERROR SetMicrophoneVolumeLevel(uint8_t aMicrophoneVolumeLevel);

    CHIP_ERROR SetMicrophoneMaxLevel(uint8_t aMicrophoneMaxLevel);

    CHIP_ERROR SetMicrophoneMinLevel(uint8_t aMicrophoneMinLevel);

    CHIP_ERROR SetMicrophoneAGCEnabled(bool aMicrophoneAGCEnabled);

    CHIP_ERROR SetImageRotation(uint16_t aImageRotation);

    CHIP_ERROR SetImageFlipHorizontal(bool aImageFlipVertical);

    CHIP_ERROR SetImageFlipVertical(bool aImageFlipVertical);

    CHIP_ERROR SetLocalVideoRecordingEnabled(bool aLocalVideoRecordingEnabled);

    CHIP_ERROR SetLocalSnapshotRecordingEnabled(bool aLocalVideoRecordingEnabled);

    CHIP_ERROR SetStatusLightEnabled(bool aStatusLightEnabled);

    CHIP_ERROR SetStatusLightBrightness(Globals::ThreeLevelAutoEnum aStatusLightBrightness);

    // Attribute Getters
    uint8_t GetMaxConcurrentEncoders() const { return mMaxConcurrentEncoders; }

    uint32_t GetMaxEncodedPixelRate() const { return mMaxEncodedPixelRate; }

    const VideoSensorParamsStruct & GetVideoSensorParams() const { return mVideoSensorParams; }

    bool GetNightVisionUsesInfrared() const { return mNightVisionUsesInfrared; }

    const VideoResolutionStruct & GetMinViewportResolution() const { return mMinViewPortResolution; }

    const std::vector<RateDistortionTradeOffStruct> & GetRateDistortionTradeOffPoints() const
    {
        return mRateDistortionTradeOffPointsList;
    }

    uint32_t GetMaxContentBufferSize() const { return mMaxContentBufferSize; }

    const AudioCapabilitiesStruct & GetMicrophoneCapabilities() const { return mMicrophoneCapabilities; }

    const AudioCapabilitiesStruct & GetSpeakerCapabilities() const { return mSpeakerCapabilities; }

    TwoWayTalkSupportTypeEnum GetTwoWayTalkSupport() const { return mTwoWayTalkSupport; }

    const std::vector<SnapshotCapabilitiesStruct> & GetSnapshotCapabilities() const { return mSnapshotCapabilitiesList; }

    uint32_t GetMaxNetworkBandwidth() const { return mMaxNetworkBandwidth; }

    uint16_t GetCurrentFrameRate() const { return mCurrentFrameRate; }

    bool GetHDRModeEnabled() const { return mHDRModeEnabled; }

    const std::vector<Globals::StreamUsageEnum> & GetSupportedStreamUsages() const { return mSupportedStreamUsages; }

    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const { return mAllocatedVideoStreams; }

    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const { return mAllocatedAudioStreams; }

    const std::vector<SnapshotStreamStruct> & GetAllocatedSnapshotStreams() const { return mAllocatedSnapshotStreams; }

    const std::vector<Globals::StreamUsageEnum> & GetStreamUsagePriorities() const { return mStreamUsagePriorities; }

    bool GetSoftRecordingPrivacyModeEnabled() const { return mSoftRecordingPrivacyModeEnabled; }

    bool GetSoftLivestreamPrivacyModeEnabled() const { return mSoftLivestreamPrivacyModeEnabled; }

    bool GetHardPrivacyModeOn() const { return mHardPrivacyModeOn; }

    TriStateAutoEnum GetNightVision() const { return mNightVision; }

    TriStateAutoEnum GetNightVisionIllum() const { return mNightVisionIllum; }

    const Globals::Structs::ViewportStruct::Type & GetViewport() const { return mViewport; }

    bool GetSpeakerMuted() const { return mSpeakerMuted; }

    uint8_t GetSpeakerVolumeLevel() const { return mSpeakerVolumeLevel; }

    uint8_t GetSpeakerMaxLevel() const { return mSpeakerMaxLevel; }

    uint8_t GetSpeakerMinLevel() const { return mSpeakerMinLevel; }

    bool GetMicrophoneMuted() const { return mMicrophoneMuted; }

    uint8_t GetMicrophoneVolumeLevel() const { return mMicrophoneVolumeLevel; }

    uint8_t GetMicrophoneMaxLevel() const { return mMicrophoneMaxLevel; }

    uint8_t GetMicrophoneMinLevel() const { return mMicrophoneMinLevel; }

    bool IsMicrophoneAGCEnabled() const { return mMicrophoneAGCEnabled; }

    uint16_t GetImageRotation() const { return mImageRotation; }

    bool GetImageFlipHorizontal() const { return mImageFlipHorizontal; }

    bool GetImageFlipVertical() const { return mImageFlipVertical; }

    bool GetLocalVideoRecordingEnabled() const { return mLocalVideoRecordingEnabled; }

    bool GetLocalSnapshotRecordingEnabled() const { return mLocalSnapshotRecordingEnabled; }

    bool GetStatusLightEnabled() const { return mStatusLightEnabled; }

    Globals::ThreeLevelAutoEnum GetStatusLightBrightness() const { return mStatusLightBrightness; }

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

    // Add/Remove Management functions for streams

    CHIP_ERROR SetStreamUsagePriorities(const std::vector<Globals::StreamUsageEnum> & newPriorities);

    CHIP_ERROR AddVideoStream(const VideoStreamStruct & videoStream);

    CHIP_ERROR UpdateVideoStreamRangeParams(VideoStreamStruct & videoStreamToUpdate, const VideoStreamStruct & videoStream,
                                            bool & wasModified);

    CHIP_ERROR RemoveVideoStream(uint16_t videoStreamId);

    CHIP_ERROR AddAudioStream(const AudioStreamStruct & audioStream);

    CHIP_ERROR RemoveAudioStream(uint16_t audioStreamId);

    CHIP_ERROR AddSnapshotStream(const SnapshotStreamStruct & snapshotStream);

    CHIP_ERROR UpdateSnapshotStreamRangeParams(SnapshotStreamStruct & snapshotStreamToUpdate,
                                               const SnapshotStreamStruct & snapshotStream);

    CHIP_ERROR RemoveSnapshotStream(uint16_t snapshotStreamId);

    CHIP_ERROR UpdateVideoStreamRefCount(uint16_t videoStreamId, bool shouldIncrement);

    CHIP_ERROR UpdateAudioStreamRefCount(uint16_t audioStreamId, bool shouldIncrement);

    CHIP_ERROR UpdateSnapshotStreamRefCount(uint16_t snapshotStreamId, bool shouldIncrement);

private:
    CameraAVStreamMgmtDelegate & mDelegate;
    EndpointId mEndpointId;
    const BitFlags<Feature> mFeatures;
    const BitFlags<OptionalAttribute> mOptionalAttrs;

    // Attributes
    const uint8_t mMaxConcurrentEncoders;
    const uint32_t mMaxEncodedPixelRate;
    const VideoSensorParamsStruct mVideoSensorParams;
    const bool mNightVisionUsesInfrared;
    const VideoResolutionStruct mMinViewPortResolution;
    const std::vector<RateDistortionTradeOffStruct> mRateDistortionTradeOffPointsList;
    const uint32_t mMaxContentBufferSize;
    const AudioCapabilitiesStruct mMicrophoneCapabilities;
    const AudioCapabilitiesStruct mSpeakerCapabilities;
    const TwoWayTalkSupportTypeEnum mTwoWayTalkSupport;
    const std::vector<SnapshotCapabilitiesStruct> mSnapshotCapabilitiesList;
    const uint32_t mMaxNetworkBandwidth;

    uint16_t mCurrentFrameRate                       = 0;
    bool mHDRModeEnabled                             = false;
    bool mSoftRecordingPrivacyModeEnabled            = false;
    bool mSoftLivestreamPrivacyModeEnabled           = false;
    bool mHardPrivacyModeOn                          = false;
    TriStateAutoEnum mNightVision                    = TriStateAutoEnum::kOn;
    TriStateAutoEnum mNightVisionIllum               = TriStateAutoEnum::kOn;
    Globals::Structs::ViewportStruct::Type mViewport = { 0, 0, 0, 0 };
    bool mSpeakerMuted                               = false;
    uint8_t mSpeakerVolumeLevel                      = 0;
    uint8_t mSpeakerMaxLevel                         = kMaxSpeakerLevel;
    uint8_t mSpeakerMinLevel                         = 0;
    bool mMicrophoneMuted                            = false;
    uint8_t mMicrophoneVolumeLevel                   = 0;
    uint8_t mMicrophoneMaxLevel                      = kMaxMicrophoneLevel;
    uint8_t mMicrophoneMinLevel                      = 0;
    bool mMicrophoneAGCEnabled                       = false;
    uint16_t mImageRotation                          = 0;
    bool mImageFlipHorizontal                        = false;
    bool mImageFlipVertical                          = false;
    bool mLocalVideoRecordingEnabled                 = false;
    bool mLocalSnapshotRecordingEnabled              = false;
    bool mStatusLightEnabled                         = false;

    Globals::ThreeLevelAutoEnum mStatusLightBrightness = Globals::ThreeLevelAutoEnum::kMedium;

    // Managed lists
    std::vector<Globals::StreamUsageEnum> mSupportedStreamUsages;

    std::vector<Globals::StreamUsageEnum> mStreamUsagePriorities;
    std::vector<VideoStreamStruct> mAllocatedVideoStreams;
    std::vector<AudioStreamStruct> mAllocatedAudioStreams;
    std::vector<SnapshotStreamStruct> mAllocatedSnapshotStreams;

    // Utility function to set and persist attributes
    template <typename T>
    CHIP_ERROR SetAttributeIfDifferent(T & currentValue, const T & newValue, AttributeId attributeId, bool shouldPersist = true)
    {
        if (currentValue != newValue)
        {
            currentValue = newValue;
            auto path    = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, attributeId);
            if (shouldPersist)
            {
                ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, currentValue));
            }
            mDelegate.OnAttributeChanged(attributeId);
            MatterReportingAttributeChangeCallback(path);
        }
        return CHIP_NO_ERROR;
    }

    bool IsBitDepthValid(uint8_t bitDepth) { return (bitDepth == 8 || bitDepth == 16 || bitDepth == 24 || bitDepth == 32); }

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * IM-level implementation of write
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    // Helpers to read list items via delegate APIs
    CHIP_ERROR ReadAndEncodeRateDistortionTradeOffPoints(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeSnapshotCapabilities(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeSupportedStreamUsages(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeStreamUsagePriorities(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR StoreViewport(const Globals::Structs::ViewportStruct::Type & viewport);
    CHIP_ERROR LoadViewport(Globals::Structs::ViewportStruct::Type & viewport);

    CHIP_ERROR StoreStreamUsagePriorities();
    CHIP_ERROR LoadStreamUsagePriorities();

    void ModifyVideoStream(const uint16_t streamID, const Optional<bool> waterMarkEnabled, const Optional<bool> osdEnabled);

    void ModifySnapshotStream(const uint16_t streamID, const Optional<bool> waterMarkEnabled, const Optional<bool> osdEnabled);

    bool StreamPrioritiesHasDuplicates(const std::vector<Globals::StreamUsageEnum> & aStreamUsagePriorities);

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    void HandleVideoStreamAllocate(HandlerContext & ctx, const Commands::VideoStreamAllocate::DecodableType & req);

    void HandleVideoStreamModify(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    void HandleVideoStreamDeallocate(HandlerContext & ctx, const Commands::VideoStreamDeallocate::DecodableType & req);

    void HandleAudioStreamAllocate(HandlerContext & ctx, const Commands::AudioStreamAllocate::DecodableType & req);

    void HandleAudioStreamDeallocate(HandlerContext & ctx, const Commands::AudioStreamDeallocate::DecodableType & req);

    void HandleSnapshotStreamAllocate(HandlerContext & ctx, const Commands::SnapshotStreamAllocate::DecodableType & req);

    void HandleSnapshotStreamModify(HandlerContext & ctx, const Commands::SnapshotStreamModify::DecodableType & req);

    void HandleSnapshotStreamDeallocate(HandlerContext & ctx, const Commands::SnapshotStreamDeallocate::DecodableType & req);

    void HandleSetStreamPriorities(HandlerContext & ctx, const Commands::SetStreamPriorities::DecodableType & req);

    void HandleCaptureSnapshot(HandlerContext & ctx, const Commands::CaptureSnapshot::DecodableType & req);

    bool CheckSnapshotStreamsAvailability(HandlerContext & ctx);

    bool ValidateSnapshotStreamId(const DataModel::Nullable<uint16_t> & snapshotStreamID, HandlerContext & ctx);
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
