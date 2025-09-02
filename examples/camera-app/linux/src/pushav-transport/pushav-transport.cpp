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
                                 AudioStreamStruct aAudioStreamParams, VideoStreamStruct aVideoStreamParams) :
    audioStreamParams(aAudioStreamParams),
    videoStreamParams(aVideoStreamParams)
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
                                                AudioStreamStruct mAudioStreamParams, VideoStreamStruct mVideoStreamParams)
{
    bool debug = true; // Set this to false for debug purpose

    if (debug)
    {
        clipInfo.mHasAudio    = true;
        clipInfo.mHasVideo    = true;
        clipInfo.mUrl         = transportOptions.url.data();
        clipInfo.mTriggerType = static_cast<int>(transportOptions.triggerOptions.triggerType);
        if (transportOptions.triggerOptions.maxPreRollLen.HasValue())
        {
            clipInfo.mPreRollLength = transportOptions.triggerOptions.maxPreRollLen.Value();
        }
        else
        {
            clipInfo.mPreRollLength = 0; // Default pre roll length is zero
        }
        if (transportOptions.triggerOptions.motionTimeControl.HasValue())
        {
            clipInfo.mInitialDuration      = transportOptions.triggerOptions.motionTimeControl.Value().initialDuration;
            clipInfo.mAugmentationDuration = transportOptions.triggerOptions.motionTimeControl.Value().augmentationDuration;
            clipInfo.mMaxClipDuration      = transportOptions.triggerOptions.motionTimeControl.Value().maxDuration;
            clipInfo.mBlindDuration        = transportOptions.triggerOptions.motionTimeControl.Value().blindDuration;
        }
        if (transportOptions.containerOptions.CMAFContainerOptions.HasValue())
        {
            clipInfo.mChunkDuration = transportOptions.containerOptions.CMAFContainerOptions.Value().chunkDuration;
        }
    }
    else
    {
        clipInfo.mInitialDuration      = 20;
        clipInfo.mAugmentationDuration = 10;
        clipInfo.mBlindDuration        = 5;
        clipInfo.mMaxClipDuration      = 30;
        clipInfo.mChunkDuration        = 5;
        clipInfo.mTriggerType          = 0;
        clipInfo.mPreRollLength        = 0;
        clipInfo.mUrl                  = "https://localhost:1234/streams/1/";
    }

    mTransportTriggerType   = transportOptions.triggerOptions.triggerType;
    clipInfo.mClipId        = 0;
    clipInfo.mOutputPath    = "/tmp";
    clipInfo.mInputTimeBase = { 1, 1000000 };

    uint8_t audioCodec = static_cast<uint8_t>(mAudioStreamParams.audioCodec);
    if (mAudioStreamParams.channelCount == 0)
    {
        mAudioStreamParams.channelCount = 1;
    }
    audioInfo.mChannels = mAudioStreamParams.channelCount;

    if (audioCodec == 0)
    {
        audioInfo.mAudioCodecId       = AV_CODEC_ID_OPUS;
        audioInfo.mAudioTimeBase      = { 1, 48000 };
        audioInfo.mAudioFrameDuration = 19200;
    }
    else if (audioCodec == 2)
    {
        ChipLogError(Camera, "Unknown Audio codec")
    }
    else
    {
        ChipLogError(Camera, "Unsupported Audio codec");
    }

    if (mAudioStreamParams.sampleRate == 0)
    {
        mAudioStreamParams.sampleRate = 48000; // Fallback value for invalid sample rate
    }
    audioInfo.mSampleRate = mAudioStreamParams.sampleRate;
    if (mAudioStreamParams.bitRate == 0)
    {
        mAudioStreamParams.bitRate = 96000;
    }
    audioInfo.mBitRate          = mAudioStreamParams.bitRate;
    audioInfo.mAudioPts         = 0;
    audioInfo.mAudioDts         = 0;
    audioInfo.mAudioStreamIndex = -1;

    int8_t VideoCodec = static_cast<uint8_t>(mVideoStreamParams.videoCodec);
    if (VideoCodec == 0)
    {
        videoInfo.mVideoCodecId  = AV_CODEC_ID_H264;
        videoInfo.mVideoTimeBase = { 1, 90000 };
    }
    else if (VideoCodec == 4)
    {
        ChipLogError(Camera, "Unknown Video codec")
    }
    else
    {
        ChipLogError(Camera, "Unsupported Video codec");
    }
    videoInfo.mVideoPts = 0;
    videoInfo.mVideoDts = 0;
    if (mVideoStreamParams.maxResolution.width == 0 || mVideoStreamParams.maxResolution.height == 0)
    {
        mVideoStreamParams.maxResolution.width  = 640;
        mVideoStreamParams.maxResolution.height = 320;
    }
    videoInfo.mWidth  = mVideoStreamParams.maxResolution.width;
    videoInfo.mHeight = mVideoStreamParams.maxResolution.height;
    if (mVideoStreamParams.minFrameRate == 0)
    {
        ChipLogError(Camera, "Invalid frame rate: 0. Using fallback 15 fps.");
        mVideoStreamParams.minFrameRate = 15;
    }
    videoInfo.mFrameRate = mVideoStreamParams.minFrameRate;

    videoInfo.mVideoFrameDuration = 900000 / videoInfo.mFrameRate;
    videoInfo.mVideoStreamIndex   = -1;
    videoInfo.mBitRate            = mVideoStreamParams.minBitRate;

    PrintTransportSettings(clipInfo, audioInfo, videoInfo);
    ChipLogProgress(Camera, "PushAvStreamTransportManager, Configure Recorder Settings done !!!");
}

