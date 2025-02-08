/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *
 */

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::CameraAvStreamManagement::Structs;
using namespace chip::app::Clusters::CameraAvStreamManagement::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

CameraAVStreamMgmtServer::CameraAVStreamMgmtServer(
    CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeature,
    const BitFlags<OptionalAttribute> aOptionalAttrs, PersistentStorageDelegate & aPersistentStorage,
    uint8_t aMaxConcurrentVideoEncoders, uint32_t aMaxEncodedPixelRate, const VideoSensorParamsStruct & aVideoSensorParams,
    bool aNightVisionCapable, const VideoResolutionStruct & aMinViewPort,
    const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> & aRateDistortionTradeOffPoints,
    uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
    const AudioCapabilitiesStruct & aSpeakerCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
    const std::vector<Structs::SnapshotParamsStruct::Type> & aSupportedSnapshotParams, uint32_t aMaxNetworkBandwidth) :
    CommandHandlerInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeature(aFeature), mOptionalAttrs(aOptionalAttrs), mPersistentStorage(&aPersistentStorage),
    mMaxConcurrentVideoEncoders(aMaxConcurrentVideoEncoders), mMaxEncodedPixelRate(aMaxEncodedPixelRate),
    mVideoSensorParams(aVideoSensorParams), mNightVisionCapable(aNightVisionCapable), mMinViewPort(aMinViewPort),
    mRateDistortionTradeOffPointsList(aRateDistortionTradeOffPoints), mMaxContentBufferSize(aMaxContentBufferSize),
    mMicrophoneCapabilities(aMicrophoneCapabilities), mSpeakerCapabilities(aSpeakerCapabilities),
    mTwoWayTalkSupport(aTwoWayTalkSupport), mSupportedSnapshotParamsList(aSupportedSnapshotParams),
    mMaxNetworkBandwidth(aMaxNetworkBandwidth)
{
    mDelegate.SetCameraAVStreamMgmtServer(this);
}

CameraAVStreamMgmtServer::~CameraAVStreamMgmtServer()
{
    // Explicitly set the CameraAVStreamMgmtServer pointer in the Delegate to
    // null.
    mDelegate.SetCameraAVStreamMgmtServer(nullptr);

    // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR CameraAVStreamMgmtServer::Init()
{
    // Add necessary feature checks
    VerifyOrReturnError(
        HasFeature(Feature::kVideo) || HasFeature(Feature::kAudio) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(
            Zcl, "CameraAVStreamMgmt: Feature configuration error. At least one of Audio, Video, or Snapshot feature required"));

    if (HasFeature(Feature::kImageControl))
    {
        VerifyOrReturnError(HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt: Feature configuration error. If ImageControl, then at least one of "
                                         "Video, or Snapshot feature required"));
    }

    if (HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt: Feature configuration error. if Watermark or OSD, then Video feature required"));
    }

    // Ensure Optional attribute bits have been correctly passed.
    if (SupportsOptAttr(OptionalAttribute::kSupportsHDRModeEnabled))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt: Feature configuration error. if HDRModeEnabled, then Video feature required"));
    }

    if (SupportsOptAttr(OptionalAttribute::kSupportsNightVision) || SupportsOptAttr(OptionalAttribute::kSupportsNightVisionIllum))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt: Feature configuration error. if NIghtVision is enabled, then Video|Snapshot feature required"));
    }

    if (SupportsOptAttr(OptionalAttribute::kSupportsMicrophoneAGCEnabled))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt: Feature configuration error. if MicrophoneAGCEnabled, then Audio feature required"));
    }

    if (SupportsOptAttr(OptionalAttribute::kSupportsImageFlipHorizontal) || SupportsOptAttr(OptionalAttribute::kSupportsImageFlipVertical) ||
        SupportsOptAttr(OptionalAttribute::kSupportsImageRotation))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kImageControl), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt: Feature configuration error. if ImageFlip or Rotation enabled, then ImageControl feature required"));
    }

    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool CameraAVStreamMgmtServer::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

bool CameraAVStreamMgmtServer::SupportsOptAttr(OptionalAttribute aOptionalAttr) const
{
    return mOptionalAttrs.Has(aOptionalAttr);
}

bool CameraAVStreamMgmtServer::IsLocalVideoRecordingEnabled() const
{
    return mLocalVideoRecordingEnabled;
}

