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
#include <app/reporting/reporting.h>

#include <app/SafeAttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TypeTraits.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

using VideoStreamStruct            = Structs::VideoStreamStruct::Type;
using AudioStreamStruct            = Structs::AudioStreamStruct::Type;
using SnapshotStreamStruct         = Structs::SnapshotStreamStruct::Type;
using AudioCapabilitiesStruct      = Structs::AudioCapabilitiesStruct::Type;
using VideoSensorParamsStruct      = Structs::VideoSensorParamsStruct::Type;
using SnapshotParamsStruct         = Structs::SnapshotParamsStruct::Type;
using VideoResolutionStruct        = Structs::VideoResolutionStruct::Type;
using ViewportStruct               = Structs::ViewportStruct::Type;
using RateDistortionTradeOffStruct = Structs::RateDistortionTradeOffPointsStruct::Type;
using SnapshotParamsStruct         = Structs::SnapshotParamsStruct::Type;

constexpr uint8_t kMaxSpeakerLevel          = 254;
constexpr uint8_t kMaxMicrophoneLevel       = 254;
constexpr uint16_t kMaxImageRotationDegrees = 359;
constexpr uint8_t kMaxChannelCount          = 8;
constexpr uint8_t kMaxImageQualityMetric    = 100;
constexpr uint16_t kMaxFragmentLenMaxValue  = 65500;

constexpr size_t kViewportStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t));

// The number of possible values of StreamUsageEnum.
constexpr size_t kNumOfStreamUsageTypes = 4;

// StreamUsageEnum + Anonymous tag ( 1 byte ).
// Assumes min-size encoding ( 1 byte ) for the integer.
constexpr size_t kStreamUsageTlvSize = sizeof(StreamUsageEnum) + 1;

// 1 control byte + end-of-array marker
constexpr size_t kArrayTlvOverhead = 2;

constexpr size_t kRankedVideoStreamPrioritiesTlvSize = kArrayTlvOverhead + kStreamUsageTlvSize * kNumOfStreamUsageTypes;