void PushAVTransport::InitializeRecorder()
{
    if (recorder.get() == nullptr)
    {
        recorder = std::make_unique<PushAVClipRecorder>(clipInfo, audioInfo, videoInfo, uploader.get());
    }
    else
    {
        ChipLogError(Camera, "Recorder already initialized");
    }
    clipInfo.mClipId++;
}

PushAVTransport::~PushAVTransport()
{
    // TODO cleanup the existing recorded files here.
    mCanSendVideo = false;
    mCanSendAudio = false;
    recorder.reset();
    uploader.reset();
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

    if (InBlindPeriod(blindStartTime, recorder->mClipInfo.mBlindDuration))
    {
        return false;
    }

    elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - recorder->mClipInfo.activationTime).count();
    ChipLogError(Camera, "PushAVTransport HandleTriggerDetected elapsed: %ld", elapsed);

    if (!recorder->GetRecorderStatus())
    {
        // Start new recording
        ChipLogError(Camera, "PushAVTransport starting new recording");
        hasAugmented                       = false;
        recorder->mClipInfo.activationTime = std::chrono::steady_clock::now();
        recorder->Start();
        mStreaming = true;
    }
    else
    {
        // Extend existing recording
        uint16_t previousDuration = recorder->mClipInfo.mInitialDuration - recorder->mClipInfo.mAugmentationDuration;

        if ((elapsed < recorder->mClipInfo.mInitialDuration) && (!hasAugmented || elapsed >= previousDuration))
        {
            ChipLogError(Camera, "PushAVTransport extending recording %d -> %d", recorder->mClipInfo.mInitialDuration,
                         static_cast<uint16_t>(std::min(static_cast<uint32_t>(recorder->mClipInfo.mInitialDuration +
                                                                              recorder->mClipInfo.mAugmentationDuration),
                                                        static_cast<uint32_t>(recorder->mClipInfo.mMaxClipDuration))));
            recorder->mClipInfo.mInitialDuration = static_cast<uint16_t>(
                std::min(static_cast<uint32_t>(recorder->mClipInfo.mInitialDuration + recorder->mClipInfo.mAugmentationDuration),
                         static_cast<uint32_t>(recorder->mClipInfo.mMaxClipDuration)));
            hasAugmented = true;
            mStreaming   = true;
        }
    }
    blindStartTime = recorder->mClipInfo.activationTime + std::chrono::seconds(recorder->mClipInfo.mInitialDuration);
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
                         recorder->mClipInfo.mInitialDuration);
        }
        else
        {
            ChipLogError(
                Camera,
                "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration. "
                "Clip duration [%d seconds]",
                recorder->mClipInfo.mInitialDuration);
        }
    }
    else if (mTransportTriggerType == TransportTriggerTypeEnum::kMotion)
    {
        if (HandleTriggerDetected())
        {
            ChipLogError(Camera, "PushAVTransport command/motion transport trigger received. Clip duration [%d seconds]",
                         recorder->mClipInfo.mInitialDuration);
        }
        else
        {
            ChipLogError(
                Camera,
                "PushAVTransport command/motion transport trigger received but ignored due to blind period. Clip duration. "
                "Clip duration [%d seconds]",
                recorder->mClipInfo.mInitialDuration);
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

        uploader = std::make_unique<PushAVUploader>(mCertPath);
        uploader->Start();
        InitializeRecorder();

        if (mTransportTriggerType == TransportTriggerTypeEnum::kContinuous)
        {
            recorder->Start();
            mStreaming = true;
            if (IsStreaming())
            {
                ChipLogProgress(Camera, "Ready to stream");
            }
        }
        else
        {
            // Check if activationTime is set (non-default)
            if (recorder->mClipInfo.activationTime == std::chrono::steady_clock::time_point())
            {
                ChipLogProgress(Camera, "No active trigger to start recording");
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto elapsedSeconds =
                    std::chrono::duration_cast<std::chrono::seconds>(now - recorder->mClipInfo.activationTime).count();

                // Check if recording duration has expired
                if (elapsedSeconds >= recorder->mClipInfo.mInitialDuration)
                {
                    ChipLogProgress(Camera, "No active trigger (time expired) to start recording");
                }
                else
                {
                    // Calculate remaining duration safely
                    recorder->mClipInfo.mInitialDuration -= static_cast<uint16_t>(elapsedSeconds);
                    ChipLogProgress(Camera, "Active trigger is present. Recording will start for [%d seconds]",
                                    recorder->mClipInfo.mInitialDuration);
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
        recorder.reset();
        ChipLogError(Camera, "Recorder destruction done");
        InitializeRecorder();
        uploader.reset();

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
    if (recorder->mDeinitializeRecorder.load()) // Current clip is completed, Next clip will start on trigger
    {
        recorder.reset();
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
        recorder->PushPacket(data, size, 1);
    }
}

void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    if (CanSendPacketsToRecorder())
    {
        recorder->PushPacket(data, size, 0);
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
    return mCanSendVideo;
}

// Dummy implementation of CanfSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return mCanSendAudio;
}

void PushAVTransport::ModifyPushTransport(const TransportOptionsStorage transportOptions)
{
    ConfigureRecorderSettings(transportOptions, audioStreamParams, videoStreamParams);
}

bool PushAVTransport::GetBusyStatus()
{
    return (uploader.get() != nullptr && uploader->GetUploadQueueSize() > 0);
}

uint16_t PushAVTransport::GetPreRollLength()
{
    return clipInfo.mPreRollLength;
}
