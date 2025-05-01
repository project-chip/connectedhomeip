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

#include <cstring>
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
    CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
    const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxConcurrentEncoders, uint32_t aMaxEncodedPixelRate,
    const VideoSensorParamsStruct & aVideoSensorParams, bool aNightVisionCapable, const VideoResolutionStruct & aMinViewPort,
    const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> & aRateDistortionTradeOffPoints,
    uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
    const AudioCapabilitiesStruct & aSpeakerCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
    const std::vector<Structs::SnapshotCapabilitiesStruct::Type> & aSnapshotCapabilities, uint32_t aMaxNetworkBandwidth,
    const std::vector<StreamUsageEnum> & aSupportedStreamUsages, const std::vector<StreamUsageEnum> & aRankedStreamPriorities) :
    CommandHandlerInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), CameraAvStreamManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs), mMaxConcurrentEncoders(aMaxConcurrentEncoders),
    mMaxEncodedPixelRate(aMaxEncodedPixelRate), mVideoSensorParams(aVideoSensorParams), mNightVisionCapable(aNightVisionCapable),
    mMinViewPort(aMinViewPort), mRateDistortionTradeOffPointsList(aRateDistortionTradeOffPoints),
    mMaxContentBufferSize(aMaxContentBufferSize), mMicrophoneCapabilities(aMicrophoneCapabilities),
    mSpeakerCapabilities(aSpeakerCapabilities), mTwoWayTalkSupport(aTwoWayTalkSupport),
    mSnapshotCapabilitiesList(aSnapshotCapabilities), mMaxNetworkBandwidth(aMaxNetworkBandwidth),
    mSupportedStreamUsages(aSupportedStreamUsages), mRankedVideoStreamPriorities(aRankedStreamPriorities)
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
    // Constraint checks for RateDistortionTardeOffPoints vector
    for (const auto & rateDistortionTradeOffPoints : mRateDistortionTradeOffPointsList)
    {
        VerifyOrReturnError(
            rateDistortionTradeOffPoints.minBitRate >= 1, CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: RateDistortionTradeOffPoints configuration error", mEndpointId));
    }

    // At least one of Video, Audio or Snapshot needs to be supported.
    VerifyOrReturnError(
        HasFeature(Feature::kVideo) || HasFeature(Feature::kAudio) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(
            Zcl,
            "CameraAVStreamMgmt[ep=%d]: Feature configuration error. At least one of Audio, Video, or Snapshot feature required",
            mEndpointId));

    if (HasFeature(Feature::kImageControl) || HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay) ||
        HasFeature(Feature::kHighDynamicRange))
    {
        VerifyOrReturnError(HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageControl, Watermark, "
                                         "OnScreenDisplay or HighDynamicRange, then Video or Snapshot feature required",
                                         mEndpointId));
    }

    if (HasFeature(Feature::kSpeaker))
    {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if Speaker is enabled, then "
                                         "Audio feature required",
                                         mEndpointId));
    }

    // Ensure Optional attribute bits have been correctly passed.
    if (SupportsOptAttr(OptionalAttribute::kNightVision) || SupportsOptAttr(OptionalAttribute::kNightVisionIllum))
    {
        VerifyOrReturnError(HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if NIghtVision is enabled, then "
                                         "Video|Snapshot feature required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(
                Zcl, "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if MicrophoneAGCEnabled, then Audio feature required",
                mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal) || SupportsOptAttr(OptionalAttribute::kImageFlipVertical) ||
        SupportsOptAttr(OptionalAttribute::kImageRotation))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kImageControl), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageFlip or Rotation enabled, then "
                         "ImageControl feature required",
                         mEndpointId));
    }

    if (HasFeature(Feature::kImageControl))
    {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal) || SupportsOptAttr(OptionalAttribute::kImageFlipVertical) ||
                SupportsOptAttr(OptionalAttribute::kImageRotation),
            CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageControl feature supported, then "
                         "at least one of ImageFlip or Rotation should be supported",
                         mEndpointId));
    }

    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool CameraAVStreamMgmtServer::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
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

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeSnapshotCapabilities(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & snapshotCapabilities : mSnapshotCapabilitiesList)
    {
        ReturnErrorOnFailure(encoder.Encode(snapshotCapabilities));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeSupportedStreamUsages(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & supportedStreamUsage : mSupportedStreamUsages)
    {
        ReturnErrorOnFailure(encoder.Encode(supportedStreamUsage));
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
    for (const auto & snapshotStream : mAllocatedSnapshotStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(snapshotStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamMgmtServer::ReadAndEncodeRankedVideoStreamPrioritiesList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & streamUsage : mRankedVideoStreamPriorities)
    {
        ReturnErrorOnFailure(encoder.Encode(streamUsage));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetRankedVideoStreamPriorities(const std::vector<StreamUsageEnum> & newPriorities)
{
    mRankedVideoStreamPriorities = newPriorities;
    ReturnErrorOnFailure(StoreRankedVideoStreamPriorities());
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::RankedVideoStreamPrioritiesList::Id);
    mDelegate.OnAttributeChanged(Attributes::RankedVideoStreamPrioritiesList::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddVideoStream(const VideoStreamStruct & videoStream)
{
    mAllocatedVideoStreams.push_back(videoStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedVideoStreams::Id);
    mDelegate.OnAttributeChanged(Attributes::AllocatedVideoStreams::Id);
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
    mDelegate.OnAttributeChanged(Attributes::AllocatedVideoStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddAudioStream(const AudioStreamStruct & audioStream)
{
    mAllocatedAudioStreams.push_back(audioStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedAudioStreams::Id);
    mDelegate.OnAttributeChanged(Attributes::AllocatedAudioStreams::Id);
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
    mDelegate.OnAttributeChanged(Attributes::AllocatedAudioStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::AddSnapshotStream(const SnapshotStreamStruct & snapshotStream)
{
    mAllocatedSnapshotStreams.push_back(snapshotStream);
    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::AllocatedSnapshotStreams::Id);
    mDelegate.OnAttributeChanged(Attributes::AllocatedSnapshotStreams::Id);
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
    mDelegate.OnAttributeChanged(Attributes::AllocatedSnapshotStreams::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR CameraAVStreamMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvStreamManagement::Id);
    ChipLogProgress(Zcl, "Camera AVStream Management[ep=%d]: Reading", mEndpointId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;
    case MaxConcurrentEncoders::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MaxConcurrentEncoders, feature is not supported",
                                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxConcurrentEncoders));
        break;
    case MaxEncodedPixelRate::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MaxEncodedPixelRate, feature is not supported", mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxEncodedPixelRate));
        break;
    case VideoSensorParams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get VideoSensorParams, feature is not supported", mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mVideoSensorParams));
        break;
    case NightVisionCapable::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get VideoSensorParams, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionCapable));
        break;
    case MinViewport::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MinViewport, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMinViewPort));
        break;
    case RateDistortionTradeOffPoints::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get RateDistortionTradeOffPoints, feature is not supported",
                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeRateDistortionTradeOffPoints(encoder); }));
        break;
    case MaxContentBufferSize::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxContentBufferSize));
        break;
    case MicrophoneCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneCapabilities, feature is not supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneCapabilities));
        break;
    case SpeakerCapabilities::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerCapabilities, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerCapabilities));
        break;
    case TwoWayTalkSupport::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get TwoWayTalkSupport, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mTwoWayTalkSupport));
        break;
    case SnapshotCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get SnapshotCapabilities, feature is not supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSnapshotCapabilities(encoder); }));
        break;
    case MaxNetworkBandwidth::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxNetworkBandwidth));
        break;
    case CurrentFrameRate::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get CurrentFrameRate, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mCurrentFrameRate));
        break;
    case HDRModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kHighDynamicRange), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get HDRModeEnabled, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mHDRModeEnabled));
        break;
    case SupportedStreamUsages::Id:
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSupportedStreamUsages(encoder); }));
        break;
    case AllocatedVideoStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get AllocatedVideoStreams, feature is not supported",
                                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedVideoStreams(encoder); }));
        break;
    case AllocatedAudioStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get AllocatedAudioStreams, feature is not supported",
                                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedAudioStreams(encoder); }));
        break;
    case AllocatedSnapshotStreams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get AllocatedSnapshotStreams, feature is not supported",
                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedSnapshotStreams(encoder); }));
        break;
    case RankedVideoStreamPrioritiesList::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get RankedVideoStreamPrioritiesList, feature is not supported",
                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeRankedVideoStreamPrioritiesList(encoder); }));
        break;
    case SoftRecordingPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SoftRecordingPrivacyModeEnabled, feature is not supported",
                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftRecordingPrivacyModeEnabled));
        break;
    case SoftLivestreamPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SoftLivestreamPrivacyModeEnabled, feature is not supported",
                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftLivestreamPrivacyModeEnabled));
        break;
    case HardPrivacyModeOn::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kHardPrivacyModeOn), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get HardPrivacyModeOn, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mHardPrivacyModeOn));
        break;
    case NightVision::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kNightVision), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get NightVision, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVision));
        break;
    case NightVisionIllum::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kNightVisionIllum), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get NightVisionIllumination, feature is not supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionIllum));
        break;
    case Viewport::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get Viewport, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mViewport));
        break;
    case SpeakerMuted::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMuted, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMuted));
        break;
    case SpeakerVolumeLevel::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerVolumeLevel, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerVolumeLevel));
        break;
    case SpeakerMaxLevel::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMaxLevel, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMaxLevel));
        break;
    case SpeakerMinLevel::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMinLevel, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMinLevel));
        break;
    case MicrophoneMuted::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMuted, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMuted));
        break;
    case MicrophoneVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneVolumeLevel, feature is not supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneVolumeLevel));
        break;
    case MicrophoneMaxLevel::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMaxLevel, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMaxLevel));
        break;
    case MicrophoneMinLevel::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMinLevel, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMinLevel));
        break;
    case MicrophoneAGCEnabled::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneAGCEnabled, feature is not supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneAGCEnabled));
        break;
    case ImageRotation::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageRotation), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get ImageRotation, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageRotation));
        break;
    case ImageFlipHorizontal::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get ImageFlipHorizontal, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipHorizontal));
        break;
    case ImageFlipVertical::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipVertical), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get ImageFlipHorizontal, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipVertical));
        break;
    case LocalVideoRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get LocalVideoRecordingEnabled, feature is not supported",
                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalVideoRecordingEnabled));
        break;
    case LocalSnapshotRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get LocalSnapshotRecordingEnabled, feature is not supported",
                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalSnapshotRecordingEnabled));
        break;
    case StatusLightEnabled::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kStatusLightEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get StatusLightEnabled, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightEnabled));
        break;
    case StatusLightBrightness::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightBrightness), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get StatusLightBrightness, feature is not supported",
                                         mEndpointId));
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
        VerifyOrReturnError(
            HasFeature(Feature::kHighDynamicRange), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set HDRModeEnabled, feature is not supported", mEndpointId));

        bool hdrModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(hdrModeEnabled));
        return SetHDRModeEnabled(hdrModeEnabled);
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SoftRecordingPrivacyModeEnabled, feature is not supported",
                         mEndpointId));

        bool softRecPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softRecPrivModeEnabled));
        return SetSoftRecordingPrivacyModeEnabled(softRecPrivModeEnabled);
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SoftLivestreamPrivacyModeEnabled, feature is not supported",
                         mEndpointId));

        bool softLivestreamPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softLivestreamPrivModeEnabled));
        return SetSoftLivestreamPrivacyModeEnabled(softLivestreamPrivModeEnabled);
    }
    case NightVision::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kNightVision), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set NightVision, feature is not supported", mEndpointId));

        TriStateAutoEnum nightVision;
        ReturnErrorOnFailure(aDecoder.Decode(nightVision));
        return SetNightVision(nightVision);
    }
    case NightVisionIllum::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kNightVisionIllum), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set NightVisionIllumination, feature is not supported",
                                         mEndpointId));

        TriStateAutoEnum nightVisionIllum;
        ReturnErrorOnFailure(aDecoder.Decode(nightVisionIllum));
        return SetNightVisionIllum(nightVisionIllum);
    }
    case Viewport::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set Viewport, feature is not supported", mEndpointId));
        ViewportStruct viewPort;
        ReturnErrorOnFailure(aDecoder.Decode(viewPort));
        return SetViewport(viewPort);
    }
    case SpeakerMuted::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SpeakerMuted, feature is not supported", mEndpointId));
        bool speakerMuted;
        ReturnErrorOnFailure(aDecoder.Decode(speakerMuted));
        return SetSpeakerMuted(speakerMuted);
    }
    case SpeakerVolumeLevel::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SpeakerVolumeLevel, feature is not supported", mEndpointId));
        uint8_t speakerVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(speakerVolLevel));
        return SetSpeakerVolumeLevel(speakerVolLevel);
    }
    case MicrophoneMuted::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneMuted, feature is not supported", mEndpointId));
        bool micMuted;
        ReturnErrorOnFailure(aDecoder.Decode(micMuted));
        return SetMicrophoneMuted(micMuted);
    }
    case MicrophoneVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneVolumeLevel, feature is not supported",
                                         mEndpointId));
        uint8_t micVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(micVolLevel));
        return SetMicrophoneVolumeLevel(micVolLevel);
    }
    case MicrophoneAGCEnabled::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneAGCEnabled, feature is not supported",
                                         mEndpointId));
        bool micAGCEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(micAGCEnabled));
        return SetMicrophoneAGCEnabled(micAGCEnabled);
    }
    case ImageRotation::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageRotation), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set ImageRotation, feature is not supported", mEndpointId));
        uint16_t imageRotation;
        ReturnErrorOnFailure(aDecoder.Decode(imageRotation));
        return SetImageRotation(imageRotation);
    }
    case ImageFlipHorizontal::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set ImageFlipHorizontal, feature is not supported", mEndpointId));
        bool imageFlipHorizontal;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipHorizontal));
        return SetImageFlipHorizontal(imageFlipHorizontal);
    }
    case ImageFlipVertical::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipVertical), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set ImageFlipVertical, feature is not supported", mEndpointId));
        bool imageFlipVertical;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipVertical));
        return SetImageFlipVertical(imageFlipVertical);
    }
    case LocalVideoRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set LocalVideoRecordingEnabled, feature is not supported",
                         mEndpointId));
        bool localVidRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localVidRecEnabled));
        return SetLocalVideoRecordingEnabled(localVidRecEnabled);
    }
    case LocalSnapshotRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set LocalSnapshotRecordingEnabled, feature is not supported",
                         mEndpointId));
        bool localSnapshotRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localSnapshotRecEnabled));
        return SetLocalSnapshotRecordingEnabled(localSnapshotRecEnabled);
    }
    case StatusLightEnabled::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kStatusLightEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set StatusLightEnabled, feature is not supported", mEndpointId));
        bool statusLightEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightEnabled));
        return SetStatusLightEnabled(statusLightEnabled);
    }
    case StatusLightBrightness::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightBrightness), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set StatusLightBrightness, feature is not supported",
                                         mEndpointId));
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
        mDelegate.OnAttributeChanged(Attributes::NightVision::Id);
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
        mDelegate.OnAttributeChanged(Attributes::NightVisionIllum::Id);
        MatterReportingAttributeChangeCallback(path);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::SetViewport(const ViewportStruct & aViewport)
{
    mViewport = aViewport;

    StoreViewport(mViewport);
    mDelegate.OnAttributeChanged(Attributes::Viewport::Id);
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
        mDelegate.OnAttributeChanged(Attributes::StatusLightBrightness::Id);
        MatterReportingAttributeChangeCallback(path);
    }
    return CHIP_NO_ERROR;
}

void CameraAVStreamMgmtServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load HDR Mode Enabled
    bool storedHDRModeEnabled = false;
    err                       = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::HDRModeEnabled::Id), storedHDRModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mHDRModeEnabled = storedHDRModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded HDRModeEnabled as %u", mEndpointId, mHDRModeEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the HDRModeEnabled from the KVS. Defaulting to %u",
                      mEndpointId, mHDRModeEnabled);
    }

    // Load AllocatedVideoStreams
    err = mDelegate.LoadAllocatedVideoStreams(mAllocatedVideoStreams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated video streams from the KVS.", mEndpointId);
    }
    // Load AllocatedAudioStreams
    err = mDelegate.LoadAllocatedAudioStreams(mAllocatedAudioStreams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated audio streams from the KVS.", mEndpointId);
    }

    // Load AllocatedSnapshotStreams
    err = mDelegate.LoadAllocatedSnapshotStreams(mAllocatedSnapshotStreams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated snapshot streams from the KVS.", mEndpointId);
    }

    err = LoadRankedVideoStreamPriorities();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the RankedVideoStreamPrioritiesList from the KVS.",
                      mEndpointId);
    }

    // Load SoftRecordingPrivacyModeEnabled
    bool softRecordingPrivacyModeEnabled = false;
    err                                  = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftRecordingPrivacyModeEnabled::Id),
        softRecordingPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftRecordingPrivacyModeEnabled = softRecordingPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SoftRecordingPrivacyModeEnabled as %u", mEndpointId,
                      mSoftRecordingPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(
            Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SoftRecordingPrivacyModeEnabled from the KVS. Defaulting to %u",
            mEndpointId, mSoftRecordingPrivacyModeEnabled);
    }

    // Load SoftLivestreamPrivacyModeEnabled
    bool softLivestreamPrivacyModeEnabled = false;
    err                                   = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftLivestreamPrivacyModeEnabled::Id),
        softLivestreamPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftLivestreamPrivacyModeEnabled = softLivestreamPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SoftLivestreamPrivacyModeEnabled as %u", mEndpointId,
                      mSoftLivestreamPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(
            Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SoftLivestreamPrivacyModeEnabled from the KVS. Defaulting to %u",
            mEndpointId, mSoftLivestreamPrivacyModeEnabled);
    }

    // Load NightVision
    uint8_t nightVision = 0;
    err                 = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVision::Id), nightVision);
    if (err == CHIP_NO_ERROR)
    {
        mNightVision = static_cast<TriStateAutoEnum>(nightVision);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded NightVision as %d", mEndpointId, to_underlying(mNightVision));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the NightVision from the KVS. Defaulting to %d", mEndpointId,
                      to_underlying(mNightVision));
    }

    // Load NightVisionIllum
    uint8_t nightVisionIllum = 0;
    err                      = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVisionIllum::Id), nightVisionIllum);
    if (err == CHIP_NO_ERROR)
    {
        mNightVisionIllum = static_cast<TriStateAutoEnum>(nightVisionIllum);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded NightVisionIllum as %d", mEndpointId,
                      to_underlying(mNightVisionIllum));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the NightVisionIllum from the KVS. Defaulting to %d",
                      mEndpointId, to_underlying(mNightVisionIllum));
    }

    // Load Viewport
    ViewportStruct viewport;
    err = LoadViewport(viewport);
    if (err == CHIP_NO_ERROR)
    {
        mViewport = viewport;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded Viewport", mEndpointId);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the Viewport from the KVS.", mEndpointId);
    }

    // Load SpeakerMuted
    bool speakerMuted = false;
    err               = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerMuted::Id), speakerMuted);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerMuted = speakerMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SpeakerMuted as %u", mEndpointId, mSpeakerMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SpeakerMuted from the KVS. Defaulting to %u", mEndpointId,
                      mSpeakerMuted);
    }

    // Load SpeakerVolumeLevel
    uint8_t speakerVolumeLevel = 0;
    err                        = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerVolumeLevel::Id), speakerVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerVolumeLevel = speakerVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SpeakerVolumeLevel as %u", mEndpointId, mSpeakerVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SpeakerVolumeLevel from the KVS. Defaulting to %u",
                      mEndpointId, mSpeakerVolumeLevel);
    }

    // Load MicrophoneMuted
    bool microphoneMuted = false;
    err                  = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneMuted::Id), microphoneMuted);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneMuted = microphoneMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneMuted as %u", mEndpointId, mMicrophoneMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneMuted from the KVS. Defaulting to %u",
                      mEndpointId, mMicrophoneMuted);
    }

    // Load MicrophoneVolumeLevel
    uint8_t microphoneVolumeLevel = 0;
    err                           = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneVolumeLevel::Id),
        microphoneVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneVolumeLevel = microphoneVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneVolumeLevel as %u", mEndpointId, mMicrophoneVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneVolumeLevel from the KVS. Defaulting to %u",
                      mEndpointId, mMicrophoneVolumeLevel);
    }

    // Load MicrophoneAGCEnabled
    bool microphoneAGCEnabled = false;
    err                       = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneAGCEnabled::Id),
        microphoneAGCEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneAGCEnabled = microphoneAGCEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneAGCEnabled as %u", mEndpointId, mMicrophoneAGCEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneAGCEnabled from the KVS. Defaulting to %u",
                      mEndpointId, mMicrophoneAGCEnabled);
    }

    // Load ImageRotation
    uint16_t imageRotation = 0;
    err                    = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageRotation::Id), imageRotation);
    if (err == CHIP_NO_ERROR)
    {
        mImageRotation = imageRotation;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageRotation as %u", mEndpointId, mImageRotation);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageRotation from the KVS. Defaulting to %u",
                      mEndpointId, mImageRotation);
    }

    // Load ImageFlipHorizontal
    bool imageFlipHorizontal = false;
    err                      = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipHorizontal::Id), imageFlipHorizontal);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipHorizontal = imageFlipHorizontal;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageFlipHorizontal as %u", mEndpointId, mImageFlipHorizontal);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageFlipHorizontal from the KVS. Defaulting to %u",
                      mEndpointId, mImageFlipHorizontal);
    }

    // Load ImageFlipVertical
    bool imageFlipVertical = false;
    err                    = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipVertical::Id), imageFlipVertical);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipVertical = imageFlipVertical;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageFlipVertical as %u", mEndpointId, mImageFlipVertical);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageFlipVertical from the KVS. Defaulting to %u",
                      mEndpointId, mImageFlipVertical);
    }

    // Load LocalVideoRecordingEnabled
    bool localVideoRecordingEnabled = false;
    err                             = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalVideoRecordingEnabled::Id),
        localVideoRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalVideoRecordingEnabled = localVideoRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded LocalVideoRecordingEnabled as %u", mEndpointId,
                      mLocalVideoRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl,
                      "CameraAVStreamMgmt[ep=%d]: Unable to load the LocalVideoRecordingEnabled from the KVS. Defaulting to %u",
                      mEndpointId, mLocalVideoRecordingEnabled);
    }

    // Load LocalSnapshotRecordingEnabled
    bool localSnapshotRecordingEnabled = false;
    err                                = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalSnapshotRecordingEnabled::Id),
        localSnapshotRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalSnapshotRecordingEnabled = localSnapshotRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded LocalSnapshotRecordingEnabled as %u", mEndpointId,
                      mLocalSnapshotRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl,
                      "CameraAVStreamMgmt[ep=%d]: Unable to load the LocalSnapshotRecordingEnabled from the KVS. Defaulting to %u",
                      mEndpointId, mLocalSnapshotRecordingEnabled);
    }

    // Load StatusLightEnabled
    bool statusLightEnabled = false;
    err                     = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightEnabled::Id), statusLightEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightEnabled = statusLightEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded StatusLightEnabled as %u", mEndpointId, mStatusLightEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the StatusLightEnabled from the KVS. Defaulting to %u",
                      mEndpointId, mStatusLightEnabled);
    }

    // Load StatusLightBrightness
    uint8_t statusLightBrightness = 0;
    err                           = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightBrightness::Id),
        statusLightBrightness);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightBrightness = static_cast<Globals::ThreeLevelAutoEnum>(statusLightBrightness);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded StatusLightBrightness as %d", mEndpointId,
                      to_underlying(mStatusLightBrightness));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the StatusLightBrightness from the KVS. Defaulting to %d",
                      mEndpointId, to_underlying(mStatusLightBrightness));
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate.PersistentAttributesLoadedCallback();
}

