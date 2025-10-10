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

#include "pushav-clip-recorder.h"
#include <cstring>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <sys/stat.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

#ifndef LIBAVCODEC_VERSION_INT
#error "LIBAVCODEC_VERSION_INT not defined. Please use a version of FFmpeg/libavcodec that defines this macro."
#endif

#define IS_H264_FRAME_NALU_HEAD(frame)                                                                                             \
    (((frame)[0] == 0x00) && ((frame)[1] == 0x00) && (((frame)[2] == 0x01) || (((frame)[2] == 0x00) && ((frame)[3] == 0x01))))

AVDictionary * options = NULL;

PushAVClipRecorder::PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo,
                                       PushAVUploader * aUploader) :
    mClipInfo(aClipInfo),
    mAudioInfo(aAudioInfo), mVideoInfo(aVideoInfo), mUploader(aUploader)
{
    mFormatContext        = nullptr;
    mInputFormatContext   = nullptr;
    mVideoStream          = nullptr;
    mAudioStream          = nullptr;
    mAudioEncoderContext  = nullptr;
    mVideoInfo.mVideoPts  = 0;
    mVideoInfo.mVideoDts  = 0;
    mAudioInfo.mAudioPts  = 0;
    mAudioInfo.mAudioDts  = 0;
    int streamIndex       = 0;
    mMetadataSet          = false;
    mDeinitializeRecorder = false;
    mUploadedInitSegment  = false;
    mUploadMPD            = false;
    mAudioFragment        = 1;
    mVideoFragment        = 1;
    mCurrentClipStartPts  = AV_NOPTS_VALUE;
    mFoundFirstIFramePts  = -1;
    currentPts            = AV_NOPTS_VALUE;
    if (mClipInfo.mHasVideo)
    {
        mVideoInfo.mVideoStreamIndex = streamIndex++;
    }
    else
    {
        ChipLogError(Camera, "ERROR: No video stream provided");
    }
    if (mClipInfo.mHasAudio)
    {
        mAudioInfo.mAudioStreamIndex = streamIndex;
    }
    SetRecorderStatus(false); // Start off as not running

    mClipInfo.mRecorderId = std::to_string(mVideoInfo.mVideoStreamIndex) + "-" + std::to_string(mAudioInfo.mAudioStreamIndex);
    ChipLogProgress(Camera, "PushAVClipRecorder initialized with ID: %s, output path: %s", mClipInfo.mRecorderId.c_str(),
                    mClipInfo.mOutputPath.c_str());
}

PushAVClipRecorder::~PushAVClipRecorder()
{
    Stop();
    if (mWorkerThread.joinable())
    {
        mWorkerThread.join();
    }
}

namespace {
int ReadPacket(void * opaque, uint8_t * buf, int bufSize)
{
    struct BufferData * bd = (struct BufferData *) opaque;
    if (static_cast<size_t>(bufSize) > bd->mSize)
    {
        bufSize = static_cast<int>(bd->mSize);
    }

    if (!bufSize)
        return AVERROR_EOF;

    /* copy internal buffer data to buf */
    memcpy(buf, bd->mPtr, static_cast<size_t>(bufSize));
    bd->mPtr += bufSize;
    bd->mSize -= static_cast<size_t>(bufSize);

    return bufSize;
}
} // namespace

bool PushAVClipRecorder::IsH264IFrame(const uint8_t * data, unsigned int length)
{
    unsigned int idx = 0;
    int frameType    = 0;
    int foundIdr     = 0;
    bool ret         = false;

    if (data == nullptr || (length < 5))
    {
        return ret;
    }

    do
    {
        if (IS_H264_FRAME_NALU_HEAD(data + idx))
        {
            if (data[idx + 2] == 0x01)
                frameType = data[idx + 3] & 0x1f;
            else if ((data[idx + 2] == 0x00) && (data[idx + 3] == 0x01))
                frameType = data[idx + 4] & 0x1f;

            if (frameType == 5)
            {
                foundIdr = 1;
                break;
            }
            if ((data[idx + 2] == 0x00) && (data[idx + 3] == 0x01))
                idx++;

            idx += 4;
        }
        else
        {
            idx++;
        }
    } while (idx < (length - 4));

    // If we found an IDR frame, it's an I-frame regardless of SPS/PPS presence
    // SPS/PPS may have been sent in previous packets or out-of-band
    if (foundIdr == 1)
    {
        ret = true;
    }

    return ret;
}

