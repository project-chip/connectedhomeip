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
#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/CameraAvStreamManagement/ClusterId.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <clusters/CameraAvStreamManagement/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cmath>
#include <cstring>
#include <optional>
#include <set>

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

CameraAVStreamManagementCluster::CameraAVStreamManagementCluster(
    CameraAVStreamManagementDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
    const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxConcurrentEncoders, uint32_t aMaxEncodedPixelRate,
    const VideoSensorParamsStruct & aVideoSensorParams, bool aNightVisionUsesInfrared,
    const VideoResolutionStruct & aMinViewPortRes,
    const std::vector<Structs::RateDistortionTradeOffPointsStruct::Type> & aRateDistortionTradeOffPoints,
    uint32_t aMaxContentBufferSize, const AudioCapabilitiesStruct & aMicrophoneCapabilities,
    const AudioCapabilitiesStruct & aSpeakerCapabilities, TwoWayTalkSupportTypeEnum aTwoWayTalkSupport,
    const std::vector<Structs::SnapshotCapabilitiesStruct::Type> & aSnapshotCapabilities, uint32_t aMaxNetworkBandwidth,
    const std::vector<Globals::StreamUsageEnum> & aSupportedStreamUsages,
    const std::vector<Globals::StreamUsageEnum> & aStreamUsagePriorities) :
    DefaultServerCluster({ aEndpointId, CameraAvStreamManagement::Id }),
    mDelegate(aDelegate), mEnabledFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs),
    mMaxConcurrentEncoders(aMaxConcurrentEncoders), mMaxEncodedPixelRate(aMaxEncodedPixelRate),
    mVideoSensorParams(aVideoSensorParams), mNightVisionUsesInfrared(aNightVisionUsesInfrared),
    mMinViewPortResolution(aMinViewPortRes), mRateDistortionTradeOffPointsList(aRateDistortionTradeOffPoints),
    mMaxContentBufferSize(aMaxContentBufferSize), mMicrophoneCapabilities(aMicrophoneCapabilities),
    mSpeakerCapabilities(aSpeakerCapabilities), mTwoWayTalkSupport(aTwoWayTalkSupport),
    mSnapshotCapabilitiesList(aSnapshotCapabilities), mMaxNetworkBandwidth(aMaxNetworkBandwidth),
    mSupportedStreamUsages(aSupportedStreamUsages), mStreamUsagePriorities(aStreamUsagePriorities)
{
    mDelegate.SetCameraAVStreamManagementCluster(this);
}

CameraAVStreamManagementCluster::~CameraAVStreamManagementCluster()
{
    // Explicitly set the CameraAVStreamManagementCluster pointer in the Delegate to
    // null.
    mDelegate.SetCameraAVStreamManagementCluster(nullptr);
}

CHIP_ERROR CameraAVStreamManagementCluster::Init()
{
    // Constraint checks for RateDistortionTardeOffPoints vector
    for (const auto & rateDistortionTradeOffPoints : mRateDistortionTradeOffPointsList)
    {
        VerifyOrReturnError(
            rateDistortionTradeOffPoints.minBitRate >= 1, CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: RateDistortionTradeOffPoints configuration error", mPath.mEndpointId));
    }

    // At least one of Video, Audio or Snapshot needs to be supported.
    VerifyOrReturnError(
        HasFeature(Feature::kVideo) || HasFeature(Feature::kAudio) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(
            Zcl,
            "CameraAVStreamMgmt[ep=%d]: Feature configuration error. At least one of Audio, Video, or Snapshot feature required",
            mPath.mEndpointId));

    // Verify cross-feature dependencies
    if (HasFeature(Feature::kImageControl) || HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay) ||
        HasFeature(Feature::kHighDynamicRange))
    {
        VerifyOrReturnError(HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageControl, Watermark, "
                                         "OnScreenDisplay or HighDynamicRange, then Video or Snapshot feature required",
                                         mPath.mEndpointId));
    }

    if (HasFeature(Feature::kSpeaker))
    {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if Speaker is enabled, then "
                                         "Audio feature required",
                                         mPath.mEndpointId));
    }

    // Ensure Optional attribute bits have been correctly passed and have supporting feature bits set.
    if (SupportsOptAttr(OptionalAttribute::kNightVisionIllum))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kNightVision), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if NIghtVisionIllum is enabled, then "
                         "NightVision feature required",
                         mPath.mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kAudio), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(
                Zcl, "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if MicrophoneAGCEnabled, then Audio feature required",
                mPath.mEndpointId));
    }

    // If any of the image control attributes are present, we must have the feature.
    // Conversely, if we have the feature, at least one of the attributes must be present
    if (SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal) || SupportsOptAttr(OptionalAttribute::kImageFlipVertical) ||
        SupportsOptAttr(OptionalAttribute::kImageRotation))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kImageControl), CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageFlip or Rotation enabled, then "
                         "ImageControl feature required",
                         mPath.mEndpointId));
    }

    if (HasFeature(Feature::kImageControl))
    {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal) || SupportsOptAttr(OptionalAttribute::kImageFlipVertical) ||
                SupportsOptAttr(OptionalAttribute::kImageRotation),
            CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVStreamMgmt[ep=%d]: Feature configuration error. if ImageControl feature supported, then "
                         "at least one of the ImageFlip or Rotation attributes shall be supported",
                         mPath.mEndpointId));
    }

    LoadPersistentAttributes();

    return CHIP_NO_ERROR;
}

bool CameraAVStreamManagementCluster::HasFeature(Feature feature) const
{
    return mEnabledFeatures.Has(feature);
}

bool CameraAVStreamManagementCluster::SupportsOptAttr(OptionalAttribute aOptionalAttr) const
{
    return mOptionalAttrs.Has(aOptionalAttr);
}

