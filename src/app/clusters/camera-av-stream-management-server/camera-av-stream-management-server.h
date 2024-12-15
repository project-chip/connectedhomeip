/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

using VideoStreamStruct       = Structs::VideoStreamStruct::Type;
using AudioStreamStruct       = Structs::AudioStreamStruct::Type;
using SnapshotStreamStruct    = Structs::SnapshotStreamStruct::Type;
using AudioCapabilitiesStruct = Structs::AudioCapabilitiesStruct::Type;
using VideoSensorParamsStruct = Structs::VideoSensorParamsStruct::Type;
using SnapshotParamsStruct    = Structs::SnapshotParamsStruct::Type;
using VideoResolutionStruct   = Structs::VideoResolutionStruct::Type;
using ViewportStruct          = Structs::ViewportStruct::Type;

constexpr uint8_t kMaxSpeakerLevel          = 254;
constexpr uint8_t kMaxMicrophoneLevel       = 254;
constexpr uint16_t kMaxImageRotationDegrees = 359;

constexpr size_t kViewportStructMaxSerializedSize = TLV::EstimateStructOverhead(sizeof(uint16_t) * 4);

constexpr size_t kNumOfStreamTypes = 4;

// StreamTypeEnum + Anonymous tag(1 byte)
constexpr size_t kStreamTypeTlvSize = sizeof(StreamTypeEnum) + 1;

// 1 control byte + end-of-array marker
constexpr size_t kArrayTlvOverhead = 2;

constexpr size_t kRankedVideoStreamPrioritiesTlvSize = kArrayTlvOverhead + kStreamTypeTlvSize * kNumOfStreamTypes;

class CameraAVStreamMgmtServer;

/** @brief
 *  Defines methods for implementing application-specific logic for the CameraAvStreamManagement Cluster.
 */
class CameraAVStreamMgmtDelegate
{
public:
    CameraAVStreamMgmtDelegate() = default;

    virtual ~CameraAVStreamMgmtDelegate() = default;

    /**
     *   @brief Handle Command Delegate for Video stream allocation.
     */
    virtual Protocols::InteractionModel::Status
    VideoStreamAllocate(StreamTypeEnum streamType, CameraAvStreamManagement::VideoCodecEnum videoCodec, const uint16_t minFrameRate,
                        const uint16_t maxFrameRate, VideoResolutionStruct minResolution, VideoResolutionStruct maxResolution,
                        const uint32_t minBitRate, const uint32_t maxBitRate, const uint16_t minFragmentLen,
                        const uint16_t maxFragmentLen, bool waterMarkEnabled, bool osdEnabled) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream modification.
     */
    virtual Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID,
                                                                  Optional<VideoResolutionStruct> videoResolution,
                                                                  bool waterMarkEnabled, bool osdEnabled) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream deallocation.
     */
    virtual Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream allocation.
     */
    virtual Protocols::InteractionModel::Status AudioStreamAllocate(StreamTypeEnum streamType, AudioCodecEnum audioCodec,
                                                                    const uint8_t channelCount, const uint32_t sampleRate,
                                                                    const uint32_t bitRate, const uint32_t bitDepth) = 0;

    /**
     *   @brief Handle Command Delegate for Audio stream deallocation.
     */
    virtual Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream allocation.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamAllocate(ImageCodecEnum imageCodec, const uint16_t frameRate,
                                                                       const uint32_t bitRate, VideoResolutionStruct minResolution,
                                                                       VideoResolutionStruct maxResolution,
                                                                       const uint8_t quality) = 0;

    /**
     *   @brief Handle Command Delegate for Snapshot stream deallocation.
     */
    virtual Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) = 0;

    /**
     *   @brief Handle Command Delegate for CaptureSnapshot.
     */
    virtual Protocols::InteractionModel::Status CaptureSnapshot(const uint16_t streamID, VideoResolutionStruct videoResolution) = 0;

    /**
     *   @brief Handle Command Delegate for SetStreamPriorities.
     */
    virtual Protocols::InteractionModel::Status
    SetStreamPriorities(const DataModel::DecodableList<StreamTypeEnum> streamPriorities) = 0;

    /**
     *  Delegate functions to load the allocated video, audio, and snapshot streams.
     *  The delegate application is responsible for creating and persisting
     *  these streams(based on the Allocation commands). These Load APIs would be
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

protected:
    friend class CameraAVStreamMgmtServer;

    CameraAVStreamMgmtServer * mCameraAVStreamMgmtServer = nullptr;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a
     * CameraAVStreamMgmtServer object.
     * @param aCameraAVStreamMgmtServer A pointer to the CameraAVStreamMgmtServer object related to this delegate object.
     */
    void SetCameraAVStreamMgmtServer(CameraAVStreamMgmtServer * aCameraAVStreamMgmtServer)
    {
        mCameraAVStreamMgmtServer = aCameraAVStreamMgmtServer;
    }

    CameraAVStreamMgmtServer * GetCameraAVStreamMgmtServer() const { return mCameraAVStreamMgmtServer; }
};