AVPacket * PushAVClipRecorder::CreatePacket(const uint8_t * data, int size, bool isVideo)
{
    AVPacket * packet = av_packet_alloc();
    if (!packet)
    {
        ChipLogError(Camera, "ERROR: AVPacket allocation failed!");
        return nullptr;
    }

    packet->data = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(size)));
    if (!packet->data)
    {
        ChipLogError(Camera, "ERROR: AVPacket data allocation failed!");
        av_packet_free(&packet);
        return nullptr;
    }

    memcpy(packet->data, data, static_cast<size_t>(size));
    packet->size = size;

    if (isVideo)
    {
        if (IsH264IFrame(data, static_cast<unsigned int>(size)))
        {
            mFoundFirstIFramePts = mVideoInfo.mVideoPts;
            packet->flags        = AV_PKT_FLAG_KEY;
            ChipLogProgress(Camera, "Found I-frame at PTS: %ld", mVideoInfo.mVideoPts);
        }

        if (mFoundFirstIFramePts < 0)
        {
            ChipLogError(Camera, "ERROR: First frame is not an I-frame. Dropping packet.");
            av_packet_free(&packet);
            return nullptr;
        }

        packet->pts          = mVideoInfo.mVideoPts;
        packet->dts          = mVideoInfo.mVideoDts;
        packet->stream_index = mVideoInfo.mVideoStreamIndex;
        packet->duration     = mVideoInfo.mVideoFrameDuration;
        mVideoInfo.mVideoDts += mVideoInfo.mVideoFrameDuration;
        mVideoInfo.mVideoPts += mVideoInfo.mVideoFrameDuration;
    }
    else
    {
        if (mFoundFirstIFramePts < 0 && mFoundFirstIFramePts <= mAudioInfo.mAudioPts)
        {
            ChipLogError(Camera, "ERROR: frames will be dropped till an Iframe is recived");
            av_packet_free(&packet);
            return nullptr;
        }
        packet->pts          = mAudioInfo.mAudioPts;
        packet->dts          = mAudioInfo.mAudioDts;
        packet->stream_index = mAudioInfo.mAudioStreamIndex;
        packet->duration     = mAudioInfo.mAudioFrameDuration;
        mAudioInfo.mAudioDts += mAudioInfo.mAudioFrameDuration;
        mAudioInfo.mAudioPts += mAudioInfo.mAudioFrameDuration;
    }

    return (mFoundFirstIFramePts < 0) ? nullptr : packet;
}

void PushAVClipRecorder::Start()
{
    if (GetRecorderStatus())
    {
        ChipLogError(Camera, "ERROR: Recording is already running. Stop before starting again");
        return;
    }
    SetRecorderStatus(true);
    mWorkerThread = std::thread(&PushAVClipRecorder::StartClipRecording, this);
    ChipLogProgress(Camera, "Recording started for ID: %s", mClipInfo.mRecorderId.c_str());
}

void PushAVClipRecorder::Stop()
{
    if (GetRecorderStatus())
    {
        // Call the cluster server's NotifyTransportStopped method asynchronously to prevent blocking
        if (mPushAvStreamTransportServer != nullptr)
        {
            ChipLogProgress(Camera, "PushAVClipRecorder::Stop - Scheduling async cluster server API call for connection %u",
                            mConnectionID);

            uint16_t connectionID = mConnectionID;
            auto triggerType      = mTriggerType;
            auto * server         = mPushAvStreamTransportServer;

            std::thread([server, connectionID, triggerType]() {
                ChipLogProgress(Camera, "Async thread: Calling NotifyTransportStopped for connection %u", connectionID);
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                server->NotifyTransportStopped(connectionID, triggerType);
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                ChipLogProgress(Camera, "Async thread: NotifyTransportStopped completed for connection %u", connectionID);
            }).detach();
        }
        else
        {
            ChipLogError(Camera, "PushAVClipRecorder::Stop - Cluster server reference is null for connection %u", mConnectionID);
        }

        SetRecorderStatus(false);
        mCondition.notify_one();
        while (!mVideoQueue.empty())
        {
            av_packet_free(&mVideoQueue.front());
            mVideoQueue.pop();
        }
        while (!mAudioQueue.empty())
        {
            av_packet_free(&mAudioQueue.front());
            mAudioQueue.pop();
        }
    }
    else
    {
        ChipLogError(Camera, "Error recording is not running");
    }
    mDeinitializeRecorder = true;
    ChipLogProgress(Camera, "Recording stopped for ID: %s", mClipInfo.mRecorderId.c_str());
}