bool CameraAVStreamManagementCluster::IsLocalVideoRecordingEnabled() const
{
    return mLocalVideoRecordingEnabled;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeRateDistortionTradeOffPoints(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & rateDistortionTradeOffPoints : mRateDistortionTradeOffPointsList)
    {
        ReturnErrorOnFailure(encoder.Encode(rateDistortionTradeOffPoints));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeSnapshotCapabilities(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & snapshotCapabilities : mSnapshotCapabilitiesList)
    {
        ReturnErrorOnFailure(encoder.Encode(snapshotCapabilities));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeSupportedStreamUsages(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & supportedStreamUsage : mSupportedStreamUsages)
    {
        ReturnErrorOnFailure(encoder.Encode(supportedStreamUsage));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeAllocatedVideoStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ClearInMemoryAllocatedVideoStreams, mAllocatedVideoStreams.clear(););
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_LoadPersistentCameraAVSMAttributes, LoadPersistentAttributes(););

    for (const auto & videoStream : mAllocatedVideoStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(videoStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeAllocatedAudioStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ClearInMemoryAllocatedAudioStreams, mAllocatedAudioStreams.clear(););
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_LoadPersistentCameraAVSMAttributes, LoadPersistentAttributes(););

    for (const auto & audioStream : mAllocatedAudioStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(audioStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeAllocatedSnapshotStreams(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ClearInMemoryAllocatedSnapshotStreams, mAllocatedSnapshotStreams.clear(););
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_LoadPersistentCameraAVSMAttributes, LoadPersistentAttributes(););

    for (const auto & snapshotStream : mAllocatedSnapshotStreams)
    {
        ReturnErrorOnFailure(encoder.Encode(snapshotStream));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManagementCluster::ReadAndEncodeStreamUsagePriorities(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & streamUsage : mStreamUsagePriorities)
    {
        ReturnErrorOnFailure(encoder.Encode(streamUsage));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::SetStreamUsagePriorities(const std::vector<Globals::StreamUsageEnum> & newPriorities)
{
    mStreamUsagePriorities = newPriorities;
    ReturnErrorOnFailure(StoreStreamUsagePriorities());
    mDelegate.OnAttributeChanged(Attributes::StreamUsagePriorities::Id);
    NotifyAttributeChanged(Attributes::StreamUsagePriorities::Id);

    return CHIP_NO_ERROR;
}

std::optional<uint16_t> CameraAVStreamManagementCluster::GetReusableVideoStreamId(const VideoStreamStruct & requestedArgs) const
{
    for (const auto & stream : mAllocatedVideoStreams)
    {
        // 1. Codec must match exactly (Allocated stream already has the codec)
        if (requestedArgs.videoCodec != stream.videoCodec)
        {
            continue;
        }

        // 2. Framerate check (request must be within allocated stream's current range)
        if (requestedArgs.minFrameRate < stream.minFrameRate || requestedArgs.maxFrameRate > stream.maxFrameRate)
        {
            continue;
        }

        // 3. Resolution check
        if (requestedArgs.minResolution.width < stream.minResolution.width ||
            requestedArgs.minResolution.height < stream.minResolution.height ||
            requestedArgs.maxResolution.width > stream.maxResolution.width ||
            requestedArgs.maxResolution.height > stream.maxResolution.height)
        {
            continue;
        }

        // 4. Bitrate check
        if (requestedArgs.minBitRate < stream.minBitRate || requestedArgs.maxBitRate > stream.maxBitRate)
        {
            continue;
        }

        // 5. KeyFrameInterval check
        if (requestedArgs.keyFrameInterval != stream.keyFrameInterval)
        {
            continue;
        }

        return stream.videoStreamID;
    }
    return std::nullopt;
}

CHIP_ERROR CameraAVStreamManagementCluster::AddVideoStream(const VideoStreamStruct & videoStream)
{
    mAllocatedVideoStreams.push_back(videoStream);

    return PersistAndNotify<Attributes::AllocatedVideoStreams::Id>();
}

CHIP_ERROR CameraAVStreamManagementCluster::UpdateVideoStreamRangeParams(VideoStreamStruct & videoStreamToUpdate,
                                                                         const VideoStreamStruct & videoStream, bool & wasModified)
{
    // Store original values to detect changes
    uint16_t origMinFrameRate = videoStreamToUpdate.minFrameRate;
    uint16_t origMaxFrameRate = videoStreamToUpdate.maxFrameRate;
    uint16_t origMinResWidth  = videoStreamToUpdate.minResolution.width;
    uint16_t origMinResHeight = videoStreamToUpdate.minResolution.height;
    uint16_t origMaxResWidth  = videoStreamToUpdate.maxResolution.width;
    uint16_t origMaxResHeight = videoStreamToUpdate.maxResolution.height;
    uint32_t origMinBitRate   = videoStreamToUpdate.minBitRate;
    uint32_t origMaxBitRate   = videoStreamToUpdate.maxBitRate;

    // Adjust the range parameters for the allocated video stream to be the
    // intersection of the existing and the new one.
    videoStreamToUpdate.minFrameRate         = std::max(videoStreamToUpdate.minFrameRate, videoStream.minFrameRate);
    videoStreamToUpdate.maxFrameRate         = std::min(videoStreamToUpdate.maxFrameRate, videoStream.maxFrameRate);
    videoStreamToUpdate.minResolution.width  = std::max(videoStreamToUpdate.minResolution.width, videoStream.minResolution.width);
    videoStreamToUpdate.minResolution.height = std::max(videoStreamToUpdate.minResolution.height, videoStream.minResolution.height);
    videoStreamToUpdate.maxResolution.width  = std::min(videoStreamToUpdate.maxResolution.width, videoStream.maxResolution.width);
    videoStreamToUpdate.maxResolution.height = std::min(videoStreamToUpdate.maxResolution.height, videoStream.maxResolution.height);
    videoStreamToUpdate.minBitRate           = std::max(videoStreamToUpdate.minBitRate, videoStream.minBitRate);
    videoStreamToUpdate.maxBitRate           = std::min(videoStreamToUpdate.maxBitRate, videoStream.maxBitRate);

    // Check if any parameter was actually modified
    wasModified = (origMinFrameRate != videoStreamToUpdate.minFrameRate) ||
        (origMaxFrameRate != videoStreamToUpdate.maxFrameRate) || (origMinResWidth != videoStreamToUpdate.minResolution.width) ||
        (origMinResHeight != videoStreamToUpdate.minResolution.height) ||
        (origMaxResWidth != videoStreamToUpdate.maxResolution.width) ||
        (origMaxResHeight != videoStreamToUpdate.maxResolution.height) || (origMinBitRate != videoStreamToUpdate.minBitRate) ||
        (origMaxBitRate != videoStreamToUpdate.maxBitRate);

    if (wasModified)
    {
        ReturnErrorOnFailure(PersistAndNotify<Attributes::AllocatedVideoStreams::Id>());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::RemoveVideoStream(uint16_t videoStreamId)
{
    mAllocatedVideoStreams.erase(
        std::remove_if(mAllocatedVideoStreams.begin(), mAllocatedVideoStreams.end(),
                       [&](const VideoStreamStruct & vStream) { return vStream.videoStreamID == videoStreamId; }),
        mAllocatedVideoStreams.end());

    return PersistAndNotify<Attributes::AllocatedVideoStreams::Id>();
}

CHIP_ERROR CameraAVStreamManagementCluster::AddAudioStream(const AudioStreamStruct & audioStream)
{
    mAllocatedAudioStreams.push_back(audioStream);

    return PersistAndNotify<Attributes::AllocatedAudioStreams::Id>();
}

CHIP_ERROR CameraAVStreamManagementCluster::RemoveAudioStream(uint16_t audioStreamId)
{
    mAllocatedAudioStreams.erase(
        std::remove_if(mAllocatedAudioStreams.begin(), mAllocatedAudioStreams.end(),
                       [&](const AudioStreamStruct & aStream) { return aStream.audioStreamID == audioStreamId; }),
        mAllocatedAudioStreams.end());

    return PersistAndNotify<Attributes::AllocatedAudioStreams::Id>();
}

std::optional<uint16_t> CameraAVStreamManagementCluster::GetReusableSnapshotStreamId(
    const CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs & requestedArgs) const
{
    for (const auto & stream : mAllocatedSnapshotStreams)
    {
        // 1. Codec must match allocated stream's codec.
        if (requestedArgs.imageCodec != stream.imageCodec)
        {
            continue;
        }

        // 2. Quality must match allocated stream's quality.
        if (requestedArgs.quality != stream.quality)
        {
            continue;
        }

        // 3. Framerate check (request must be within allocated stream's current range)
        if (requestedArgs.maxFrameRate > stream.frameRate)
        {
            continue;
        }

        // 4. Resolution check
        if (requestedArgs.minResolution.width < stream.minResolution.width ||
            requestedArgs.minResolution.height < stream.minResolution.height ||
            requestedArgs.maxResolution.width > stream.maxResolution.width ||
            requestedArgs.maxResolution.height > stream.maxResolution.height)
        {
            continue;
        }

        return stream.snapshotStreamID;
    }
    return std::nullopt;
}

CHIP_ERROR CameraAVStreamManagementCluster::AddSnapshotStream(const SnapshotStreamStruct & snapshotStream)
{
    mAllocatedSnapshotStreams.push_back(snapshotStream);

    return PersistAndNotify<Attributes::AllocatedSnapshotStreams::Id>();
}

CHIP_ERROR CameraAVStreamManagementCluster::UpdateSnapshotStreamRangeParams(
    SnapshotStreamStruct & snapshotStreamToUpdate,
    const CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs & snapshotStream)
{
    // Store original values to detect changes
    uint16_t origMinResWidth  = snapshotStreamToUpdate.minResolution.width;
    uint16_t origMinResHeight = snapshotStreamToUpdate.minResolution.height;
    uint16_t origMaxResWidth  = snapshotStreamToUpdate.maxResolution.width;
    uint16_t origMaxResHeight = snapshotStreamToUpdate.maxResolution.height;

    // Adjust the range parameters for the allocated snapshot stream to be the
    // intersection of the existing and the new one.
    snapshotStreamToUpdate.minResolution.width =
        std::max(snapshotStreamToUpdate.minResolution.width, snapshotStream.minResolution.width);
    snapshotStreamToUpdate.minResolution.height =
        std::max(snapshotStreamToUpdate.minResolution.height, snapshotStream.minResolution.height);
    snapshotStreamToUpdate.maxResolution.width =
        std::min(snapshotStreamToUpdate.maxResolution.width, snapshotStream.maxResolution.width);
    snapshotStreamToUpdate.maxResolution.height =
        std::min(snapshotStreamToUpdate.maxResolution.height, snapshotStream.maxResolution.height);

    // Check if any parameter was actually modified
    bool wasModified = (origMinResWidth != snapshotStreamToUpdate.minResolution.width) ||
        (origMinResHeight != snapshotStreamToUpdate.minResolution.height) ||
        (origMaxResWidth != snapshotStreamToUpdate.maxResolution.width) ||
        (origMaxResHeight != snapshotStreamToUpdate.maxResolution.height);

    if (wasModified)
    {
        ReturnErrorOnFailure(PersistAndNotify<Attributes::AllocatedSnapshotStreams::Id>());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::RemoveSnapshotStream(uint16_t snapshotStreamId)
{
    mAllocatedSnapshotStreams.erase(
        std::remove_if(mAllocatedSnapshotStreams.begin(), mAllocatedSnapshotStreams.end(),
                       [&](const SnapshotStreamStruct & sStream) { return sStream.snapshotStreamID == snapshotStreamId; }),
        mAllocatedSnapshotStreams.end());

    return PersistAndNotify<Attributes::AllocatedSnapshotStreams::Id>();
}

CHIP_ERROR CameraAVStreamManagementCluster::UpdateVideoStreamRefCount(uint16_t videoStreamId, bool shouldIncrement)
{
    auto it = std::find_if(mAllocatedVideoStreams.begin(), mAllocatedVideoStreams.end(),
                           [videoStreamId](const VideoStreamStruct & vStream) { return vStream.videoStreamID == videoStreamId; });

    if (it == mAllocatedVideoStreams.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    if (shouldIncrement)
    {
        return (it->referenceCount < UINT8_MAX) ? (it->referenceCount++, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
    }

    return (it->referenceCount > 0) ? (it->referenceCount--, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
}

CHIP_ERROR CameraAVStreamManagementCluster::UpdateAudioStreamRefCount(uint16_t audioStreamId, bool shouldIncrement)
{
    auto it = std::find_if(mAllocatedAudioStreams.begin(), mAllocatedAudioStreams.end(),
                           [audioStreamId](const AudioStreamStruct & aStream) { return aStream.audioStreamID == audioStreamId; });

    if (it == mAllocatedAudioStreams.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    if (shouldIncrement)
    {
        return (it->referenceCount < UINT8_MAX) ? (it->referenceCount++, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
    }

    return (it->referenceCount > 0) ? (it->referenceCount--, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
}

CHIP_ERROR CameraAVStreamManagementCluster::UpdateSnapshotStreamRefCount(uint16_t snapshotStreamId, bool shouldIncrement)
{
    auto it = std::find_if(
        mAllocatedSnapshotStreams.begin(), mAllocatedSnapshotStreams.end(),
        [snapshotStreamId](const SnapshotStreamStruct & sStream) { return sStream.snapshotStreamID == snapshotStreamId; });

    if (it == mAllocatedSnapshotStreams.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    if (shouldIncrement)
    {
        return (it->referenceCount < UINT8_MAX) ? (it->referenceCount++, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
    }

    return (it->referenceCount > 0) ? (it->referenceCount--, CHIP_NO_ERROR) : CHIP_ERROR_INVALID_INTEGER_VALUE;
}

DataModel::ActionReturnStatus CameraAVStreamManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(request.path.mClusterId == CameraAvStreamManagement::Id);
    ChipLogProgress(Zcl, "Camera AVStream Management[ep=%d]: Reading", mPath.mEndpointId);

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mEnabledFeatures));
        break;

    case ClusterRevision::Id:
        ReturnErrorOnFailure(aEncoder.Encode(CameraAvStreamManagement::kRevision));
        break;

    case MaxConcurrentEncoders::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MaxConcurrentEncoders, feature is not supported",
                                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxConcurrentEncoders));
        break;
    case MaxEncodedPixelRate::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MaxEncodedPixelRate, feature is not supported",
                                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxEncodedPixelRate));
        break;
    case VideoSensorParams::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get VideoSensorParams, feature is not supported",
                                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mVideoSensorParams));
        break;
    case NightVisionUsesInfrared::Id:
        VerifyOrReturnError(HasFeature(Feature::kNightVision), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get NightVisionUsesInfrared, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionUsesInfrared));
        break;
    case MinViewportResolution::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MinViewportResolution, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMinViewPortResolution));
        break;
    case RateDistortionTradeOffPoints::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get RateDistortionTradeOffPoints, feature is not supported",
                         mPath.mEndpointId));
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
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneCapabilities));
        break;
    case SpeakerCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get SpeakerCapabilities, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerCapabilities));
        break;
    case TwoWayTalkSupport::Id:
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get TwoWayTalkSupport, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mTwoWayTalkSupport));
        break;
    case SnapshotCapabilities::Id:
        VerifyOrReturnError(HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get SnapshotCapabilities, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeSnapshotCapabilities(encoder); }));
        break;
    case MaxNetworkBandwidth::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxNetworkBandwidth));
        break;
    case CurrentFrameRate::Id:
        VerifyOrReturnError(HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get CurrentFrameRate, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mCurrentFrameRate));
        break;
    case HDRModeEnabled::Id:
        VerifyOrReturnError(HasFeature(Feature::kHighDynamicRange), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get HDRModeEnabled, feature is not supported",
                                         mPath.mEndpointId));
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
                                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedVideoStreams(encoder); }));
        break;
    case AllocatedAudioStreams::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get AllocatedAudioStreams, feature is not supported",
                                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedAudioStreams(encoder); }));
        break;
    case AllocatedSnapshotStreams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get AllocatedSnapshotStreams, feature is not supported",
                         mPath.mEndpointId));

        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeAllocatedSnapshotStreams(encoder); }));
        break;
    case StreamUsagePriorities::Id:
        ReturnErrorOnFailure(aEncoder.EncodeList(
            [this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeStreamUsagePriorities(encoder); }));
        break;
    case SoftRecordingPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SoftRecordingPrivacyModeEnabled, feature is not supported",
                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftRecordingPrivacyModeEnabled));
        break;
    case SoftLivestreamPrivacyModeEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SoftLivestreamPrivacyModeEnabled, feature is not supported",
                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSoftLivestreamPrivacyModeEnabled));
        break;
    case HardPrivacyModeOn::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kHardPrivacyModeOn), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get HardPrivacyModeOn, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mHardPrivacyModeOn));
        break;
    case NightVision::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kNightVision), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get NightVision, attribute is not supported", mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVision));
        break;
    case NightVisionIllum::Id:
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kNightVisionIllum), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get NightVisionIllumination, attribute is not supported",
                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mNightVisionIllum));
        break;
    case Viewport::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get Viewport, feature is not supported", mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mViewport));
        break;
    case SpeakerMuted::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMuted, feature is not supported", mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMuted));
        break;
    case SpeakerVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerVolumeLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerVolumeLevel));
        break;
    case SpeakerMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMaxLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMaxLevel));
        break;
    case SpeakerMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get SpeakerMinLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSpeakerMinLevel));
        break;
    case MicrophoneMuted::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMuted, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMuted));
        break;
    case MicrophoneVolumeLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneVolumeLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneVolumeLevel));
        break;
    case MicrophoneMaxLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMaxLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMaxLevel));
        break;
    case MicrophoneMinLevel::Id:
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneMinLevel, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneMinLevel));
        break;
    case MicrophoneAGCEnabled::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get MicrophoneAGCEnabled, feature is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMicrophoneAGCEnabled));
        break;
    case ImageRotation::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kImageRotation), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get ImageRotation, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageRotation));
        break;
    case ImageFlipHorizontal::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get ImageFlipHorizontal, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipHorizontal));
        break;
    case ImageFlipVertical::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kImageFlipVertical), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get ImageFlipHorizontal, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mImageFlipVertical));
        break;
    case LocalVideoRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get LocalVideoRecordingEnabled, feature is not supported",
                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalVideoRecordingEnabled));
        break;
    case LocalSnapshotRecordingEnabled::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not get LocalSnapshotRecordingEnabled, feature is not supported",
                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mLocalSnapshotRecordingEnabled));
        break;
    case StatusLightEnabled::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get StatusLightEnabled, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightEnabled));
        break;
    case StatusLightBrightness::Id:
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightBrightness), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not get StatusLightBrightness, attribute is not supported",
                                         mPath.mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mStatusLightBrightness));
        break;
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus CameraAVStreamManagementCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                              AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(request.path.mClusterId == CameraAvStreamManagement::Id);

    switch (request.path.mAttributeId)
    {
    case HDRModeEnabled::Id: {
        VerifyOrReturnError(HasFeature(Feature::kHighDynamicRange), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set HDRModeEnabled, feature is not supported",
                                         mPath.mEndpointId));

        bool hdrModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(hdrModeEnabled));
        return SetHDRModeEnabled(hdrModeEnabled);
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SoftRecordingPrivacyModeEnabled, feature is not supported",
                         mPath.mEndpointId));

        bool softRecPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softRecPrivModeEnabled));
        return SetSoftRecordingPrivacyModeEnabled(softRecPrivModeEnabled);
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kPrivacy), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SoftLivestreamPrivacyModeEnabled, feature is not supported",
                         mPath.mEndpointId));

        bool softLivestreamPrivModeEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(softLivestreamPrivModeEnabled));
        return SetSoftLivestreamPrivacyModeEnabled(softLivestreamPrivModeEnabled);
    }
    case NightVision::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kNightVision), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set NightVision, feature is not supported", mPath.mEndpointId));

        TriStateAutoEnum nightVision;
        ReturnErrorOnFailure(aDecoder.Decode(nightVision));
        return SetNightVision(nightVision);
    }
    case NightVisionIllum::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kNightVisionIllum), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set NightVisionIllumination, feature is not supported",
                                         mPath.mEndpointId));

        TriStateAutoEnum nightVisionIllum;
        ReturnErrorOnFailure(aDecoder.Decode(nightVisionIllum));
        return SetNightVisionIllum(nightVisionIllum);
    }
    case Viewport::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set Viewport, feature is not supported", mPath.mEndpointId));
        Globals::Structs::ViewportStruct::Type viewPort;
        ReturnErrorOnFailure(aDecoder.Decode(viewPort));
        return SetViewport(viewPort);
    }
    case SpeakerMuted::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SpeakerMuted, feature is not supported", mPath.mEndpointId));
        bool speakerMuted;
        ReturnErrorOnFailure(aDecoder.Decode(speakerMuted));
        return SetSpeakerMuted(speakerMuted);
    }
    case SpeakerVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kSpeaker), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set SpeakerVolumeLevel, feature is not supported",
                                         mPath.mEndpointId));
        uint8_t speakerVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(speakerVolLevel));
        return SetSpeakerVolumeLevel(speakerVolLevel);
    }
    case MicrophoneMuted::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneMuted, feature is not supported",
                                         mPath.mEndpointId));
        bool micMuted;
        ReturnErrorOnFailure(aDecoder.Decode(micMuted));
        return SetMicrophoneMuted(micMuted);
    }
    case MicrophoneVolumeLevel::Id: {
        VerifyOrReturnError(HasFeature(Feature::kAudio), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneVolumeLevel, feature is not supported",
                                         mPath.mEndpointId));
        uint8_t micVolLevel;
        ReturnErrorOnFailure(aDecoder.Decode(micVolLevel));
        return SetMicrophoneVolumeLevel(micVolLevel);
    }
    case MicrophoneAGCEnabled::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set MicrophoneAGCEnabled, feature is not supported",
                                         mPath.mEndpointId));
        bool micAGCEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(micAGCEnabled));
        return SetMicrophoneAGCEnabled(micAGCEnabled);
    }
    case ImageRotation::Id: {
        VerifyOrReturnError(
            SupportsOptAttr(OptionalAttribute::kImageRotation), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set ImageRotation, feature is not supported", mPath.mEndpointId));
        uint16_t imageRotation;
        ReturnErrorOnFailure(aDecoder.Decode(imageRotation));
        return SetImageRotation(imageRotation);
    }
    case ImageFlipHorizontal::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set ImageFlipHorizontal, feature is not supported",
                                         mPath.mEndpointId));
        bool imageFlipHorizontal;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipHorizontal));
        return SetImageFlipHorizontal(imageFlipHorizontal);
    }
    case ImageFlipVertical::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kImageFlipVertical), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set ImageFlipVertical, feature is not supported",
                                         mPath.mEndpointId));
        bool imageFlipVertical;
        ReturnErrorOnFailure(aDecoder.Decode(imageFlipVertical));
        return SetImageFlipVertical(imageFlipVertical);
    }
    case LocalVideoRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set LocalVideoRecordingEnabled, feature is not supported",
                         mPath.mEndpointId));
        bool localVidRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localVidRecEnabled));
        return SetLocalVideoRecordingEnabled(localVidRecEnabled);
    }
    case LocalSnapshotRecordingEnabled::Id: {
        VerifyOrReturnError(
            HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set LocalSnapshotRecordingEnabled, feature is not supported",
                         mPath.mEndpointId));
        bool localSnapshotRecEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(localSnapshotRecEnabled));
        return SetLocalSnapshotRecordingEnabled(localSnapshotRecEnabled);
    }
    case StatusLightEnabled::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightEnabled), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: can not set StatusLightEnabled, feature is not supported",
                                         mPath.mEndpointId));
        bool statusLightEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightEnabled));
        return SetStatusLightEnabled(statusLightEnabled);
    }
    case StatusLightBrightness::Id: {
        VerifyOrReturnError(SupportsOptAttr(OptionalAttribute::kStatusLightBrightness), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVStreamMgmt[ep=%d]: can not set StatusLightBrightness, feature is not supported",
                                         mPath.mEndpointId));
        Globals::ThreeLevelAutoEnum statusLightBrightness;
        ReturnErrorOnFailure(aDecoder.Decode(statusLightBrightness));
        return SetStatusLightBrightness(statusLightBrightness);
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