enum class OptionalAttributes : uint32_t
{
    kSupportsHDRModeEnabled        = 0x0001,
    kSupportsHardPrivacyModeOn     = 0x0002,
    kSupportsNightVision           = 0x0004,
    kSupportsNightVisionIllum      = 0x0008,
    kSupportsMicrophoneAGCEnabled  = 0x0010,
    kSupportsImageRotation         = 0x0020,
    kSupportsImageFlipHorizontal   = 0x0040,
    kSupportsImageFlipVertical     = 0x0080,
    kSupportsStatusLightEnabled    = 0x0100,
    kSupportsStatusLightBrightness = 0x0200,
};

class CameraAVStreamMgmtServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * @brief Creates a Camera AV Stream Management cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the Camera AV Stream Management cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    CameraAVStreamMgmtServer(CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                             BitMask<Feature> aFeature, OptionalAttributes aOptionalAttrs,
                             PersistentStorageDelegate & aPersistentStorage, uint8_t aMaxConcurrentVideoEncoders,
                             uint32_t aMaxEncodedPixelRate, const VideoSensorParamsStruct & aVideoSensorParams,
                             bool aNightVisionCapable, const VideoResolutionStruct & aMinViewPort,
                             const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> & aRateDistortionTradeOffPoints,
                             uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
                             const AudioCapabilitiesStruct & aSpkrCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
                             const std::vector<Structs::SnapshotParamsStruct::Type> & aSupportedSnapshotParams,
                             uint32_t aMaxNetworkBandwidth);

    ~CameraAVStreamMgmtServer() override;

    /**
     * @brief Initialise the Camera AV Stream Management server instance.
     * This function must be called after defining an CameraAVStreamMgmtServer class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will returns CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    bool HasFeature(Feature feature) const;

    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

    bool IsLocalVideoRecordingEnabled() const;

    // Attribute Setters
    Protocols::InteractionModel::Status SetCurrentFrameRate(uint16_t aCurrentFrameRate);

    Protocols::InteractionModel::Status SetHDRModeEnabled(bool aHDRModeEnabled);

    Protocols::InteractionModel::Status SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled);

    Protocols::InteractionModel::Status SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled);

    Protocols::InteractionModel::Status SetHardPrivacyModeOn(bool aHardPrivacyModeOn);

    Protocols::InteractionModel::Status SetNightVision(TriStateAutoEnum aNightVision);

    Protocols::InteractionModel::Status SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum);

    Protocols::InteractionModel::Status SetViewport(const ViewportStruct & aViewport);

    Protocols::InteractionModel::Status SetSpeakerMuted(bool aSpeakerMuted);

    Protocols::InteractionModel::Status SetSpeakerVolumeLevel(uint8_t aSpeakerVolumeLevel);

    Protocols::InteractionModel::Status SetSpeakerMaxLevel(uint8_t aSpeakerMaxLevel);

    Protocols::InteractionModel::Status SetSpeakerMinLevel(uint8_t aSpeakerMinLevel);

    Protocols::InteractionModel::Status SetMicrophoneMuted(bool aMicrophoneMuted);

    Protocols::InteractionModel::Status SetMicrophoneVolumeLevel(uint8_t aMicrophoneVolumeLevel);

    Protocols::InteractionModel::Status SetMicrophoneMaxLevel(uint8_t aMicrophoneMaxLevel);

    Protocols::InteractionModel::Status SetMicrophoneMinLevel(uint8_t aMicrophoneMinLevel);

    Protocols::InteractionModel::Status SetMicrophoneAGCEnabled(bool aMicrophoneAGCEnabled);

    Protocols::InteractionModel::Status SetImageRotation(uint16_t aImageRotation);

    Protocols::InteractionModel::Status SetImageFlipHorizontal(bool aImageFlipVertical);

    Protocols::InteractionModel::Status SetImageFlipVertical(bool aImageFlipVertical);

    Protocols::InteractionModel::Status SetLocalVideoRecordingEnabled(bool aLocalVideoRecordingEnabled);

    Protocols::InteractionModel::Status SetLocalSnapshotRecordingEnabled(bool aLocalVideoRecordingEnabled);

    Protocols::InteractionModel::Status SetStatusLightEnabled(bool aStatusLightEnabled);

    Protocols::InteractionModel::Status SetStatusLightBrightness(Globals::ThreeLevelAutoEnum aStatusLightBrightness);

    // Attribute Getters
    uint8_t GetMaxConcurrentVideoEncoders() const { return mMaxConcurrentVideoEncoders; }

    uint32_t GetMaxEncodedPixelRate() const { return mMaxEncodedPixelRate; }

    const VideoSensorParamsStruct & GetVideoSensorParams() const { return mVideoSensorParams; }

    bool GetNightVisionCapable() const { return mNightVisionCapable; }

    const VideoResolutionStruct & GetMinViewport() const { return mMinViewPort; }

    const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> & GetRateDistortionTradeOffPoints() const
    {
        return mRateDistortionTradeOffPointsList;
    }

    uint32_t GetMaxContentBufferSize() const { return mMaxContentBufferSize; }

    const AudioCapabilitiesStruct & GetMicrophoneCapabilities() const { return mMicrophoneCapabilities; }

    const AudioCapabilitiesStruct & GetSpeakerCapabilities() const { return mSpeakerCapabilities; }

    TwoWayTalkSupportTypeEnum GetTwoWayTalkSupport() const { return mTwoWayTalkSupport; }

    const std::vector<Structs::SnapshotParamsStruct::Type> & GetSupportedSnapshotParams() const
    {
        return mSupportedSnapshotParamsList;
    }

    uint32_t GetMaxNetworkBandwidth() const { return mMaxNetworkBandwidth; }

    uint16_t GetCurrentFrameRate() const { return mCurrentFrameRate; }

    bool GetHDRModeEnabled() const { return mHDRModeEnabled; }

    const std::unordered_set<chip::FabricIndex> & GetFabricsUsingCamera() const { return mFabricsUsingCamera; }

    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const { return mAllocatedVideoStreams; }

    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const { return mAllocatedAudioStreams; }

    const std::vector<SnapshotStreamStruct> & GetAllocatedSnapshotStreams() const { return mAllocatedSnapshotStreams; }

    const StreamTypeEnum * GetRankedVideoStreamPriorities() const { return mRankedVideoStreamPriorities; }

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

    CHIP_ERROR SetRankedVideoStreamPriorities(const StreamTypeEnum newPriorities[kNumOfStreamTypes]);

    CHIP_ERROR AddVideoStream(const VideoStreamStruct & videoStream);

    CHIP_ERROR RemoveVideoStream(uint16_t videoStreamId);

    CHIP_ERROR AddAudioStream(const AudioStreamStruct & audioStream);

    CHIP_ERROR RemoveAudioStream(uint16_t audioStreamId);

    CHIP_ERROR AddSnapshotStream(const SnapshotStreamStruct & snapshotStream);

    CHIP_ERROR RemoveSnapshotStream(uint16_t snapshotStreamId);

private:
    CameraAVStreamMgmtDelegate & mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;
    PersistentStorageDelegate * mPersistentStorage = nullptr;

    // Attributes
    const uint8_t mMaxConcurrentVideoEncoders;
    const uint32_t mMaxEncodedPixelRate;
    const VideoSensorParamsStruct mVideoSensorParams;
    const bool mNightVisionCapable;
    const VideoResolutionStruct mMinViewPort;
    const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> mRateDistortionTradeOffPointsList;
    const uint32_t mMaxContentBufferSize;
    const AudioCapabilitiesStruct mMicrophoneCapabilities;
    const AudioCapabilitiesStruct mSpeakerCapabilities;
    const TwoWayTalkSupportTypeEnum mTwoWayTalkSupport;
    const std::vector<Structs::SnapshotParamsStruct::Type> mSupportedSnapshotParamsList;
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

    StreamTypeEnum mRankedVideoStreamPriorities[kNumOfStreamTypes];

    std::vector<VideoStreamStruct> mAllocatedVideoStreams;
    std::vector<AudioStreamStruct> mAllocatedAudioStreams;
    std::vector<SnapshotStreamStruct> mAllocatedSnapshotStreams;

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
