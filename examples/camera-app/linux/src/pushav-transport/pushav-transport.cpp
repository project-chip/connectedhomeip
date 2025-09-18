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

#include <pushav-transport.h>

using namespace chip::app::Clusters::PushAvStreamTransport;

PushAVTransport::PushAVTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID,
                                 AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams) :
    mAudioStreamParams(audioStreamParams),
    mVideoStreamParams(videoStreamParams)
{
    ConfigureRecorderSettings(transportOptions, audioStreamParams, videoStreamParams);
    mConnectionID    = connectionID;
    mTransportStatus = TransportStatusEnum::kInactive;
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
    ChipLogProgress(Camera, "Initial Duration: %d sec", clipInfo.mInitialDuration);
    ChipLogProgress(Camera, "Augmentation Duration: %d sec", clipInfo.mAugmentationDuration);
    ChipLogProgress(Camera, "Max Clip Duration: %d sec", clipInfo.mMaxClipDuration);
    ChipLogProgress(Camera, "Chunk Duration: %d sec", clipInfo.mChunkDuration);
    ChipLogProgress(Camera, "Blind Duration: %d sec", clipInfo.mBlindDuration);
    ChipLogProgress(Camera, "PreRoll Length: %d ", clipInfo.mPreRollLength);
    ChipLogProgress(Camera, "URL: %s", clipInfo.mUrl.c_str());
    ChipLogProgress(Camera, "Trigger Type: %d", clipInfo.mTriggerType);
    ChipLogProgress(Camera, "Recorder ID: %s", clipInfo.mRecorderId.c_str());
    ChipLogProgress(Camera, "Output Path: %s", clipInfo.mOutputPath.c_str());
    ChipLogProgress(Camera, "Input Time Base: %d/%d", clipInfo.mInputTimeBase.num, clipInfo.mInputTimeBase.den);

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

void PushAVTransport::ConfigureRecorderSettings(const TransportOptionsStruct & transportOptions,
                                                AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams)
{
    bool debug = false; // Set this to true for debug purposes

    if (debug)
    {
        mClipInfo.mInitialDuration      = 20;
        mClipInfo.mAugmentationDuration = 10;
        mClipInfo.mBlindDuration        = 5;
        mClipInfo.mMaxClipDuration      = 30;
        mClipInfo.mChunkDuration        = 5;
        mClipInfo.mTriggerType          = 0;
        mClipInfo.mPreRollLength        = 0;
        mClipInfo.mUrl                  = "https://localhost:1234/streams/1/";
    }
    else
    {
        mClipInfo.mHasAudio    = true;
        mClipInfo.mHasVideo    = true;
        mClipInfo.mUrl         = std::string(transportOptions.url.begin(), transportOptions.url.end());
        mClipInfo.mTriggerType = static_cast<int>(transportOptions.triggerOptions.triggerType);
        if (transportOptions.triggerOptions.maxPreRollLen.HasValue())
        {
            mClipInfo.mPreRollLength = transportOptions.triggerOptions.maxPreRollLen.Value();
        }
        else
        {
            mClipInfo.mPreRollLength = 0; // Default pre roll length is zero
        }
        if (transportOptions.triggerOptions.motionTimeControl.HasValue())
        {
            mClipInfo.mInitialDuration      = transportOptions.triggerOptions.motionTimeControl.Value().initialDuration;
            mClipInfo.mAugmentationDuration = transportOptions.triggerOptions.motionTimeControl.Value().augmentationDuration;
            mClipInfo.mMaxClipDuration      = transportOptions.triggerOptions.motionTimeControl.Value().maxDuration;
            mClipInfo.mBlindDuration        = transportOptions.triggerOptions.motionTimeControl.Value().blindDuration;
        }
        if (transportOptions.containerOptions.CMAFContainerOptions.HasValue())
        {
            mClipInfo.mChunkDuration = transportOptions.containerOptions.CMAFContainerOptions.Value().chunkDuration;
        }
    }

    mTransportTriggerType = transportOptions.triggerOptions.triggerType;
    mClipInfo.mClipId     = 0;
    mClipInfo.mOutputPath = "/tmp/"; // CAUTION: If path is not accessible to executable, the program may fail to write and crash.
    mClipInfo.mInputTimeBase = { 1, 1000000 };

    uint8_t audioCodec   = static_cast<uint8_t>(audioStreamParams.audioCodec);
    mAudioInfo.mChannels = (audioStreamParams.channelCount == 0) ? 1 : audioStreamParams.channelCount;

    if (audioCodec == 0)
    {
        mAudioInfo.mAudioCodecId       = AV_CODEC_ID_OPUS;
        mAudioInfo.mAudioTimeBase      = { 1, 48000 };
        mAudioInfo.mAudioFrameDuration = 19200;
    }
    else if (audioCodec == 2)
    {
        ChipLogError(Camera, "Unknown Audio codec")
    }
    else
    {
        ChipLogError(Camera, "Unsupported Audio codec");
    }

    if (audioStreamParams.sampleRate == 0)
    {
        audioStreamParams.sampleRate = 48000; // Fallback value for invalid sample rate
    }
    mAudioInfo.mSampleRate = audioStreamParams.sampleRate;
    if (audioStreamParams.bitRate == 0)
    {
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
        ChipLogError(Camera, "Invalid frame rate: 0. Using fallback 15 fps.");
        videoStreamParams.minFrameRate = 15;
    }
    mVideoInfo.mFrameRate = videoStreamParams.minFrameRate;

    mVideoInfo.mVideoFrameDuration = 900000 / mVideoInfo.mFrameRate;
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
    }
    else
    {
        ChipLogError(Camera, "Recorder already initialized");
    }
    mClipInfo.mClipId++;
}