class CameraAVStreamMgmtServer;

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
     *   @param streamUsage        Indicates the type of usage of stream ( Recording, Liveview, etc ) that this allocation is for.
     *
     *   @param videoCodec         Indicates the type of video codec the stream should support.
     *
     *   @param minFrameRate       Indicates the minimum frame rate ( frames/second ) of the video stream.
     *
     *   @param maxFrameRate       Indicates the maximum frame rate ( frames/second ) of the video stream.
     *
     *   @param minResolution      Indicates the minimum resolution of the video stream.
     *
     *   @param maxResolution      Indicates the maximum resolution of the video stream.
     *
     *   @param minBitRate         Indicates the minimum bit rate ( bits/second ) of the video stream.
     *
     *   @param maxBitRate         Indicates the maximum bit rate ( bits/second ) of the video stream.
     *
     *   @param minFragmentLen     Indicates the minimum length ( msecs ) of a clip fragment for the video stream.
     *
     *   @param maxFragmentLen     Indicates the maximum length ( msecs ) of a clip fragment for the video stream.
     *
     *   @param waterMarkEnabled   Indicates  whether a watermark can be applied on the video stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @param osdEnabled         Indicates  whether the on-screen display can be applied on the video stream.
     *                             Value defaults to false if feature unsupported.
     *
     *   @param outStreamID        Indicates the ID of the allocated Video Stream.
     *
     *   @return Success if the allocation is successful and a VideoStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status
    VideoStreamAllocate(StreamUsageEnum streamUsage, VideoCodecEnum videoCodec, const uint16_t minFrameRate,
                        const uint16_t maxFrameRate, const VideoResolutionStruct & minResolution,
                        const VideoResolutionStruct & maxResolution, const uint32_t minBitRate, const uint32_t maxBitRate,
                        const uint16_t minFragmentLen, const uint16_t maxFragmentLen, const chip::Optional<bool> waterMarkEnabled,
                        const chip::Optional<bool> osdEnabled, uint16_t & outStreamID) = 0;

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
    virtual Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID,
                                                                  const chip::Optional<bool> waterMarkEnabled,
                                                                  const chip::Optional<bool> osdEnabled) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream deallocation for the
     *   provided streamID.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success if the stream deallocation is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     *
     */
    virtual Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream allocation.
     *
     *   @param streamUsage        Indicates the type of usage of stream ( Recording, Liveview, etc ) that this allocation is for.
     *
     *   @param audioCodec         Indicates the type of audio codec the stream should support.
     *
     *   @param channelCount       Indicates the the number of channels used by the stream, e.g., Mono ( 1 ), Stereo ( 2 ), etc.
     *
     *   @param sampleRate         Indicates the sampling rate of the audio stream in Hz.
     *
     *   @param bitRate            Indicates the bitrate ( bits/sec ) of the specified audio codec.
     *
     *   @param bitDepth           Indicates the number of information bits ( 8, 16, 24 or 32 ) used to represent each sample.
     *
     *   @param outStreamID        Indicates the ID of the allocated Audio Stream.
     *
     *   @return Success if the allocation is successful and an AudioStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status AudioStreamAllocate(StreamUsageEnum streamUsage, AudioCodecEnum audioCodec,
                                                                    const uint8_t channelCount, const uint32_t sampleRate,
                                                                    const uint32_t bitRate, const uint8_t bitDepth,
                                                                    uint16_t & outStreamID) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream deallocation.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success if the stream deallocation is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream allocation.
     *
     *   @param imageCodec          Indicates the type of image codec to be used by the stream.
     *
     *   @param frameRate           Indicates the frame rate ( frames/second ) of the stream.
     *
     *   @param bitRate             Indicates the bitrate ( bits/sec ) of the stream.
     *
     *   @param minResolution       Indicates the minimum resolution of the stream.
     *
     *   @param maxResolution       Indicates the maximum resolution of the stream.
     *
     *   @param quality             Indicates a codec quality metric ( integer between 1 and 100 ) for the stream.
     *
     *   @param outStreamID         Indicates the ID of the allocated Audio Stream.
     *
     *
     *   @return Success if the allocation is successful and a SnapshotStreamID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamAllocate(ImageCodecEnum imageCodec, const uint16_t frameRate,
                                                                       const uint32_t bitRate,
                                                                       const VideoResolutionStruct & minResolution,
                                                                       const VideoResolutionStruct & maxResolution,
                                                                       const uint8_t quality, uint16_t & outStreamID) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream deallocation.
     *
     *   @param streamID       Indicates the streamID to deallocate.
     *
     *   @return Success if the stream deallocation is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Command Delegate for notifying change in StreamPriorities.
     *
     */
    virtual void OnRankedStreamPrioritiesChanged() = 0;

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
    virtual Protocols::InteractionModel::Status CaptureSnapshot(const uint16_t streamID,
                                                                const VideoResolutionStruct & videoResolution) = 0;

    /**
     *  Delegate functions to load the allocated video, audio, and snapshot streams.
     *  The delegate application is responsible for creating and persisting
     *  these streams ( based on the Allocation commands ). These Load APIs would be
     *  used to load the pre-allocated stream context information into the cluster server list,
     *  at initialization.
     *  Once loaded, the cluster server would be serving Reads on these
     *  attributes. The list is updatable via the Add/Remove functions for the
     *  respective streams.
     */
    virtual Protocols::InteractionModel::Status
    LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams) = 0;

    virtual Protocols::InteractionModel::Status
    LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams) = 0;

    virtual Protocols::InteractionModel::Status
    LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() = 0;