void CameraAVStreamManagementCluster::ModifyVideoStream(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                        const Optional<bool> osdEnabled)
{
    for (VideoStreamStruct & stream : mAllocatedVideoStreams)
    {
        if (stream.videoStreamID == streamID)
        {
            bool wasModified = false;
            if (waterMarkEnabled.HasValue())
            {
                wasModified             = (waterMarkEnabled != stream.watermarkEnabled);
                stream.watermarkEnabled = waterMarkEnabled;
            }
            if (osdEnabled.HasValue())
            {
                wasModified       = wasModified || (osdEnabled != stream.OSDEnabled);
                stream.OSDEnabled = osdEnabled;
            }
            if (wasModified)
            {
                TEMPORARY_RETURN_IGNORED PersistAndNotify<Attributes::AllocatedVideoStreams::Id>();
                ChipLogProgress(Camera, "Modified video stream with ID: %d", streamID);
            }
            return;
        }
    }
}

void CameraAVStreamManagementCluster::ModifySnapshotStream(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                           const Optional<bool> osdEnabled)
{
    for (SnapshotStreamStruct & stream : mAllocatedSnapshotStreams)
    {
        if (stream.snapshotStreamID == streamID)
        {
            bool wasModified = false;
            if (waterMarkEnabled.HasValue())
            {
                wasModified             = (waterMarkEnabled != stream.watermarkEnabled);
                stream.watermarkEnabled = waterMarkEnabled;
            }
            if (osdEnabled.HasValue())
            {
                wasModified       = wasModified || (osdEnabled != stream.OSDEnabled);
                stream.OSDEnabled = osdEnabled;
            }
            if (wasModified)
            {
                TEMPORARY_RETURN_IGNORED PersistAndNotify<Attributes::AllocatedSnapshotStreams::Id>();
                ChipLogProgress(Camera, "Modified snapshot stream with ID: %d", streamID);
            }
            return;
        }
    }
}