void PushAVClipRecorder::PushPacket(const char * data, size_t size, bool isVideo)
{
    if (!GetRecorderStatus())
    {
        ChipLogError(Camera, "ERROR: Push packet dropped as recorder is not active");
        return;
    }

    AVPacket * packet = CreatePacket((const uint8_t *) data, static_cast<int>(size), isVideo);
    if (!packet)
    {
        ChipLogError(Camera, "ERROR: PACKET DROPPED!");
        return;
    }

    std::lock_guard<std::mutex> lock(mQueueMutex);
    std::queue<AVPacket *> & queue = isVideo ? mVideoQueue : mAudioQueue;
    if (queue.size() >= kMaxQueueSize)
    {
        AVPacket * oldPacket = queue.front();
        queue.pop();
        av_packet_free(&oldPacket);
        ChipLogProgress(Camera, "Queue full. Dropped old packet");
    }
    queue.push(packet);
    mCondition.notify_one();
    if (mClipInfo.activationTime == std::chrono::steady_clock::time_point())
    {
        mClipInfo.activationTime = std::chrono::steady_clock::now();
    }
}

int PushAVClipRecorder::SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern,
                                    const std::string & mediaSegPattern)
{
    const std::string mpdFilename = outputPrefix + ".mpd";
    if (avformat_alloc_output_context2(&mFormatContext, nullptr, nullptr, mpdFilename.c_str()) < 0)
    {
        ChipLogError(Camera, "ERROR: Failed to allocate output context");
        Stop();
        return -1;
    }
    if (!mFormatContext)
    {
        ChipLogError(Camera, "ERROR: Output context is null");
    }
    double segSeconds = static_cast<double>(mClipInfo.mSegmentDuration) / 1000.0;
    // Set DASH/CMAF options
    av_opt_set(mFormatContext->priv_data, "increment_tc", "1", 0);
    av_opt_set(mFormatContext->priv_data, "use_timeline", "1", 0);
    av_opt_set(mFormatContext->priv_data, "movflags", "+cmaf+dash+delay_moov+skip_sidx+skip_trailer+frag_custom", 0);
    av_opt_set(mFormatContext->priv_data, "seg_duration", std::to_string(segSeconds).c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "frag_duration", std::to_string(mClipInfo.mChunkDuration).c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "init_seg_name", initSegPattern.c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "media_seg_name", mediaSegPattern.c_str(), 0);
    av_opt_set_int(mFormatContext->priv_data, "use_template", 1, 0);
    av_dict_set_int(&options, "dash_segment_type", 1, 0);
    av_dict_set_int(&options, "use_timeline", 1, 0);
    av_dict_set(&options, "strict", "experimental", 0);

    if (mClipInfo.mHasVideo && (AddStreamToOutput(AVMEDIA_TYPE_VIDEO) < 0))
    {
        ChipLogError(Camera, "ERROR: adding video stream to output");
        return -1;
    }
    if (mClipInfo.mHasAudio && (AddStreamToOutput(AVMEDIA_TYPE_AUDIO) < 0))
    {
        ChipLogError(Camera, "ERROR: adding video stream to output");
        return -1;
    }

    if (!(mFormatContext->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&mFormatContext->pb, mpdFilename.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            ChipLogError(Camera, "ERROR: Failed to open output file: %s", mpdFilename.c_str());
            Stop();
            return -1;
        }
    }

    if (avformat_write_header(mFormatContext, &options) < 0)
    {
        ChipLogError(Camera, "Error: writing output header");
        Stop();
        return -1;
    }
    return 0;
}

int PushAVClipRecorder::StartClipRecording()
{
    if (!mClipInfo.mHasVideo)
    {
        ChipLogError(Camera, "ERROR: No video stream available. Stopping recording");
        return -1;
    }

    while (GetRecorderStatus())
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCondition.wait(
            lock, [this] { return !mVideoQueue.empty() || !mAudioQueue.empty() || !GetRecorderStatus() || mDeinitializeRecorder; });
        if (!GetRecorderStatus() || mDeinitializeRecorder)
        {
            ChipLogProgress(Camera, "Recorder thread received stop signal for ID: %s", mClipInfo.mRecorderId.c_str());
            break; // Exit loop
        }
        ProcessBuffersAndWrite();
    }
    CleanupOutput();
    ChipLogProgress(Camera, "Recorder thread closing");
    return 0;
}

