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

#include <ctime>
#include <filesystem>
#include <push-av-stream-manager.h>
#include <pushav-transport.h>
#include <time.h>

using namespace chip::app::Clusters::PushAvStreamTransport;

PushAVTransport::PushAVTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID,
                                 AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams) :
    mAudioStreamParams(audioStreamParams), mVideoStreamParams(videoStreamParams)
{
    ConfigureRecorderSettings(transportOptions, audioStreamParams, videoStreamParams);
    mConnectionID    = connectionID;
    mTransportStatus = TransportStatusEnum::kInactive;

    auto now               = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_r(&time_t_now, &tm_now);

    char datetime_str[32];
    std::strftime(datetime_str, sizeof(datetime_str), "%Y%m%d_%H%M%S", &tm_now);

    // Example folder name: FabricIdx1_ConnectionId2_20251103_225428
    std::string uniqueDirName =
        "FabricIdx" + std::to_string(mFabricIndex) + "_ConnectionId" + std::to_string(connectionID) + "_" + datetime_str;

    std::filesystem::path outputPath = std::filesystem::path("/tmp") / uniqueDirName / "";
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

void PrintTransportSettings(PushAVClipRecorder::ClipInfoStruct clipInfo, PushAVClipRecorder::AudioInfoStruct audioInfo,
                            PushAVClipRecorder::VideoInfoStruct videoInfo)
{
    ChipLogProgress(Camera, "=== Clip Configuration ===");
    ChipLogProgress(Camera, "Has Audio: %s", clipInfo.mHasAudio ? "true" : "false");
    ChipLogProgress(Camera, "Has Video: %s", clipInfo.mHasVideo ? "true" : "false");
    ChipLogProgress(Camera, "Session Group: %d", clipInfo.mSessionGroup);
    ChipLogProgress(Camera, "Chunk Duration: %d ms", clipInfo.mChunkDurationMs);
    ChipLogProgress(Camera, "Segment Duration: %d ms", clipInfo.mSegmentDurationMs);
    ChipLogProgress(Camera, "PreRoll Length: %d ms", clipInfo.mPreRollLengthMs);
    ChipLogProgress(Camera, "URL: %s", clipInfo.mUrl.c_str());
    ChipLogProgress(Camera, "Trigger Type: %d", clipInfo.mTriggerType);
    ChipLogProgress(Camera, "Output Path: %s", clipInfo.mOutputPath.c_str());
    ChipLogProgress(Camera, "Track Name: %s", clipInfo.mTrackName.c_str());
    // Time control parameters are only passed during transport allocation for motion triggers.
    if (clipInfo.mTriggerType == 1)
    {
        ChipLogProgress(Camera, "Initial Duration: %d sec", clipInfo.mInitialDurationS);
        ChipLogProgress(Camera, "Augmentation Duration: %d sec", clipInfo.mAugmentationDurationS);
        ChipLogProgress(Camera, "Max Clip Duration: %d sec", clipInfo.mMaxClipDurationS);
        ChipLogProgress(Camera, "Blind Duration: %d sec", clipInfo.mBlindDurationS);
    }

    ChipLogProgress(Camera, "=== Audio Configuration ===");
    ChipLogProgress(Camera, "Codec: %s", GetAudioCodecName(audioInfo.mAudioCodecId));
    ChipLogProgress(Camera, "Channels: %d", audioInfo.mChannels);
    ChipLogProgress(Camera, "Sample Rate: %d Hz", audioInfo.mSampleRate);
    ChipLogProgress(Camera, "Bit Rate: %d bps", audioInfo.mBitRate);
    ChipLogProgress(Camera, "Audio Time Base: %d/%d", audioInfo.mAudioTimeBase.num, audioInfo.mAudioTimeBase.den);
    ChipLogProgress(Camera, "Frame Duration: %d samples", audioInfo.mAudioFrameDuration);

    ChipLogProgress(Camera, "=== Video Configuration ===");
    ChipLogProgress(Camera, "Codec: %s", GetVideoCodecName(videoInfo.mVideoCodecId));
    ChipLogProgress(Camera, "Resolution: %dx%d", videoInfo.mWidth, videoInfo.mHeight);
    ChipLogProgress(Camera, "Frame Rate: %d fps", videoInfo.mFrameRate);
    ChipLogProgress(Camera, "Video Time Base: %d/%d", videoInfo.mVideoTimeBase.num, videoInfo.mVideoTimeBase.den);
    ChipLogProgress(Camera, "Frame Duration: %d ticks", videoInfo.mVideoFrameDuration);
    ChipLogProgress(Camera, "Bit Rate: %d bps", videoInfo.mBitRate);
}

void PushAVTransport::ConfigureRecorderTimeSetting(
    const chip::app::Clusters::PushAvStreamTransport::Structs::TransportMotionTriggerTimeControlStruct::DecodableType & timeControl)
{
    mClipInfo.mInitialDurationS      = timeControl.initialDuration;
    mClipInfo.mAugmentationDurationS = timeControl.augmentationDuration;
    mClipInfo.mMaxClipDurationS      = timeControl.maxDuration;
    mClipInfo.mBlindDurationS        = timeControl.blindDuration;
    mClipInfo.mElapsedTimeS          = 0;
    ChipLogDetail(Camera, "=== PushAVTransport ConfigureRecorderTimeSetting ===");
    ChipLogDetail(Camera, "Initial Duration: %d sec", mClipInfo.mInitialDurationS);
    ChipLogDetail(Camera, "Augmentation Duration: %d sec", mClipInfo.mAugmentationDurationS);
    ChipLogDetail(Camera, "Max Clip Duration: %d sec", mClipInfo.mMaxClipDurationS);
    ChipLogDetail(Camera, "Blind Duration: %d sec", mClipInfo.mBlindDurationS);
    if (mRecorder.get() != nullptr)
    {
        mRecorder->mClipInfo = mClipInfo;
    }
}

void PushAVTransport::ConfigureRecorderSettings(const TransportOptionsStruct & transportOptions,
                                                AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams)
{
    mClipInfo.mHasAudio = transportOptions.audioStreamID.HasValue();
    mClipInfo.mHasVideo = transportOptions.videoStreamID.HasValue();
    if (mClipInfo.mHasAudio && mClipInfo.mHasVideo)
    {
        mClipInfo.mHasAudio = false;
    }
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
    if (transportOptions.triggerOptions.motionTimeControl.HasValue())
    {
        ConfigureRecorderTimeSetting(transportOptions.triggerOptions.motionTimeControl.Value());
    }
    if (transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        mClipInfo.mSessionGroup      = transportOptions.containerOptions.CMAFContainerOptions.Value().sessionGroup;
        mClipInfo.mTrackName         = std::string(transportOptions.containerOptions.CMAFContainerOptions.Value().trackName.data(),
                                                   transportOptions.containerOptions.CMAFContainerOptions.Value().trackName.size());
        mClipInfo.mChunkDurationMs   = transportOptions.containerOptions.CMAFContainerOptions.Value().chunkDuration;
        mClipInfo.mSegmentDurationMs = transportOptions.containerOptions.CMAFContainerOptions.Value().segmentDuration;
    }

    mTransportTriggerType = transportOptions.triggerOptions.triggerType;

    uint8_t audioCodec = static_cast<uint8_t>(audioStreamParams.audioCodec);
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
            ChipLogError(Camera, "Invalid sample rate: 0. Using fallback 48000 Hz.");
            audioStreamParams.sampleRate = 48000; // Fallback value for invalid sample rate
        }
        mAudioInfo.mAudioCodecId       = AV_CODEC_ID_OPUS;
        mAudioInfo.mAudioTimeBase      = { 1, static_cast<int>(audioStreamParams.sampleRate) };
        mAudioInfo.mAudioFrameDuration = 20000; // Default OPUS frame duration
    }
    else if (audioCodec == 2)
    {
        ChipLogError(Camera, "Unknown Audio codec")
    }
    else
    {
        ChipLogError(Camera, "Unsupported Audio codec");
    }

    mAudioInfo.mSampleRate = audioStreamParams.sampleRate;
    if (audioStreamParams.bitRate == 0)
    {
        ChipLogError(Camera, "Invalid audio bit rate: 0. Using fallback 96000 bps.");
        audioStreamParams.bitRate = 96000;
    }
    mAudioInfo.mBitRate          = audioStreamParams.bitRate;
    mAudioInfo.mAudioPts         = 0;
    mAudioInfo.mAudioDts         = 0;
    mAudioInfo.mAudioStreamIndex = -1;

    int8_t VideoCodec = static_cast<uint8_t>(videoStreamParams.videoCodec);
    if (VideoCodec == 0)
    {
        mVideoInfo.mVideoCodecId  = AV_CODEC_ID_H264;
        mVideoInfo.mVideoTimeBase = { 1, 90000 };
    }
    else if (VideoCodec == 4)
    {
        ChipLogError(Camera, "Unknown Video codec")
    }
    else
    {
        ChipLogError(Camera, "Unsupported Video codec");
    }
    mVideoInfo.mVideoPts = 0;
    mVideoInfo.mVideoDts = 0;
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

    mVideoInfo.mVideoFrameDuration = mVideoInfo.mVideoTimeBase.den / (mVideoInfo.mFrameRate * mVideoInfo.mVideoTimeBase.num);
    mVideoInfo.mVideoStreamIndex   = -1;
    mVideoInfo.mBitRate            = videoStreamParams.minBitRate;

    PrintTransportSettings(mClipInfo, mAudioInfo, mVideoInfo);
    ChipLogProgress(Camera, "PushAvStreamTransportManager, Configure Recorder Settings done !!!");
}