CHIP_ERROR CameraAVStreamManagementCluster::SetCurrentFrameRate(uint16_t aCurrentFrameRate)
{
    return SetAttributeIfDifferent(mCurrentFrameRate, aCurrentFrameRate, Attributes::CurrentFrameRate::Id,
                                   /* shouldPersist = */ false);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetHDRModeEnabled(bool aHDRModeEnabled)
{
    return SetAttributeIfDifferent(mHDRModeEnabled, aHDRModeEnabled, Attributes::HDRModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSoftRecordingPrivacyModeEnabled(bool aSoftRecordingPrivacyModeEnabled)
{
    return SetAttributeIfDifferent(mSoftRecordingPrivacyModeEnabled, aSoftRecordingPrivacyModeEnabled,
                                   Attributes::SoftRecordingPrivacyModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSoftLivestreamPrivacyModeEnabled(bool aSoftLivestreamPrivacyModeEnabled)
{
    return SetAttributeIfDifferent(mSoftLivestreamPrivacyModeEnabled, aSoftLivestreamPrivacyModeEnabled,
                                   Attributes::SoftLivestreamPrivacyModeEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetHardPrivacyModeOn(bool aHardPrivacyModeOn)
{
    return SetAttributeIfDifferent(mHardPrivacyModeOn, aHardPrivacyModeOn, Attributes::HardPrivacyModeOn::Id,
                                   /* shouldPersist = */ false);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetNightVision(TriStateAutoEnum aNightVision)
{
    if (mNightVision != aNightVision)
    {
        mNightVision = aNightVision;
        auto path    = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVision::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mNightVision)));
        mDelegate.OnAttributeChanged(Attributes::NightVision::Id);
        NotifyAttributeChanged(Attributes::NightVision::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::SetNightVisionIllum(TriStateAutoEnum aNightVisionIllum)
{
    if (mNightVisionIllum != aNightVisionIllum)
    {
        mNightVisionIllum = aNightVisionIllum;
        auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVisionIllum::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mNightVisionIllum)));
        mDelegate.OnAttributeChanged(Attributes::NightVisionIllum::Id);
        NotifyAttributeChanged(Attributes::NightVisionIllum::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::SetViewport(const Globals::Structs::ViewportStruct::Type & aViewport)
{
    // The following validation steps are required
    // 1. the new viewport is not larger than the sensor max
    // 2. the new viewport is not snaller than the sensor min
    // 3. the new viewport has the same aspect ratio as the sensor
    //
    uint16_t requestedWidth  = static_cast<uint16_t>(aViewport.x2 - aViewport.x1);
    uint16_t requestedHeight = static_cast<uint16_t>(aViewport.y2 - aViewport.y1);
    if ((requestedWidth < mMinViewPortResolution.width) || (requestedHeight < mMinViewPortResolution.height) ||
        (requestedWidth > mVideoSensorParams.sensorWidth) || (requestedHeight > mVideoSensorParams.sensorHeight))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: SetViewport with invalid viewport dimensions", mPath.mEndpointId);
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Get the ARs to no more than 2DP.  Otherwise you get mismatches e.g. 16:9 ratio calculation for 480p isn't the same as
    // 1080p beyond 2DP.
    float requestedAR = floorf((static_cast<float>(requestedWidth) / requestedHeight) * 100) / 100;
    float deviceAR    = floorf((static_cast<float>(mVideoSensorParams.sensorWidth) / mVideoSensorParams.sensorHeight) * 100) / 100;

    // Ensure that the aspect ration of the viewport matches the aspect ratio of the sensor
    if (requestedAR != deviceAR)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: SetViewport with mismatching aspect ratio.", mPath.mEndpointId);
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    mViewport = aViewport;

    TEMPORARY_RETURN_IGNORED StoreViewport(mViewport);
    mDelegate.OnAttributeChanged(Attributes::Viewport::Id);
    NotifyAttributeChanged(Attributes::Viewport::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSpeakerMuted(bool aSpeakerMuted)
{
    return SetAttributeIfDifferent(mSpeakerMuted, aSpeakerMuted, Attributes::SpeakerMuted::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSpeakerVolumeLevel(uint8_t aSpeakerVolumeLevel)
{
    if (aSpeakerVolumeLevel < mSpeakerMinLevel || aSpeakerVolumeLevel > mSpeakerMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerVolumeLevel, aSpeakerVolumeLevel, Attributes::SpeakerVolumeLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSpeakerMaxLevel(uint8_t aSpeakerMaxLevel)
{
    if (aSpeakerMaxLevel < mSpeakerMinLevel || aSpeakerMaxLevel > kMaxSpeakerLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerMaxLevel, aSpeakerMaxLevel, Attributes::SpeakerMaxLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetSpeakerMinLevel(uint8_t aSpeakerMinLevel)
{
    if (aSpeakerMinLevel > mSpeakerMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mSpeakerMinLevel, aSpeakerMinLevel, Attributes::SpeakerMinLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetMicrophoneMuted(bool aMicrophoneMuted)
{
    return SetAttributeIfDifferent(mMicrophoneMuted, aMicrophoneMuted, Attributes::MicrophoneMuted::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetMicrophoneVolumeLevel(uint8_t aMicrophoneVolumeLevel)
{
    if (aMicrophoneVolumeLevel < mMicrophoneMinLevel || aMicrophoneVolumeLevel > mMicrophoneMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneVolumeLevel, aMicrophoneVolumeLevel, Attributes::MicrophoneVolumeLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetMicrophoneMaxLevel(uint8_t aMicrophoneMaxLevel)
{
    if (aMicrophoneMaxLevel < mMicrophoneMinLevel || aMicrophoneMaxLevel > kMaxMicrophoneLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneMaxLevel, aMicrophoneMaxLevel, Attributes::MicrophoneMaxLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetMicrophoneMinLevel(uint8_t aMicrophoneMinLevel)
{
    if (aMicrophoneMinLevel > mMicrophoneMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mMicrophoneMinLevel, aMicrophoneMinLevel, Attributes::MicrophoneMinLevel::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetMicrophoneAGCEnabled(bool aMicrophoneAGCEnabled)
{
    return SetAttributeIfDifferent(mMicrophoneAGCEnabled, aMicrophoneAGCEnabled, Attributes::MicrophoneAGCEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetImageRotation(uint16_t aImageRotation)
{
    if (aImageRotation > kMaxImageRotationDegrees)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return SetAttributeIfDifferent(mImageRotation, aImageRotation, Attributes::ImageRotation::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetImageFlipHorizontal(bool aImageFlipHorizontal)
{
    return SetAttributeIfDifferent(mImageFlipHorizontal, aImageFlipHorizontal, Attributes::ImageFlipHorizontal::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetImageFlipVertical(bool aImageFlipVertical)
{
    return SetAttributeIfDifferent(mImageFlipVertical, aImageFlipVertical, Attributes::ImageFlipVertical::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetLocalVideoRecordingEnabled(bool aLocalVideoRecordingEnabled)
{
    return SetAttributeIfDifferent(mLocalVideoRecordingEnabled, aLocalVideoRecordingEnabled,
                                   Attributes::LocalVideoRecordingEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetLocalSnapshotRecordingEnabled(bool aLocalSnapshotRecordingEnabled)
{
    return SetAttributeIfDifferent(mLocalSnapshotRecordingEnabled, aLocalSnapshotRecordingEnabled,
                                   Attributes::LocalSnapshotRecordingEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetStatusLightEnabled(bool aStatusLightEnabled)
{
    return SetAttributeIfDifferent(mStatusLightEnabled, aStatusLightEnabled, Attributes::StatusLightEnabled::Id);
}

CHIP_ERROR CameraAVStreamManagementCluster::SetStatusLightBrightness(Globals::ThreeLevelAutoEnum aStatusLightBrightness)
{
    if (mStatusLightBrightness != aStatusLightBrightness)
    {
        mStatusLightBrightness = aStatusLightBrightness;
        auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightBrightness::Id);
        ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, to_underlying(mStatusLightBrightness)));
        mDelegate.OnAttributeChanged(Attributes::StatusLightBrightness::Id);
        NotifyAttributeChanged(Attributes::StatusLightBrightness::Id);
    }
    return CHIP_NO_ERROR;
}

void CameraAVStreamManagementCluster::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load HDR Mode Enabled
    bool storedHDRModeEnabled = false;
    err                       = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::HDRModeEnabled::Id),
        storedHDRModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mHDRModeEnabled = storedHDRModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded HDRModeEnabled as %u", mPath.mEndpointId, mHDRModeEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the HDRModeEnabled from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mHDRModeEnabled);
    }

    // Load AllocatedVideoStreams
    err = LoadAllocatedStreams<Attributes::AllocatedVideoStreams::Id>();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated video streams from the KVS.", mPath.mEndpointId);
    }
    // Load AllocatedAudioStreams
    err = LoadAllocatedStreams<Attributes::AllocatedAudioStreams::Id>();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated audio streams from the KVS.", mPath.mEndpointId);
    }

    // Load AllocatedSnapshotStreams
    err = LoadAllocatedStreams<Attributes::AllocatedSnapshotStreams::Id>();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load allocated snapshot streams from the KVS.", mPath.mEndpointId);
    }

    // Load StreamUsagePriorities
    err = LoadStreamUsagePriorities();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the StreamUsagePriorities from the KVS.", mPath.mEndpointId);
    }

    // Load SoftRecordingPrivacyModeEnabled
    bool softRecordingPrivacyModeEnabled = false;
    err                                  = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftRecordingPrivacyModeEnabled::Id),
        softRecordingPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftRecordingPrivacyModeEnabled = softRecordingPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SoftRecordingPrivacyModeEnabled as %u", mPath.mEndpointId,
                      mSoftRecordingPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(
            Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SoftRecordingPrivacyModeEnabled from the KVS. Defaulting to %u",
            mPath.mEndpointId, mSoftRecordingPrivacyModeEnabled);
    }

    // Load SoftLivestreamPrivacyModeEnabled
    bool softLivestreamPrivacyModeEnabled = false;
    err                                   = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::SoftLivestreamPrivacyModeEnabled::Id),
        softLivestreamPrivacyModeEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mSoftLivestreamPrivacyModeEnabled = softLivestreamPrivacyModeEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SoftLivestreamPrivacyModeEnabled as %u", mPath.mEndpointId,
                      mSoftLivestreamPrivacyModeEnabled);
    }
    else
    {
        ChipLogDetail(
            Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SoftLivestreamPrivacyModeEnabled from the KVS. Defaulting to %u",
            mPath.mEndpointId, mSoftLivestreamPrivacyModeEnabled);
    }

    // Load NightVision
    uint8_t nightVision = 0;
    err                 = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVision::Id), nightVision);
    if (err == CHIP_NO_ERROR)
    {
        mNightVision = static_cast<TriStateAutoEnum>(nightVision);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded NightVision as %d", mPath.mEndpointId, to_underlying(mNightVision));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the NightVision from the KVS. Defaulting to %d",
                      mPath.mEndpointId, to_underlying(mNightVision));
    }

    // Load NightVisionIllum
    uint8_t nightVisionIllum = 0;
    err                      = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::NightVisionIllum::Id), nightVisionIllum);
    if (err == CHIP_NO_ERROR)
    {
        mNightVisionIllum = static_cast<TriStateAutoEnum>(nightVisionIllum);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded NightVisionIllum as %d", mPath.mEndpointId,
                      to_underlying(mNightVisionIllum));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the NightVisionIllum from the KVS. Defaulting to %d",
                      mPath.mEndpointId, to_underlying(mNightVisionIllum));
    }

    // Load Viewport
    Globals::Structs::ViewportStruct::Type viewport;
    err = LoadViewport(viewport);
    if (err == CHIP_NO_ERROR)
    {
        mViewport = viewport;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded Viewport", mPath.mEndpointId);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the Viewport from the KVS.", mPath.mEndpointId);
    }

    // Load SpeakerMuted
    bool speakerMuted = false;
    err               = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerMuted::Id), speakerMuted);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerMuted = speakerMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SpeakerMuted as %u", mPath.mEndpointId, mSpeakerMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SpeakerMuted from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mSpeakerMuted);
    }

    // Load SpeakerVolumeLevel
    uint8_t speakerVolumeLevel = 0;
    err                        = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::SpeakerVolumeLevel::Id),
        speakerVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mSpeakerVolumeLevel = speakerVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded SpeakerVolumeLevel as %u", mPath.mEndpointId, mSpeakerVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the SpeakerVolumeLevel from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mSpeakerVolumeLevel);
    }

    // Load MicrophoneMuted
    bool microphoneMuted = false;
    err                  = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneMuted::Id), microphoneMuted);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneMuted = microphoneMuted;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneMuted as %u", mPath.mEndpointId, mMicrophoneMuted);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneMuted from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mMicrophoneMuted);
    }

    // Load MicrophoneVolumeLevel
    uint8_t microphoneVolumeLevel = 0;
    err                           = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneVolumeLevel::Id),
        microphoneVolumeLevel);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneVolumeLevel = microphoneVolumeLevel;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneVolumeLevel as %u", mPath.mEndpointId,
                      mMicrophoneVolumeLevel);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneVolumeLevel from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mMicrophoneVolumeLevel);
    }

    // Load MicrophoneAGCEnabled
    bool microphoneAGCEnabled = false;
    err                       = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::MicrophoneAGCEnabled::Id),
        microphoneAGCEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mMicrophoneAGCEnabled = microphoneAGCEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded MicrophoneAGCEnabled as %u", mPath.mEndpointId,
                      mMicrophoneAGCEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the MicrophoneAGCEnabled from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mMicrophoneAGCEnabled);
    }

    // Load ImageRotation
    uint16_t imageRotation = 0;
    err                    = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageRotation::Id), imageRotation);
    if (err == CHIP_NO_ERROR)
    {
        mImageRotation = imageRotation;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageRotation as %u", mPath.mEndpointId, mImageRotation);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageRotation from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mImageRotation);
    }

    // Load ImageFlipHorizontal
    bool imageFlipHorizontal = false;
    err                      = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipHorizontal::Id),
        imageFlipHorizontal);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipHorizontal = imageFlipHorizontal;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageFlipHorizontal as %u", mPath.mEndpointId, mImageFlipHorizontal);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageFlipHorizontal from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mImageFlipHorizontal);
    }

    // Load ImageFlipVertical
    bool imageFlipVertical = false;
    err                    = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::ImageFlipVertical::Id),
        imageFlipVertical);
    if (err == CHIP_NO_ERROR)
    {
        mImageFlipVertical = imageFlipVertical;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded ImageFlipVertical as %u", mPath.mEndpointId, mImageFlipVertical);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the ImageFlipVertical from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mImageFlipVertical);
    }

    // Load LocalVideoRecordingEnabled
    bool localVideoRecordingEnabled = false;
    err                             = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalVideoRecordingEnabled::Id),
        localVideoRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalVideoRecordingEnabled = localVideoRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded LocalVideoRecordingEnabled as %u", mPath.mEndpointId,
                      mLocalVideoRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl,
                      "CameraAVStreamMgmt[ep=%d]: Unable to load the LocalVideoRecordingEnabled from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mLocalVideoRecordingEnabled);
    }

    // Load LocalSnapshotRecordingEnabled
    bool localSnapshotRecordingEnabled = false;
    err                                = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::LocalSnapshotRecordingEnabled::Id),
        localSnapshotRecordingEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mLocalSnapshotRecordingEnabled = localSnapshotRecordingEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded LocalSnapshotRecordingEnabled as %u", mPath.mEndpointId,
                      mLocalSnapshotRecordingEnabled);
    }
    else
    {
        ChipLogDetail(Zcl,
                      "CameraAVStreamMgmt[ep=%d]: Unable to load the LocalSnapshotRecordingEnabled from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mLocalSnapshotRecordingEnabled);
    }

    // Load StatusLightEnabled
    bool statusLightEnabled = false;
    err                     = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightEnabled::Id),
        statusLightEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightEnabled = statusLightEnabled;
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded StatusLightEnabled as %u", mPath.mEndpointId, mStatusLightEnabled);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the StatusLightEnabled from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mStatusLightEnabled);
    }

    // Load StatusLightBrightness
    uint8_t statusLightBrightness = 0;
    err                           = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::StatusLightBrightness::Id),
        statusLightBrightness);
    if (err == CHIP_NO_ERROR)
    {
        mStatusLightBrightness = static_cast<Globals::ThreeLevelAutoEnum>(statusLightBrightness);
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Loaded StatusLightBrightness as %d", mPath.mEndpointId,
                      to_underlying(mStatusLightBrightness));
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Unable to load the StatusLightBrightness from the KVS. Defaulting to %d",
                      mPath.mEndpointId, to_underlying(mStatusLightBrightness));
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    CHIP_ERROR cbErr = mDelegate.PersistentAttributesLoadedCallback();
    if (cbErr != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: PersistentAttributesLoadedCallback() failed with error %" CHIP_ERROR_FORMAT,
                     mPath.mEndpointId, cbErr.Format());
    }
}