CHIP_ERROR
CameraAVStreamMgmtServer::ReadAndEncodeRateDistortionTradeOffPoints(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & rateDistortionTradeOffPoints : mRateDistortionTradeOffPointsList)
    {
        ReturnErrorOnFailure(encoder.Encode(rateDistortionTradeOffPoints));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeSupportedSnapshotParams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & snapshotParams : mSupportedSnapshotParamsList)
    {
        ReturnErrorOnFailure(encoder.Encode(snapshotParams));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeFabricsUsingCamera(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & fabricIndex : mFabricsUsingCamera)
    {
        ReturnErrorOnFailure(encoder.Encode(fabricIndex));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & videoStream : mAllocatedVideoStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(videoStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & audioStream : mAllocatedAudioStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(audioStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & audioStream : mAllocatedAudioStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(audioStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamMgmtServer::ReadAndEncodeRankedVideoStreamPrioritiesList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (uint8_t i = 0; i < kNumOfStreamUsageTypes; i++)
    {
        ReturnErrorOnFailure(encoder.Encode(mRankedVideoStreamPriorities[i]));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddToFabricsUsingCamera(FabricIndex aFabricIndex)
{
    mFabricsUsingCamera.insert(aFabricIndex);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::FabricsUsingCamera::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::RemoveFromFabricsUsingCamera(FabricIndex aFabricIndex)
{
    mFabricsUsingCamera.erase(aFabricIndex);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::FabricsUsingCamera::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetRankedVideoStreamPriorities(
    const StreamUsageEnum newPriorities[ArraySize(mRankedVideoStreamPriorities)])
{
    std::copy(newPriorities, newPriorities + ArraySize(mRankedVideoStreamPriorities), mRankedVideoStreamPriorities);

    ReturnErrorOnFailure(StoreRankedVideoStreamPriorities());
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::RankedVideoStreamPrioritiesList::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddVideoStream(const VideoStreamStruct & videoStream)
{
    mAllocatedVideoStreams.push_back(videoStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedVideoStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::RemoveVideoStream(uint16_t videoStreamId)
{
    mAllocatedVideoStreams.erase(
        std::remove_if(mAllocatedVideoStreams.begin(), mAllocatedVideoStreams.end(),
                       [&](const VideoStreamStruct & vStream) { return vStream.videoStreamID == videoStreamId; }),
        mAllocatedVideoStreams.end());
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedVideoStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddAudioStream(const AudioStreamStruct & audioStream)
{
    mAllocatedAudioStreams.push_back(audioStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedAudioStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::RemoveAudioStream(uint16_t audioStreamId)
{
    mAllocatedAudioStreams.erase(
        std::remove_if(mAllocatedAudioStreams.begin(), mAllocatedAudioStreams.end(),
                       [&](const AudioStreamStruct & aStream) { return aStream.audioStreamID == audioStreamId; }),
        mAllocatedAudioStreams.end());
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedAudioStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddSnapshotStream(const SnapshotStreamStruct & snapshotStream)
{
    mAllocatedSnapshotStreams.push_back(snapshotStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedSnapshotStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::RemoveSnapshotStream(uint16_t snapshotStreamId)
{
    mAllocatedSnapshotStreams.erase(
        std::remove_if(mAllocatedSnapshotStreams.begin(), mAllocatedSnapshotStreams.end(),
                       [&](const SnapshotStreamStruct & sStream) { return sStream.snapshotStreamID == snapshotStreamId; }),
        mAllocatedSnapshotStreams.end());
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedSnapshotStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR CameraAVStreamMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvStreamManagement::Id);
    ChipLogError(Zcl, "Camera AVStream Management: Reading");

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case MaxConcurrentVideoEncoders::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MaxConcurrentVideoEncoders, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxConcurrentVideoEncoders));
        break;
    case MaxEncodedPixelRate::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MaxEncodedPixelRate, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxEncodedPixelRate));
        break;
    case VideoSensorParams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get VideoSensorParams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mVideoSensorParams));
        break;
    case NightVisionCapable::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get VideoSensorParams, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionCapable));
        break;
    case MinViewport::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MinViewport, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMinViewPort));
        break;
    case RateDistortionTradeOffPoints::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RateDistortionTradeOffPoints, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeRateDistortionTradeOffPoints(encoder); }));
        break;
    case MaxContentBufferSize::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MaxContentBufferSize, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMaxContentBufferSize));
        break;
    case MicrophoneCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneCapabilities, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneCapabilities));
        break;
    case SpeakerCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerCapabilities, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerCapabilities));
        break;
    case TwoWayTalkSupport::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get TwoWayTalkSupport, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mTwoWayTalkSupport));
        break;
    case SupportedSnapshotParams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RateDistortionTradeOffPoints, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSupportedSnapshotParams(encoder); }));
        break;
    case MaxNetworkBandwidth::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxNetworkBandwidth));
        break;
    case CurrentFrameRate::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get CurrentFrameRate, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mCurrentFrameRate));
        break;
    case HDRModeEnabled::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo) && SupportsOptAttr(OptionalAttribute::kSupportsHDRModeEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get HDRModeEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mHDRModeEnabled));
        break;
    case FabricsUsingCamera::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get FabricsUsingCamera, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeFabricsUsingCamera(encoder); }));
        break;
    case AllocatedVideoStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedVideoStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedVideoStreams(encoder); }));
        break;
    case AllocatedAudioStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedAudioStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedAudioStreams(encoder); }));
        break;
    case AllocatedSnapshotStreams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedSnapshotStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedSnapshotStreams(encoder); }));
        break;
    case RankedVideoStreamPrioritiesList::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RankedVideoStreamPrioritiesList, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeRankedVideoStreamPrioritiesList(encoder); }));
        break;
    case SoftRecordingPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SoftRecordingPrivacyModeEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftRecordingPrivacyModeEnabled));
        break;
    case SoftLivestreamPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SoftLivestreamPrivacyModeEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftLivestreamPrivacyModeEnabled));
        break;
    case HardPrivacyModeOn::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kSupportsHardPrivacyModeOn),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get HardPrivacyModeOn, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mHardPrivacyModeOn));
        break;
    case NightVision::Id:
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttribute::kSupportsNightVision),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get NightVision, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVision));
        break;
    case NightVisionIllum::Id:
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttribute::kSupportsNightVisionIllum),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get NightVisionIllumination, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionIllum));
        break;
    case Viewport::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get Viewport, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mViewport));
        break;
    case SpeakerMuted::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMuted, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMuted));
        break;
    case SpeakerVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerVolumeLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerVolumeLevel));
        break;
    case SpeakerMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMaxLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMaxLevel));
        break;
    case SpeakerMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMinLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMinLevel));
        break;
    case MicrophoneMuted::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMuted, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMuted));
        break;
    case MicrophoneVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneVolumeLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneVolumeLevel));
        break;
    case MicrophoneMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMaxLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMaxLevel));
        break;
    case MicrophoneMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMinLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMinLevel));
        break;
    case MicrophoneAGCEnabled::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && SupportsOptAttr(OptionalAttribute::kSupportsMicrophoneAGCEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneAGCEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneAGCEnabled));
        break;
    case ImageRotation::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageRotation),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageRotation, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageRotation));
        break;
    case ImageFlipHorizontal::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageFlipHorizontal),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageFlipHorizontal, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipHorizontal));
        break;
    case ImageFlipVertical::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageFlipVertical),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageFlipHorizontal, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipVertical));
        break;
    case LocalVideoRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get LocalVideoRecordingEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalVideoRecordingEnabled));
        break;
    case LocalSnapshotRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get LocalSnapshotRecordingEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalSnapshotRecordingEnabled));
        break;
    case StatusLightEnabled::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kSupportsStatusLightEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get StatusLightEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightEnabled));
        break;
    case StatusLightBrightness::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kSupportsStatusLightBrightness),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get StatusLightBrightness, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightBrightness));
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvStreamManagement::Id);

    switch (aPath.mAttributeId)
    {
    case HDRModeEnabled::Id: {
        // Optional Attribute if Video is supported
        VerifyOrReturnError(HasFeature(Feature::kVideo) && SupportsOptAttr(OptionalAttribute::kSupportsHDRModeEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set HDRModeEnabled, feature is not supported"));

        bool hdrModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(hdrModeEnabled));
        return SetHDRModeEnabled(hdrModeEnabled);
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SoftRecordingPrivacyModeEnabled, feature is not supported"));

        bool softRecPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softRecPrivModeEnabled));
        return SetSoftRecordingPrivacyModeEnabled(softRecPrivModeEnabled);
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SoftLivestreamPrivacyModeEnabled, feature is not supported"));

        bool softLivestreamPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softLivestreamPrivModeEnabled));
        return SetSoftLivestreamPrivacyModeEnabled(softLivestreamPrivModeEnabled);
    }
    case NightVision::Id: {
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttribute::kSupportsNightVision),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set NightVision, feature is not supported"));

        TriStateAutoEnum nightVision;
        ReturnErrorOnFailure(aDecoder.Decode(nightVision));
        return SetNightVision(nightVision);
    }
    case NightVisionIllum::Id: {
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttribute::kSupportsNightVisionIllum),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set NightVisionIllumination, feature is not supported"));

        TriStateAutoEnum nightVisionIllum;
        ReturnErrorOnFailure(aDecoder.Decode(nightVisionIllum));
        return SetNightVisionIllum(nightVisionIllum);
    }
    case Viewport::Id: {
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set Viewport, feature is not supported"));
        ViewportStruct viewPort;
        ReturnErrorOnFailure(aDecoder.Decode(viewPort));
        return SetViewport(viewPort);
    }
    case SpeakerMuted::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SpeakerMuted, feature is not supported"));
        bool speakerMuted;
        ReturnErrorOnFailure(aDecoder.Decode(speakerMuted));
        return SetSpeakerMuted(speakerMuted);
    }
    case SpeakerVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SpeakerVolumeLevel, feature is not supported"));
        uint8_t speakerVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(speakerVolLevel));
        return SetSpeakerVolumeLevel(speakerVolLevel);
    }
    case MicrophoneMuted::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneMuted, feature is not supported"));
        bool micMuted;
        ReturnErrorOnFailure(aDecoder.Decode(micMuted));
        return SetMicrophoneMuted(micMuted);
    }
    case MicrophoneVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneVolumeLevel, feature is not supported"));
        uint8_t micVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(micVolLevel));
        return SetMicrophoneVolumeLevel(micVolLevel);
    }
    case MicrophoneAGCEnabled::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && SupportsOptAttr(OptionalAttribute::kSupportsMicrophoneAGCEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneAGCEnabled, feature is not supported"));
        bool micAGCEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(micAGCEnabled));
        return SetMicrophoneAGCEnabled(micAGCEnabled);
    }
    case ImageRotation::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageRotation),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageRotation, feature is not supported"));
        uint16_t imageRotation;
        ReturnErrorOnFailure(aDecoder.Decode(imageRotation));
        return SetImageRotation(imageRotation);
    }
    case ImageFlipHorizontal::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageFlipHorizontal),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageFlipHorizontal, feature is not supported"));
        bool imageFlipHorizontal;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipHorizontal));
        return SetImageFlipHorizontal(imageFlipHorizontal);
    }
    case ImageFlipVertical::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttribute::kSupportsImageFlipVertical),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageFlipVertical, feature is not supported"));
        bool imageFlipVertical;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipVertical));
        return SetImageFlipVertical(imageFlipVertical);
    }
    case LocalVideoRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set LocalVideoRecordingEnabled, feature is not supported"));
        bool localVidRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localVidRecEnabled));
        return SetLocalVideoRecordingEnabled(localVidRecEnabled);
    }
    case LocalSnapshotRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set LocalSnapshotRecordingEnabled, feature is not supported"));
        bool localSnapshotRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localSnapshotRecEnabled));
        return SetLocalSnapshotRecordingEnabled(localSnapshotRecEnabled);
    }
    case StatusLightEnabled::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kSupportsStatusLightEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set StatusLightEnabled, feature is not supported"));
        bool statusLightEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightEnabled));
        return SetStatusLightEnabled(statusLightEnabled);
    }
    case StatusLightBrightness::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kSupportsStatusLightBrightness),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set StatusLightBrightness, feature is not supported"));
        Globals::ThreeLevelAutoEnum statusLightBrightness;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightBrightness));
        return SetStatusLightBrightness(statusLightBrightness);
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