void PushAVTransport::InitializeRecorder()
{
    if (mRecorder.get() == nullptr)
    {
        mRecorder = std::make_unique<PushAVClipRecorder>(mClipInfo, mAudioInfo, mVideoInfo, mUploader.get());
        mRecorder->SetFabricIndex(mFabricIndex);
        mRecorder->SetPushAvStreamTransportServer(mPushAvStreamTransportServer);
        mRecorder->SetPushAvStreamTransportManager(mPushAvStreamTransportManager);
        mRecorder->SetConnectionInfo(mConnectionID, mTransportTriggerType,
                                     chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum>());
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

    if (std::filesystem::exists(uniqueDirPath) && std::filesystem::is_directory(uniqueDirPath))
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

bool InBlindPeriod(std::chrono::steady_clock::time_point blindStartTime, uint16_t blindDuration)
{
    if (blindStartTime == std::chrono::steady_clock::time_point())
    {
        ChipLogProgress(Camera, "PushAVTransport no blind period");
        return false;
    }
    else
    {
        auto now     = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - blindStartTime).count();
        ChipLogProgress(Camera, "PushAVTransport blind period elapsed: %lld", static_cast<long long int>(elapsed));
        return ((elapsed >= 0) && (elapsed < blindDuration));
    }
}

bool PushAVTransport::HandleTriggerDetected()
{
    int64_t elapsed = 0;
    auto now        = std::chrono::steady_clock::now();

    if (mTransportTriggerType != TransportTriggerTypeEnum::kCommand && InBlindPeriod(mBlindStartTime, mClipInfo.mBlindDurationS))
    {
        return false;
    }

    if (mClipInfo.activationTime != std::chrono::steady_clock::time_point())
    {
        elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - mClipInfo.activationTime).count();
    }

    ChipLogDetail(Camera, "PushAVTransport HandleTriggerDetected elapsed: %ld", elapsed);

    if (!mRecorder->GetRecorderStatus())
    {
        // Start new recording
        ChipLogError(Camera, "PushAVTransport starting new recording");
        mHasAugmented                       = false;
        mClipInfo.activationTime            = std::chrono::steady_clock::now();
        mRecorder->mClipInfo.activationTime = mClipInfo.activationTime;
        mRecorder->mClipInfo.mSessionNumber =
            mPushAvStreamTransportManager->OnTriggerActivated(mFabricIndex, mClipInfo.mSessionGroup, mConnectionID);

        mRecorder->Start();
        mStreaming = true;
    }
    else
    {
        // Extend existing recording
        uint16_t previousDuration = mRecorder->mClipInfo.mInitialDurationS - mRecorder->mClipInfo.mAugmentationDurationS;

        if ((elapsed < mRecorder->mClipInfo.mInitialDurationS) && (!mHasAugmented || elapsed >= previousDuration))
        {
            ChipLogError(Camera, "PushAVTransport extending recording %d -> %d", mRecorder->mClipInfo.mInitialDurationS,
                         static_cast<uint16_t>(std::min(static_cast<uint32_t>(mRecorder->mClipInfo.mInitialDurationS +
                                                                              mRecorder->mClipInfo.mAugmentationDurationS),
                                                        static_cast<uint32_t>(mRecorder->mClipInfo.mMaxClipDurationS))));
            mRecorder->mClipInfo.mInitialDurationS = static_cast<uint16_t>(std::min(
                static_cast<uint32_t>(mRecorder->mClipInfo.mInitialDurationS + mRecorder->mClipInfo.mAugmentationDurationS),
                static_cast<uint32_t>(mRecorder->mClipInfo.mMaxClipDurationS)));
            mHasAugmented                          = true;
            mStreaming                             = true;
        }
    }
    mBlindStartTime = mRecorder->mClipInfo.activationTime + std::chrono::seconds(mRecorder->mClipInfo.mInitialDurationS);
    return true;
}

