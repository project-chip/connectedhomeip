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

#include <clusters/push-av-stream-transport/push-av-stream-manager.h>
#include <ctime>
#include <filesystem>
#include <pushav-transport.h>
#include <time.h>

using namespace chip::app::Clusters::PushAvStreamTransport;

PushAVTransport::PushAVTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID,
                                 AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams) :
    mAudioStreamParams(audioStreamParams),
    mVideoStreamParams(videoStreamParams)
{
    mConnectionID                      = connectionID;
    mTransportStatus                   = TransportStatusEnum::kInactive;
    mBlindStartTime                    = std::chrono::steady_clock::time_point();
    mCurrentActivationByManualTrigger  = false;
    mPreviousActivationByManualTrigger = false;

    auto now               = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_r(&time_t_now, &tm_now);

    char datetime_str[32];
    std::strftime(datetime_str, sizeof(datetime_str), "%Y%m%d_%H%M%S", &tm_now);

    // Example folder name: FabricIdx1_ConnectionId2_20251103_225428
    std::string uniqueDirName =
        "FabricIdx" + std::to_string(mFabricIndex) + "_ConnectionId" + std::to_string(connectionID) + "_" + datetime_str;

    std::filesystem::path outputPath = std::filesystem::path("/tmp") / uniqueDirName;
    mClipInfo.mOutputPath            = outputPath.string();
}

const char * GetAudioCodecName(int codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_OPUS:
        return "OPUS";
    default:
        return "Unknown";
    }
}

const char * GetVideoCodecName(int codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_H264:
        return "H.264";
    default:
        return "Unknown";
    }
}

void PrintRecorderTimeSetting(const PushAVClipRecorder::ClipInfoStruct & clipInfo)
{
    ChipLogDetail(Camera, "=== PushAVTransport ConfigureRecorderTimeSetting ===");
    ChipLogDetail(Camera, "Initial Duration: %d sec", clipInfo.mInitialDurationS);
    ChipLogDetail(Camera, "Augmentation Duration: %d sec", clipInfo.mAugmentationDurationS);
    ChipLogDetail(Camera, "Max Clip Duration: %d sec", clipInfo.mMaxClipDurationS);
    ChipLogDetail(Camera, "Blind Duration: %d sec", clipInfo.mBlindDurationS);
}

void PrintTransportSettings(PushAVClipRecorder::ClipInfoStruct clipInfo, PushAVClipRecorder::AudioInfoStruct audioInfo,
                            PushAVClipRecorder::VideoInfoStruct videoInfo)
{
    ChipLogProgress(Camera, "=== Clip Configuration ===");
    ChipLogProgress(Camera, "Has Audio: %s", clipInfo.mHasAudio ? "true" : "false");
    ChipLogProgress(Camera, "Has Video: %s", clipInfo.mHasVideo ? "true" : "false");
    ChipLogProgress(Camera, "Chunk Duration: %d ms", clipInfo.mChunkDurationMs);
    ChipLogProgress(Camera, "Segment Duration: %d ms", clipInfo.mSegmentDurationMs);
    ChipLogProgress(Camera, "PreRoll Length: %d ms", clipInfo.mPreRollLengthMs);
    ChipLogProgress(Camera, "URL: %s", clipInfo.mUrl.c_str());
    ChipLogProgress(Camera, "Trigger Type: %d", clipInfo.mTriggerType);
    ChipLogProgress(Camera, "Output Path: %s", clipInfo.mOutputPath.c_str());
    ChipLogProgress(Camera, "Track Name: %s", clipInfo.mTrackName.c_str());

    ChipLogProgress(Camera, "=== Audio Configuration ===");
    ChipLogProgress(Camera, "Codec: %s", GetAudioCodecName(audioInfo.mAudioCodecId));
    ChipLogProgress(Camera, "Channels: %d", audioInfo.mChannels);
    ChipLogProgress(Camera, "Sample Rate: %d Hz", audioInfo.mSampleRate);
    ChipLogProgress(Camera, "Bit Rate: %d bps", audioInfo.mBitRate);
    ChipLogProgress(Camera, "Audio Time Base: %d/%d", audioInfo.mAudioTimeBase.num, audioInfo.mAudioTimeBase.den);
    ChipLogProgress(Camera, "Audio Stream Name: %s", audioInfo.mAudioStreamName.c_str());

    ChipLogProgress(Camera, "=== Video Configuration ===");
    ChipLogProgress(Camera, "Codec: %s", GetVideoCodecName(videoInfo.mVideoCodecId));
    ChipLogProgress(Camera, "Resolution: %dx%d", videoInfo.mWidth, videoInfo.mHeight);
    ChipLogProgress(Camera, "Frame Rate: %d fps", videoInfo.mFrameRate);
    ChipLogProgress(Camera, "Video Time Base: %d/%d", videoInfo.mVideoTimeBase.num, videoInfo.mVideoTimeBase.den);
    ChipLogProgress(Camera, "Bit Rate: %d bps", videoInfo.mBitRate);
    ChipLogProgress(Camera, "Video Stream Name: %s", videoInfo.mVideoStreamName.c_str());
}

