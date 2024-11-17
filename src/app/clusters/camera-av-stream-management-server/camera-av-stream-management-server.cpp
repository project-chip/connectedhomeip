/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAVStreamMgmt;
using namespace chip::app::Clusters::CameraAVStreamMgmt::Structs;
using namespace chip::app::Clusters::CameraAVStreamMgmt::Attributes;
using namespace Protocols::InteractionModel;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAVStreamMgmt {

CameraAVStreamMgmtServer::CameraAVStreamMgmtServer(CameraAVStreamMgmtDelegate & aDelegate, EndpointId aEndpointId,
                                                   ClusterId aClusterId, BitMask<Feature> aFeature,
                                                   OptionalAttributes aOptionalAttrs, uint8_t aMaxConcurrentVideoEncoders,
                                                   uint32_t aMaxEncodedPixelRate,
                                                   const VideoSensorParamsStruct & aVideoSensorParams, bool aNightVisionCapable,
                                                   const VideoResolutionStruct & aMinViewPort, uint32_t aMaxContentBufferSize,
                                                   const AudioCapabilitiesStruct & aMicrophoneCapabilities,
                                                   const AudioCapabilitiesStruct & aSpkrCapabilities,
                                                   TwoWayTalkSupportTypeEnum aTwoWayTalkSupport, uint32_t aMaxNetworkBandwidth) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId),
    AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mClusterId(aClusterId), mFeature(aFeature), mOptionalAttrs(aOptionalAttrs),
    mMaxConcurrentVideoEncoders(aMaxConcurrentVideoEncoders), mMaxEncodedPixelRate(aMaxEncodedPixelRate),
    mVideoSensorParams(aVideoSensorParams), mNightVisionCapable(aNightVisionCapable), mMinViewPort(aMinViewPort),
    mMaxContentBufferSize(aMaxContentBufferSize), mMicrophoneCapabilities(aMicrophoneCapabilities),
    mSpeakerCapabilities(aSpeakerCapabilities), mTwoWayTalkSupport(aTwoWayTalkSupport), mMaxNetworkBandwidth(aMaxNetworkBandwidth)
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
    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool CameraAVStreamMgmtServer::HasFeature(Feature feature) const
{
    return mFeature.Has(feature);
}

bool CameraAVStreamMgmtServer::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

bool CameraAVStreamMgmtServer::IsLocalVideoRecordingEnabled() const
{
    return mLocalVideoRecordingEnabled;
}

CHIP_ERROR
CameraAVStreamMgmtServer::ReadAndEncodeRateDistortionTradeOffPoints(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartRateDistortionTradeOffPointsRead());

    for (uint8_t i = 0; true; i++)
    {
        Structs::RateDistortionTradeOffPointsStruct::Type rateDistortionTradeOffPoints;

        err = mDelegate.GetRateDistortionTradeOffPointByIndex(i, rateDistortionTradeOffPoints);
        SuccessOrExit(err);

        err = encoder.Encode(rateDistortionTradeOffPoints);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndRateDistortionTradeOffPointsRead();
    return err;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeSupportedSnapshotParams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartSupportedSnapshotParamsRead());

    for (uint8_t i = 0; true; i++)
    {
        Structs::SupportedSnapshotParamsStruct::Type supportedSnapshotParams;

        err = mDelegate.GetSupportedSnapshotParamByIndex(i, supportedSnapshotParams);
        SuccessOrExit(err);

        err = encoder.Encode(supportedSnapshotParams);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndSupportedSnapshotParamsRead();
    return err;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeFabricsUsingCamera(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartFabricsUsingCameraRead());

    for (uint8_t i = 0; true; i++)
    {
        chip::FabricIndex fabricIndex;

        err = mDelegate.GetFabricUsingCameraByIndex(i, fabricIndex);
        SuccessOrExit(err);

        err = encoder.Encode(fabricIndex);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndFabricsUsingCameraRead();
    return err;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartAllocatedVideoStreamsRead());

    for (uint8_t i = 0; true; i++)
    {
        Structs::VideoStreamStruct::Type videoStream;

        err = mDelegate.GetAllocatedVideoStreamByIndex(i, videoStream);
        SuccessOrExit(err);

        err = encoder.Encode(videoStream);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndAllocatedVideoStreamsRead();
    return err;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartAllocatedAudioStreamsRead());

    for (uint8_t i = 0; true; i++)
    {
        Structs::AudioStreamStruct::Type audioStream;

        err = mDelegate.GetAllocatedAudioStreamByIndex(i, audioStream);
        SuccessOrExit(err);

        err = encoder.Encode(audioStream);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndAllocatedAudioStreamsRead();
    return err;
}

CHIP_ERROR CameraAVStreamMgmtServer::ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartAllocatedSnapshotStreamsRead());

    for (uint8_t i = 0; true; i++)
    {
        Structs::SnapshotStreamStruct::Type snapshotStream;

        err = mDelegate.GetAllocatedSnapshotStreamByIndex(i, snapshotStream);
        SuccessOrExit(err);

        err = encoder.Encode(snapshotStream);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndAllocatedSnapshotStreamsRead();
    return err;
}