void PushAVTransport::TriggerTransport(TriggerActivationReasonEnum activationReason, int zoneId, int sensitivity)
{
    ChipLogProgress(Camera, "PushAVTransport trigger transport, activation reason: [%u], ZoneId: [%d], Sensitivity: [%d]",
                    (uint16_t) activationReason, zoneId, sensitivity);
    if (mTransportTriggerType == TransportTriggerTypeEnum::kCommand)
    {
        mRecorder->SetConnectionInfo(mConnectionID, mTransportTriggerType, chip::MakeOptional(activationReason));
        if (HandleTriggerDetected())
        {
            ChipLogError(Camera, "PushAVTransport command/motion transport trigger received. Clip duration [%d seconds]",
                         mRecorder->mClipInfo.mInitialDurationS);
            // Begin event already generated at cluster server
        }
        else
        {
            ChipLogError(Camera,
                         "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration "
                         "[%d seconds]",
                         mRecorder->mClipInfo.mInitialDurationS);
        }
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
    {
        bool zoneFound = false; // Zone found flag
        for (auto zone : mZoneSensitivityList)
        {
            // A Null ZoneId means all zones
            if (zone.first.IsNull())
            {
                zoneFound = true;
            }
            else
            {
                zoneFound = (zone.first.Value() = zoneId);
            }

            if (zoneFound)
            {
                if (zone.second > sensitivity)
                {
                    ChipLogProgress(Camera, "PushAVTransport motion transport trigger received but ignored due to sensitivity");
                }
                else
                {
                    if (HandleTriggerDetected())
                    {
                        ChipLogError(Camera,
                                     "PushAVTransport command/motion transport trigger received. Clip duration [%d seconds]",
                                     mRecorder->mClipInfo.mInitialDurationS);
                        if (mPushAvStreamTransportServer != nullptr)
                        {
                            mPushAvStreamTransportServer->NotifyTransportStarted(
                                mConnectionID, mTransportTriggerType,
                                chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum>());
                        }
                        else
                        {
                            ChipLogError(Camera, "PushAvStreamTransportServer is null for connection %u", mConnectionID);
                        }
                    }
                    else
                    {
                        ChipLogError(Camera,
                                     "PushAVTransport command/motion transport trigger received but ignored due to blind period. "
                                     "Clip duration. "
                                     "Clip duration [%d seconds]",
                                     mRecorder->mClipInfo.mInitialDurationS);
                    }
                }
                break;
            }
        }
        if (!zoneFound)
        {
            ChipLogProgress(Camera, "PushAVTransport motion transport trigger received but ignored due to unknown zone id");
        }
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
    {
        ChipLogProgress(Camera, "PushAVTransport continuous transport trigger received. No action needed");
        return;
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

        mUploader = std::make_unique<PushAVUploader>();
        mUploader->setCertificateBuffer(mCertBuffer);
        mUploader->setCertificatePath(mCertPath);
        mUploader->Start();
        InitializeRecorder();

        mRecorder->mClipInfo.mElapsedTimeS = 0;
        if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
        {
            mRecorder->mClipInfo.mSessionNumber =
                mPushAvStreamTransportManager->OnTriggerActivated(mFabricIndex, mClipInfo.mSessionGroup, mConnectionID);
            mRecorder->Start();
            mStreaming = true;
            if (IsStreaming())
            {
                ChipLogProgress(Camera, "Ready to stream");
                if (mPushAvStreamTransportServer != nullptr)
                {
                    mPushAvStreamTransportServer->NotifyTransportStarted(
                        mConnectionID, mTransportTriggerType,
                        chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum>());
                }
                else
                {
                    ChipLogError(Camera, "PushAvStreamTransportServer is null for connection %u", mConnectionID);
                }
            }
        }
        else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
        {
            // Check if activationTime is set (non-default)
            if (mClipInfo.activationTime == std::chrono::steady_clock::time_point())
            {
                ChipLogProgress(Camera, "No active trigger to start recording");
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto elapsedSeconds =
                    std::chrono::duration_cast<std::chrono::seconds>(now - mRecorder->mClipInfo.activationTime).count();

                // Check if recording duration has expired
                if (elapsedSeconds >= mRecorder->mClipInfo.mInitialDurationS)
                {
                    ChipLogProgress(Camera, "No active trigger (time expired) to start recording");
                }
                else
                {
                    // Calculate remaining duration safely
                    mRecorder->mClipInfo.mElapsedTimeS = static_cast<uint16_t>(elapsedSeconds);
                    ChipLogProgress(Camera, "Active trigger is present. Recording will start for [%d seconds]",
                                    mRecorder->mClipInfo.mInitialDurationS);
                }
            }
        }
    }
    else if (status == TransportStatusEnum::kInactive)
    {
        ChipLogProgress(Camera, "PushAVTransport transport status change requested to inactive");
        mStreaming    = false; // Stop streaming
        mCanSendVideo = false;
        mCanSendAudio = false;
        mRecorder.reset();
        ChipLogProgress(Camera, "Recorder destruction done");
        InitializeRecorder();
        mUploader.reset();
        ChipLogProgress(Camera, "Uploader destruction done");
        ChipLogProgress(Camera, "PushAVTransport transport status changed to inactive");
    }
}

bool PushAVTransport::IsStreaming()
{
    if (mStreaming && (mTransportStatus == TransportStatusEnum::kActive))
    {
        mCanSendVideo = true;
        mCanSendAudio = true;
        return true;
    }
    else
    {
        mCanSendVideo = false;
        mCanSendAudio = false;
        return false;
    }
}

bool PushAVTransport::CanSendPacketsToRecorder()
{
    if (!IsStreaming())
    {
        return false;
    }
    if (mRecorder->mDeinitializeRecorder.load())
    {
        ChipLogProgress(Camera, "Current clip is completed, Next clip will start on trigger");
        mRecorder.reset(); // Redundant cleanup to make sure no dangling pointer left
        InitializeRecorder();
        mStreaming = false;
        return false;
    }
    return true;
}

void PushAVTransport::SendVideo(const chip::ByteSpan & data, int64_t timestamp, uint16_t videoStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data.data(), data.size(), 1);
    }
}