CHIP_ERROR CameraAVStreamManagementCluster::StoreViewport(const Globals::Structs::ViewportStruct::Type & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;

    writer.Init(bufferSpan);
    ReturnErrorOnFailure(viewport.Encode(writer, TLV::AnonymousTag()));

    auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    bufferSpan.reduce_size(writer.GetLengthWritten());

    return GetSafeAttributePersistenceProvider()->SafeWriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAVStreamManagementCluster::LoadViewport(Globals::Structs::ViewportStruct::Type & viewport)
{
    uint8_t buffer[kViewportStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::Viewport::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeReadValue(path, bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(viewport.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::StoreStreamUsagePriorities()
{
    uint8_t buffer[kStreamUsagePrioritiesTlvSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;
    writer.Init(bufferSpan);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < mStreamUsagePriorities.size(); i++)
    {
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), mStreamUsagePriorities[i]));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    bufferSpan.reduce_size(writer.GetLengthWritten());

    auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::StreamUsagePriorities::Id);
    return GetSafeAttributePersistenceProvider()->SafeWriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAVStreamManagementCluster::LoadStreamUsagePriorities()
{
    uint8_t buffer[kStreamUsagePrioritiesTlvSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, Attributes::StreamUsagePriorities::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeReadValue(path, bufferSpan));

    TLV::TLVReader reader;
    reader.Init(bufferSpan);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    mStreamUsagePriorities.clear();
    Globals::StreamUsageEnum streamUsage;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(streamUsage));
        mStreamUsagePriorities.push_back(streamUsage);
    }

    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

// Stream helper template struct containing necessary items for the
// StoreAllocatedStreams and LoadAllocatedStream functions to work on
// all the 3 stream types.
template <AttributeId TAttributeId>
struct StreamTraits;

template <>
struct StreamTraits<Attributes::AllocatedVideoStreams::Id>
{
    using StreamStructType                     = VideoStreamStruct;
    static constexpr size_t kMaxSerializedSize = kMaxAllocatedVideoStreamsSerializedSize;
    static constexpr StreamType kStreamType    = StreamType::kVideo;
    static constexpr auto kStreamVectorMember  = &CameraAVStreamManagementCluster::mAllocatedVideoStreams;
};

template <>
struct StreamTraits<Attributes::AllocatedAudioStreams::Id>
{
    using StreamStructType                     = AudioStreamStruct;
    static constexpr size_t kMaxSerializedSize = kMaxAllocatedAudioStreamsSerializedSize;
    static constexpr StreamType kStreamType    = StreamType::kAudio;
    static constexpr auto kStreamVectorMember  = &CameraAVStreamManagementCluster::mAllocatedAudioStreams;
};

template <>
struct StreamTraits<Attributes::AllocatedSnapshotStreams::Id>
{
    using StreamStructType                     = SnapshotStreamStruct;
    static constexpr size_t kMaxSerializedSize = kMaxAllocatedSnapshotStreamsSerializedSize;
    static constexpr StreamType kStreamType    = StreamType::kSnapshot;
    static constexpr auto kStreamVectorMember  = &CameraAVStreamManagementCluster::mAllocatedSnapshotStreams;
};

template <AttributeId TAttributeId>
CHIP_ERROR CameraAVStreamManagementCluster::PersistAndNotify()
{
    ReturnErrorAndLogOnFailure(StoreAllocatedStreams<TAttributeId>(), Zcl,
                               "CameraAVStreamMgmt[ep=%d]: Failed to persist allocated streams", mPath.mEndpointId);

    mDelegate.OnAttributeChanged(TAttributeId);
    NotifyAttributeChanged(TAttributeId);

    return CHIP_NO_ERROR;
}

template <AttributeId attributeId>
CHIP_ERROR CameraAVStreamManagementCluster::StoreAllocatedStreams()
{
    using Traits = StreamTraits<attributeId>;

    uint8_t buffer[Traits::kMaxSerializedSize];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    const auto & streams = (*this).*Traits::kStreamVectorMember;
    for (const auto & stream : streams)
    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), stream));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    size_t len = writer.GetLengthWritten();

    auto path = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, attributeId);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeWriteValue(path, ByteSpan(buffer, len)));

    ChipLogProgress(Zcl, "Saved %u %s streams", static_cast<unsigned int>(streams.size()), StreamTypeToString(Traits::kStreamType));
    return CHIP_NO_ERROR;
}

