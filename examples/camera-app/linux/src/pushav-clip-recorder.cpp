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
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <push-av-stream-manager.h>
#include <regex>
#include <sys/stat.h>

constexpr int kSegmentIdOffset       = 1000;
constexpr int kMPDDefaultStartNumber = 1001;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <unistd.h>
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
    mUploadSegmentID      = 0001;
    mCurrentClipStartPts  = AV_NOPTS_VALUE;
    mFoundFirstIFramePts  = -1;
    currentPts            = AV_NOPTS_VALUE;

    mVideoInfo.mVideoStreamIndex = -1;
    mAudioInfo.mAudioStreamIndex = -1;

    if (mClipInfo.mHasAudio && mClipInfo.mHasVideo)
    {
        ChipLogDetail(Camera, "Both audio and video streams are active. Only one stream at a time is supported currently.");
        mClipInfo.mHasAudio = false;
    }
    if (mClipInfo.mHasVideo)
    {
        mVideoInfo.mVideoStreamIndex = streamIndex++;
    }
    if (mClipInfo.mHasAudio)
    {
        mAudioInfo.mAudioStreamIndex = streamIndex++;
    }
    SetRecorderStatus(false); // Start off as not running
    ChipLogProgress(Camera, "PushAVClipRecorder initialized for Track name: %s, output path: %s", mClipInfo.mTrackName.c_str(),
                    mClipInfo.mOutputPath.c_str());
}