CHIP_ERROR
CameraAVStreamMgmtServer::ReadAndEncodeRankedVideoStreamPrioritiesList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Tell the delegate the read is starting..
    ReturnErrorOnFailure(mDelegate.StartRankedVideoStreamPrioritiesListRead());

    for (uint8_t i = 0; true; i++)
    {
        StreamTypeEnum streamType;

        err = mDelegate.GetRankedVideoStreamPrioritiesListByIndex(i, streamType);
        SuccessOrExit(err);

        err = encoder.Encode(streamType);
        SuccessOrExit(err);
    }

exit:
    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        // Convert end of list to CHIP_NO_ERROR
        err = CHIP_NO_ERROR;
    }

    // Tell the delegate the read is complete
    err = mDelegate.EndStartRankedVideoStreamPrioritiesListRead();
    return err;
}

// AttributeAccessInterface
CHIP_ERROR CameraAVStreamMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAVStreamMgmt::Id);
    ChipLogError(Zcl, "Camera AVStream Management: Reading");

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case MaxConcurrentVideoEncoders::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxConcurrentVideoEncoders));
        break;
    case MaxEncodedPixelRate::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxEncodedPixelRate));
        break;
    case VideoSensorParams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get VideoSensorParams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mVideoSensorParams));
        break;
    case NightVisionCapable::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get VideoSensorParams, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionCapable));
        break;
    case MinViewPort::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MinViewPort, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMinViewPort));
    case RateDistortionTradeOffPoints::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RateDistortionTradeOffPoints, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeRateDistortionTradeOffPoints(encoder); }));
        break;
    case MaxContentBufferSize::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MaxContentBufferSize, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMaxContentBufferSize));
        break;
    case MicrophoneCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneCapabilities, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneCapabilities));
        break;
    case SpeakerCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerCapabilities, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerCapabilities));
        break;
    case TwoWayTalkSupport::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get TwoWayTalkSupport, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mTwoWayTalkSupport));
        break;
    case SupportedSnapshotParams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RateDistortionTradeOffPoints, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSupportedSnapshotParams(encoder); }));
        break;
    case MaxNetworkBandwidth::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxNetworkBandwidth));
        break;
    case CurrentFrameRate::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get CurrentFrameRate, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mCurrentFrameRate));
        break;
    case HDRModeEnabled::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo) && SupportsOptAttr(OptionalAttributes::kSupportsHDRModeEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get HDRModeEnabled, feature is not supported");
        ReturnErrorOnFailure(aEncoder.Encode(mHDRModeEnabled));
        break;
    case FabricsUsingCamera::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get FabricsUsingCamera, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR {
            return this->ReadAndEncodeFabricsUsingCamera(encoder); }));
        break;
    case AllocatedVideoStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedVideoStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR {
            return this->ReadAndEncodeAllocatedVideoStreams(encoder); }));
        break;
    case AllocatedAudioStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedAudioStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR {
            return this->ReadAndEncodeAllocatedAudioStreams(encoder); }));
        break;
    case AllocatedSnapshotStreams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get AllocatedSnapshotStreams, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR {
            return this->ReadAndEncodeAllocatedSnapshotStreams(encoder); }));
        break;
    case RankedVideoStreamPrioritiesList::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get RankedVideoStreamPrioritiesList, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR {
            return this->ReadAndEncodeRankedVideoStreamPrioritiesList(encoder); }));
        break;
    case SoftRecordingPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SoftRecordingPrivacyModeEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftRecordingPrivacyModeEnabled));
        break;
    case SoftLivestreamPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SoftLivestreamPrivacyModeEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftLivestreamPrivacyModeEnabled));
        break;
    case HardPrivacyModeOn::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttributes::kSupportsHardPrivacyModeOn),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get HardPrivacyModeOn, feature is not supported");
        ReturnErrorOnFailure(aEncoder.Encode(mHardPrivacyModeOn));
        break;
    case NightVision::Id:
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                            SupportsOptAttr(OptionalAttributes::kSupportsNightVision), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get NightVision, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVision));
        break;
    case NightVisionIllum::Id:
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                            SupportsOptAttr(OptionalAttributes::kSupportsNightVisionIllum), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get NightVisionIllumination, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionIllum));
        break;
    case Viewport::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get Viewport, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mViewport));
        break;
    case SpeakerMuted::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMuted, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMuted));
        break;
    case SpeakerVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerVolumeLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerVolumeLevel));
        break;
    case SpeakerMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMaxLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMaxLevel));
        break;
    case SpeakerMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get SpeakerMinLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMinLevel));
        break;
    case MicrophoneMuted::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMuted, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMuted));
        break;
    case MicrophoneVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneVolumeLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneVolumeLevel));
        break;
    case MicrophoneMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMaxLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMaxLevel));
        break;
    case MicrophoneMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneMinLevel, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMinLevel));
        break;
    case MicrophoneAGCEnabled::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio) && SupportsOptAttr(OptionalAttributes::kSupportsMicrophoneAGCEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get MicrophoneAGCEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneAGCEnabled));
        break;
    case ImageRotation::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageRotation),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageRotation, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageRotation));
        break;
    case ImageFlipHorizontal::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageFlipHorizontal),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageFlipHorizontal, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipHorizontal));
        break;
    case ImageFlipVertical::Id:
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageFlipVertical),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get ImageFlipHorizontal, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipVertical));
        break;
    case LocalVideoRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kStorage), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get LocalVideoRecordingEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalVideoRecordingEnabled));
        break;
    case LocalSnapshotRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kStorage), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get LocalSnapshotRecordingEnabled, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalSnapshotRecordingEnabled));
        break;
    case StatusLightEnabled::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttributes::kSupportsStatusLightEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get StatusLightEnabled, feature is not supported");
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightEnabled));
        break;
    case StatusLightBrightness::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttributes::kSupportsStatusLightBrightness),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not get StatusLightBrightness, feature is not supported");
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightBrightness));
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamMgmtServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAVStreamMgmt::Id);

    switch (aPath.mAttributeId)
    {
    case HDRModeEnabled::Id: {
        // Optional Attribute if Video is supported
        VerifyOrReturnError(HasFeature(Feature::kVideo) && SupportsOptAttr(OptionalAttributes::kSupportsHDRModeEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set HDRModeEnabled, feature is not supported");

        bool hdrModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(hdrModeEnabled));
        ReturnErrorOnFailure(SetHDRModeEnabled(hdrModeEnabled));
        return CHIP_NO_ERROR;
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SoftRecordingPrivacyModeEnabled, feature is not supported"));

        bool softRecPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softRecPrivModeEnabled));
        ReturnErrorOnFailure(SetSoftRecordingPrivacyModeEnabled(hdrModeEnabled));
        return CHIP_NO_ERROR;
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SoftLivestreamPrivacyModeEnabled, feature is not supported"));

        bool softLivestreamPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softLivestreamPrivModeEnabled));
        ReturnErrorOnFailure(SetSoftLivestreamPrivacyModeEnabled(softLiveStreamPrivModeEnabled));
        return CHIP_NO_ERROR;
    }
    case NightVision::Id: {
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttributes::kSupportsNightVision),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set NightVision, feature is not supported"));

        TriStateAutoEnum nightVision;
        ReturnErrorOnFailure(aDecoder.Decode(nightVision));
        ReturnErrorOnFailure(SetNightVision(nightVision));
        return CHIP_NO_ERROR;
    }
    case NightVisionIllum::Id: {
        VerifyOrReturnError((HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot)) &&
                                SupportsOptAttr(OptionalAttributes::kSupportsNightVisionIllum),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set NightVisionIllumination, feature is not supported"));

        TriStateAutoEnum nightVisionIllum;
        ReturnErrorOnFailure(aDecoder.Decode(nightVisionIllum));
        ReturnErrorOnFailure(SetNightVisionIllum(nightVisionIllum));
        return CHIP_NO_ERROR;
    }
    case Viewport::Id: {
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set Viewport, feature is not supported"));
        ViewportStruct viewPort;
        ReturnErrorOnFailure(aDecoder.Decode(viewPort));
        ReturnErrorOnFailure(SetViewport(viewPort));
        return CHIP_NO_ERROR;
    }
    case SpeakerMuted::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SpeakerMuted, feature is not supported"));
        bool speakerMuted;
        ReturnErrorOnFailure(aDecoder.Decode(speakerMuted));
        ReturnErrorOnFailure(SetSpeakerMuted(speakerMuted));
        return CHIP_NO_ERROR;
    }
    case SpeakerVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && HasFeature(Feature::kSpeaker), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set SpeakerVolumeLevel, feature is not supported"));
        uint8_t speakerVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(speakerVolLevel));
        ReturnErrorOnFailure(SetSpeakerVolumeLevel(speakerVolLevel));
        return CHIP_NO_ERROR;
    }
    case MicrophoneMuted::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneMuted, feature is not supported"));
        bool micMuted;
        ReturnErrorOnFailure(aDecoder.Decode(micMuted));
        ReturnErrorOnFailure(SetMicrophoneMuted(micMuted));
        return CHIP_NO_ERROR;
    }
    case MicrophoneVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneVolumeLevel, feature is not supported"));
        uint8_t micVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(micVolLevel));
        ReturnErrorOnFailure(SetMicrophoneVolumeLevel(micVolLevel));
        return CHIP_NO_ERROR;
    }
    case MicrophoneAGCEnabled::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio) && SupportsOptAttr(OptionalAttributes::kSupportsMicrophoneAGCEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set MicrophoneAGCEnabled, feature is not supported"));
        bool micAGCEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(micAGCEnabled));
        ReturnErrorOnFailure(SetMicrophoneAGCEnabled(micAGCEnabled));
        return CHIP_NO_ERROR;
    }
    case ImageRotation::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageRotation),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageRotation, feature is not supported"));
        uint16_t imageRotation;
        ReturnErrorOnFailure(aDecoder.Decode(imageRotation));
        ReturnErrorOnFailure(SetImageRotation(imageRotation));
        return CHIP_NO_ERROR;
    }
    case ImageFlipHorizontal::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageFlipHorizontal),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageFlipHorizontal, feature is not supported"));
        bool imageFlipHorizontal;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipHorizontal));
        ReturnErrorOnFailure(SetImageFlipHorizontal(imageFlipHorizontal));
        return CHIP_NO_ERROR;
    }
    case ImageFlipVertical::Id: {
        VerifyOrReturnError(HasFeature(Feature::kImageControl) && SupportsOptAttr(OptionalAttributes::kSupportsImageFlipVertical),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set ImageFlipVertical, feature is not supported"));
        bool imageFlipVertical;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipVertical));
        ReturnErrorOnFailure(SetImageFlipVertical(imageFlipVertical));
        return CHIP_NO_ERROR;
    }
    case LocalVideoRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kStorage), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set LocalVideoRecordingEnabled, feature is not supported"));
        bool localVidRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localVidRecEnabled));
        ReturnErrorOnFailure(SetLocalVideoRecordingEnabled(localVidRecEnabled));
        return CHIP_NO_ERROR;
    }
    case LocalSnapshotRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kStorage), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set LocalSnapshotRecordingEnabled, feature is not supported"));
        bool localSnapshotRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localSnapshotRecEnabled));
        ReturnErrorOnFailure(SetLocalSnapshotRecordingEnabled(localSnapshotRecEnabled));
        return CHIP_NO_ERROR;
    }
    case StatusLightEnabled::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttributes::kSupportsStatusLightEnabled),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set StatusLightEnabled, feature is not supported");
        bool statusLightEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightEnabled));
        ReturnErrorOnFailure(SetStatusLightEnabled(statusLightEnabled));
        return CHIP_NO_ERROR;
    }
    case StatusLightBrightness::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttributes::kSupportsStatusLightBrightness),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "CameraAVStreamMgmt: can not set StatusLightBrightness, feature is not supported");
        ThreeLevelAutoEnum statusLightBrightness;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightBrightness));
        ReturnErrorOnFailure(SetStatusLightBrightness(statusLightBrightness));
        return CHIP_NO_ERROR;
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