CHIP_ERROR CameraAVStreamMgmtServer::SetCurrentFrameRate(uint16_t aCurrentFrameRate)
{
    return SetAttributeIfDifferent(mCurrentFrameRate, aCurrentFrameRate, Attributes::CurrentFrameRate::Id,
                                   /* shouldPersist = */ false);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetHDRModeEnabled(bool aHDRModeEnabled)
{
    return SetAttributeIfDifferent(mHDRModeEnabled, aHDRModeEnabled, Attributes::HDRModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled)
{
    return SetAttributeIfDifferent(mSoftRecordingPrivacyModeEnabled, aSoftRecordingPrivacyModeEnabled,
                                   Attributes::SoftRecordingPrivacyModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled)
{
    return SetAttributeIfDifferent(mSoftLivestreamPrivacyModeEnabled, aSoftLivestreamPrivacyModeEnabled,
                                   Attributes::SoftLivestreamPrivacyModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetHardPrivacyModeOn(bool aHardPrivacyModeOn)
{
    return SetAttributeIfDifferent(mHardPrivacyModeOn, aHardPrivacyModeOn, Attributes::HardPrivacyModeOn::Id,
                                   /* shouldPersist = */ false);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetNightVision(TriStateAutoEnum aNightVision)
{
    if (mNightVision != aNightVision)
    {
        mNightVision = aNightVision;
        auto path    = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVision::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mNightVision)));
        MatterReportingAttributeChangeCallback(path);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum)
{
    if (mNightVisionIllum != aNightVisionIllum)
    {
        mNightVisionIllum = aNightVisionIllum;
        auto path         = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVisionIllum::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mNightVisionIllum)));
        MatterReportingAttributeChangeCallback(path);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetViewport(const ViewportStruct & aViewport)
{
    mViewport = aViewport;

    StoreViewport(mViewport);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSpeakerMuted(bool aSpeakerMuted)
{
    return SetAttributeIfDifferent(mSpeakerMuted, aSpeakerMuted, Attributes::SpeakerMuted::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSpeakerVolumeLevel(uint8_t aSpeakerVolumeLevel)
{
    if (aSpeakerVolumeLevel < mSpeakerMinLevel || aSpeakerVolumeLevel > mSpeakerMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerVolumeLevel, aSpeakerVolumeLevel, Attributes::SpeakerVolumeLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSpeakerMaxLevel(uint8_t aSpeakerMaxLevel)
{
    if (aSpeakerMaxLevel < mSpeakerMinLevel || aSpeakerMaxLevel > kMaxSpeakerLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerMaxLevel, aSpeakerMaxLevel, Attributes::SpeakerMaxLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetSpeakerMinLevel(uint8_t aSpeakerMinLevel)
{
    if (aSpeakerMinLevel > mSpeakerMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerMinLevel, aSpeakerMinLevel, Attributes::SpeakerMinLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetMicrophoneMuted(bool aMicrophoneMuted)
{
    return SetAttributeIfDifferent(mMicrophoneMuted, aMicrophoneMuted, Attributes::MicrophoneMuted::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetMicrophoneVolumeLevel(uint8_t aMicrophoneVolumeLevel)
{
    if (aMicrophoneVolumeLevel < mMicrophoneMinLevel || aMicrophoneVolumeLevel > mMicrophoneMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneVolumeLevel, aMicrophoneVolumeLevel, Attributes::MicrophoneVolumeLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetMicrophoneMaxLevel(uint8_t aMicrophoneMaxLevel)
{
    if (aMicrophoneMaxLevel < mMicrophoneMinLevel || aMicrophoneMaxLevel > kMaxMicrophoneLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneMaxLevel, aMicrophoneMaxLevel, Attributes::MicrophoneMaxLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetMicrophoneMinLevel(uint8_t aMicrophoneMinLevel)
{
    if (aMicrophoneMinLevel > mMicrophoneMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneMinLevel, aMicrophoneMinLevel, Attributes::MicrophoneMinLevel::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetMicrophoneAGCEnabled(bool aMicrophoneAGCEnabled)
{
    return SetAttributeIfDifferent(mMicrophoneAGCEnabled, aMicrophoneAGCEnabled, Attributes::MicrophoneAGCEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetImageRotation(uint16_t aImageRotation)
{
    if (mImageRotation > kMaxImageRotationDegrees)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mImageRotation, aImageRotation, Attributes::ImageRotation::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetImageFlipHorizontal(bool aImageFlipHorizontal)
{
    return SetAttributeIfDifferent(mImageFlipHorizontal, aImageFlipHorizontal, Attributes::ImageFlipHorizontal::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetImageFlipVertical(bool aImageFlipVertical)
{
    return SetAttributeIfDifferent(mImageFlipVertical, aImageFlipVertical, Attributes::ImageFlipVertical::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetLocalVideoRecordingEnabled(bool aLocalVideoRecordingEnabled)
{
    return SetAttributeIfDifferent(mLocalVideoRecordingEnabled, aLocalVideoRecordingEnabled,
                                   Attributes::LocalVideoRecordingEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetLocalSnapshotRecordingEnabled(bool aLocalSnapshotRecordingEnabled)
{
    return SetAttributeIfDifferent(mLocalSnapshotRecordingEnabled, aLocalSnapshotRecordingEnabled,
                                   Attributes::LocalSnapshotRecordingEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetStatusLightEnabled(bool aStatusLightEnabled)
{
    return SetAttributeIfDifferent(mStatusLightEnabled, aStatusLightEnabled, Attributes::StatusLightEnabled::Id);
}

CHIP_ERROR CameraAVStreamMgmtServer::SetStatusLightBrightness(Globals::ThreeLevelAutoEnum aStatusLightBrightness)
{
    if (mStatusLightBrightness != aStatusLightBrightness)
    {
        mStatusLightBrightness = aStatusLightBrightness;
        auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightBrightness::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mStatusLightBrightness)));
        MatterReportingAttributeChangeCallback(path);
    }
    return CHIP_NO_ERROR;
}

void CameraAVStreamMgmtServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load HDR Mode Enabled
    bool storedHDRModeEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::HDRModeEnabled::Id), storedHDRModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mHDRModeEnabled = storedHDRModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded HDRModeEnabled as %u", mHDRModeEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the HDRModeEnabled from the KVS. Defaulting to %u", mHDRModeEnabled);
    }

    // Load AllocatedVideoStreams
    mDelegate.LoadAllocatedVideoStreams(mAllocatedVideoStreams);

    // Load AllocatedAudioStreams
    mDelegate.LoadAllocatedAudioStreams(mAllocatedAudioStreams);

    // Load AllocatedSnapshotStreams
    mDelegate.LoadAllocatedSnapshotStreams(mAllocatedSnapshotStreams);

    err = LoadRankedVideoStreamPriorities();
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded RankedVideoStreamPrioritiesList");
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the RankedVideoStreamPrioritiesList from the KVS.");
    }

    // Load SoftRecordingPrivacyModeEnabled
    bool softRecordingPrivacyModeEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftRecordingPrivacyModeEnabled::Id),
        softRecordingPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftRecordingPrivacyModeEnabled = softRecordingPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded SoftRecordingPrivacyModeEnabled as %u", mSoftRecordingPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the SoftRecordingPrivacyModeEnabled from the KVS. Defaulting to %u",
                      mSoftRecordingPrivacyModeEnabled);
    }

    // Load SoftLivestreamPrivacyModeEnabled
    bool softLivestreamPrivacyModeEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftLivestreamPrivacyModeEnabled::Id),
        softLivestreamPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftLivestreamPrivacyModeEnabled = softLivestreamPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded SoftLivestreamPrivacyModeEnabled as %u", mSoftLivestreamPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the SoftLivestreamPrivacyModeEnabled from the KVS. Defaulting to %u",
                      mSoftLivestreamPrivacyModeEnabled);
    }

    // Load NightVision
    uint8_t nightVision;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVision::Id), nightVision);
    if (err == CHIP_NO_ERROR)
    {
        mNightVision = static_cast<TriStateAutoEnum>(nightVision);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded NightVision as %d", to_underlying(mNightVision));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the NightVision from the KVS. Defaulting to %d",
                      to_underlying(mNightVision));
    }

    // Load NightVisionIllum
    uint8_t nightVisionIllum;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVisionIllum::Id), nightVisionIllum);
    if (err == CHIP_NO_ERROR)
    {
        mNightVisionIllum = static_cast<TriStateAutoEnum>(nightVisionIllum);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded NightVisionIllum as %d", to_underlying(mNightVisionIllum));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the NightVisionIllum from the KVS. Defaulting to %d",
                      to_underlying(mNightVisionIllum));
    }

    // Load Viewport
    ViewportStruct viewport;
    err = LoadViewport(viewport);
    if (err == CHIP_NO_ERROR)
    {
        mViewport = viewport;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded Viewport");
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the Viewport from the KVS.");
    }

    // Load SpeakerMuted
    bool speakerMuted;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerMuted::Id), speakerMuted);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerMuted = speakerMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded SpeakerMuted as %u", mSpeakerMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the SpeakerMuted from the KVS. Defaulting to %u", mSpeakerMuted);
    }

    // Load SpeakerVolumeLevel
    uint8_t speakerVolumeLevel;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerVolumeLevel::Id), speakerVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerVolumeLevel = speakerVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded SpeakerVolumeLevel as %u", mSpeakerVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the SpeakerVolumeLevel from the KVS. Defaulting to %u",
                      mSpeakerVolumeLevel);
    }

    // Load MicrophoneMuted
    bool microphoneMuted;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneMuted::Id), microphoneMuted);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneMuted = microphoneMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded MicrophoneMuted as %u", mMicrophoneMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the MicrophoneMuted from the KVS. Defaulting to %u",
                      mMicrophoneMuted);
    }

    // Load MicrophoneVolumeLevel
    uint8_t microphoneVolumeLevel;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneVolumeLevel::Id),
        microphoneVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneVolumeLevel = microphoneVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded MicrophoneVolumeLevel as %u", mMicrophoneVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the MicrophoneVolumeLevel from the KVS. Defaulting to %u",
                      mMicrophoneVolumeLevel);
    }

    // Load MicrophoneAGCEnabled
    bool microphoneAGCEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneAGCEnabled::Id),
        microphoneAGCEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneAGCEnabled = microphoneAGCEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded MicrophoneAGCEnabled as %u", mMicrophoneAGCEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the MicrophoneAGCEnabled from the KVS. Defaulting to %u",
                      mMicrophoneAGCEnabled);
    }

    // Load ImageRotation
    uint16_t imageRotation;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageRotation::Id), imageRotation);
    if (err == CHIP_NO_ERROR)
    {
        mImageRotation = imageRotation;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded ImageRotation as %u", mImageRotation);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the ImageRotation from the KVS. Defaulting to %u", mImageRotation);
    }

    // Load ImageFlipHorizontal
    bool imageFlipHorizontal;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipHorizontal::Id), imageFlipHorizontal);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipHorizontal = imageFlipHorizontal;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded ImageFlipHorizontal as %u", mImageFlipHorizontal);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the ImageFlipHorizontal from the KVS. Defaulting to %u",
                      mImageFlipHorizontal);
    }

    // Load ImageFlipVertical
    bool imageFlipVertical;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipVertical::Id), imageFlipVertical);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipVertical = imageFlipVertical;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded ImageFlipVertical as %u", mImageFlipVertical);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the ImageFlipVertical from the KVS. Defaulting to %u",
                      mImageFlipVertical);
    }

    // Load LocalVideoRecordingEnabled
    bool localVideoRecordingEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalVideoRecordingEnabled::Id),
        localVideoRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalVideoRecordingEnabled = localVideoRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded LocalVideoRecordingEnabled as %u", mLocalVideoRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the LocalVideoRecordingEnabled from the KVS. Defaulting to %u",
                      mLocalVideoRecordingEnabled);
    }

    // Load LocalSnapshotRecordingEnabled
    bool localSnapshotRecordingEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalSnapshotRecordingEnabled::Id),
        localSnapshotRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalSnapshotRecordingEnabled = localSnapshotRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded LocalSnapshotRecordingEnabled as %u", mLocalSnapshotRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the LocalSnapshotRecordingEnabled from the KVS. Defaulting to %u",
                      mLocalSnapshotRecordingEnabled);
    }

    // Load StatusLightEnabled
    bool statusLightEnabled;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightEnabled::Id), statusLightEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightEnabled = statusLightEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded StatusLightEnabled as %u", mStatusLightEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the StatusLightEnabled from the KVS. Defaulting to %u",
                      mStatusLightEnabled);
    }

    // Load StatusLightBrightness
    uint8_t statusLightBrightness;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightBrightness::Id),
        statusLightBrightness);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightBrightness = static_cast<Globals::ThreeLevelAutoEnum>(statusLightBrightness);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded StatusLightBrightness as %d", to_underlying(mStatusLightBrightness));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the StatusLightBrightness from the KVS. Defaulting to %d",
                      to_underlying(mStatusLightBrightness));
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate.PersistentAttributesLoadedCallback();
}