template <AttributeId attributeId>
CHIP_ERROR CameraAVStreamManagementCluster::LoadAllocatedStreams()
{
    using Traits = StreamTraits<attributeId>;

    uint8_t buffer[Traits::kMaxSerializedSize];
    MutableByteSpan span(buffer);

    auto path      = ConcreteAttributePath(mPath.mEndpointId, CameraAvStreamManagement::Id, attributeId);
    auto & streams = (*this).*Traits::kStreamVectorMember;

    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->SafeReadValue(path, span);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        streams.clear();
        ChipLogProgress(Zcl, "No persisted %s streams found.", StreamTypeToString(Traits::kStreamType));
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(span);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    streams.clear();
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        typename Traits::StreamStructType stream;
        ReturnErrorOnFailure(DataModel::Decode(reader, stream));
        streams.push_back(stream);
    }

    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));

    ChipLogProgress(Zcl, "Loaded %u %s streams", static_cast<unsigned int>(streams.size()),
                    StreamTypeToString(Traits::kStreamType));

    return reader.VerifyEndOfContainer();
}

CHIP_ERROR CameraAVStreamManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (HasFeature(Feature::kAudio))
    {
        ReturnErrorOnFailure(builder.AppendElements(
            { Commands::AudioStreamAllocate::kMetadataEntry, Commands::AudioStreamDeallocate::kMetadataEntry }));
    }

    if (HasFeature(Feature::kVideo))
    {
        ReturnErrorOnFailure(builder.AppendElements(
            { Commands::VideoStreamAllocate::kMetadataEntry, Commands::VideoStreamDeallocate::kMetadataEntry }));
    }

    if (HasFeature(Feature::kVideo) && (HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay)))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::VideoStreamModify::kMetadataEntry }));
    }

    if (HasFeature(Feature::kSnapshot) && (HasFeature(Feature::kWatermark) || HasFeature(Feature::kOnScreenDisplay)))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SnapshotStreamModify::kMetadataEntry }));
    }

    if (HasFeature(Feature::kSnapshot))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SnapshotStreamAllocate::kMetadataEntry,
                                                      Commands::SnapshotStreamDeallocate::kMetadataEntry,
                                                      Commands::CaptureSnapshot::kMetadataEntry }));
    }

    ReturnErrorOnFailure(builder.AppendElements({ Commands::SetStreamPriorities::kMetadataEntry }));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (HasFeature(Feature::kAudio))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::AudioStreamAllocateResponse::Id }));
    }

    if (HasFeature(Feature::kVideo))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::VideoStreamAllocateResponse::Id }));
    }

    if (HasFeature(Feature::kSnapshot))
    {
        ReturnErrorOnFailure(
            builder.AppendElements({ Commands::SnapshotStreamAllocateResponse::Id, Commands::CaptureSnapshotResponse::Id }));
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::InvokeCommand(const DataModel::InvokeRequest & aRequest, TLV::TLVReader & aInputArgs,
                                               CommandHandler * aHandler)
{
    switch (aRequest.path.mCommandId)
    {
    case Commands::VideoStreamAllocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Video Stream", mPath.mEndpointId);

        Commands::VideoStreamAllocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleVideoStreamAllocate(*aHandler, aRequest.path, commandData);
    }

    case Commands::VideoStreamModify::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Modifying Video Stream", mPath.mEndpointId);

        Commands::VideoStreamModify::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleVideoStreamModify(commandData);
    }

    case Commands::VideoStreamDeallocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Video Stream", mPath.mEndpointId);

        Commands::VideoStreamDeallocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleVideoStreamDeallocate(commandData);
    }

    case Commands::AudioStreamAllocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Audio Stream", mPath.mEndpointId);

        Commands::AudioStreamAllocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleAudioStreamAllocate(*aHandler, aRequest.path, commandData);
    }

    case Commands::AudioStreamDeallocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Audio Stream", mPath.mEndpointId);

        Commands::AudioStreamDeallocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleAudioStreamDeallocate(commandData);
    }

    case Commands::SnapshotStreamAllocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Allocating Snapshot Stream", mPath.mEndpointId);

        Commands::SnapshotStreamAllocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleSnapshotStreamAllocate(*aHandler, aRequest.path, commandData);
    }

    case Commands::SnapshotStreamModify::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Modifying Snapshot Stream", mPath.mEndpointId);

        Commands::SnapshotStreamModify::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleSnapshotStreamModify(commandData);
    }

    case Commands::SnapshotStreamDeallocate::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Deallocating Snapshot Stream", mPath.mEndpointId);

        Commands::SnapshotStreamDeallocate::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleSnapshotStreamDeallocate(commandData);
    }

    case Commands::SetStreamPriorities::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Set Stream Priorities", mPath.mEndpointId);

        Commands::SetStreamPriorities::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleSetStreamPriorities(commandData);
    }

    case Commands::CaptureSnapshot::Id: {
        ChipLogDetail(Zcl, "CameraAVStreamMgmt[ep=%d]: Capture Snapshot image", mPath.mEndpointId);

        Commands::CaptureSnapshot::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        return HandleCaptureSnapshot(*aHandler, aRequest.path, commandData);
    }
    }
    return Status::UnsupportedCommand;
}

