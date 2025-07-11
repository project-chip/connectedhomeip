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

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}
// TODO: This dummy file to be removed after merging #38998
struct BufferData
{
    uint8_t * mPtr;
    size_t mSize;
};

class PushAVUploader;

class PushAVClipRecorder
{
public:
    struct ClipInfoStruct
    {
        bool mHasVideo                 = false;
        bool mHasAudio                 = false;
        int mClipId                    = 0;
        uint32_t mMaxClipDuration      = 0;
        uint16_t mInitialDuration      = 0;
        uint16_t mAugmentationDuration = 0;
        uint16_t mChunkDuration        = 0;
        uint16_t mBlindDuration        = 0;
        std::string mRecorderId;
        std::string mOutputPath;
        struct
        {
            int num;
            int den;
        } mInputTimeBase;
        std::string mUrl;
        int mTriggerType = 0;
        std::chrono::steady_clock::time_point activationTime;
        int mPreRollLength = 0;
    };

    struct AudioInfoStruct
    {
        uint64_t mChannelLayout = 0;
        int mChannels           = 0;
        AVCodecID mAudioCodecId = AV_CODEC_ID_NONE;
        int mSampleRate         = 0;
        int mBitRate            = 0;
        int64_t mAudioPts       = 0;
        int64_t mAudioDts       = 0;
        int mAudioStreamIndex   = -1;
        int mAudioFrameDuration = 0;
        struct
        {
            int num;
            int den;
        } mAudioTimeBase;
    };

    struct VideoInfoStruct
    {
        AVCodecID mVideoCodecId = AV_CODEC_ID_NONE;
        int64_t mVideoPts       = 0;
        int64_t mVideoDts       = 0;
        int mWidth              = 0;
        int mHeight             = 0;
        int mFrameRate          = 0;
        int mVideoFrameDuration = 0;
        struct
        {
            int num;
            int den;
        } mVideoTimeBase;
        int mVideoStreamIndex = -1;
        uint32_t mBitRate     = 0;
    };

    PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo,
                       PushAVUploader * aUploader) :
        mClipInfo(aClipInfo),
        mAudioInfo(aAudioInfo), mVideoInfo(aVideoInfo), mUploader(aUploader)
    {}

    ~PushAVClipRecorder() = default;

    void Start()
    {
        mRunning              = true;
        mDeinitializeRecorder = false;
    }

    void Stop() { mRunning = false; }

    void PushPacket(const char * data, size_t size, bool isVideo)
    {
        // Dummy implementation
    }

    void SetRecorderStatus(bool status) { mRunning = status; }

    bool GetRecorderStatus() { return mRunning; }

    std::atomic<bool> mDeinitializeRecorder{ false };
    ClipInfoStruct mClipInfo;

private:
    std::atomic<bool> mRunning{ false };
    AudioInfoStruct mAudioInfo;
    VideoInfoStruct mVideoInfo;
    PushAVUploader * mUploader;
    std::mutex mQueueMutex;
    std::condition_variable mCondition;
    std::queue<AVPacket *> mAudioQueue;
    std::queue<AVPacket *> mVideoQueue;
};