PushAVClipRecorder::~PushAVClipRecorder()
{
    ChipLogDetail(Camera, "PushAVClipRecorder destructor called for sessionID: %" PRIu64 " Track name: %s",
                  mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
    Stop();
    if (mWorkerThread.joinable())
    {
        mWorkerThread.join();
    }
}

bool PushAVClipRecorder::EnsureDirectoryExists(const std::string & path)
{
    // Base output path
    std::filesystem::path basePath(path);
    std::filesystem::path sessionDir = basePath / ("session_" + std::to_string(mClipInfo.mSessionNumber));
    std::filesystem::path trackDir   = sessionDir / mClipInfo.mTrackName;

    // Helper lambda to ensure a directory exists and is writable, creating it with mode 0755
    auto ensure = [&](const std::filesystem::path & p) -> bool {
        std::error_code ec;
        if (!std::filesystem::exists(p, ec))
        {
            if (!std::filesystem::create_directories(p, ec))
            {
                ChipLogError(Camera, "Failed to create directory: %s, error code: %d (%s)", p.c_str(), ec.value(),
                             ec.message().c_str());
                return false;
            }
            // Set permissions to file: (owner rwx, group rx)
            std::filesystem::permissions(
                p, std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::group_exec,
                std::filesystem::perm_options::replace, ec);
            ChipLogProgress(Camera, "Created directory: %s", p.c_str());
        }
        else if (!std::filesystem::is_directory(p, ec))
        {
            ChipLogError(Camera, "Path is not a directory: %s, error code: %d (%s)", p.c_str(), ec.value(), ec.message().c_str());
            return false;
        }

        auto perms = std::filesystem::status(p, ec).permissions();
        if ((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
        {
            ChipLogError(Camera, "Directory is not writable: %s, error code: %d (%s)", p.c_str(), ec.value(), ec.message().c_str());
            return false;
        }
        return true;
    };

    // Ensure base directory exists

    std::string basePathStr = basePath.string();
    std::string pathExists;
    pathExists.reserve(basePathStr.length());

    size_t startIndex = (basePathStr[0] == '/') ? 1 : 0;

    for (size_t i = startIndex; i < basePathStr.length(); ++i)
    {
        if (basePathStr[i] == '/' || i == basePathStr.length() - 1)
        {
            // Include the current character if it's the last character and not a slash
            size_t endPos = (basePathStr[i] == '/' && i != basePathStr.length() - 1) ? i : i + 1;

            // Build the path incrementally
            pathExists = basePathStr.substr(0, endPos);

            // Skip empty paths (can happen with consecutive slashes)
            if (pathExists.empty() || pathExists.back() == '/')
            {
                continue;
            }

            if (!ensure(pathExists))
            {
                ChipLogError(Camera, "Failed to ensure directory exists: %s", pathExists.c_str());
                return false;
            }
        }
    }

    // Clean up previous session directory if it exists
    std::filesystem::remove_all(sessionDir);

    // Create session and track directories
    if (!ensure(sessionDir) || !ensure(trackDir))
    {
        return false;
    }

    return true;
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
            ChipLogProgress(Camera, "Found I-frame at PTS: %" PRId64, mVideoInfo.mVideoPts);
        }

        if (mClipInfo.mHasVideo && mFoundFirstIFramePts < 0)
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
        if (mClipInfo.mHasVideo && mFoundFirstIFramePts < 0 && mFoundFirstIFramePts <= mAudioInfo.mAudioPts)
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

    return (mClipInfo.mHasVideo && mFoundFirstIFramePts < 0) ? nullptr : packet;
}

void PushAVClipRecorder::Start()
{
    if (GetRecorderStatus())
    {
        ChipLogError(Camera, "ERROR: Recording is already running. Stop before starting again");
        return;
    }

    if (!EnsureDirectoryExists(mClipInfo.mOutputPath))
    {
        ChipLogError(Camera, "ERROR: Invalid output directory");
        Stop();
    }

    SetRecorderStatus(true);
    mWorkerThread = std::thread(&PushAVClipRecorder::StartClipRecording, this);
    ChipLogProgress(Camera, "Recording started for sessionID: %" PRIu64 " Track name: %s", mClipInfo.mSessionNumber,
                    mClipInfo.mTrackName.c_str());
}

void PushAVClipRecorder::Stop()
{
    if (GetRecorderStatus())
    {
        mPushAvStreamTransportManager->OnTriggerDeactivated(mFabricIndex, mClipInfo.mSessionGroup, mConnectionID);

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
    ChipLogProgress(Camera, "Recording stopped for sessionID: %" PRIu64 " Track name: %s", mClipInfo.mSessionNumber,
                    mClipInfo.mTrackName.c_str());
}

void PushAVClipRecorder::PushPacket(const uint8_t * data, size_t size, bool isVideo)
{
    if (!GetRecorderStatus())
    {
        ChipLogError(Camera, "ERROR: Push packet dropped as recorder is not active");
        return;
    }

    AVPacket * packet = CreatePacket(data, static_cast<int>(size), isVideo);
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
    double segSeconds = static_cast<double>(mClipInfo.mSegmentDurationMs) / 1000.0;
    // Set DASH/CMAF options
    av_opt_set(mFormatContext->priv_data, "increment_tc", "1", 0);
    av_opt_set(mFormatContext->priv_data, "use_timeline", "1", 0);

    if (mClipInfo.mChunkDurationMs == 0)
    {
        av_opt_set(mFormatContext->priv_data, "movflags", "+cmaf+dash+delay_moov+skip_sidx+skip_trailer", 0);
    }
    else
    {
        av_opt_set(mFormatContext->priv_data, "movflags", "+cmaf+dash+delay_moov+skip_sidx+skip_trailer+frag_custom", 0);
        av_opt_set(mFormatContext->priv_data, "frag_duration", std::to_string(mClipInfo.mChunkDurationMs).c_str(), 0);
    }

    av_opt_set(mFormatContext->priv_data, "seg_duration", std::to_string(segSeconds).c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "init_seg_name", initSegPattern.c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "media_seg_name", mediaSegPattern.c_str(), 0);
    av_opt_set_int(mFormatContext->priv_data, "use_template", 1, 0);
    av_dict_set_int(&options, "dash_segment_type", 1, 0);
    av_dict_set_int(&options, "use_timeline", 1, 0);
    av_dict_set(&options, "strict", "experimental", 0);
    av_dict_set(&options, "start_number", std::to_string(kSegmentIdOffset).c_str(), 0);
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
    if (!mClipInfo.mHasVideo && !mClipInfo.mHasAudio)
    {
        ChipLogError(Camera, "ERROR: No video or audio stream available. Not starting recording");
        return -1;
    }

    while (GetRecorderStatus())
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCondition.wait(
            lock, [this] { return !mVideoQueue.empty() || !mAudioQueue.empty() || !GetRecorderStatus() || mDeinitializeRecorder; });
        if (!GetRecorderStatus() || mDeinitializeRecorder)
        {
            ChipLogProgress(Camera, "Recorder thread received stop signal for sessionID: %" PRIu64 " Track name: %s",
                            mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
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

        std::string initSegName =
            mClipInfo.mTrackName + std::filesystem::path::preferred_separator + mClipInfo.mTrackName + ".init";
        std::string mediaSegName = mClipInfo.mTrackName + std::filesystem::path::preferred_separator + "segment_$Number%04d$.m4s";
        std::string mpdPrefix = "session_" + std::to_string(mClipInfo.mSessionNumber) + std::filesystem::path::preferred_separator +
            mClipInfo.mTrackName;

        mInputFormatContext          = avformat_alloc_context();
        int64_t avioCtxBufferSize    = (static_cast<int64_t>(mVideoInfo.mBitRate) * mClipInfo.mSegmentDurationMs) / (8 * 1000);
        uint8_t * mAvioContextBuffer = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(avioCtxBufferSize)));
        struct BufferData data       = { 0 };
        data.mPtr                    = static_cast<uint8_t *>(pkt->data);
        data.mSize                   = static_cast<size_t>(pkt->size);
        mInputFormatContext->pb =
            avio_alloc_context(mAvioContextBuffer, avioCtxBufferSize, 0, &data, &ReadPacket, nullptr, nullptr);
        mInputFormatContext->flags = AVFMT_FLAG_CUSTOM_IO;

        if (mClipInfo.mHasVideo)
        {
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
        }
        else if (mClipInfo.mHasAudio)
        {
            // For audio-only streams, we don't need to open input format or find stream info
            // We'll set up the audio stream directly in AddStreamToOutput
            ChipLogProgress(Camera, "Setting up audio-only stream, skipping input format initialization");
        }

        if (SetupOutput(mClipInfo.mOutputPath + mpdPrefix, initSegName, mediaSegName) < 0)
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

    pkt->pos = -1;

    if (pkt->pts < 0)
    {
        ChipLogError(Camera, "Warning Negative PTS detected: %" PRId64, pkt->pts);
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
    FinalizeCurrentClip(0);
    mVideoStream = nullptr;
    mAudioStream = nullptr;
    mMetadataSet = false;
    ChipLogProgress(Camera, "Cleanup completed");
}

std::string RenameSegmentFile(const std::string & originalPath)
{
    std::regex segment_regex(R"((.*/segment_)(\d+)(\.m4s))");
    std::smatch match;

    if (std::regex_match(originalPath, match, segment_regex) && match.size() == 4)
    {
        std::string pathPrefix = match[1].str();
        std::string numberStr  = match[2].str();
        std::string pathSuffix = match[3].str();

        char * endPtr;
        long originalNumber = std::strtol(numberStr.c_str(), &endPtr, 10);

        // Check for conversion errors: no digits were converted, or extra characters remain.
        if (endPtr == numberStr.c_str() || *endPtr != '\0' || originalNumber > INT_MAX || originalNumber < INT_MIN)
        {
            ChipLogDetail(Camera, "Invalid segment number format in path %s, not renaming.", originalPath.c_str());
            return originalPath;
        }

        int newNumber = static_cast<int>(originalNumber) + kSegmentIdOffset;

        if (newNumber > 9999)
        {
            ChipLogDetail(Camera, "Segment %s (new number %d) exceeds 9999, stopping clip recording", originalPath.c_str(),
                          newNumber);
            return originalPath;
        }

        char newPathBuffer[1024];
        snprintf(newPathBuffer, sizeof(newPathBuffer), "%s%04d%s", pathPrefix.c_str(), newNumber, pathSuffix.c_str());
        std::string newPath = newPathBuffer;

        std::error_code error;
        std::filesystem::rename(originalPath.c_str(), newPath.c_str(), error);
        if (error.value() == 0)
        {
            ChipLogDetail(Camera, "Renamed segment %s to %s", originalPath.c_str(), newPath.c_str());
            return newPath;
        }
        else
        {
            ChipLogDetail(Camera, "Failed to rename segment %s to %s, error: %d", originalPath.c_str(), newPath.c_str(),
                          error.value());
            return originalPath;
        }
    }

    ChipLogDetail(Camera, "Path %s does not match expected segment format, not renaming.", originalPath.c_str());
    return originalPath;
}

void UpdateMPDStartNumber(const std::string & mpdPath)
{
    std::ifstream file(mpdPath);
    if (!file)
    {
        ChipLogError(Camera, "ERROR: Failed to open MPD file for reading: %s", mpdPath.c_str());
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Replace startNumber="<digits>" with startNumber="kMPDDefaultStartNumber"
    std::regex startNumberRegex(R"(startNumber="\d+")");
    int newStartNumber      = kMPDDefaultStartNumber;
    std::string replacement = "startNumber=\"" + std::to_string(newStartNumber) + "\"";
    std::string newContent  = std::regex_replace(content, startNumberRegex, replacement);

    std::ofstream outFile(mpdPath);
    if (!outFile)
    {
        ChipLogError(Camera, "ERROR: Failed to open MPD file for writing: %s", mpdPath.c_str());
        return;
    }
    outFile << newContent;
    outFile.close();

    ChipLogProgress(Camera, "Successfully updated startNumber to 1001 in MPD file: %s", mpdPath.c_str());
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
    const int64_t remainingDuration = mClipInfo.mInitialDurationS - mClipInfo.mElapsedTimeS + (mClipInfo.mPreRollLengthMs / 1000);
    int64_t clipDuration            = 0;

    if (remainingDuration <= 0)
    {
        ChipLogError(Camera,
                     "Invalid remaining duration: %" PRId64 " for sessionID: %" PRIu64 " Track name: %s - stopping recording",
                     remainingDuration, mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
        reason = 1; // Set reason to trigger existing stop logic
    }
    else
    {
        const auto & timeBase = mClipInfo.mHasVideo ? mVideoInfo.mVideoTimeBase : mAudioInfo.mAudioTimeBase;
        if (timeBase.num == 0)
        {
            ChipLogError(Camera, "Invalid timebase (num=0) for %s stream in sessionID: %" PRIu64 " Track name: %s",
                         mClipInfo.mHasVideo ? "video" : "audio", mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
        }
        else
        {
            clipDuration = (remainingDuration * timeBase.den) / timeBase.num;
        }
    }

    // Pre-calculate common path components
    std::filesystem::path basePath = std::filesystem::path(mClipInfo.mOutputPath) /
        ("session_" + std::to_string(mClipInfo.mSessionNumber)) / mClipInfo.mTrackName;

    if (reason || ((clipLengthInPTS >= clipDuration) && (mClipInfo.mTriggerType != 2)))
    {
        ChipLogDetail(Camera, "Clip record completed, finalizing clip for sessionID: %" PRIu64 " Track name: %s, Reason: %s",
                      mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str(),
                      (reason == 0) ? "End of clip reached" : "Error occurred");
        Stop();
        mCurrentClipStartPts = AV_NOPTS_VALUE;
    }

    // Helper function for safe path formatting using std::filesystem
    auto make_segment_path = [&](int number) -> std::filesystem::path {
        std::ostringstream oss;
        oss << "segment_" << std::setw(4) << std::setfill('0') << number << ".m4s";
        return basePath / oss.str();
    };

    std::filesystem::path segment_path = make_segment_path(mUploadSegmentID);
    while (std::filesystem::exists(segment_path) && !std::filesystem::exists(segment_path.string() + ".tmp"))
    {
        mUploadMPD                       = true;
        std::string renamed_segment_path = RenameSegmentFile(segment_path.string());
        CheckAndUploadFile(renamed_segment_path);
        mUploadSegmentID++;
        segment_path = make_segment_path(mUploadSegmentID);
    }

    // Handle MPD and init file upload
    if (mUploadMPD)
    {
        std::filesystem::path mpd_path = basePath;
        mpd_path += ".mpd";
        if (std::filesystem::exists(mpd_path) && !std::filesystem::exists(mpd_path.string() + ".tmp"))
        {
            mUploader->setMPDPath(std::make_pair(mpd_path.string(), mClipInfo.mUrl));
            UpdateMPDStartNumber(mpd_path.string());
            CheckAndUploadFile(mpd_path.string());
            mUploadMPD = false; // Reset flag after successful upload
        }

        // Handle init segment upload if needed
        if (!mUploadedInitSegment)
        {
            std::filesystem::path init_path = basePath / (mClipInfo.mTrackName + ".init");
            if (std::filesystem::exists(init_path) && !std::filesystem::exists(init_path.string() + ".tmp"))
            {
                CheckAndUploadFile(init_path.string());
            }
            mUploadedInitSegment = true;
        }
    }
}

bool PushAVClipRecorder::CheckAndUploadFile(std::string filename)
{
    mUploader->AddUploadData(filename, mClipInfo.mUrl);
    return true;
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