CHIP_ERROR CameraAVStreamMgmtServer::StoreViewport(const ViewportStruct & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(viewport.Encode(writer, TLV::AnonymousTag()));

    StorageKeyName key =
        DefaultStorageKeyAllocator::AttributeValue(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    return mPersistentStorage->SyncSetKeyValue(key.KeyName(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR CameraAVStreamMgmtServer::ClearViewport()
{
    StorageKeyName key =
        DefaultStorageKeyAllocator::AttributeValue(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    return mPersistentStorage->SyncDeleteKeyValue(key.KeyName());
}

CHIP_ERROR CameraAVStreamMgmtServer::LoadViewport(ViewportStruct & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    uint16_t size = static_cast<uint16_t>(bufferSpan.size());

    StorageKeyName key =
        DefaultStorageKeyAllocator::AttributeValue(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key.KeyName(), bufferSpan.data(), size));
    bufferSpan.reduce_size(size);

    TLV::TLVReader reader;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(viewport.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::StoreRankedVideoStreamPriorities()
{
    uint8_t buffer[kRankedVideoStreamPrioritiesTlvSize];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (uint8_t i = 0; i < kNumOfStreamUsageTypes; i++)
    {
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), mRankedVideoStreamPriorities[i]));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    StorageKeyName key = DefaultStorageKeyAllocator::AttributeValue(mEndpointId, CameraAvStreamManagement::Id,
                                                                    Attributes::RankedVideoStreamPrioritiesList::Id);
    return mPersistentStorage->SyncSetKeyValue(key.KeyName(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR CameraAVStreamMgmtServer::LoadRankedVideoStreamPriorities()
{
    uint8_t buffer[kRankedVideoStreamPrioritiesTlvSize];
    MutableByteSpan bufferSpan(buffer);
    uint16_t size = static_cast<uint16_t>(bufferSpan.size());

    StorageKeyName key = DefaultStorageKeyAllocator::AttributeValue(mEndpointId, CameraAvStreamManagement::Id,
                                                                    Attributes::RankedVideoStreamPrioritiesList::Id);
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key.KeyName(), bufferSpan.data(), size));
    bufferSpan.reduce_size(size);

    TLV::TLVReader reader;
    reader.Init(bufferSpan);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    for (uint8_t i = 0; i < kNumOfStreamUsageTypes; i++)
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag()));
        ReturnErrorOnFailure(reader.Get(mRankedVideoStreamPriorities[i]));
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

// CommandHandlerInterface
void CameraAVStreamMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "CameraAV: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::VideoStreamAllocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Allocating Video Stream");

        if (!HasFeature(Feature::kVideo))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::VideoStreamAllocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleVideoStreamAllocate(ctx, commandData); });
        }
        return;

    case Commands::VideoStreamModify::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Modifying Video Stream");

        if (!HasFeature(Feature::kVideo))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::VideoStreamModify::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleVideoStreamModify(ctx, commandData); });
        }
        return;

    case Commands::VideoStreamDeallocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Deallocating Video Stream");

        if (!HasFeature(Feature::kVideo))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::VideoStreamDeallocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleVideoStreamDeallocate(ctx, commandData); });
        }
        return;

    case Commands::AudioStreamAllocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Allocating Audio Stream");

        if (!HasFeature(Feature::kAudio))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::AudioStreamAllocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleAudioStreamAllocate(ctx, commandData); });
        }
        return;

    case Commands::AudioStreamDeallocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Deallocating Audio Stream");

        if (!HasFeature(Feature::kAudio))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::AudioStreamDeallocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleAudioStreamDeallocate(ctx, commandData); });
        }
        return;

    case Commands::SnapshotStreamAllocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Allocating Snapshot Stream");

        if (!HasFeature(Feature::kSnapshot))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::SnapshotStreamAllocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleSnapshotStreamAllocate(ctx, commandData); });
        }
        return;

    case Commands::SnapshotStreamDeallocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Deallocating Snapshot Stream");

        if (!HasFeature(Feature::kSnapshot))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::SnapshotStreamDeallocate::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleSnapshotStreamDeallocate(ctx, commandData); });
        }
        return;

    case Commands::SetStreamPriorities::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Set Stream Priorities");
        HandleCommand<Commands::SetStreamPriorities::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSetStreamPriorities(ctx, commandData); });
        return;

    case Commands::CaptureSnapshot::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Capture Snapshot image");

        if (!HasFeature(Feature::kSnapshot))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::CaptureSnapshot::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleCaptureSnapshot(ctx, commandData); });
        }
        return;
    }
}