void PushAVTransport::ConfigureRecorderTimeSetting(
    const chip::app::Clusters::PushAvStreamTransport::Structs::TransportMotionTriggerTimeControlStruct::DecodableType & timeControl)
{
    mClipInfo.mInitialDurationS      = std::min(static_cast<uint32_t>(timeControl.initialDuration), timeControl.maxDuration);
    mClipInfo.mAugmentationDurationS = timeControl.augmentationDuration;
    mClipInfo.mMaxClipDurationS      = timeControl.maxDuration;
    mClipInfo.mBlindDurationS        = timeControl.blindDuration;

    PrintRecorderTimeSetting(mClipInfo);
}

CHIP_ERROR PushAVTransport::ConfigureRecorderSettings(const TransportOptionsStruct & transportOptions,
                                                      AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams)
{
    uint8_t audioCodec = static_cast<uint8_t>(audioStreamParams.audioCodec);
    if (audioCodec == 2)
    {
        ChipLogError(Camera, "Unknown Audio codec");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (audioCodec != 0)
    {
        ChipLogError(Camera, "Unsupported Audio codec");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int8_t videoCodec = static_cast<uint8_t>(videoStreamParams.videoCodec);
    if (videoCodec == 4)
    {
        ChipLogError(Camera, "Unknown Video codec");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (videoCodec != 0)
    {
        ChipLogError(Camera, "Unsupported Video codec");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Codecs are valid, proceed with configuration
    mClipInfo.mHasAudio      = (transportOptions.audioStreams.HasValue() || transportOptions.audioStreamID.HasValue());
    mClipInfo.mHasVideo      = (transportOptions.videoStreams.HasValue() || transportOptions.videoStreamID.HasValue());
    mSessionStartedTimestamp = std::chrono::system_clock::time_point();

    mClipInfo.mUrl         = std::string(transportOptions.url.data(), transportOptions.url.size());
    mClipInfo.mTriggerType = static_cast<int>(transportOptions.triggerOptions.triggerType);
    if (transportOptions.triggerOptions.maxPreRollLen.HasValue())
    {
        mClipInfo.mPreRollLengthMs = transportOptions.triggerOptions.maxPreRollLen.Value();
    }
    else
    {
        mClipInfo.mPreRollLengthMs = 0; // Default pre roll length is zero
    }

    mClipInfo.mMotionDetectedDurationS         = 0;
    mClipInfo.mPreviousMotionDetectedDurationS = 0;
    mClipInfo.mElapsedTimeS                    = 0;

    if (transportOptions.triggerOptions.motionTimeControl.HasValue())
    {
        ConfigureRecorderTimeSetting(transportOptions.triggerOptions.motionTimeControl.Value());
    }
    if (transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        mClipInfo.mTrackName         = std::string(transportOptions.containerOptions.CMAFContainerOptions.Value().trackName.data(),
                                                   transportOptions.containerOptions.CMAFContainerOptions.Value().trackName.size());
        mClipInfo.mChunkDurationMs   = transportOptions.containerOptions.CMAFContainerOptions.Value().chunkDuration;
        mClipInfo.mSegmentDurationMs = transportOptions.containerOptions.CMAFContainerOptions.Value().segmentDuration;
    }

    mTransportTriggerType = transportOptions.triggerOptions.triggerType;

    // Configure audio settings
    if (audioStreamParams.channelCount == 0)
    {
        ChipLogError(Camera, "Invalid channel count: 0. Using fallback 1 channel.");
        audioStreamParams.channelCount = 1;
    }
    mAudioInfo.mChannels = audioStreamParams.channelCount;

    if (audioCodec == 0)
    {
        if (audioStreamParams.sampleRate == 0)
        {
            audioStreamParams.sampleRate = 48000; // Fallback value for invalid sample rate
            ChipLogError(Camera, "Invalid sample rate: 0. Using fallback 48000 Hz.");
        }
        mAudioInfo.mAudioCodecId  = AV_CODEC_ID_OPUS;
        mAudioInfo.mAudioTimeBase = { 1, static_cast<int>(audioStreamParams.sampleRate) };
    }

    mAudioInfo.mSampleRate = audioStreamParams.sampleRate;
    if (audioStreamParams.bitRate == 0)
    {
        ChipLogError(Camera, "Invalid audio bit rate: 0. Using fallback 96000 bps.");
        audioStreamParams.bitRate = 96000;
    }
    mAudioInfo.mBitRate = audioStreamParams.bitRate;
    if (mClipInfo.mHasAudio)
    {
        if (transportOptions.audioStreams.HasValue() && !transportOptions.audioStreams.Value().empty())
        {
            // Recoder module currently only supports one audio and one video stream
            auto audioStreamName        = transportOptions.audioStreams.Value().begin()->audioStreamName;
            mAudioInfo.mAudioStreamName = std::string(audioStreamName.data(), audioStreamName.size());
        }
        else
        {
            mAudioInfo.mAudioStreamName = "audio";
        }
    }

    // Configure video settings
    if (videoCodec == 0)
    {
        mVideoInfo.mVideoCodecId  = AV_CODEC_ID_H264;
        mVideoInfo.mVideoTimeBase = { 1, 90000 };
    }
    if (videoStreamParams.maxResolution.width == 0 || videoStreamParams.maxResolution.height == 0)
    {
        videoStreamParams.maxResolution.width  = 640;
        videoStreamParams.maxResolution.height = 320;
    }
    mVideoInfo.mWidth  = videoStreamParams.maxResolution.width;
    mVideoInfo.mHeight = videoStreamParams.maxResolution.height;
    if (videoStreamParams.minFrameRate == 0)
    {
        ChipLogError(Camera, "Invalid frame rate: 0. Using fallback 30 fps.");
        videoStreamParams.minFrameRate = 30;
    }
    mVideoInfo.mFrameRate = videoStreamParams.minFrameRate;
    mVideoInfo.mBitRate   = videoStreamParams.minBitRate;
    if (mClipInfo.mHasVideo)
    {
        if (transportOptions.videoStreams.HasValue() && !transportOptions.videoStreams.Value().empty())
        {
            // Recoder module currently only supports one audio and one video stream
            auto videoStreamName        = transportOptions.videoStreams.Value().begin()->videoStreamName;
            mVideoInfo.mVideoStreamName = std::string(videoStreamName.data(), videoStreamName.size());
        }
        else
        {
            mVideoInfo.mVideoStreamName = "video";
        }
    }

    PrintTransportSettings(mClipInfo, mAudioInfo, mVideoInfo);
    UpdateSendFlags();
    ChipLogProgress(Camera, "Transport[%u] Session[%lu] ConfigureRecorderSettings success - Track=%s HasVideo=%s HasAudio=%s",
                    mConnectionID, mSessionNumber, mClipInfo.mTrackName.c_str(), mClipInfo.mHasVideo ? "true" : "false",
                    mClipInfo.mHasAudio ? "true" : "false");

    return CHIP_NO_ERROR;
}

void PushAVTransport::InitializeRecorder()
{
    if (mRecorder.get() == nullptr)
    {
        mSessionStartedTimestamp = std::chrono::system_clock::time_point();
        mClipInfo.mSessionNumber = mSessionNumber;
        mRecorder                = std::make_unique<PushAVClipRecorder>(mClipInfo, mAudioInfo, mVideoInfo, mUploader.get());
        mRecorder->SetFabricIndex(mFabricIndex);
        mRecorder->SetPushAvStreamTransportServer(mPushAvStreamTransportServer);
        mRecorder->SetConnectionInfo(mConnectionID, mTransportTriggerType,
                                     chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum>());
        ChipLogProgress(Camera, "PushAVTransport, Initialize Recorder done !!! FabricIdx: %u Session Id: %ld", mFabricIndex,
                        mClipInfo.mSessionNumber);
    }
    else
    {
        ChipLogError(Camera, "Recorder already initialized");
    }
}

PushAVTransport::~PushAVTransport()
{
    mCanSendVideo = false;
    mCanSendAudio = false;

    mRecorder.reset();
    mUploader.reset();

    std::filesystem::path uniqueDirPath(mClipInfo.mOutputPath);

    if (uniqueDirPath.empty())
    {
        ChipLogDetail(Camera, "Output path is empty, skipping directory removal");
    }
    else if (std::filesystem::exists(uniqueDirPath) && std::filesystem::is_directory(uniqueDirPath))
    {
        std::error_code ec;
        std::filesystem::remove_all(uniqueDirPath, ec);
        if (ec)
        {
            ChipLogError(Camera, "Failed to remove directory %s: %s", uniqueDirPath.c_str(), ec.message().c_str());
        }
        else
        {
            ChipLogProgress(Camera, "Successfully removed directory: %s", uniqueDirPath.c_str());
        }
    }
    else
    {
        ChipLogDetail(Camera, "Directory does not exist: %s", uniqueDirPath.c_str());
    }
}

bool InBlindPeriod(std::chrono::steady_clock::time_point blindStartTime, uint16_t blindDuration,
                   std::chrono::steady_clock::time_point now)
{
    if (blindDuration == 0 || blindStartTime == std::chrono::steady_clock::time_point())
    {
        ChipLogProgress(Camera, "PushAVTransport: No active blind period");
        return false;
    }

    const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - blindStartTime).count();
    ChipLogProgress(Camera, "PushAVTransport blind period, blindDuration: %u seconds", blindDuration);
    return ((elapsedSeconds >= 0) && (elapsedSeconds < blindDuration));
}

bool PushAVTransport::HandleTriggerDetected()
{
    auto now = std::chrono::steady_clock::now();

    // Blind duration is considered in case of motion event only
    if (!mPreviousActivationByManualTrigger && !mCurrentActivationByManualTrigger &&
        InBlindPeriod(mBlindStartTime, mClipInfo.mBlindDurationS, now))
    {
        ChipLogError(Camera,
                     "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration "
                     "[%d seconds]",
                     mRecorder->mClipInfo.mMotionDetectedDurationS);
        return false;
    }

    int64_t elapsedClipDurationS = std::chrono::duration_cast<std::chrono::seconds>(now - mClipInfo.mActivationTime).count();
    if (mClipInfo.mActivationTime == std::chrono::steady_clock::time_point() ||
        elapsedClipDurationS >= mClipInfo.mMotionDetectedDurationS)
    {
        // Start new recording
        ChipLogDetail(Camera, "PushAVTransport starting new recording");
        mHasAugmented             = false;
        mClipInfo.mActivationTime = now;

        mPreviousActivationByManualTrigger         = mCurrentActivationByManualTrigger;
        mClipInfo.mMotionDetectedDurationS         = mClipInfo.mInitialDurationS;
        mClipInfo.mPreviousMotionDetectedDurationS = 0;
        mClipInfo.mElapsedTimeS                    = 0;
    }
    else
    {
        // Handle augmentation for ongoing recording
        ChipLogDetail(Camera, "PushAVTransport HandleTriggerDetected time since last trigger: %" PRId64, elapsedClipDurationS);

        if (!mHasAugmented || (mHasAugmented && (elapsedClipDurationS >= mClipInfo.mPreviousMotionDetectedDurationS)))
        {
            uint16_t newDuration = static_cast<uint16_t>(
                std::min(static_cast<uint32_t>(mClipInfo.mMotionDetectedDurationS + mClipInfo.mAugmentationDurationS),
                         static_cast<uint32_t>(mClipInfo.mMaxClipDurationS)));

            ChipLogDetail(Camera, "PushAVTransport extending recording %d -> %d", mClipInfo.mMotionDetectedDurationS, newDuration);

            // Update tracking variables
            mClipInfo.mPreviousMotionDetectedDurationS = mClipInfo.mMotionDetectedDurationS;
            mClipInfo.mMotionDetectedDurationS         = newDuration;
            mHasAugmented                              = true;
        }
    }

    // Calculate blind start time based on when the current recording will actually end
    // Use the current motion detected duration which represents when this recording session will end
    mBlindStartTime = mClipInfo.mActivationTime + std::chrono::seconds(mClipInfo.mMotionDetectedDurationS);

    if (mRecorder.get() != nullptr)
    {
        mRecorder->mClipInfo.mMotionDetectedDurationS = mClipInfo.mMotionDetectedDurationS;
        if (!mRecorder->GetRecorderStatus())
        {
            mRecorder->SetConnectionInfo(mConnectionID, mTransportTriggerType, mActivationReason);
            // Initiate recording if the recorder is not currently recording
            StartRecordingAndStreaming();
        }
    }
    return true;
}

void PushAVTransport::StartRecordingAndStreaming()
{
    // Notify manager to reset transport sink state before starting recording
    if (mManager != nullptr)
    {
        mManager->ResetTransportSinkStateForTransport(this);
    }

    mRecorder->mClipInfo.mSessionNumber = mClipInfo.mSessionNumber;
    mRecorder->Start();
    mStreaming = true;
    UpdateSendFlags();
    if (IsStreaming() && (mTransportTriggerType != TransportTriggerTypeEnum::kCommand))
    {
        ChipLogDetail(Camera, "Ready to stream");
        GeneratePushTransportBeginEvent();
    }
}

void PushAVTransport::GeneratePushTransportBeginEvent()
{
    if (mPushAvStreamTransportServer != nullptr)
    {
        // mActivationReason is optional - if not set, it defaults to empty value
        mPushAvStreamTransportServer->NotifyTransportStarted(mConnectionID, mTransportTriggerType, mActivationReason);
    }
    else
    {
        ChipLogError(Camera, "PushAvStreamTransportServer is null or activation reason not set for connection %u", mConnectionID);
    }
}

bool PushAVTransport::ValidateZoneAndSensitivity(
    const std::vector<std::pair<chip::app::DataModel::Nullable<uint16_t>, uint8_t>> & zoneSensitivityList, int zoneId,
    int sensitivity)
{
    // Validate input parameters

    if (sensitivity < 0 || sensitivity > 10)
    {
        ChipLogError(Camera, "PushAVTransport invalid sensitivity value: %d (must be 0-10)", sensitivity);
        return false;
    }

    if (zoneSensitivityList.empty())
    {
        ChipLogProgress(Camera, "PushAVTransport zoneSensitivityList NOT set, accepting all zones");
        return true;
    }

    // Check for specific zone match
    for (const auto & zone : zoneSensitivityList)
    {
        if (zone.first.IsNull() || (zone.first.Value() == static_cast<uint16_t>(zoneId)))
        {
            if (zone.second <= static_cast<uint8_t>(sensitivity))
            {
                ChipLogDetail(Camera, "PushAVTransport zone %d accepted (sensitivity %d >= threshold %u)", zoneId, sensitivity,
                              zone.second);
                return true;
            }
            else
            {
                ChipLogProgress(Camera,
                                "PushAVTransport motion transport trigger ignored - zone %d sensitivity %d exceeds threshold %u",
                                zoneId, sensitivity, zone.second);
                return false;
            }
        }
    }

    ChipLogProgress(Camera, "PushAVTransport motion transport trigger ignored - zone %d not found in configuration", zoneId);
    return false;
}

void PushAVTransport::TriggerTransport(TriggerActivationReasonEnum activationReason, int zoneId, int sensitivity)
{
    ChipLogProgress(Camera, "PushAVTransport trigger transport, activation reason: [%u], ZoneId: [%d], Sensitivity: [%d]",
                    (uint16_t) activationReason, zoneId, sensitivity);

    mCurrentActivationByManualTrigger = (zoneId == kInvalidZoneId) ? true : false;
    mActivationReason                 = chip::MakeOptional(activationReason);

    // Check if trigger should be processed based on transport type
    bool shouldProcessTrigger = false;

    if (mTransportTriggerType == TransportTriggerTypeEnum::kCommand)
    {
        shouldProcessTrigger = true;
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
    {
        shouldProcessTrigger =
            mCurrentActivationByManualTrigger || ValidateZoneAndSensitivity(mZoneSensitivityList, zoneId, sensitivity);
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
    {
        ChipLogProgress(Camera, "PushAVTransport continuous transport trigger received. No action needed");
        return;
    }
    // Process the trigger if conditions are met
    if (shouldProcessTrigger)
    {
        if (HandleTriggerDetected())
        {
            // Event generation is handled differently based on trigger type:
            // - Command: Begin event already generated at cluster server
            // - Motion: GeneratePushTransportBeginEvent() would be called inside `HandleTriggerDetected` API if needed
        }
        else
        {
            ChipLogError(Camera, "PushAVTransport trigger detection ignored or failed for connection %u", mConnectionID);
        }
    }
}

void PushAVTransport::SetTLSCertPath(std::string rootCert, std::string devCert, std::string devKey)
{
    mCertPath.mRootCert = rootCert;
    mCertPath.mDevCert  = devCert;
    mCertPath.mDevKey   = devKey;
}

void PushAVTransport::SetTLSCert(std::vector<uint8_t> bufferRootCert, std::vector<uint8_t> bufferClientCert,
                                 std::vector<uint8_t> bufferClientCertKey,
                                 std::vector<std::vector<uint8_t>> bufferIntermediateCerts)
{
    mCertBuffer.mRootCertBuffer         = bufferRootCert;
    mCertBuffer.mClientCertBuffer       = bufferClientCert;
    mCertBuffer.mClientKeyBuffer        = bufferClientCertKey;
    mCertBuffer.mIntermediateCertBuffer = bufferIntermediateCerts;
}

void PushAVTransport::SetTransportStatus(TransportStatusEnum status)
{
    if (mTransportStatus == status)
    {
        ChipLogProgress(Camera, "PushAVTransport transport status unchanged");
        return;
    }

    mTransportStatus = status;
    if (status == TransportStatusEnum::kActive)
    {
        ChipLogProgress(Camera, "PushAVTransport transport status changed to active");

        if (mUploader.get() == nullptr)
        {
            mUploader = std::make_unique<PushAVUploader>();
            mUploader->setCertificateBuffer(mCertBuffer);
            mUploader->setCertificatePath(mCertPath);
            mUploader->Start();
        }
        InitializeRecorder();

        if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
        {
            mClipInfo.mMotionDetectedDurationS = 0;
            mClipInfo.mElapsedTimeS            = 0;
            StartRecordingAndStreaming();
        }
        else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
        {
            // Check if mActivationTime is set (non-default)
            if (mClipInfo.mActivationTime == std::chrono::steady_clock::time_point())
            {
                ChipLogProgress(Camera, "No active trigger to start recording");
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto elapsedSeconds =
                    std::chrono::duration_cast<std::chrono::seconds>(now - mRecorder->mClipInfo.mActivationTime).count();

                // Check if recording duration has expired
                if (elapsedSeconds >= mRecorder->mClipInfo.mMotionDetectedDurationS)
                {
                    ChipLogProgress(Camera, "No active trigger (time expired) to start recording");
                }
                else
                {
                    mRecorder->mClipInfo.mElapsedTimeS = static_cast<uint16_t>(elapsedSeconds);
                    ChipLogProgress(Camera, "Active trigger is present. Recording will start for [%d seconds]",
                                    mRecorder->mClipInfo.mMotionDetectedDurationS);
                    StartRecordingAndStreaming();
                }
            }
        }
    }
    else if (status == TransportStatusEnum::kInactive)
    {
        ChipLogProgress(Camera, "PushAVTransport transport status change requested to inactive");
        mStreaming = false; // Stop streaming
        UpdateSendFlags();
        mRecorder.reset();
        ChipLogProgress(Camera, "Recorder destruction done");
        // Clear activationTime for manual triggers when setting status to inactive
        if (mCurrentActivationByManualTrigger)
        {
            mClipInfo.mActivationTime = std::chrono::steady_clock::time_point();
            ChipLogDetail(Camera, "PushAVTransport, cleared mActivationTime for manual trigger");
        }
        mUploader.reset();
        ChipLogProgress(Camera, "Uploader destruction done");
        ChipLogProgress(Camera, "PushAVTransport transport status changed to inactive");
    }
}

void PushAVTransport::UpdateSendFlags()
{
    bool canSend  = mStreaming && (mTransportStatus == TransportStatusEnum::kActive);
    mCanSendVideo = canSend && mClipInfo.mHasVideo;
    mCanSendAudio = canSend && mClipInfo.mHasAudio;
}

bool PushAVTransport::IsStreaming() const
{
    return mStreaming && (mTransportStatus == TransportStatusEnum::kActive);
}

bool PushAVTransport::CanSendPacketsToRecorder()
{
    if (!IsStreaming())
    {
        return false;
    }

    CheckAndUpdateSession();

    if (mRecorder->mDeinitializeRecorder.load())
    {
        ChipLogProgress(Camera, "Current clip is completed, Next clip will start on trigger");
        mRecorder.reset(); // Redundant cleanup to make sure no dangling pointer left
        InitializeRecorder();
        mStreaming = false;
        UpdateSendFlags();
        return false;
    }
    return true;
}

void PushAVTransport::SendVideo(const chip::ByteSpan & data, int64_t timestampMs, uint16_t videoStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data.data(), data.size(), timestampMs, 1);
    }
}

void PushAVTransport::SendAudio(const chip::ByteSpan & data, int64_t timestampMs, uint16_t audioStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data.data(), data.size(), timestampMs, 0);
    }
}

void PushAVTransport::SendAudioVideo(const chip::ByteSpan & data, uint16_t videoStreamID, uint16_t audioStreamID) {}

// Utility API for Test purpose
void PushAVTransport::readFromFile(char * filename, uint8_t ** videoBuffer, size_t * videoBufferBytes)
{
    const char * in_f_name = filename;
    FILE * infile;
    size_t result;
    /* open an existing file for reading */
    infile = fopen(in_f_name, "r");
    /* quit if the file does not exist */
    if (infile == nullptr)
    {
        return;
    }
    /* Get the number of bytes */
    fseek(infile, 0L, SEEK_END);
    *videoBufferBytes = ftell(infile);

    /* reset the file position indicator to the beginning of the file */
    fseek(infile, 0L, SEEK_SET);
    /* grab sufficient memory for the fileBuffer to hold the text */
    *videoBuffer = (uint8_t *) calloc(*videoBufferBytes, sizeof(uint8_t));
    /* memory error */
    if (*videoBuffer == nullptr)
    {
        fclose(infile);
        return;
    }

    /* copy all the text into the fileBuffer */
    result = fread(*videoBuffer, sizeof(uint8_t), *videoBufferBytes, infile);
    fclose(infile);
    if ((size_t) result != *videoBufferBytes)
    {
        return;
    }
}

// Implementation of CanSendVideo method
bool PushAVTransport::CanSendVideo()
{
    return IsStreaming() && mClipInfo.mHasVideo;
}

// Dummy implementation of CanfSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return IsStreaming() && mClipInfo.mHasAudio;
}