Status CameraAVStreamMgmtServer::SetCurrentFrameRate(uint16_t aCurrentFrameRate)
{
    if (mCurrentFrameRate != aCurrentFrameRate)
    {
        mCurrentFrameRate          = aCurrentFrameRate;
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::CurrentFrameRate::Id);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetHDRModeEnabled(bool aHDRModeEnabled)
{
    if (mHDRModeEnabled != aHDRModeEnabled)
    {
        mHDRModeEnabled = aHDRModeEnabled;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::HDRModeEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mHDRModeEnabled);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled)
{
    if (mSoftRecordingPrivacyModeEnabled != aSoftRecordingPrivacyModeEnabled)
    {
        mSoftRecordingPrivacyModeEnabled = aSoftRecordingPrivacyModeEnabled;

        ConcreteAttributePath path =
            ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SoftRecordingPrivacyModeEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSoftRecordingPrivacyModeEnabled);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled)
{
    if (mSoftLivestreamPrivacyModeEnabled != aSoftLivestreamPrivacyModeEnabled)
    {
        mSoftLivestreamPrivacyModeEnabled = aSoftLivestreamPrivacyModeEnabled;

        ConcreteAttributePath path =
            ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SoftLivestreamPrivacyModeEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSoftLivestreamPrivacyModeEnabled);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetHardPrivacyModeOn(bool aHardPrivacyModeOn)
{
    if (mHardPrivacyModeOn != aHardPrivacyModeOn)
    {
        mHardPrivacyModeOn = aHardPrivacyModeOn;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::HardPrivacyModeOn::Id);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetNightVision(TriStateAutoEnum aNightVision)
{
    if (mNightVision != aNightVision)
    {
        mNightVision = aNightVision;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::NightVision::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mNightVision);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum)
{
    if (mNightVisionIllum != aNightVisionIllum)
    {
        mNightVisionIllum = aNightVisionIllum;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::NightVisionIllum::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mNightVisionIllum);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetViewport(const ViewportStruct & aViewport)
{
    if (mViewport != aViewport)
    {
        mViewport = aViewport;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::Viewport::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mViewport);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSpeakerMuted(bool aSpeakerMuted)
{
    if (mSpeakerMuted != aSpeakerMuted)
    {
        mSpeakerMuted = aSpeakerMuted;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerMuted::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSpeakerMuted);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSpeakerVolumeLevel(uint8_t aSpeakerVolumeLevel)
{
    if (aSpeakerVolumeLevel < mSpeakerMinLevel || aSpeakerVolumeLevel > mSpeakerMaxLevel)
    {
        return Status::ConstraintError;
    }

    if (mSpeakerVolumeLevel != aSpeakerVolumeLevel)
    {
        mSpeakerVolumeLevel = aSpeakerVolumeLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerVolumeLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSpeakerVolumeLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSpeakerMaxLevel(uint8_t aSpeakerMaxLevel)
{
    if (aSpeakerMaxLevel < mSpeakerMinLevel || aSpeakerMaxLevel > kMaxSpeakerLevel)
    {
        return Status::ConstraintError;
    }

    if (mSpeakerMaxLevel != aSpeakerMaxLevel)
    {
        mSpeakerMaxLevel = aSpeakerMaxLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerMaxLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSpeakerMaxLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetSpeakerMinLevel(uint8_t aSpeakerMinLevel)
{
    if (aSpeakerMinLevel > mSpeakerMaxLevel)
    {
        return Status::ConstraintError;
    }

    if (mSpeakerMinLevel != aSpeakerMinLevel)
    {
        mSpeakerMinLevel = aSpeakerMinLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerMinLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSpeakerMinLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetMicrophoneMuted(bool aMicrophoneMuted)
{

    if (mMicrophoneMuted != aMicrophoneMuted)
    {
        mMicrophoneMuted = aMicrophoneMuted;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneMuted::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMicrophoneMuted);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetMicrophoneVolumeLevel(uint8_t aMicrophoneVolumeLevel)
{
    if (aMicrophoneVolumeLevel < mMicrophoneMinLevel || aMicrophoneVolumeLevel > mMicrophoneMaxLevel)
    {
        return Status::ConstraintError;
    }

    if (mMicrophoneVolumeLevel != aMicrophoneVolumeLevel)
    {
        mMicrophoneVolumeLevel = aMicrophoneVolumeLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneVolumeLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMicrophoneVolumeLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetMicrophoneMaxLevel(uint8_t aMicrophoneMaxLevel)
{
    if (aMicrophoneMaxLevel < mMicrophoneMinLevel || aMicrophoneMaxLevel > kMaxMicrophoneLevel)
    {
        return Status::ConstraintError;
    }

    if (mMicrophoneMaxLevel != aMicrophoneMaxLevel)
    {
        mMicrophoneMaxLevel = aMicrophoneMaxLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneMaxLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMicrophoneMaxLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetMicrophoneMinLevel(uint8_t aMicrophoneMinLevel)
{
    if (aMicrophoneMinLevel > mMicrophoneMaxLevel)
    {
        return Status::ConstraintError;
    }

    if (mMicrophoneMinLevel != aMicrophoneMinLevel)
    {
        mMicrophoneMinLevel = aMicrophoneMinLevel;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneMinLevel::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMicrophoneMinLevel);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetMicrophoneAGCEnabled(bool aMicrophoneAGCEnabled)
{

    if (mMicrophoneAGCEnabled != aMicrophoneAGCEnabled)
    {
        mMicrophoneAGCEnabled = aMicrophoneAGCEnabled;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneAGCEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mMicrophoneAGCEnabled);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetImageRotation(uint16_t aImageRotation)
{
    if (mImageRotation > kMaxImageRotationDegrees)
    {
        return Status::ConstraintError;
    }

    if (mImageRotation != aImageRotation)
    {
        mImageRotation = aImageRotation;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageRotation::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mImageRotation);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetImageFlipHorizontal(bool aImageFlipHorizontal)
{

    if (mImageFlipHorizontal != aImageFlipHorizontal)
    {
        mImageFlipHorizontal = aImageFlipHorizontal;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageFlipHorizontal::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mImageFlipHorizontal);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetImageFlipVertical(bool aImageFlipVertical)
{

    if (mImageFlipVertical != aImageFlipVertical)
    {
        mImageFlipVertical = aImageFlipVertical;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageFlipVertical::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mImageFlipVertical);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetLocalVideoRecordingEnabled(bool aLocalVideoRecordingEnabled)
{

    if (mLocalVideoRecordingEnabled != aLocalVideoRecordingEnabled)
    {
        mLocalVideoRecordingEnabled = aLocalVideoRecordingEnabled;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LocalVideoRecordingEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mLocalVideoRecordingEnabled);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetLocalSnapshotRecordingEnabled(bool aLocalSnapshotRecordingEnabled)
{

    if (mLocalSnapshotRecordingEnabled != aLocalSnapshotRecordingEnabled)
    {
        mLocalSnapshotRecordingEnabled = aLocalSnapshotRecordingEnabled;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LocalSnapshotRecordingEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mLocalSnapshotRecordingEnabled);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetStatusLightEnabled(bool aStatusLightEnabled)
{

    if (mStatusLightEnabled != aStatusLightEnabled)
    {
        mStatusLightEnabled = aStatusLightEnabled;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::StatusLightEnabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mStatusLightEnabled);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status CameraAVStreamMgmtServer::SetStatusLightBrightness(ThreeLevelAutoEnum aStatusLightBrightness)
{

    if (mStatusLightBrightness != aStatusLightBrightness)
    {
        mStatusLightBrightness = aStatusLightBrightness;

        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::StatusLightBrightness::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mStatusLightBrightness);
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

void CameraAVStreamMgmtServer::LoadPersistentAttributes()
{
    // Load HDR Mode Enabled
    bool storedHDRModeEnabled;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::HDRModeEnabled::Id), storedHDRModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mHDRModeEnabled = storedHDRModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded HDRModeEnabled as %u", mHDRModeenabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the HDRModeEnabled from the KVS. Defaulting to %u", mHDRModeEnabled);
    }

    // TODO: Load allocated streams and ranked priorities list

    // Load SoftRecordingPrivacyModeEnabled
    bool softRecordingPrivacyModeEnabled;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SoftRecordingPrivacyModeEnabled::Id),
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SoftLivestreamPrivacyModeEnabled::Id),
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
    TriStateAutoEnum nightVision;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::NightVision::Id), nightVision);
    if (err == CHIP_NO_ERROR)
    {
        mNightVision = nightVision;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded NightVision as %u", mNightVision);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the NightVision from the KVS. Defaulting to %u", mNightVision);
    }

    // Load NightVisionIllum
    TriStateAutoEnum nightVisionIllum;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::NightVisionIllum::Id), nightVisionIllum);
    if (err == CHIP_NO_ERROR)
    {
        mNightVisionIllum = nightVisionIllum;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded NightVisionIllum as %u", mNightVisionIllum);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the NightVisionIllum from the KVS. Defaulting to %u",
                      mNightVisionIllum);
    }

    // Load Viewport
    ViewportStruct viewport;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::Viewport::Id), viewport);
    if (err == CHIP_NO_ERROR)
    {
        mViewport = viewport;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded Viewport as %u", mViewport);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the Viewport from the KVS. Defaulting to %u", mViewport);
    }

    // Load SpeakerMuted
    bool speakerMuted;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerMuted::Id), speakerMuted);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::SpeakerVolumeLevel::Id), speakerVolumeLevel);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneMuted::Id), microphoneMuted);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneVolumeLevel::Id), microphoneVolumeLevel);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MicrophoneAGCEnabled::Id), microphoneAGCEnabled);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageRotation::Id), imageRotation);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageFlipHorizontal::Id), imageFlipHorizontal);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::ImageFlipVertical::Id), imageFlipVertical);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LocalVideoRecordingEnabled::Id), localVideoRecordingEnabled);
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LocalSnapshotRecordingEnabled::Id),
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
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::StatusLightEnabled::Id), statusLightEnabled);
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
    ThreeLevelAutoEnum statusLightBrightness;
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::StatusLightBrightness::Id), statusLightBrightness);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightBrightness = statusLightBrightness;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Loaded StatusLightBrightness as %u", mStatusLightBrightness);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Unable to load the StatusLightBrightness from the KVS. Defaulting to %u",
                      mStatusLightBrightness);
    }
}