void PushAVTransport::SendAudio(const chip::ByteSpan & data, int64_t timestamp, uint16_t audioStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data.data(), data.size(), 0);
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

void PushAVTransport::ModifyPushTransport(const TransportOptionsStorage & transportOptions)
{
    ConfigureRecorderSettings(transportOptions, mAudioStreamParams, mVideoStreamParams);
}

bool PushAVTransport::GetBusyStatus()
{
    return (mUploader.get() != nullptr && mUploader->GetUploadQueueSize() > 0);
}

uint16_t PushAVTransport::GetPreRollLength()
{
    return mClipInfo.mPreRollLengthMs;
}

void PushAVTransport::StartNewSession(uint64_t newSessionID)
{
    mClipInfo.mSessionNumber = newSessionID;
    ChipLogProgress(Camera, "Session completed, New session started with session number [%ld]", mClipInfo.mSessionNumber);
    mStreaming    = false;
    mCanSendVideo = false;
    mCanSendAudio = false;
    mRecorder.reset();

    InitializeRecorder();
    auto now            = std::chrono::steady_clock::now();
    auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - mRecorder->mClipInfo.activationTime).count();
    mRecorder->mClipInfo.mElapsedTimeS = static_cast<uint16_t>(elapsedSeconds);

    mRecorder->Start();
    mStreaming = true;
}