PushAVTransport::~PushAVTransport()
{
    // TODO cleanup the existing recorded files here.
    mCanSendVideo = false;
    mCanSendAudio = false;
    mRecorder.reset();
    mUploader.reset();
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
    int64_t elapsed;
    auto now = std::chrono::steady_clock::now();

    if (InBlindPeriod(mBlindStartTime, mRecorder->mClipInfo.mBlindDuration))
    {
        return false;
    }

    elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - mRecorder->mClipInfo.activationTime).count();
    ChipLogError(Camera, "PushAVTransport HandleTriggerDetected elapsed: %ld", elapsed);

    if (!mRecorder->GetRecorderStatus())
    {
        // Start new recording
        ChipLogError(Camera, "PushAVTransport starting new recording");
        mHasAugmented                       = false;
        mRecorder->mClipInfo.activationTime = std::chrono::steady_clock::now();
        mRecorder->Start();
        mStreaming = true;
    }
    else
    {
        // Extend existing recording
        uint16_t previousDuration = mRecorder->mClipInfo.mInitialDuration - mRecorder->mClipInfo.mAugmentationDuration;

        if ((elapsed < mRecorder->mClipInfo.mInitialDuration) && (!mHasAugmented || elapsed >= previousDuration))
        {
            ChipLogError(Camera, "PushAVTransport extending recording %d -> %d", mRecorder->mClipInfo.mInitialDuration,
                         static_cast<uint16_t>(std::min(static_cast<uint32_t>(mRecorder->mClipInfo.mInitialDuration +
                                                                              mRecorder->mClipInfo.mAugmentationDuration),
                                                        static_cast<uint32_t>(mRecorder->mClipInfo.mMaxClipDuration))));
            mRecorder->mClipInfo.mInitialDuration = static_cast<uint16_t>(
                std::min(static_cast<uint32_t>(mRecorder->mClipInfo.mInitialDuration + mRecorder->mClipInfo.mAugmentationDuration),
                         static_cast<uint32_t>(mRecorder->mClipInfo.mMaxClipDuration)));
            mHasAugmented = true;
            mStreaming    = true;
        }
    }
    mBlindStartTime = mRecorder->mClipInfo.activationTime + std::chrono::seconds(mRecorder->mClipInfo.mInitialDuration);
    return true;
}

void PushAVTransport::TriggerTransport(TriggerActivationReasonEnum activationReason)
{
    ChipLogProgress(Camera, "PushAVTransport trigger transport, activation reason: [%u]", (uint16_t) activationReason);

    if (mTransportTriggerType == TransportTriggerTypeEnum::kCommand)
    {
        if (HandleTriggerDetected())
        {
            ChipLogError(Camera, "PushAVTransport command/motion transport trigger received. Clip duration [%d seconds]",
                         mRecorder->mClipInfo.mInitialDuration);
        }
        else
        {
            ChipLogError(
                Camera,
                "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration. "
                "Clip duration [%d seconds]",
                mRecorder->mClipInfo.mInitialDuration);
        }
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
    {
        if (HandleTriggerDetected())
        {
            ChipLogError(Camera, "PushAVTransport command/motion transport trigger received. Clip duration [%d seconds]",
                         mRecorder->mClipInfo.mInitialDuration);
        }
        else
        {
            ChipLogError(
                Camera,
                "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration. "
                "Clip duration [%d seconds]",
                mRecorder->mClipInfo.mInitialDuration);
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

        mUploader = std::make_unique<PushAVUploader>(mCertPath);
        mUploader->Start();
        InitializeRecorder();

        if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
        {
            mRecorder->Start();
            mStreaming = true;
            if (IsStreaming())
            {
                ChipLogProgress(Camera, "Ready to stream");
            }
        }
        else
        {
            // Check if activationTime is set (non-default)
            if (mRecorder->mClipInfo.activationTime == std::chrono::steady_clock::time_point())
            {
                ChipLogProgress(Camera, "No active trigger to start recording");
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto elapsedSeconds =
                    std::chrono::duration_cast<std::chrono::seconds>(now - mRecorder->mClipInfo.activationTime).count();

                // Check if recording duration has expired
                if (elapsedSeconds >= mRecorder->mClipInfo.mInitialDuration)
                {
                    ChipLogProgress(Camera, "No active trigger (time expired) to start recording");
                }
                else
                {
                    // Calculate remaining duration safely
                    mRecorder->mClipInfo.mInitialDuration -= static_cast<uint16_t>(elapsedSeconds);
                    ChipLogProgress(Camera, "Active trigger is present. Recording will start for [%d seconds]",
                                    mRecorder->mClipInfo.mInitialDuration);
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
        ChipLogError(Camera, "Recorder destruction done");
        InitializeRecorder();
        mUploader.reset();

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
    if (mRecorder->mDeinitializeRecorder.load()) // Current clip is completed, Next clip will start on trigger
    {
        mRecorder.reset();
        InitializeRecorder();
        mStreaming = false;
        return false;
    }
    return true;
}

void PushAVTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data, size, 1);
    }
}

void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        mRecorder->PushPacket(data, size, 0);
    }
}

void PushAVTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID) {}

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
    return IsStreaming();
}

// Dummy implementation of CanfSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return IsStreaming();
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
    return mClipInfo.mPreRollLength;
}