int PushAVClipRecorder::AddStreamToOutput(AVMediaType type)
{
    if (type == AVMEDIA_TYPE_VIDEO)
    {
        mVideoStream = avformat_new_stream(mFormatContext, nullptr);
        if (avcodec_parameters_copy(mVideoStream->codecpar, mInputFormatContext->streams[0]->codecpar) < 0)
        {
            ChipLogError(Camera, "ERROR: Failed to copy codec parameters for media type: %d", type);
            Stop();
            return -1;
        }
        mVideoStream->codecpar->codec_tag = 0;
        mVideoStream->codecpar->width     = mVideoInfo.mWidth;
        mVideoStream->codecpar->height    = mVideoInfo.mHeight;
        mVideoStream->avg_frame_rate      = (AVRational){ mVideoInfo.mFrameRate, 1 };
    }
    else if (type == AVMEDIA_TYPE_AUDIO)
    {
        mAudioStream = avformat_new_stream(mFormatContext, nullptr);
        if (!mAudioStream)
        {
            ChipLogError(Camera, "ERROR: Failed to add audio stream");
            Stop();
            return -1;
        }
        const AVCodec * audioCodec = avcodec_find_encoder(mAudioInfo.mAudioCodecId);
        if (!audioCodec)
        {
            ChipLogError(Camera, "ERROR: Audio encoder not found");
            Stop();
            return -1;
        }
        mAudioEncoderContext = avcodec_alloc_context3(audioCodec);
        if (!mAudioEncoderContext)
        {
            ChipLogError(Camera, "Error: failed to allocate the encoder context");
            Stop();
            return -1;
        }

        mAudioEncoderContext->sample_rate = mAudioInfo.mSampleRate;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 37, 100)
        ChipLogProgress(Camera, "PushAVClipRecorder using FFMPEG version < 5.1");
        mAudioEncoderContext->channels       = mAudioInfo.mChannels;
        mAudioEncoderContext->channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(mAudioEncoderContext->channels));
#else
        ChipLogProgress(Camera, "PushAVClipRecorder using FFMPEG version >= 5.1");
        av_channel_layout_default(&mAudioEncoderContext->ch_layout, mAudioInfo.mChannels);
#endif

        mAudioEncoderContext->bit_rate              = mAudioInfo.mBitRate;
        mAudioEncoderContext->sample_fmt            = audioCodec->sample_fmts[0];
        mAudioEncoderContext->time_base             = (AVRational){ 1, mAudioInfo.mSampleRate };
        mAudioEncoderContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        AVDictionary * opts                         = NULL;
        av_dict_set(&opts, "strict", "experimental", 0);
        if (avcodec_open2(mAudioEncoderContext, audioCodec, &opts) < 0)
        {
            ChipLogError(Camera, "Error: Cannot open audio encoder for audio stream");
            Stop();
            return -1;
        }
        if (avcodec_parameters_from_context(mAudioStream->codecpar, mAudioEncoderContext) < 0)
        {
            ChipLogError(Camera, "Error: Failed to copy encoder parameters to audio output stream");
            Stop();
            return -1;
        }
        if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        {
            mAudioEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    return 0;
}