private:
    friend class CameraAVStreamMgmtServer;

    CameraAVStreamMgmtServer * mCameraAVStreamMgmtServer = nullptr;

    /**
     * This method is used by the SDK to set the CameraAVStreamMgmtServer pointer member in the delegate.
     * This is done in the ctor during the instantiation of the CameraAVStreamMgmtServer object.
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
    kHDRModeEnabled        = 0x0001,
    kHardPrivacyModeOn     = 0x0002,
    kNightVision           = 0x0004,
    kNightVisionIllum      = 0x0008,
    kMicrophoneAGCEnabled  = 0x0010,
    kImageRotation         = 0x0020,
    kImageFlipHorizontal   = 0x0040,
    kImageFlipVertical     = 0x0080,
    kStatusLightEnabled    = 0x0100,
    kStatusLightBrightness = 0x0200,
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
     * @param aFeature                          The bitflags value that identifies which features are supported by this instance.
     * @param aOptionalAttrs                    The bitflags value that identifies the optional attributes supported by this
     *                                          instance.
     * @param aPersistentStorage                The storage delegate to use to persist attributes.
     * @param aMaxConcurrentVideoEncoders       The maximum number of video encoders supported by camera.
     * @param aMaxEncodedPixelRate              The maximum data rate ( encoded pixels/dec ) supported by camera.
     * @param aVideoSensorParams                The set of video sensor parameters for the camera.
     * @param aNightVisionCapable               Indicates whether the camera supports night vision.
     * @param aMinViewPort                      Indicates minimum resolution ( width/height ) in pixels allowed for camera viewport.
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
     * @param aSupportedSnapshotParams          Indicates the set of supported snapshot parameters by the device, e.g., the image
     *                                          codec, the resolution and the maximum frame rate.
     * @param aMaxNetworkBandwidth              Indicates the maximum network bandwidth ( in mbps ) that the device would consume for
     *                                          the transmission of its media streams.
     *
     */
    CameraAVStreamMgmtServer(CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeature,
                             const BitFlags<OptionalAttribute> aOptionalAttrs, PersistentStorageDelegate & aPersistentStorage,
                             uint8_t aMaxConcurrentVideoEncoders, uint32_t aMaxEncodedPixelRate,
                             const VideoSensorParamsStruct & aVideoSensorParams, bool aNightVisionCapable,
                             const VideoResolutionStruct & aMinViewPort,
                             const std::vector<RateDistortionTradeOffStruct> & aRateDistortionTradeOffPoints,
                             uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
                             const AudioCapabilitiesStruct & aSpkrCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
                             const std::vector<SnapshotParamsStruct> & aSupportedSnapshotParams, uint32_t aMaxNetworkBandwidth);

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

    CHIP_ERROR SetNightVision(TriStateAutoEnum aNightVision);

    CHIP_ERROR SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum);

    CHIP_ERROR SetViewport(const ViewportStruct & aViewport);

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
    uint8_t GetMaxConcurrentVideoEncoders() const { return mMaxConcurrentVideoEncoders; }

    uint32_t GetMaxEncodedPixelRate() const { return mMaxEncodedPixelRate; }

    const VideoSensorParamsStruct & GetVideoSensorParams() const { return mVideoSensorParams; }

    bool GetNightVisionCapable() const { return mNightVisionCapable; }

    const VideoResolutionStruct & GetMinViewport() const { return mMinViewPort; }

    const std::vector<RateDistortionTradeOffStruct> & GetRateDistortionTradeOffPoints() const
    {
        return mRateDistortionTradeOffPointsList;
    }

    uint32_t GetMaxContentBufferSize() const { return mMaxContentBufferSize; }

    const AudioCapabilitiesStruct & GetMicrophoneCapabilities() const { return mMicrophoneCapabilities; }

    const AudioCapabilitiesStruct & GetSpeakerCapabilities() const { return mSpeakerCapabilities; }

    TwoWayTalkSupportTypeEnum GetTwoWayTalkSupport() const { return mTwoWayTalkSupport; }

    const std::vector<SnapshotParamsStruct> & GetSupportedSnapshotParams() const { return mSupportedSnapshotParamsList; }

    uint32_t GetMaxNetworkBandwidth() const { return mMaxNetworkBandwidth; }

    uint16_t GetCurrentFrameRate() const { return mCurrentFrameRate; }

    bool GetHDRModeEnabled() const { return mHDRModeEnabled; }

    const std::unordered_set<chip::FabricIndex> & GetFabricsUsingCamera() const { return mFabricsUsingCamera; }

    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const { return mAllocatedVideoStreams; }

    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const { return mAllocatedAudioStreams; }

    const std::vector<SnapshotStreamStruct> & GetAllocatedSnapshotStreams() const { return mAllocatedSnapshotStreams; }

    const StreamUsageEnum * GetRankedVideoStreamPriorities() const { return mRankedVideoStreamPriorities; }

    bool GetSoftRecordingPrivacyModeEnabled() const { return mSoftRecordingPrivacyModeEnabled; }

    bool GetSoftLivestreamPrivacyModeEnabled() const { return mSoftLivestreamPrivacyModeEnabled; }

    bool GetHardPrivacyModeOn() const { return mHardPrivacyModeOn; }

    TriStateAutoEnum GetNightVision() const { return mNightVision; }

    TriStateAutoEnum GetNightVisionIllum() const { return mNightVisionIllum; }

    const ViewportStruct & GetViewport() const { return mViewport; }

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
    CHIP_ERROR AddToFabricsUsingCamera(chip::FabricIndex aFabricIndex);

    CHIP_ERROR RemoveFromFabricsUsingCamera(chip::FabricIndex aFabricIndex);

    CHIP_ERROR SetRankedVideoStreamPriorities(const StreamUsageEnum newPriorities[kNumOfStreamUsageTypes]);

    CHIP_ERROR AddVideoStream(const VideoStreamStruct & videoStream);

    CHIP_ERROR RemoveVideoStream(uint16_t videoStreamId);

    CHIP_ERROR AddAudioStream(const AudioStreamStruct & audioStream);

    CHIP_ERROR RemoveAudioStream(uint16_t audioStreamId);

    CHIP_ERROR AddSnapshotStream(const SnapshotStreamStruct & snapshotStream);

    CHIP_ERROR RemoveSnapshotStream(uint16_t snapshotStreamId);