// CommandHandlerInterface
void CameraAVStreamMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "CameraAV: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::VideoStreamAllocate::Id:
        ChipLogDetail(Zcl, "CameraAVStreamMgmt: Allocating Video Stream");

        if (!HasFeature(CameraAVStreamMgmt::Feature::kVideo))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kVideo))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kVideo))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kAudio))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kAudio))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kSnapshot))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::kSnapshot))
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

        if (!HasFeature(CameraAVStreamMgmt::Feature::Snapshot))
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

    Commands::VideoStreamAllocateResponse::Type response;
    auto & streamType     = commandData.streamType;
    auto & videoCodec     = commandData.videoCodec;
    auto & minFrameRate   = commandData.minFrameRate;
    auto & maxFrameRate   = commandData.maxFrameRate;
    auto & minResolution  = commandData.minResolution;
    auto & maxResolution  = commandData.maxResolution;
    auto & minBitRate     = commandData.minBitRate;
    auto & maxBitRate     = commandData.maxBitRate;
    auto & minFragmentLen = commandData.minFragmentLen;
    auto & maxFragmentLen = commandData.maxFragmentLen;

    // Call the delegate
    Status status = mDelegate.VideoStreamAllocate(streamType, videoCodec, minFrameRate, maxFrameRate, minResolution, maxResolution,
                                                  minBitRate, maxBitRate, minFragmentLen, maxFragmentLen);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleVideoStreamModify(HandlerContext & ctx,
                                                       const Commands::VideoStreamAllocate::DecodableType & commandData)
{

    auto & videoStreamID = commandData.videoStreamID;
    auto & videoRes      = commandData.resolution;

    // Call the delegate
    Status status = mDelegate.VideoStreamModify(videoStreamID, videoRes);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleVideoStreamDeallocate(HandlerContext & ctx,
                                                           const Commands::VideoStreamAllocate::DecodableType & commandData)
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
    auto & streamType   = commandData.streamType;
    auto & audioCodec   = commandData.audioCodec;
    auto & channelCount = commandData.channelCount;
    auto & sampleRate   = commandData.sampleRate;
    auto & bitRate      = commandData.bitRate;
    auto & bitDepth     = commandData.bitDepth;

    // Call the delegate
    Status status = mDelegate.AudioStreamAllocate(streamType, audioCodec, channelCount, sampleRate, bitRate, bitDepth);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleAudioStreamDeallocate(HandlerContext & ctx,
                                                           const Commands::VideoStreamAllocate::DecodableType & commandData)
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
    auto & imageCodec    = commandData.imageCodec;
    auto & maxFrameRate  = commandData.maxFrameRate;
    auto & bitRate       = commandData.bitRate;
    auto & minResolution = commandData.minResolution;
    auto & maxResolution = commandData.maxResolution;
    auto & quality       = commandData.quality;

    // Call the delegate
    Status status = mDelegate.SnapshotStreamAllocate(imageCodec, maxFrameRate, bitRate, minResolution, maxResolution, quality);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleSnapshotStreamDeallocate(HandlerContext & ctx,
                                                              const Commands::SnapshotStreamAllocate::DecodableType & commandData)
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
                                                         const Commands::SnapshotStreamAllocate::DecodableType & commandData)
{

    auto & streamPriorities = commandData.streamPriorities;

    // Call the delegate
    Status status = mDelegate.SetStreamPriorities(streamPriorities);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAVStreamMgmtServer::HandleCaptureSnapshot(HandlerContext & ctx,
                                                     const Commands::VideoStreamAllocate::DecodableType & commandData)
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

} // namespace CameraAVStreamMgmt
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Camera AV Stream Management Cluster Server Init
 *
 * Server Init
 *
 */
void MatterCameraAVStreamMgmtPluginServerInitCallback() {}