CHIP_ERROR CameraAVStreamManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // Counts only via feature bits; other optionals via optional attribute set.
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), Attributes::MaxConcurrentEncoders::kMetadataEntry },
        { HasFeature(Feature::kVideo) || HasFeature(Feature::kSnapshot), Attributes::MaxEncodedPixelRate::kMetadataEntry },
        { HasFeature(Feature::kVideo), VideoSensorParams::kMetadataEntry },
        { HasFeature(Feature::kNightVision), NightVisionUsesInfrared::kMetadataEntry },
        { HasFeature(Feature::kVideo), MinViewportResolution::kMetadataEntry },
        { HasFeature(Feature::kVideo), RateDistortionTradeOffPoints::kMetadataEntry },
        { HasFeature(Feature::kAudio), MicrophoneCapabilities::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), SpeakerCapabilities::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), TwoWayTalkSupport::kMetadataEntry },
        { HasFeature(Feature::kSnapshot), SnapshotCapabilities::kMetadataEntry },
        { HasFeature(Feature::kVideo), CurrentFrameRate::kMetadataEntry },
        { HasFeature(Feature::kHighDynamicRange), HDRModeEnabled::kMetadataEntry },
        { HasFeature(Feature::kVideo), AllocatedVideoStreams::kMetadataEntry },
        { HasFeature(Feature::kAudio), AllocatedAudioStreams::kMetadataEntry },
        { HasFeature(Feature::kSnapshot), AllocatedSnapshotStreams::kMetadataEntry },
        { HasFeature(Feature::kPrivacy), SoftRecordingPrivacyModeEnabled::kMetadataEntry },
        { HasFeature(Feature::kPrivacy), SoftLivestreamPrivacyModeEnabled::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kHardPrivacyModeOn), HardPrivacyModeOn::kMetadataEntry },
        { HasFeature(Feature::kNightVision), NightVision::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kNightVisionIllum), NightVisionIllum::kMetadataEntry },
        { HasFeature(Feature::kVideo), Viewport::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), SpeakerMuted::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), SpeakerVolumeLevel::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), SpeakerMaxLevel::kMetadataEntry },
        { HasFeature(Feature::kSpeaker), SpeakerMinLevel::kMetadataEntry },
        { HasFeature(Feature::kAudio), MicrophoneMuted::kMetadataEntry },
        { HasFeature(Feature::kAudio), MicrophoneVolumeLevel::kMetadataEntry },
        { HasFeature(Feature::kAudio), MicrophoneMaxLevel::kMetadataEntry },
        { HasFeature(Feature::kAudio), MicrophoneMinLevel::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kMicrophoneAGCEnabled), MicrophoneAGCEnabled::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kImageRotation), ImageRotation::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kImageFlipHorizontal), ImageFlipHorizontal::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kImageFlipVertical), ImageFlipVertical::kMetadataEntry },
        { HasFeature(Feature::kVideo) && HasFeature(Feature::kLocalStorage), LocalVideoRecordingEnabled::kMetadataEntry },
        { HasFeature(Feature::kSnapshot) && HasFeature(Feature::kLocalStorage), LocalSnapshotRecordingEnabled::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kStatusLightEnabled), StatusLightEnabled::kMetadataEntry },
        { SupportsOptAttr(OptionalAttribute::kStatusLightBrightness), StatusLightBrightness::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

bool CameraAVStreamManagementCluster::StreamPrioritiesHasDuplicates(
    const std::vector<Globals::StreamUsageEnum> & aStreamUsagePriorities)
{
    std::set<Globals::StreamUsageEnum> seenStreamUsages;

    for (auto streamUsage : aStreamUsagePriorities)
    {
        if (!seenStreamUsages.insert(streamUsage).second)
        {
            return true;
        }
    }

    return false;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleVideoStreamAllocate(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                           const Commands::VideoStreamAllocate::DecodableType & commandData)
{
    Status status = Status::Success;

    uint16_t videoStreamID = 0;

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturnError((HasFeature(Feature::kWatermark) == commandData.watermarkEnabled.HasValue()), Status::InvalidCommand);

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturnError((HasFeature(Feature::kOnScreenDisplay) == commandData.OSDEnabled.HasValue()), Status::InvalidCommand);

    if (!(commandData.streamUsage == Globals::StreamUsageEnum::kRecording ||
          commandData.streamUsage == Globals::StreamUsageEnum::kAnalysis ||
          commandData.streamUsage == Globals::StreamUsageEnum::kLiveView))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid stream usage", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    if (commandData.videoCodec == VideoCodecEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid video codec", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    VerifyOrReturnError(commandData.minFrameRate >= 1 && commandData.minFrameRate <= commandData.maxFrameRate &&
                            commandData.maxFrameRate >= 1,
                        Status::ConstraintError);

    VerifyOrReturnError(commandData.minResolution.width >= 1 && commandData.minResolution.height >= 1, Status::ConstraintError);

    VerifyOrReturnError(commandData.maxResolution.width >= 1 && commandData.maxResolution.height >= 1, Status::ConstraintError);

    if (commandData.minResolution.width > commandData.maxResolution.width ||
        commandData.minResolution.height > commandData.maxResolution.height)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: minResolution cannot be greater than maxResolution", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    VerifyOrReturnError(commandData.minBitRate >= 1 && commandData.minBitRate <= commandData.maxBitRate &&
                            commandData.maxBitRate >= 1,
                        Status::ConstraintError);

    VerifyOrReturnError(commandData.keyFrameInterval <= kMaxKeyFrameIntervalMaxValue, Status::ConstraintError);

    bool streamUsageSupported = std::find_if(mStreamUsagePriorities.begin(), mStreamUsagePriorities.end(),
                                             [&commandData](const Globals::StreamUsageEnum & entry) {
                                                 return entry == commandData.streamUsage;
                                             }) != mStreamUsagePriorities.end();

    VerifyOrReturnError(streamUsageSupported, Status::InvalidInState);

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
    videoStreamArgs.keyFrameInterval = commandData.keyFrameInterval;
    videoStreamArgs.watermarkEnabled = commandData.watermarkEnabled;
    videoStreamArgs.OSDEnabled       = commandData.OSDEnabled;
    videoStreamArgs.referenceCount   = 0;

    // Call the delegate
    status = mDelegate.VideoStreamAllocate(videoStreamArgs, videoStreamID);

    if (status == Status::Success)
    {
        // Check if the streamID matches an existing one in the
        // mAllocatedVideoStreams.
        auto it =
            std::find_if(mAllocatedVideoStreams.begin(), mAllocatedVideoStreams.end(),
                         [videoStreamID](const VideoStreamStruct & vStream) { return vStream.videoStreamID == videoStreamID; });

        if (it == mAllocatedVideoStreams.end())
        {
            // Add the allocated videostream object in the AllocatedVideoStreams list.
            videoStreamArgs.videoStreamID = videoStreamID;
            TEMPORARY_RETURN_IGNORED AddVideoStream(videoStreamArgs);

            // Call delegate with the allocated stream parameters and new allocation action
            mDelegate.OnVideoStreamAllocated(videoStreamArgs, StreamAllocationAction::kNewAllocation);
        }
        else
        {
            bool wasModified = false;

            VideoStreamStruct & videoStreamToUpdate = *it;
            // Reusing the existing stream. Update range parameters and check if they were modified
            TEMPORARY_RETURN_IGNORED UpdateVideoStreamRangeParams(videoStreamToUpdate, videoStreamArgs, wasModified);

            // Call delegate with the final updated stream parameters and appropriate action
            mDelegate.OnVideoStreamAllocated(videoStreamToUpdate,
                                             wasModified ? StreamAllocationAction::kModification : StreamAllocationAction::kReuse);
        }

        Commands::VideoStreamAllocateResponse::Type response;

        response.videoStreamID = videoStreamID;
        handler.AddResponse(commandPath, response);

        return std::nullopt;
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleVideoStreamModify(const Commands::VideoStreamModify::DecodableType & commandData)
{
    auto & isWaterMarkEnabled = commandData.watermarkEnabled;
    auto & isOSDEnabled       = commandData.OSDEnabled;
    auto & videoStreamID      = commandData.videoStreamID;

    // If WatermarkEnabled is provided then the Watermark feature has to be supported
    if (commandData.watermarkEnabled.HasValue())
    {
        if (!HasFeature(Feature::kWatermark))
        {
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: WatermarkEnabled provided but Watermark Feature not set",
                         mPath.mEndpointId);
            return Status::InvalidCommand;
        }
    }

    // If OSDEnabled is provided then the OSD feature has to be supported
    if (commandData.OSDEnabled.HasValue())
    {
        if (!HasFeature(Feature::kOnScreenDisplay))
        {
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: OSDEnabled provided but OSD Feature not set", mPath.mEndpointId);
            return Status::InvalidCommand;
        }
    }

    // One of WatermarkEnabled or OSDEnabled has to be present
    if (!(commandData.watermarkEnabled.HasValue() || commandData.OSDEnabled.HasValue()))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: One of WatermarkEnabled or OSDEnabled must be provided in VideoStreamModify",
                     mPath.mEndpointId);
        return Status::InvalidCommand;
    }

    Status status = ValidateVideoStreamForModifyOrDeallocate(videoStreamID, /* isDeallocate = */ false);
    if (status != Status::Success)
    {
        return status;
    }

    // Call the delegate
    status = mDelegate.VideoStreamModify(videoStreamID, isWaterMarkEnabled, isOSDEnabled);

    if (status == Status::Success)
    {
        ModifyVideoStream(videoStreamID, isWaterMarkEnabled, isOSDEnabled);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleVideoStreamDeallocate(const Commands::VideoStreamDeallocate::DecodableType & commandData)
{
    auto & videoStreamID = commandData.videoStreamID;

    Status status = ValidateVideoStreamForModifyOrDeallocate(videoStreamID, /* isDeallocate = */ true);
    if (status != Status::Success)
    {
        return status;
    }

    // Call the delegate
    status = mDelegate.VideoStreamDeallocate(videoStreamID);

    if (status == Status::Success)
    {
        TEMPORARY_RETURN_IGNORED RemoveVideoStream(videoStreamID);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleAudioStreamAllocate(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                           const Commands::AudioStreamAllocate::DecodableType & commandData)
{
    uint16_t audioStreamID = 0;

    if (!(commandData.streamUsage == Globals::StreamUsageEnum::kRecording ||
          commandData.streamUsage == Globals::StreamUsageEnum::kAnalysis ||
          commandData.streamUsage == Globals::StreamUsageEnum::kLiveView))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid stream usage", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    if (commandData.audioCodec == AudioCodecEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid audio codec", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    if (!(commandData.channelCount >= 1 && commandData.channelCount <= kMaxChannelCount))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid channel count", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    VerifyOrReturnError(commandData.sampleRate > 0, Status::ConstraintError);

    VerifyOrReturnError(commandData.bitRate > 0, Status::ConstraintError);

    if (!IsBitDepthValid(commandData.bitDepth))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid bitDepth", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    bool streamUsageSupported = std::find_if(mStreamUsagePriorities.begin(), mStreamUsagePriorities.end(),
                                             [&commandData](const Globals::StreamUsageEnum & entry) {
                                                 return entry == commandData.streamUsage;
                                             }) != mStreamUsagePriorities.end();

    VerifyOrReturnError(streamUsageSupported, Status::InvalidInState);

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
        return status;
    }

    // Check if the streamID matches an existing one in the
    // mAllocatedAudioStreams.
    bool streamExists = std::find_if(mAllocatedAudioStreams.begin(), mAllocatedAudioStreams.end(),
                                     [&audioStreamID](const AudioStreamStruct & entry) {
                                         return entry.audioStreamID == audioStreamID;
                                     }) != mAllocatedAudioStreams.end();
    if (!streamExists)
    {
        // Add the allocated audiostream object in the AllocatedAudioStreams list.
        audioStreamArgs.audioStreamID = audioStreamID;
        TEMPORARY_RETURN_IGNORED AddAudioStream(audioStreamArgs);
    }

    Commands::AudioStreamAllocateResponse::Type response;

    response.audioStreamID = audioStreamID;
    handler.AddResponse(commandPath, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleAudioStreamDeallocate(const Commands::AudioStreamDeallocate::DecodableType & commandData)
{
    auto & audioStreamID = commandData.audioStreamID;

    Status status = ValidateAudioStreamForDeallocate(audioStreamID);

    if (status != Status::Success)
    {
        return status;
    }

    // Call the delegate
    status = mDelegate.AudioStreamDeallocate(audioStreamID);

    if (status == Status::Success)
    {
        TEMPORARY_RETURN_IGNORED RemoveAudioStream(audioStreamID);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleSnapshotStreamAllocate(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                              const Commands::SnapshotStreamAllocate::DecodableType & commandData)
{

    uint16_t snapshotStreamID = 0;

    // If Watermark feature is supported, then command should have the
    // isWaterMarkEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturnError((HasFeature(Feature::kWatermark) == commandData.watermarkEnabled.HasValue()), Status::InvalidCommand);

    // If OSD feature is supported, then command should have the
    // isOSDEnabled param. Or, if it is not supported, then command should
    // not have the param.
    VerifyOrReturnError((HasFeature(Feature::kOnScreenDisplay) == commandData.OSDEnabled.HasValue()), Status::InvalidCommand);

    if (commandData.imageCodec == ImageCodecEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid image codec", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    if (commandData.maxFrameRate <= 0)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid maxFrameRate", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    VerifyOrReturnError(commandData.minResolution.width >= 1 && commandData.minResolution.height >= 1, Status::ConstraintError);

    VerifyOrReturnError(commandData.maxResolution.width >= 1 && commandData.maxResolution.height >= 1, Status::ConstraintError);

    if (commandData.minResolution.width > commandData.maxResolution.width ||
        commandData.minResolution.height > commandData.maxResolution.height)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: minResolution cannot be greater than maxResolution", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    if (!(commandData.quality > 0 && commandData.quality <= kMaxImageQualityMetric))
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Invalid image quality", mPath.mEndpointId);
        return Status::ConstraintError;
    }

    CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs snapshotStreamArgs;
    snapshotStreamArgs.imageCodec       = commandData.imageCodec;
    snapshotStreamArgs.maxFrameRate     = commandData.maxFrameRate;
    snapshotStreamArgs.minResolution    = commandData.minResolution;
    snapshotStreamArgs.maxResolution    = commandData.maxResolution;
    snapshotStreamArgs.quality          = commandData.quality;
    snapshotStreamArgs.encodedPixels    = false;
    snapshotStreamArgs.hardwareEncoder  = false;
    snapshotStreamArgs.watermarkEnabled = commandData.watermarkEnabled;
    snapshotStreamArgs.OSDEnabled       = commandData.OSDEnabled;

    // Fetch matching snapshot capabilities to figure out the requirement for
    // hardware encoder for this snapshot stream
    auto snCapabIt = std::find_if(mSnapshotCapabilitiesList.begin(), mSnapshotCapabilitiesList.end(), [&](const auto & capability) {
        return capability.imageCodec == commandData.imageCodec && capability.maxFrameRate >= commandData.maxFrameRate &&
            capability.resolution.width >= commandData.minResolution.width &&
            capability.resolution.height >= commandData.minResolution.height &&
            capability.resolution.width <= commandData.maxResolution.width &&
            capability.resolution.height <= commandData.maxResolution.height;
    });

    if (snCapabIt != mSnapshotCapabilitiesList.end())
    {
        // Set the configuration for the hardware encoder for the snapshot
        // stream
        snapshotStreamArgs.encodedPixels = snCapabIt->requiresEncodedPixels;
        if (snCapabIt->requiresEncodedPixels && snCapabIt->requiresHardwareEncoder.HasValue())
        {
            snapshotStreamArgs.hardwareEncoder = snCapabIt->requiresHardwareEncoder.Value();
        }
    }
    else
    {
        // Stream allocation request does not match any SnapshotCapabilities struct
        return Status::DynamicConstraintError;
    }

    // Call the delegate
    Status status = mDelegate.SnapshotStreamAllocate(snapshotStreamArgs, snapshotStreamID);

    if (status != Status::Success)
    {
        return status;
    }

    // Check if the streamID matches an existing one in the
    // mAllocatedSnapshotStreams.
    auto it = std::find_if(
        mAllocatedSnapshotStreams.begin(), mAllocatedSnapshotStreams.end(),
        [snapshotStreamID](const SnapshotStreamStruct & sStream) { return sStream.snapshotStreamID == snapshotStreamID; });

    if (it == mAllocatedSnapshotStreams.end())
    {
        // Add the allocated snapshot stream object in the mAllocatedSnapshotStreams list.
        SnapshotStreamStruct allocatedSnapshotStream;
        allocatedSnapshotStream.snapshotStreamID = snapshotStreamID;
        allocatedSnapshotStream.referenceCount   = 0;
        allocatedSnapshotStream.imageCodec       = snapshotStreamArgs.imageCodec;
        allocatedSnapshotStream.frameRate        = snapshotStreamArgs.maxFrameRate;
        allocatedSnapshotStream.minResolution    = snapshotStreamArgs.minResolution;
        allocatedSnapshotStream.maxResolution    = snapshotStreamArgs.maxResolution;
        allocatedSnapshotStream.quality          = snapshotStreamArgs.quality;
        allocatedSnapshotStream.encodedPixels    = snapshotStreamArgs.encodedPixels;
        allocatedSnapshotStream.hardwareEncoder  = snapshotStreamArgs.hardwareEncoder;
        allocatedSnapshotStream.watermarkEnabled = snapshotStreamArgs.watermarkEnabled;
        allocatedSnapshotStream.OSDEnabled       = snapshotStreamArgs.OSDEnabled;

        TEMPORARY_RETURN_IGNORED AddSnapshotStream(allocatedSnapshotStream);
    }
    else
    {
        SnapshotStreamStruct & snapshotStreamToUpdate = *it;
        // Reusing the existing stream. Update range parameters
        TEMPORARY_RETURN_IGNORED UpdateSnapshotStreamRangeParams(snapshotStreamToUpdate, snapshotStreamArgs);
    }

    Commands::SnapshotStreamAllocateResponse::Type response;
    response.snapshotStreamID = snapshotStreamID;
    handler.AddResponse(commandPath, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleSnapshotStreamModify(const Commands::SnapshotStreamModify::DecodableType & commandData)
{
    Status status             = Status::Success;
    auto & isWaterMarkEnabled = commandData.watermarkEnabled;
    auto & isOSDEnabled       = commandData.OSDEnabled;
    auto & snapshotStreamID   = commandData.snapshotStreamID;

    // If WatermarkEnabled is provided then the Watermark feature has to be supported
    if (commandData.watermarkEnabled.HasValue())
    {
        if (!HasFeature(Feature::kWatermark))
        {
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: WatermarkEnabled provided but Watermark Feature not set",
                         mPath.mEndpointId);
            return Status::InvalidCommand;
        }
    }

    // If OSDEnabled is provided then the OSD feature has to be supported
    if (commandData.OSDEnabled.HasValue())
    {
        if (!HasFeature(Feature::kOnScreenDisplay))
        {
            ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: OSDEnabled provided but OSD Feature not set", mPath.mEndpointId);
            return Status::InvalidCommand;
        }
    }

    // One of WatermarkEnabled or OSDEnabled has to be present
    if (!(commandData.watermarkEnabled.HasValue() || commandData.OSDEnabled.HasValue()))
    {
        ChipLogError(Zcl,
                     "CameraAVStreamMgmt[ep=%d]: One of WatermarkEnabled or OSDEnabled must be provided in SnapshotStreamModify",
                     mPath.mEndpointId);
        return Status::InvalidCommand;
    }

    status = ValidateSnapshotStreamForModifyOrDeallocate(snapshotStreamID, /* isDeallocate = */ false);
    if (status != Status::Success)
    {
        return status;
    }

    status = mDelegate.SnapshotStreamModify(snapshotStreamID, isWaterMarkEnabled, isOSDEnabled);

    if (status == Status::Success)
    {
        ModifySnapshotStream(snapshotStreamID, isWaterMarkEnabled, isOSDEnabled);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus> CameraAVStreamManagementCluster::HandleSnapshotStreamDeallocate(
    const Commands::SnapshotStreamDeallocate::DecodableType & commandData)
{
    auto & snapshotStreamID = commandData.snapshotStreamID;

    Status status = ValidateSnapshotStreamForModifyOrDeallocate(snapshotStreamID, /* isDeallocate = */ true);
    if (status != Status::Success)
    {
        return status;
    }

    // Call the delegate
    status = mDelegate.SnapshotStreamDeallocate(snapshotStreamID);

    if (status == Status::Success)
    {
        TEMPORARY_RETURN_IGNORED RemoveSnapshotStream(snapshotStreamID);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleSetStreamPriorities(const Commands::SetStreamPriorities::DecodableType & commandData)
{
    auto & streamPriorities = commandData.streamPriorities;
    std::vector<Globals::StreamUsageEnum> streamUsagePriorities;
    auto iter = streamPriorities.begin();

    // If any video, audio or snapshot streams exist fail the command.
    VerifyOrReturnError(mAllocatedVideoStreams.empty() && mAllocatedAudioStreams.empty() && mAllocatedSnapshotStreams.empty(),
                        Status::InvalidInState);

    while (iter.Next())
    {
        auto & streamUsage = iter.GetValue();
        if (streamUsage == Globals::StreamUsageEnum::kUnknownEnumValue)
        {
            return Status::InvalidCommand;
        }
        // If any requested value is not found in SupportedStreamUsages,
        // return DynamicConstraintError.
        auto it = std::find(mSupportedStreamUsages.begin(), mSupportedStreamUsages.end(), streamUsage);
        VerifyOrReturnError(it != mSupportedStreamUsages.end(), Status::DynamicConstraintError);

        streamUsagePriorities.push_back(streamUsage);
    }

    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }

    // If there are duplicate stream usages in StreamPriorities,
    // return AlreadyExists
    VerifyOrReturnError(!StreamPrioritiesHasDuplicates(streamUsagePriorities), Status::AlreadyExists);

    CHIP_ERROR err = SetStreamUsagePriorities(streamUsagePriorities);

    if (err != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }

    mDelegate.OnStreamUsagePrioritiesChanged();

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
CameraAVStreamManagementCluster::HandleCaptureSnapshot(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                       const Commands::CaptureSnapshot::DecodableType & commandData)
{
    auto & snapshotStreamID    = commandData.snapshotStreamID;
    auto & requestedResolution = commandData.requestedResolution;
    ImageSnapshot image;

    if (!CheckSnapshotStreamsAvailability())
    {
        return Status::NotFound;
    }

    if (!snapshotStreamID.IsNull())
    {
        if (!ValidateSnapshotStreamId(snapshotStreamID))
        {
            return Status::NotFound;
        }
    }

    VerifyOrReturnError(commandData.requestedResolution.width >= 1 && commandData.requestedResolution.height >= 1,
                        Status::ConstraintError);

    // If SoftLivestreamPrivacyModeEnabled or HardPrivacyModeOn, return
    // InvalidInState.
    VerifyOrReturnError(!mSoftLivestreamPrivacyModeEnabled && !mHardPrivacyModeOn, Status::InvalidInState);

    // Call the delegate
    Status status = mDelegate.CaptureSnapshot(snapshotStreamID, requestedResolution, image);

    if (status != Status::Success)
    {
        return status;
    }

    if (image.data.size() > kMaxSnapshotImageSize)
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: Snapshot image file size(%lu) exceeded limit %lu", mPath.mEndpointId,
                     static_cast<unsigned long>(image.data.size()), static_cast<unsigned long>(kMaxSnapshotImageSize));
        return Status::ResourceExhausted;
    }

    // Build and send the response
    Commands::CaptureSnapshotResponse::Type response;

    // Populate the response
    response.data       = ByteSpan(image.data.data(), image.data.size());
    response.resolution = image.imageRes;
    response.imageCodec = image.imageCodec;
    handler.AddResponse(commandPath, response);

    return std::nullopt;
}

bool CameraAVStreamManagementCluster::CheckSnapshotStreamsAvailability()
{
    if (mAllocatedSnapshotStreams.empty())
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: No snapshot streams are allocated", mPath.mEndpointId);
        return false;
    }
    return true;
}

bool CameraAVStreamManagementCluster::ValidateSnapshotStreamId(const DataModel::Nullable<uint16_t> & snapshotStreamID)
{
    auto found = std::find_if(mAllocatedSnapshotStreams.begin(), mAllocatedSnapshotStreams.end(),
                              [&](const SnapshotStreamStruct & s) { return s.snapshotStreamID == snapshotStreamID.Value(); });
    if (found == mAllocatedSnapshotStreams.end())
    {
        ChipLogError(Zcl, "CameraAVStreamMgmt[ep=%d]: No snapshot stream exist by id %d", mPath.mEndpointId,
                     snapshotStreamID.Value());
        return false;
    }
    return true;
}

Protocols::InteractionModel::Status
CameraAVStreamManagementCluster::ValidateVideoStreamForModifyOrDeallocate(const uint16_t videoStreamID, bool isDeallocate)
{
    return ValidateStreamForModifyOrDeallocateImpl(
        mAllocatedVideoStreams, videoStreamID, StreamType::kVideo, [](const VideoStreamStruct & s) { return s.videoStreamID; },
        isDeallocate);
}

Protocols::InteractionModel::Status CameraAVStreamManagementCluster::ValidateAudioStreamForDeallocate(const uint16_t audioStreamID)
{
    return ValidateStreamForModifyOrDeallocateImpl(
        mAllocatedAudioStreams, audioStreamID, StreamType::kAudio, [](const AudioStreamStruct & s) { return s.audioStreamID; },
        /* isDeallocate = */ true);
}

Protocols::InteractionModel::Status
CameraAVStreamManagementCluster::ValidateSnapshotStreamForModifyOrDeallocate(const uint16_t snapshotStreamID, bool isDeallocate)
{
    return ValidateStreamForModifyOrDeallocateImpl(
        mAllocatedSnapshotStreams, snapshotStreamID, StreamType::kSnapshot,
        [](const SnapshotStreamStruct & s) { return s.snapshotStreamID; }, isDeallocate);
}

bool CameraAVStreamManagementCluster::IsResourceAvailableForStreamAllocation(uint32_t candidateEncodedPixelRate,
                                                                             bool encoderRequired)
{
    uint64_t totalEncodedPixelRate = candidateEncodedPixelRate;
    uint16_t totalEncodersRequired = encoderRequired ? 1 : 0;

    for (const VideoStreamStruct & stream : mAllocatedVideoStreams)
    {
        totalEncodedPixelRate +=
            (static_cast<uint64_t>(stream.maxFrameRate) * stream.maxResolution.height * stream.maxResolution.width);
    }

    for (const SnapshotStreamStruct & stream : mAllocatedSnapshotStreams)
    {
        if (stream.encodedPixels)
        {
            totalEncodedPixelRate +=
                (static_cast<uint64_t>(stream.frameRate) * stream.maxResolution.height * stream.maxResolution.width);
            if (stream.hardwareEncoder)
            {
                totalEncodersRequired++;
            }
        }
    }

    // Check if the cumulative encoded pixel rate is within the bounds of the MaxEncodedPixelRate
    if (totalEncodedPixelRate > mMaxEncodedPixelRate)
    {
        return false;
    }

    // Check if the number of streams is within the bounds of
    // MaxConcurrentEncoders
    if ((mAllocatedVideoStreams.size() + totalEncodersRequired) > mMaxConcurrentEncoders)
    {
        return false;
    }

    return true;
}

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