private:
    CameraAVStreamMgmtDelegate & mDelegate;
    EndpointId mEndpointId;
    const BitFlags<Feature> mFeature;
    const BitFlags<OptionalAttribute> mOptionalAttrs;
    PersistentStorageDelegate * mPersistentStorage = nullptr;

    // Attributes
    const uint8_t mMaxConcurrentVideoEncoders;
    const uint32_t mMaxEncodedPixelRate;
    const VideoSensorParamsStruct mVideoSensorParams;
    const bool mNightVisionCapable;
    const VideoResolutionStruct mMinViewPort;
    const std::vector<RateDistortionTradeOffStruct> mRateDistortionTradeOffPointsList;
    const uint32_t mMaxContentBufferSize;
    const AudioCapabilitiesStruct mMicrophoneCapabilities;
    const AudioCapabilitiesStruct mSpeakerCapabilities;
    const TwoWayTalkSupportTypeEnum mTwoWayTalkSupport;
    const std::vector<SnapshotParamsStruct> mSupportedSnapshotParamsList;
    const uint32_t mMaxNetworkBandwidth;

    uint16_t mCurrentFrameRate             = 0;
    bool mHDRModeEnabled                   = false;
    bool mSoftRecordingPrivacyModeEnabled  = false;
    bool mSoftLivestreamPrivacyModeEnabled = false;
    bool mHardPrivacyModeOn                = false;
    TriStateAutoEnum mNightVision          = TriStateAutoEnum::kOn;
    TriStateAutoEnum mNightVisionIllum     = TriStateAutoEnum::kOn;
    ViewportStruct mViewport               = { 0, 0, 0, 0 };
    bool mSpeakerMuted                     = false;
    uint8_t mSpeakerVolumeLevel            = 0;
    uint8_t mSpeakerMaxLevel               = kMaxSpeakerLevel;
    uint8_t mSpeakerMinLevel               = 0;
    bool mMicrophoneMuted                  = false;
    uint8_t mMicrophoneVolumeLevel         = 0;
    uint8_t mMicrophoneMaxLevel            = kMaxMicrophoneLevel;
    uint8_t mMicrophoneMinLevel            = 0;
    bool mMicrophoneAGCEnabled             = false;
    uint16_t mImageRotation                = 0;
    bool mImageFlipHorizontal              = false;
    bool mImageFlipVertical                = false;
    bool mLocalVideoRecordingEnabled       = false;
    bool mLocalSnapshotRecordingEnabled    = false;
    bool mStatusLightEnabled               = false;

    Globals::ThreeLevelAutoEnum mStatusLightBrightness = Globals::ThreeLevelAutoEnum::kMedium;

    // Managed lists
    std::unordered_set<chip::FabricIndex> mFabricsUsingCamera;

    StreamUsageEnum mRankedVideoStreamPriorities[kNumOfStreamUsageTypes];

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

    bool IsAudioCodecValid(AudioCodecEnum audioCodec)
    {
        return (audioCodec != AudioCodecEnum::kUnknownEnumValue);
    }

    bool IsVideoCodecValid(VideoCodecEnum videoCodec)
    {
        return (videoCodec != VideoCodecEnum::kUnknownEnumValue);
    }

    bool IsImageCodecValid(ImageCodecEnum imageCodec)
    {
        return (imageCodec != ImageCodecEnum::kUnknownEnumValue);
    }

    bool IsStreamUsageValid(StreamUsageEnum streamUsage)
    {
        return (streamUsage != StreamUsageEnum::kUnknownEnumValue);
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
    CHIP_ERROR ReadAndEncodeSupportedSnapshotParams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeFabricsUsingCamera(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeRankedVideoStreamPrioritiesList(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR StoreViewport(const ViewportStruct & viewport);
    CHIP_ERROR ClearViewport();
    CHIP_ERROR LoadViewport(ViewportStruct & viewport);

    CHIP_ERROR StoreRankedVideoStreamPriorities();
    CHIP_ERROR LoadRankedVideoStreamPriorities();
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

    void HandleSnapshotStreamDeallocate(HandlerContext & ctx, const Commands::SnapshotStreamDeallocate::DecodableType & req);

    void HandleSetStreamPriorities(HandlerContext & ctx, const Commands::SetStreamPriorities::DecodableType & req);

    void HandleCaptureSnapshot(HandlerContext & ctx, const Commands::CaptureSnapshot::DecodableType & req);
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