CHIP_ERROR PushAVTransport::ModifyPushTransport(const TransportOptionsStorage & transportOptions)
{
    CHIP_ERROR err = ConfigureRecorderSettings(transportOptions, mAudioStreamParams, mVideoStreamParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to modify push transport settings for connection %u: %s", mConnectionID, chip::ErrorStr(err));
        return err;
    }

    if (mRecorder)
    {
        mStreaming = false;
        UpdateSendFlags();
        mRecorder.reset();
        InitializeRecorder();
    }
    return CHIP_NO_ERROR;
}

bool PushAVTransport::GetBusyStatus()
{
    return (mUploader.get() != nullptr && mUploader->GetUploadQueueSize() > 0);
}

uint16_t PushAVTransport::GetPreRollLength()
{
    return mClipInfo.mPreRollLengthMs;
}

void PushAVTransport::CheckAndUpdateSession()
{
    auto now = std::chrono::system_clock::now();

    if (mSessionStartedTimestamp == std::chrono::system_clock::time_point())
    {
        mSessionStartedTimestamp = now;
        mSessionNumber++;
        ChipLogProgress(Camera, "Transport[%u] Session[%lu] SESSION_STARTED: First session initialized for Track=%s", mConnectionID,
                        mSessionNumber, mClipInfo.mTrackName.c_str());
        return;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - mSessionStartedTimestamp).count();
    if (elapsed >= kMaxSessionDurationMinutes)
    {
        mSessionStartedTimestamp = now;

        ChipLogProgress(Camera,
                        "Transport[%u] Session[%lu] SESSION_INCREMENTED: Session duration limit reached (%d min). New session "
                        "started. Track=%s",
                        mConnectionID, mSessionNumber, kMaxSessionDurationMinutes, mClipInfo.mTrackName.c_str());
        mStreaming = false;
        UpdateSendFlags();
        mRecorder.reset();

        InitializeRecorder();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                                               mRecorder->mClipInfo.mActivationTime)
                                  .count();
        mRecorder->mClipInfo.mElapsedTimeS = static_cast<uint16_t>(elapsedSeconds);

        mRecorder->Start();
        mStreaming = true;
        UpdateSendFlags();
    }
}