int PushAVClipRecorder::ProcessBuffersAndWrite()
{
    if (mVideoQueue.empty() && mAudioQueue.empty())
    {
        return -1;
    }

    bool useVideo  = false;
    AVPacket * pkt = nullptr;

    if (!mVideoQueue.empty() && !mAudioQueue.empty())
    {
        AVPacket * videoPkt = mVideoQueue.front();
        AVPacket * audioPkt = mAudioQueue.front();

        if (videoPkt->pts != AV_NOPTS_VALUE && audioPkt->pts != AV_NOPTS_VALUE)
        {
            useVideo = (videoPkt->pts <= audioPkt->pts);
        }
        else if (videoPkt->pts != AV_NOPTS_VALUE)
        {
            useVideo = true;
        }
        else if (audioPkt->pts != AV_NOPTS_VALUE)
        {
            useVideo = false;
        }
        pkt = useVideo ? videoPkt : audioPkt;
    }
    else if (!mVideoQueue.empty())
    {
        pkt      = mVideoQueue.front();
        useVideo = true;
    }
    else if (!mAudioQueue.empty())
    {
        pkt      = mAudioQueue.front();
        useVideo = false;
    }
    else
    {
        return false;
    }
    if (!pkt)
    {
        ChipLogError(Camera, "Error: No valid packet to process");
        return -1;
    }

    if (mMetadataSet == false)
    {
        if (mClipInfo.mHasVideo && !useVideo)
        {
            return false;
        }
        std::string prefix           = mClipInfo.mRecorderId + "_clip_" + std::to_string(mClipInfo.mClipId);
        std::string initSegName      = prefix + "_init-stream$RepresentationID$.fmp4";
        std::string mediaSegName     = prefix + "_chunk-stream$RepresentationID$-$Number%05d$.cmfv";
        mInputFormatContext          = avformat_alloc_context();
        int avioCtxBufferSize        = 1048576; // 1MB
        uint8_t * mAvioContextBuffer = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(avioCtxBufferSize)));
        struct BufferData data       = { 0 };
        data.mPtr                    = static_cast<uint8_t *>(pkt->data);
        data.mSize                   = static_cast<size_t>(pkt->size);
        mInputFormatContext->pb =
            avio_alloc_context(mAvioContextBuffer, avioCtxBufferSize, 0, &data, &ReadPacket, nullptr, nullptr);
        mInputFormatContext->flags = AVFMT_FLAG_CUSTOM_IO;

        if (avformat_open_input(&mInputFormatContext, "", nullptr, nullptr) < 0)
        {
            ChipLogError(Camera, "Error: Failed to open input format for video");
            Stop();
            return -1;
        }

        if (avformat_find_stream_info(mInputFormatContext, nullptr) < 0)
        {
            ChipLogError(Camera, "Error: Failed to find stream info for video");
            Stop();
            return -1;
        }
        if (SetupOutput(mClipInfo.mOutputPath + prefix, initSegName, mediaSegName) < 0)
        {
            ChipLogError(Camera, "Error: setting up output");
            return -1;
        }
        if (!mFormatContext)
        {
            ChipLogError(Camera, "Error: Output context not initialized. Skipping packet");
            Stop();
            return -1;
        }
        mMetadataSet = true;
    }

    AVRational outputTimeBase = useVideo ? mVideoInfo.mVideoTimeBase : mAudioInfo.mAudioTimeBase;

    if (pkt->pts == AV_NOPTS_VALUE && pkt->dts == AV_NOPTS_VALUE)
    {
        ChipLogError(Camera, "Warning packet has no valid timestamps\n");
        av_packet_unref(pkt);
        return 0;
    }

    currentPts = (pkt->pts != AV_NOPTS_VALUE) ? pkt->pts : pkt->dts;
    if (mCurrentClipStartPts == AV_NOPTS_VALUE)
    {
        mCurrentClipStartPts = currentPts;
    }

    pkt->pts      = av_rescale_q(pkt->pts, mClipInfo.mInputTimeBase, outputTimeBase);
    pkt->dts      = av_rescale_q(pkt->dts, mClipInfo.mInputTimeBase, outputTimeBase);
    pkt->duration = av_rescale_q(pkt->duration, mClipInfo.mInputTimeBase, outputTimeBase);
    pkt->pos      = -1;

    if (pkt->pts < 0)
    {
        ChipLogError(Camera, "Warning Negative PTS detected: %ld", pkt->pts);
        pkt->pts = (pkt->dts != AV_NOPTS_VALUE) ? pkt->dts : 0;
    }
    if (av_interleaved_write_frame(mFormatContext, pkt) < 0)
    {
        ChipLogError(Camera, "Error writing frame to output file");
        FinalizeCurrentClip(1);
        return -1;
    }

    av_packet_free(&pkt);
    if (useVideo)
    {
        mVideoQueue.pop();
    }
    else
    {
        mAudioQueue.pop();
    }
    FinalizeCurrentClip(0);

    return 0;
}

void PushAVClipRecorder::CleanupOutput()
{
    if (mFormatContext)
    {
        av_interleaved_write_frame(mFormatContext, nullptr);
        if (av_write_trailer(mFormatContext) < 0)
        {
            ChipLogError(Camera, "Error writing trailer to output file");
        }
        if (!(mFormatContext->oformat->flags & AVFMT_NOFILE) && mFormatContext->pb)
        {
            avio_closep(&mFormatContext->pb);
        }
        avformat_free_context(mFormatContext);
        mFormatContext = nullptr;
    }
    if (mInputFormatContext)
    {
        avformat_close_input(&mInputFormatContext);
        mInputFormatContext = nullptr;
    }
    if (mAudioEncoderContext)
    {
        avcodec_free_context(&mAudioEncoderContext);
    }
    mVideoStream = nullptr;
    mAudioStream = nullptr;
    mMetadataSet = false;
    ChipLogProgress(Camera, "Cleanup completed");
}

