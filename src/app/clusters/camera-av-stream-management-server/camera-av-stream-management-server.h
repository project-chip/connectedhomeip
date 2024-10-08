/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAVStreamMgmt {

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

/** @brief
 *  Defines methods for implementing application-specific logic for the CameraAVStreamMgmt Cluster.
 */
class CameraAVStreamMgmtDelegate
{
public:
    CameraAVStreamMgmtDelegate() = default;

    virtual ~CameraAVStreamMgmtDelegate() = default;

    /**
     *   @brief Handle Command Delegate for Video stream allocation.
     */
    virtual Protocols::InteractionModel::Status VideoStreamAllocate(StreamTypeEnum streamType, VideoCodecEnum videoCodec,
                                                                    const uint16_t minFrameRate, const uint16_t maxFrameRate,
                                                                    VideoResolutionStruct minResolution,
                                                                    VideoResolutionStruct maxResolution, const uint32_t minBitRate,
                                                                    const uint32_t maxBitRate, const uint16_t minFragmentLen,
                                                                    const uint16_t maxFragmentLen) = 0;

    /**
     *   @brief Handle Command Delegate for Video stream modification.
     */
    virtual Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID,
                                                                  VideoResolutionStruct videoResolution) = 0;

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

    // Get attribute methods for list index items

    virtual uint8_t GetMaxConcurrentVideoEncoders() = 0;
    virtual uint32_t GetMaxEncodedPixelRate()       = 0;

    virtual CHIP_ERROR StartAllocatedVideoStreamsRead()                                            = 0;
    virtual CHIP_ERROR GetAllocatedVideoStreamByIndex(uint8_t, Structs::VideoStreamSturct::Type &) = 0;
    virtual CHIP_ERROR EndAllocatedVideoStreamsRead()                                              = 0;

    virtual CHIP_ERROR StartAllocatedAudioStreamsRead()                                            = 0;
    virtual CHIP_ERROR GetAllocatedAudioStreamByIndex(uint8_t, Structs::AudioStreamSturct::Type &) = 0;
    virtual CHIP_ERROR EndAllocatedAudioStreamsRead()                                              = 0;

    virtual CHIP_ERROR StartAllocatedSnapshotStreamsRead()                                               = 0;
    virtual CHIP_ERROR GetAllocatedSnapshotStreamByIndex(uint8_t, Structs::SnapshotStreamSturct::Type &) = 0;
    virtual CHIP_ERROR EndAllocatedSnapshotStreamsRead()                                                 = 0;

    virtual CHIP_ERROR StartRateDistortionTradeOffPointsRead()                                                             = 0;
    virtual CHIP_ERROR GetRateDistortionTradeOffPointByIndex(uint8_t, Structs::RateDistortionTradeOffPointsStruct::Type &) = 0;
    virtual CHIP_ERROR EndRateDistortionTradeOffPointsRead()                                                               = 0;

    virtual CHIP_ERROR StartSupportedSnapshotParamsRead()                                                             = 0;
    virtual CHIP_ERROR GetSupportedSnapshotParamByIndex(uint8_t, Structs::RateDistortionTradeOffPointsStruct::Type &) = 0;
    virtual CHIP_ERROR EndSupportedSnapshotParamsRead()                                                               = 0;

    virtual CHIP_ERROR StartCurrentVideoCodecsRead()                            = 0;
    virtual CHIP_ERROR GetCurrentVideoCodecByIndex(uint8_t, VideoCodecEnumType) = 0;
    virtual CHIP_ERROR EndCurrentVideoCodecsRead()                              = 0;

    virtual CHIP_ERROR StartFabricsUsingCameraRead()                           = 0;
    virtual CHIP_ERROR GetFabricUsingCameraByIndex(uint8_t, chip::FabricIndex) = 0;
    virtual CHIP_ERROR EndFabricsUsingCameraRead()                             = 0;

    virtual CHIP_ERROR StartRankedVideoStreamPrioritiesListRead                               = 0;
    virtual CHIP_ERROR GetRankedVideoStreamPrioritiesListByIndex(uint8_t, StreamTypeEnumType) = 0;
    virtual CHIP_ERROR EndRankedVideoStreamPrioritiesListRead                                 = 0;

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

class CameraAVStreamMgmtServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * @brief Creates a Camera AV Stream Management cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the Microwave Oven Control cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    CameraAVStreamMgmtServer(CameraAVStreamMgmtDelegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                             BitMask<CameraAVStreamMgmt::Feature> aFeature, uint8_t aMaxConVideoEncoders,
                             uint32_t aMaxEncodedPixelRate, VideoSensorParamsStruct aVideoSensorParams, bool aNightVisionCapable,
                             VideoResolutionStruct minViewPort, uint32_t aMaxContentBufferSize,
                             AudioCapabilitiesStruct aMicCapabilities, AudioCapabilitiesStruct aSpkrCapabilities,
                             TwoWayTalkSupportTypeEnum aTwoWayTalkSupport, uint32_t aMaxNetworkBandwidth);

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

    bool IsLocalVideoRecordingEnabled() const;

    Protocols::InteractionModel::Status SetCurrentFrameRate(uint16_t aCurrentFrameRate);

    Protocols::InteractionModel::Status SetHDRModeEnabled(bool aHDRModeEnabled);

    Protocols::InteractionModel::Status
    CameraAVStreamMgmtServer::SetCurrentSnapshotConfig(const VideoResolutionStruct & aVideoResolution, uint16_t aMaxFrameRate,
                                                       ImageCodecEnumType aImageCodecEnum);

    Protocols::InteractionModel::Status SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled);

    Protocols::InteractionModel::Status SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled);

    Protocols::InteractionModel::Status SetHardPrivacyModeOn(bool aHardPrivacyModeOn);

    Protocols::InteractionModel::Status SetNightVision(TriStateAutoEnum aNightVision);

    Protocols::InteractionModel::Status SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum);

    Protocols::InteractionModel::Status SetAWBEnabled(bool aAWBEnabled);

    Protocols::InteractionModel::Status SetAutoShutterSpeedEnabled(bool aAutoShutterSpeedEnabled);

    Protocols::InteractionModel::Status SetAutoISOEnabled(bool aAutoISOEnabled);

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

    Protocols::InteractionModel::Status SetStatusLightBrightness(ThreeLevelAutoEnum aStatusLightBrightness);

    Protocols::InteractionModel::Status SetDepthSensorStatus(TriStateAutoEnum aDepthSensorStatus);

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    CameraAVStreamMgmtDelegate & mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;
    BitMask<Feature> mFeature;

    // Attributes with F quality
    const uint8_t mMaxConcurrentVideoEncoders;
    const uint32_t mMaxEncodedPixelRate;
    const VideoSensorParamsStruct mVideoSensorParams;
    const bool mNightVisionCapable;
    const VideoResolutionStruct mMinViewPort;
    const uint32_t mMaxContentBufferSize;
    const AudioCapabilities mMicrophoneCapabilities;
    const AudioCapabilities mSpeakerCapabilities;
    const TwoWayTalkSupportEnumType mTwoWayTalkSupport;
    const uint32_t mMaxNetworkBandwidth;

    uint16_t mCurrentFrameRate;
    bool mHDRModeEnabled = false;
    SnapshotParamsStruct mCurrentSnapshotConfig;
    bool mSoftRecordingPrivacyModeEnabled  = false;
    bool mSoftLivestreamPrivacyModeEnabled = false;
    bool mHardPrivacyModeOn                = false;
    TriStateAutoEnum mNightVision;
    TriStateAutoEnum mNightVisionIllum;
    bool mAWBEnabled              = false;
    bool mAutoShutterSpeedEnabled = false;
    bool mAutoISOEnabled          = false;
    ViewPortStruct mViewport;
    bool mSpeakerMuted = false;
    uint8_t mSpeakerVolumeLevel;
    uint8_t mSpeakerMaxLevel = kMaxSpeakerLevel;
    uint8_t mSpeakerMinLevel = 0;
    bool mMicrophoneMuted    = false;
    uint8_t mMicrophoneVolumeLevel;
    uint8_t mMicrophoneMaxLevel = kMaxMicrophoneLevel;
    uint8_t mMicrophoneMinLevel = 0;
    bool mMicrophoneAGCEnabled  = false;
    uint16_t mImageRotation;
    bool mImageFlipHorizontal           = false;
    bool mImageFlipVertical             = false;
    bool mLocalVideoRecordingEnabled    = false;
    bool mLocalSnapshotRecordingEnabled = false;
    bool mStatusLightEnabled            = false;
    ThreeLevelAutoEnum mStatusLightBrightness;
    TriStateAutoEnum mDepthSensorStatus;

    Structs::VideoSensorParamsStruct::Type mVideoSensorParams;
    CHIP_ERROR SetVideoSensorParams(const Structs::VideoSensorParamsStruct::Type & videoSensorParams)
    {
        mVideoSensorParams = videoSensorParams;
    }

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
    CHIP_ERROR ReadAndEncodeCurrentVideoCodecs(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeFabricsUsingCamera(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeRankedVideoStreamPrioritiesList(const AttributeValueEncoder::ListEncodeHelper & encoder);

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    void HandleVideoStreamAllocate(HandlerContext & ctx, const Commands::VideoStreamAllocate::DecodableType & req);

    void HandleVideoStreamModify(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    void HandleVideoStreamDeallocate(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    void HandleAudioStreamAllocate(HandlerContext & ctx, const Commands::VideoStreamAllocate::DecodableType & req);

    void HandleAudioStreamDeallocate(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    void HandleSnapshotStreamAllocate(HandlerContext & ctx, const Commands::VideoStreamAllocate::DecodableType & req);

    void HandleSnapshotStreamDeallocate(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    void HandleCaptureSnapshot(HandlerContext & ctx, const Commands::VideoStreamModify::DecodableType & req);

    /**
     * @brief Handle Command: AddMoreTime.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the cook time value is out of range, returns the Interaction Model status code of CONSTRAINT_ERROR.
     * If the operational state is in 'Error', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    void HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req);
};

} // namespace CameraAVStreamMgmt
} // namespace Clusters
} // namespace app
} // namespace chip