CHIP_ERROR CameraAVStreamMgmtServer::StoreViewport(const ViewportStruct & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;

    writer.Init(bufferSpan);
    ReturnErrorOnFailure(viewport.Encode(writer, TLV::AnonymousTag()));

    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    bufferSpan.reduce_size(writer.GetLengthWritten());

    return GetSafeAttributePersistenceProvider()->SafeWriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAVStreamMgmtServer::LoadViewport(ViewportStruct & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeReadValue(path, bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(viewport.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::StoreRankedVideoStreamPriorities()
{
    uint8_t buffer[kRankedVideoStreamPrioritiesTlvSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;
    writer.Init(bufferSpan);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < mRankedVideoStreamPriorities.size(); i++)
    {
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), mRankedVideoStreamPriorities[i]));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    bufferSpan.reduce_size(writer.GetLengthWritten());

    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::RankedVideoStreamPrioritiesList::Id);
    return GetSafeAttributePersistenceProvider()->SafeWriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAVStreamMgmtServer::LoadRankedVideoStreamPriorities()
{
    uint8_t buffer[kRankedVideoStreamPrioritiesTlvSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mEndpointId, CameraAvStreamManagement::Id, Attributes::RankedVideoStreamPrioritiesList::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeReadValue(path, bufferSpan));

    TLV::TLVReader reader;
    reader.Init(bufferSpan);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    mRankedVideoStreamPriorities.clear();
    StreamUsageEnum streamUsage;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(streamUsage));
        mRankedVideoStreamPriorities.push_back(streamUsage);
    }

    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

// CommandHandlerInterface
void CameraAVStreamMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::VideoStreamAllocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Video Stream", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Modifying Video Stream", mEndpointId);

        // VideoStreamModify should have either the WMARK or OSD feature supported
        VerifyOrReturn(HasFeature(Feature::kVideo) && (HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay)),
                       handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand));

        HandleCommand<Commands::VideoStreamModify::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleVideoStreamModify(ctx, commandData); });

        return;

    case Commands::VideoStreamDeallocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Video Stream", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Audio Stream", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Audio Stream", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Snapshot Stream", mEndpointId);

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

    case Commands::SnapshotStreamModify::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Modifying Snapshot Stream", mEndpointId);

        // SnapshotStreamModify should have either the WMARK or OSD feature supported
        VerifyOrReturn(HasFeature(Feature::kSnapshot) && (HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay)),
                       handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand));

        HandleCommand<Commands::SnapshotStreamModify::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSnapshotStreamModify(ctx, commandData); });

        return;

    case Commands::SnapshotStreamDeallocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Snapshot Stream", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Set Stream Priorities", mEndpointId);
        HandleCommand<Commands::SetStreamPriorities::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSetStreamPriorities(ctx, commandData); });
        return;

    case Commands::CaptureSnapshot::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Capture Snapshot image", mEndpointId);

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
    uint16_t videoStreamID = 0;

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kWatermark) == commandData.watermarkEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kOnScreenDisplay) == commandData.OSDEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    VerifyOrReturn(commandData.streamUsage != StreamUsageEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid stream usage", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(commandData.videoCodec != VideoCodecEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid video codec", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(commandData.minFrameRate >= 1 && commandData.minFrameRate <= commandData.maxFrameRate &&
                       commandData.maxFrameRate >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.minResolution.width >= 1 && commandData.minResolution.height >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.maxResolution.width >= 1 && commandData.maxResolution.height >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.minBitRate >= 1 && commandData.minBitRate <= commandData.maxBitRate && commandData.maxBitRate >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.minFragmentLen <= commandData.maxFragmentLen &&
                       commandData.maxFragmentLen <= kMaxFragmentLenMaxValue,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VideoStreamStruct videoStreamArgs;
    videoStreamArgs.videoStreamID    = 0;
    videoStreamArgs.streamUsage      = commandData.streamUsage;
    videoStreamArgs.videoCodec       = commandData.videoCodec;
    videoStreamArgs.minFrameRate     = commandData.minFrameRate;
    videoStreamArgs.maxFrameRate     = commandData.maxFrameRate;
    videoStreamArgs.minResolution    = commandData.minResolution;
    videoStreamArgs.maxResolution    = commandData.maxResolution;
    videoStreamArgs.minBitRate       = commandData.minBitRate;
    videoStreamArgs.maxBitRate       = commandData.maxBitRate;
    videoStreamArgs.minFragmentLen   = commandData.minFragmentLen;
    videoStreamArgs.maxFragmentLen   = commandData.maxFragmentLen;
    videoStreamArgs.watermarkEnabled = commandData.watermarkEnabled;
    videoStreamArgs.OSDEnabled       = commandData.OSDEnabled;
    videoStreamArgs.referenceCount   = 0;

    // Call the delegate
    status = mDelegate.VideoStreamAllocate(videoStreamArgs, videoStreamID);

    if (status == Status::Success)
    {
        // Add the allocated videostream object in the AllocatedVideoStreams list.
        videoStreamArgs.videoStreamID = videoStreamID;
        AddVideoStream(videoStreamArgs);

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

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kWatermark) == commandData.watermarkEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kOnScreenDisplay) == commandData.OSDEnabled.HasValue()),
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

    if (status == Status::Success)
    {
        RemoveVideoStream(videoStreamID);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAVStreamMgmtServer::HandleAudioStreamAllocate(HandlerContext & ctx,
                                                         const Commands::AudioStreamAllocate::DecodableType & commandData)
{

    Commands::AudioStreamAllocateResponse::Type response;
    uint16_t audioStreamID = 0;

    VerifyOrReturn(commandData.streamUsage != StreamUsageEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid stream usage", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(commandData.audioCodec != AudioCodecEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid audio codec", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(commandData.sampleRate > 0, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid sampleRate", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(commandData.bitRate > 0, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid bitRate", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(IsBitDepthValid(commandData.bitDepth), {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid bitDepth", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(commandData.channelCount >= 1 && commandData.channelCount <= kMaxChannelCount, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid channel count", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    AudioStreamStruct audioStreamArgs;
    audioStreamArgs.audioStreamID  = 0;
    audioStreamArgs.streamUsage    = commandData.streamUsage;
    audioStreamArgs.audioCodec     = commandData.audioCodec;
    audioStreamArgs.channelCount   = commandData.channelCount;
    audioStreamArgs.sampleRate     = commandData.sampleRate;
    audioStreamArgs.bitRate        = commandData.bitRate;
    audioStreamArgs.bitDepth       = commandData.bitDepth;
    audioStreamArgs.referenceCount = 0;

    // Call the delegate
    Status status = mDelegate.AudioStreamAllocate(audioStreamArgs, audioStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Add the allocated audiostream object in the AllocatedAudioStreams list.
    audioStreamArgs.audioStreamID = audioStreamID;
    AddAudioStream(audioStreamArgs);

    response.audioStreamID = audioStreamID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void CameraAVStreamMgmtServer::HandleAudioStreamDeallocate(HandlerContext & ctx,
                                                           const Commands::AudioStreamDeallocate::DecodableType & commandData)
{

    auto & audioStreamID = commandData.audioStreamID;

    // Call the delegate
    Status status = mDelegate.AudioStreamDeallocate(audioStreamID);

    if (status == Status::Success)
    {
        RemoveAudioStream(audioStreamID);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamAllocate(HandlerContext & ctx,
                                                            const Commands::SnapshotStreamAllocate::DecodableType & commandData)
{

    Commands::SnapshotStreamAllocateResponse::Type response;
    uint16_t snapshotStreamID = 0;

    VerifyOrReturn(commandData.imageCodec != ImageCodecEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid image codec", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
    });

    VerifyOrReturn(commandData.maxFrameRate > 0, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid maxFrameRate", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    VerifyOrReturn(commandData.minResolution.width >= 1 && commandData.minResolution.height >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.maxResolution.width >= 1 && commandData.maxResolution.height >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    VerifyOrReturn(commandData.quality > 0 && commandData.quality <= kMaxImageQualityMetric, {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid image quality", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
    });

    SnapshotStreamStruct snapshotStreamArgs;
    snapshotStreamArgs.snapshotStreamID = 0;
    snapshotStreamArgs.imageCodec       = commandData.imageCodec;
    snapshotStreamArgs.frameRate        = commandData.maxFrameRate;
    snapshotStreamArgs.minResolution    = commandData.minResolution;
    snapshotStreamArgs.maxResolution    = commandData.maxResolution;
    snapshotStreamArgs.quality          = commandData.quality;
    snapshotStreamArgs.referenceCount   = 0;

    // Call the delegate
    Status status = mDelegate.SnapshotStreamAllocate(snapshotStreamArgs, snapshotStreamID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Add the allocated snapshotstream object in the AllocatedSnapshotStreams list.
    snapshotStreamArgs.snapshotStreamID = snapshotStreamID;
    AddSnapshotStream(snapshotStreamArgs);

    response.snapshotStreamID = snapshotStreamID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamModify(HandlerContext & ctx,
                                                          const Commands::SnapshotStreamModify::DecodableType & commandData)
{
    Status status             = Status::Success;
    auto & isWaterMarkEnabled = commandData.watermarkEnabled;
    auto & isOSDEnabled       = commandData.OSDEnabled;
    auto & snapshotStreamID   = commandData.snapshotStreamID;

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kWatermark) == commandData.watermarkEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturn((HasFeature(Feature::kOnScreenDisplay) == commandData.OSDEnabled.HasValue()),
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand));

    // Call the delegate
    status = mDelegate.SnapshotStreamModify(snapshotStreamID, isWaterMarkEnabled, isOSDEnabled);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamDeallocate(HandlerContext & ctx,
                                                              const Commands::SnapshotStreamDeallocate::DecodableType & commandData)
{

    auto & snapshotStreamID = commandData.snapshotStreamID;

    // Call the delegate
    Status status = mDelegate.SnapshotStreamDeallocate(snapshotStreamID);

    if (status == Status::Success)
    {
        RemoveSnapshotStream(snapshotStreamID);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAVStreamMgmtServer::HandleSetStreamPriorities(HandlerContext & ctx,
                                                         const Commands::SetStreamPriorities::DecodableType & commandData)
{

    auto & streamPriorities = commandData.streamPriorities;
    std::vector<StreamUsageEnum> rankedStreamPriorities;
    auto iter = streamPriorities.begin();

    while (iter.Next())
    {
        auto & streamUsage = iter.GetValue();
        if (streamUsage == StreamUsageEnum::kUnknownEnumValue)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        rankedStreamPriorities.push_back(streamUsage);
    }

    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    CHIP_ERROR err = SetRankedVideoStreamPriorities(rankedStreamPriorities);

    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    mDelegate.OnRankedStreamPrioritiesChanged();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void CameraAVStreamMgmtServer::HandleCaptureSnapshot(HandlerContext & ctx,
                                                     const Commands::CaptureSnapshot::DecodableType & commandData)
{

    Commands::CaptureSnapshotResponse::Type response;
    auto & snapshotStreamID    = commandData.snapshotStreamID;
    auto & requestedResolution = commandData.requestedResolution;
    ImageSnapshot image;

    VerifyOrReturn(commandData.requestedResolution.width >= 1 && commandData.requestedResolution.height >= 1,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError));

    // Call the delegate
    Status status = mDelegate.CaptureSnapshot(snapshotStreamID, requestedResolution, image);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    if (image.data.size() > kMaxSnapshotImageSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    // Populate the response
    response.data       = ByteSpan(image.data.data(), image.data.size());
    response.resolution = image.imageRes;
    response.imageCodec = image.imageCodec;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
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
void MatterCameraAvStreamManagementPluginServerShutdownCallback() {}