/**
 * @brief Finalizes the current clip and starts a new one.
 *
 * Writes the trailer of the current clip and initializes a new output file.
 */

void PushAVClipRecorder::FinalizeCurrentClip(int reason)
{
    int64_t clipLengthInPTS = currentPts - mCurrentClipStartPts;
    // Final duration has to be (clipDuration + preRollLen) seconds
    const int64_t clipDuration = (mClipInfo.mInitialDuration + (mClipInfo.mPreRollLength / 1000)) * AV_TIME_BASE_Q.den;
    // Pre-calculate common path components
    const std::string prefix   = mClipInfo.mRecorderId + "_clip_" + std::to_string(mClipInfo.mClipId);
    const std::string basePath = mClipInfo.mOutputPath + prefix;

    if (reason || ((clipLengthInPTS >= clipDuration) && (mClipInfo.mTriggerType != 2)))
    {
        Stop();
        mCurrentClipStartPts = currentPts;
    }

    // Helper function for safe path formatting
    char path_buffer[512];
    auto make_path = [&](const char * format, int number = -1) -> std::string {
        if (number >= 0)
        {
            snprintf(path_buffer, sizeof(path_buffer), format, basePath.c_str(), number);
        }
        else
        {
            snprintf(path_buffer, sizeof(path_buffer), format, basePath.c_str());
        }
        return std::string(path_buffer);
    };

    // 1. Handle initialization files
    std::string fmp4_path = make_path("%s_init-stream0.fmp4");
    if (mUploadedInitSegment && FileExists(fmp4_path) && !FileExists(fmp4_path + ".tmp"))
    {
        mUploadedInitSegment = false;
        CheckAndUploadFile(fmp4_path);

        if (mClipInfo.mHasAudio)
        {
            std::string audio_fmp4 = make_path("%s_init-stream1.fmp4");
            if (FileExists(audio_fmp4) && !FileExists(audio_fmp4 + ".tmp"))
            {
                CheckAndUploadFile(audio_fmp4);
            }
        }
    }

    // 2. Handle video fragments
    std::string video_cmfv = make_path("%s_chunk-stream0-%05d.cmfv", mVideoFragment);
    while (FileExists(video_cmfv) && !FileExists(video_cmfv + ".tmp"))
    {
        if (mVideoFragment == 1)
        {
            mUploadedInitSegment = true;
        }
        mUploadMPD = true;
        CheckAndUploadFile(video_cmfv);
        mVideoFragment++;
        video_cmfv = make_path("%s_chunk-stream0-%05d.cmfv", mVideoFragment);
    }

    // 3. Handle audio fragments
    if (mClipInfo.mHasAudio)
    {
        std::string audio_cmfv = make_path("%s_chunk-stream1-%05d.cmfv", mAudioFragment);
        while (FileExists(audio_cmfv) && !FileExists(audio_cmfv + ".tmp"))
        {
            mUploadMPD = true;
            CheckAndUploadFile(audio_cmfv);
            mAudioFragment++;
            audio_cmfv = make_path("%s_chunk-stream1-%05d.cmfv", mAudioFragment);
        }
    }

    // 4. Handle MPD file
    if (mUploadMPD)
    {
        std::string mpd_path = make_path("%s.mpd");
        if (FileExists(mpd_path) && !FileExists(mpd_path + ".tmp"))
        {
            mUploader->setMPDPath(std::make_pair(mpd_path, mClipInfo.mUrl));
            CheckAndUploadFile(mpd_path);
            mUploadMPD = false; // Reset flag after successful upload
        }
    }
}

bool PushAVClipRecorder::CheckAndUploadFile(std::string filename)
{
    ChipLogProgress(Camera, "Recorder:File upload ready [%s] to [%s]", filename.c_str(), mClipInfo.mUrl.c_str());
    mUploader->AddUploadData(filename, mClipInfo.mUrl);
    return true;
}

bool PushAVClipRecorder::FileExists(const std::string & path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

void PushAVClipRecorder::SetRecorderStatus(bool status)
{
    ChipLogProgress(Camera, "Setting Clip Recorder to Status %d", status);
    mRunning = status;
}

bool PushAVClipRecorder::GetRecorderStatus()
{
    return mRunning;
}