void CameraAVStreamMgmtServer::HandleVideoStreamAllocate(HandlerContext & ctx,
                                                         const Commands::VideoStreamAllocate::DecodableType & commandData)
{
    Status status = Status::Success;

    Commands::VideoStreamAllocateResponse::Type response;
    auto & streamUsage        = commandData.streamUsage;
    auto & videoCodec         = commandData.videoCodec;
    auto & minFrameRate       = commandData.minFrameRate;
    auto & maxFrameRate       = commandData.maxFrameRate;
    auto & minResolution      = commandData.minResolution;
    auto & maxResolution      = commandData.maxResolution;
    auto & minBitRate         = commandData.minBitRate;
    auto & maxBitRate         = commandData.maxBitRate;
    auto & minFragmentLen     = commandData.minFragmentLen;
    auto & maxFragmentLen     = commandData.maxFragmentLen;
    auto & isWaterMarkEnabled = commandData.watermarkEnabled;
    auto & isOSDEnabled       = commandData.OSDEnabled;
    uint16_t videoStreamID  = 0;

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kWatermark) && commandData.watermarkEnabled.HasValue()) ||
                       (!HasFeature(Feature::kWatermark) && !commandData.watermarkEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kOnScreenDisplay) && commandData.OSDEnabled.HasValue()) ||
                       (!HasFeature(Feature::kOnScreenDisplay) && !commandData.OSDEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // Call the delegate
    status =
        mDelegate.VideoStreamAllocate(streamUsage, videoCodec, minFrameRate, maxFrameRate, minResolution, maxResolution, minBitRate,
                                      maxBitRate, minFragmentLen, maxFragmentLen, isWaterMarkEnabled, isOSDEnabled, videoStreamID);

    if (status == Status::Success)
    {
        response.videoStreamID = videoStreamID;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}

void CameraAVStreamMgmtServer::HandleVideoStreamModify(HandlerContext & ctx,
                                                       const Commands::VideoStreamModify::DecodableType & commandData)
{
    Status status             = Status::Success;
    auto & isWaterMarkEnabled = commandData.watermarkEnabled;
    auto & isOSDEnabled       = commandData.OSDEnabled;
    auto & videoStreamID      = commandData.videoStreamID;

    // VideoStreamModify should have either the WMARK or OSD feature supported
    VerifyOrReturn(HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kWatermark) && commandData.watermarkEnabled.HasValue()) ||
                       (!HasFeature(Feature::kWatermark) && !commandData.watermarkEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kOnScreenDisplay) && commandData.OSDEnabled.HasValue()) ||
                       (!HasFeature(Feature::kOnScreenDisplay) && !commandData.OSDEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // Call the delegate
    status = mDelegate.VideoStreamModify(videoStreamID, isWaterMarkEnabled, isOSDEnabled);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAVStreamMgmtServer::HandleVideoStreamDeallocate(HandlerContext & ctx,
                                                           const Commands::VideoStreamDeallocate::DecodableType & commandData)
{

    auto & videoStreamID = commandData.videoStreamID;

    // Call the delegate
    Status status = mDelegate.VideoStreamDeallocate(videoStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleAudioStreamAllocate(HandlerContext & ctx,
                                                         const Commands::AudioStreamAllocate::DecodableType & commandData)
{

    Commands::AudioStreamAllocateResponse::Type response;
    auto & streamUsage     = commandData.streamUsage;
    auto & audioCodec      = commandData.audioCodec;
    auto & channelCount    = commandData.channelCount;
    auto & sampleRate      = commandData.sampleRate;
    auto & bitRate         = commandData.bitRate;
    auto & bitDepth        = commandData.bitDepth;
    uint16_t audioStreamID = 0;

    // Call the delegate
    Status status =
        mDelegate.AudioStreamAllocate(streamUsage, audioCodec, channelCount, sampleRate, bitRate, bitDepth, audioStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    response.audioStreamID = audioStreamID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleAudioStreamDeallocate(HandlerContext & ctx,
                                                           const Commands::AudioStreamDeallocate::DecodableType & commandData)
{

    auto & audioStreamID = commandData.audioStreamID;

    // Call the delegate
    Status status = mDelegate.AudioStreamDeallocate(audioStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamAllocate(HandlerContext & ctx,
                                                            const Commands::SnapshotStreamAllocate::DecodableType & commandData)
{

    Commands::SnapshotStreamAllocateResponse::Type response;
    auto & imageCodec         = commandData.imageCodec;
    auto & maxFrameRate       = commandData.maxFrameRate;
    auto & bitRate            = commandData.bitRate;
    auto & minResolution      = commandData.minResolution;
    auto & maxResolution      = commandData.maxResolution;
    auto & quality            = commandData.quality;
    uint16_t snapshotStreamID = 0;

    // Call the delegate
    Status status = mDelegate.SnapshotStreamAllocate(imageCodec, maxFrameRate, bitRate, minResolution, maxResolution, quality,
                                                     snapshotStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    response.snapshotStreamID = snapshotStreamID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamDeallocate(HandlerContext & ctx,
                                                              const Commands::SnapshotStreamDeallocate::DecodableType & commandData)
{

    auto & snapshotStreamID = commandData.snapshotStreamID;

    // Call the delegate
    Status status = mDelegate.SnapshotStreamDeallocate(snapshotStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleSetStreamPriorities(HandlerContext & ctx,
                                                         const Commands::SetStreamPriorities::DecodableType & commandData)
{

    auto & streamPriorities = commandData.streamPriorities;
    //auto & streamPriorities = commandData.streamPriorities;

    auto iter = streamPriorities.begin();
    StreamUsageEnum rankedStreamPriorities[kNumOfStreamUsageTypes];
    int i = 0;
    while(iter.Next())
    {
        auto & streamUsage = iter.GetValue();
        rankedStreamPriorities[i++] = streamUsage;
    }
    CHIP_ERROR err = SetRankedVideoStreamPriorities(rankedStreamPriorities);

    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    mDelegate.OnRankedStreamPrioritiesChanged();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleCaptureSnapshot(HandlerContext & ctx,
                                                     const Commands::CaptureSnapshot::DecodableType & commandData)
{

    Commands::CaptureSnapshotResponse::Type response;
    auto & snapshotStreamID    = commandData.snapshotStreamID;
    auto & requestedResolution = commandData.requestedResolution;

    // Call the delegate
    Status status = mDelegate.CaptureSnapshot(snapshotStreamID, requestedResolution);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Camera AV Stream Management Cluster Server Init
 *
 * Server Init
 *
 */
void MatterCameraAvStreamManagementPluginServerInitCallback() {}
