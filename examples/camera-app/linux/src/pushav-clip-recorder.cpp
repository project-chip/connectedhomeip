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
#include <regex>
#include <sys/stat.h>

constexpr int kSegmentIdOffset       = 1000;
constexpr int kMPDDefaultStartNumber = 1001;
constexpr int kInitialSegmentId      = 1;

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
    mFormatContext          = nullptr;
    mInputFormatContext     = nullptr;
    mVideoStream            = nullptr;
    mAudioStream            = nullptr;
    mAudioEncoderContext    = nullptr;
    int streamIndex         = 0;
    mLastVideoPts           = 0;
    mLastAudioPts           = 0;
    mClipInfo.mClipStartPTS = 0;
    mMetadataSet            = false;
    mDeinitializeRecorder   = false;
    mUploadMPD              = true;
    mCurrentClipStartPts    = AV_NOPTS_VALUE;
    currentPts              = AV_NOPTS_VALUE;

    mVideoInfo.mVideoOutputStreamId = -1;
    mAudioInfo.mAudioOutputStreamId = -1;

    if (mClipInfo.mHasVideo)
    {
        mUploadSegmentID.push_back(kInitialSegmentId);
        mUploadedInitSegment.push_back(false);
        mStreamIdNameMap.push_back(mVideoInfo.mVideoStreamName);
        mVideoInfo.mVideoOutputStreamId = streamIndex++;
    }
    if (mClipInfo.mHasAudio)
    {
        mUploadSegmentID.push_back(kInitialSegmentId);
        mUploadedInitSegment.push_back(false);
        mStreamIdNameMap.push_back(mAudioInfo.mAudioStreamName);
        mAudioInfo.mAudioOutputStreamId = streamIndex++;
    }
    SetRecorderStatus(false); // Start off as not running
    mUploader->setStreamIdNameMap(mStreamIdNameMap);
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

    std::filesystem::path mpdPath = mUploadFileBasePath / "index.mpd";
    if (IsFileReadyForUpload(mpdPath))
    {
        UpdateMPDParams(mpdPath);
        ChipLogProgress(Camera, "Uploading final MPD: %s for track: %s, sessionID: %lu, connectionID: %u", mpdPath.c_str(),
                        mClipInfo.mTrackName.c_str(), mClipInfo.mSessionNumber, mConnectionID);
        CheckAndUploadFile(mpdPath.string());
    }
}

bool PushAVClipRecorder::EnsureDirectoryExists(const std::string & path)
{
    // Base output path
    std::filesystem::path basePath(path);
    mUploadFileBasePath = basePath / ("session_" + std::to_string(mClipInfo.mSessionNumber));

    // Helper lambda to ensure a directory exists and is writable, creating it with mode 0755
    auto ensure = [&](const std::filesystem::path & p) -> bool {
        std::error_code ec;
        if (!std::filesystem::exists(p, ec))
        {
            if (!std::filesystem::create_directories(p, ec))
            {
                ChipLogError(Camera, "Failed to create directory: %s, error code: %d (%s), category: %s", p.c_str(), ec.value(),
                             ec.message().c_str(), ec.category().name());
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
            ChipLogError(Camera, "Path is not a directory: %s, error code: %d (%s), category: %s", p.c_str(), ec.value(),
                         ec.message().c_str(), ec.category().name());
            return false;
        }

        auto perms = std::filesystem::status(p, ec).permissions();
        if ((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
        {
            ChipLogError(Camera, "Directory is not writable: %s (permissions: %o)", p.c_str(), static_cast<unsigned int>(perms));
            return false;
        }

        return true;
    };

    // Ensure base directory exists
    if (!ensure(basePath))
    {
        ChipLogError(Camera, "Failed to ensure base directory exists: %s", basePath.c_str());
        return false;
    }

    // Clean up previous session directory if it exists
    std::filesystem::remove_all(mUploadFileBasePath);

    // Create session and track directories
    if (!ensure(mUploadFileBasePath))
    {
        ChipLogError(Camera, "Failed to ensure session directory exists: %s", mUploadFileBasePath.c_str());
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

AVPacket * PushAVClipRecorder::CreatePacket(const uint8_t * data, int size, int64_t timestampMs, bool isVideo)
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
            if (mClipInfo.mClipStartPTS == 0)
                mClipInfo.mClipStartPTS = timestampMs;
            packet->flags = AV_PKT_FLAG_KEY;

            ChipLogProgress(Camera, "Found I-frame at timestamp: %ld ms", timestampMs);
        }

        if (mClipInfo.mHasVideo && mClipInfo.mClipStartPTS == 0)
        {
            ChipLogError(Camera, "ERROR: First frame is not an I-frame. Dropping packet.");
            av_packet_free(&packet);
            return nullptr;
        }

        if (mVideoInfo.mVideoTimeBase.num != 0)
        {
            mLastVideoPts = timestampMs;

            // Normalize timestamp relative to clip start
            int64_t normalizedTimestampMs = timestampMs - mClipInfo.mClipStartPTS;
            packet->pts                   = av_rescale_q(normalizedTimestampMs, (AVRational){ 1, 1000 }, mVideoInfo.mVideoTimeBase);
            packet->dts                   = packet->pts;
        }
        else
        {
            ChipLogError(Camera, "ERROR: Invalid video timebase (num=0)");
            av_packet_free(&packet);
            return nullptr;
        }

        packet->stream_index = mVideoInfo.mVideoOutputStreamId;
    }
    else
    {
        if (mClipInfo.mHasVideo && mClipInfo.mClipStartPTS == 0)
        {
            av_packet_free(&packet);
            return nullptr;
        }

        if (!mClipInfo.mHasVideo && mClipInfo.mClipStartPTS == 0)
        {
            mClipInfo.mClipStartPTS = timestampMs;
        }
        if (mAudioInfo.mAudioTimeBase.num != 0)
        {
            mLastAudioPts = timestampMs;

            // Normalize timestamp relative to clip start
            int64_t normalizedTimestampMs = timestampMs - mClipInfo.mClipStartPTS;
            packet->pts                   = av_rescale_q(normalizedTimestampMs, (AVRational){ 1, 1000 }, mAudioInfo.mAudioTimeBase);
            packet->dts                   = packet->pts;
        }
        else
        {
            ChipLogError(Camera, "ERROR: Invalid audio timebase (num=0)");
            av_packet_free(&packet);
            return nullptr;
        }

        packet->stream_index = mAudioInfo.mAudioOutputStreamId;
    }

    if (packet->pts < 0)
    {
        return nullptr;
    }

    return packet;
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
        mDeinitializeRecorder = true;
    }

    if (mWorkerThread.joinable())
    {
        mWorkerThread.join();
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

void PushAVClipRecorder::PushPacket(const uint8_t * data, size_t size, int64_t timestampMs, bool isVideo)
{
    if (!GetRecorderStatus())
    {
        ChipLogError(Camera, "ERROR: Push packet dropped as recorder is not active");
        return;
    }

    AVPacket * packet = CreatePacket(data, static_cast<int>(size), timestampMs, isVideo);
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

RecorderStatus PushAVClipRecorder::SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern,
                                               const std::string & mediaSegPattern)
{
    const std::string mpdFilename = outputPrefix + "/index.mpd";
    if (avformat_alloc_output_context2(&mFormatContext, nullptr, nullptr, mpdFilename.c_str()) < 0)
    {
        ChipLogError(Camera, "ERROR: Failed to allocate output context");
        return RecorderStatus::kFail;
    }
    if (!mFormatContext)
    {
        ChipLogError(Camera, "ERROR: Output context is null");
        return RecorderStatus::kFail;
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
    if (mClipInfo.mHasVideo && (AddStreamToOutput(AVMEDIA_TYPE_VIDEO) == RecorderStatus::kFail))
    {
        ChipLogError(Camera, "ERROR: adding video stream to output");
        return RecorderStatus::kFail;
    }
    if (mClipInfo.mHasAudio && (AddStreamToOutput(AVMEDIA_TYPE_AUDIO) == RecorderStatus::kFail))
    {
        ChipLogError(Camera, "ERROR: adding video stream to output");
        return RecorderStatus::kFail;
    }

    if (!(mFormatContext->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&mFormatContext->pb, mpdFilename.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            ChipLogError(Camera, "ERROR: Failed to open output file: %s", mpdFilename.c_str());
            return RecorderStatus::kFail;
        }
    }

    if (avformat_write_header(mFormatContext, &options) < 0)
    {
        ChipLogError(Camera, "Error: writing output header");
        return RecorderStatus::kFail;
    }
    return RecorderStatus::kSuccess;
}

RecorderStatus PushAVClipRecorder::StartClipRecording()
{
    if (!mClipInfo.mHasVideo && !mClipInfo.mHasAudio)
    {
        ChipLogError(Camera, "ERROR: No video or audio stream available. Not starting recording");
        return RecorderStatus::kFail;
    }

    RecorderStatus result = RecorderStatus::kSuccess;

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
        RecorderStatus status = ProcessBuffersAndWrite();
        if (status == RecorderStatus::kFail)
        {
            ChipLogError(Camera, "Error processing buffers and writing");
            result = RecorderStatus::kFail;
            Stop();
        }
        else if (status == RecorderStatus::kWarning)
        {
            ChipLogProgress(Camera, "Warning occurred while processing buffers and writing");
        }
    }
    CleanupOutput();
    ChipLogProgress(Camera, "Recorder thread closing");
    return result;
}

RecorderStatus PushAVClipRecorder::AddStreamToOutput(AVMediaType type)
{
    if (type == AVMEDIA_TYPE_VIDEO)
    {
        mVideoStream = avformat_new_stream(mFormatContext, nullptr);
        if (avcodec_parameters_copy(mVideoStream->codecpar, mInputFormatContext->streams[0]->codecpar) < 0)
        {
            ChipLogError(Camera, "ERROR: Failed to copy codec parameters for media type: %d", type);
            return RecorderStatus::kFail;
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
            return RecorderStatus::kFail;
        }
        const AVCodec * audioCodec = avcodec_find_encoder(mAudioInfo.mAudioCodecId);
        if (!audioCodec)
        {
            ChipLogError(Camera, "ERROR: Audio encoder not found");
            return RecorderStatus::kFail;
        }
        mAudioEncoderContext = avcodec_alloc_context3(audioCodec);
        if (!mAudioEncoderContext)
        {
            ChipLogError(Camera, "Error: failed to allocate the encoder context");
            return RecorderStatus::kFail;
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
            return RecorderStatus::kFail;
        }
        if (avcodec_parameters_from_context(mAudioStream->codecpar, mAudioEncoderContext) < 0)
        {
            ChipLogError(Camera, "Error: Failed to copy encoder parameters to audio output stream");
            return RecorderStatus::kFail;
        }
        if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        {
            mAudioEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    return RecorderStatus::kSuccess;
}

RecorderStatus PushAVClipRecorder::ProcessBuffersAndWrite()
{
    if (mVideoQueue.empty() && mAudioQueue.empty())
    {
        return RecorderStatus::kWarning;
    }

    bool useVideo  = false;
    AVPacket * pkt = nullptr;

    if (!mVideoQueue.empty() && !mAudioQueue.empty())
    {
        AVPacket * videoPkt = mVideoQueue.front();
        AVPacket * audioPkt = mAudioQueue.front();

        const bool videoHasValidPts = (videoPkt->pts != AV_NOPTS_VALUE);
        const bool audioHasValidPts = (audioPkt->pts != AV_NOPTS_VALUE);

        if (videoHasValidPts && audioHasValidPts)
        {
            useVideo = (videoPkt->pts <= audioPkt->pts);
        }
        else
        {
            useVideo = videoHasValidPts;
        }
        pkt = useVideo ? videoPkt : audioPkt;
    }
    else if (!mVideoQueue.empty())
    {
        pkt      = mVideoQueue.front();
        useVideo = true;
    }
    else
    {
        pkt      = mAudioQueue.front();
        useVideo = false;
    }

    if (mMetadataSet == false)
    {
        std::string initSegName  = "#__$RepresentationID$__#.init";
        std::string mediaSegName = "#__$RepresentationID$__#segment_$Number%04d$.m4s";
        mInputFormatContext      = avformat_alloc_context();
        int64_t avioCtxBufferSize =
            (static_cast<int64_t>(mVideoInfo.mBitRate + mAudioInfo.mBitRate) * mClipInfo.mSegmentDurationMs) / (8 * 1000);
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
                return RecorderStatus::kFail;
            }

            if (avformat_find_stream_info(mInputFormatContext, nullptr) < 0)
            {
                ChipLogError(Camera, "Error: Failed to find stream info for video");
                return RecorderStatus::kFail;
            }
        }
        else if (mClipInfo.mHasAudio)
        {
            // For audio-only streams, we don't need to open input format or find stream info
            // We'll set up the audio stream directly in AddStreamToOutput
            ChipLogProgress(Camera, "Setting up audio-only stream, skipping input format initialization");
        }

        if (SetupOutput(mUploadFileBasePath, initSegName, mediaSegName) == RecorderStatus::kFail)
        {
            ChipLogError(Camera, "Error: setting up output");
            return RecorderStatus::kFail;
        }
        if (!mFormatContext)
        {
            ChipLogError(Camera, "Error: Output context not initialized. Skipping packet");
            return RecorderStatus::kFail;
        }
        mMetadataSet = true;
    }

    if (pkt->pts == AV_NOPTS_VALUE && pkt->dts == AV_NOPTS_VALUE)
    {
        ChipLogError(Camera, "Warning packet has no valid timestamps");
        av_packet_unref(pkt);
        return RecorderStatus::kSuccess;
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
        FinalizeCurrentClip(ClipFinalizationReason::kErrorOccurred);
        return RecorderStatus::kWarning;
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
    FinalizeCurrentClip(ClipFinalizationReason::kSegmentUploadCheck);

    return RecorderStatus::kSuccess;
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
    FinalizeCurrentClip(ClipFinalizationReason::kCleanupUpload);
    mVideoStream = nullptr;
    mAudioStream = nullptr;
    mMetadataSet = false;
    ChipLogProgress(Camera, "Cleanup completed");
}

void PushAVClipRecorder::UpdateMPDParams(const std::string & mpdPath)
{
    std::ifstream inFile(mpdPath);
    if (!inFile)
    {
        ChipLogError(Camera, "ERROR: Failed to open MPD file for reading: %s", mpdPath.c_str());
        return;
    }

    // Define the exact pattern to find
    const std::string searchPattern =
        R"(initialization="#__$RepresentationID$__#.init" media="#__$RepresentationID$__#segment_$Number%04d$.m4s" startNumber=")";

    std::vector<std::string> lines;
    std::string line;
    size_t streamIndex    = 0;
    bool foundAndReplaced = false;

    // Read file line by line
    while (std::getline(inFile, line))
    {
        size_t pos = line.find(searchPattern);
        while (pos != std::string::npos && streamIndex < mStreamIdNameMap.size())
        {
            const std::string & streamName = mStreamIdNameMap[static_cast<int>(streamIndex)];

            // Find the startNumber value
            size_t startNumberStart = line.find("startNumber=\"", pos);
            size_t startNumberEnd   = line.find("\"", startNumberStart + 13);

            std::string replacement;

            if (startNumberStart != std::string::npos && startNumberEnd != std::string::npos)
            {
                // Replace the entire pattern
                replacement = "initialization=\"" + streamName + "/" + streamName + ".init\" media=\"" + streamName +
                    "/segment_$Number%04d$.m4s\" startNumber=\"" + std::to_string(kMPDDefaultStartNumber) + "\"";

                line.replace(pos, startNumberEnd - pos + 1, replacement);
                foundAndReplaced = true;
                streamIndex++;
            }

            // Look for next occurrence in the same line
            pos = line.find(searchPattern, pos + replacement.length());
        }
        lines.push_back(line);
    }
    inFile.close();

    // Write the modified lines back to the file
    if (foundAndReplaced)
    {
        std::ofstream outFile(mpdPath);
        if (!outFile)
        {
            ChipLogError(Camera, "ERROR: Failed to open MPD file for writing: %s", mpdPath.c_str());
            return;
        }

        for (size_t i = 0; i < lines.size(); ++i)
        {
            outFile << lines[i];
            if (i < lines.size() - 1) // Don't add newline after last line
                outFile << "\n";
        }
        outFile.close();
        ChipLogProgress(Camera, "Successfully updated stream info in MPD file: %s", mpdPath.c_str());
    }
    else
    {
        ChipLogProgress(Camera, "Pattern not found in MPD file, no changes made: %s", mpdPath.c_str());
    }
}

bool PushAVClipRecorder::IsFileReadyForUpload(const std::filesystem::path & path) const
{
    return std::filesystem::exists(path) && !std::filesystem::exists(path.string() + ".tmp");
}

/**
 * @brief Finalizes the current clip and starts a new one.
 *
 * Writes the trailer of the current clip and initializes a new output file.
 */

void PushAVClipRecorder::FinalizeCurrentClip(ClipFinalizationReason reason)
{
    int64_t clipLengthInPTS = currentPts - mCurrentClipStartPts;
    // Final duration has to be (clipDuration + preRollLen) seconds
    const int64_t remainingDuration =
        mClipInfo.mMotionDetectedDurationS - mClipInfo.mElapsedTimeS + (mClipInfo.mPreRollLengthMs / 1000);
    int64_t clipDuration = 0;

    if ((mClipInfo.mTriggerType != 2) && remainingDuration <= 0)
    {
        ChipLogError(Camera,
                     "Invalid remaining duration: %" PRId64 " for sessionID: %" PRIu64 " Track name: %s - stopping recording",
                     remainingDuration, mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
        reason = ClipFinalizationReason::kErrorOccurred;
    }
    else
    {
        const auto & timeBase = mClipInfo.mHasVideo ? mVideoInfo.mVideoTimeBase : mAudioInfo.mAudioTimeBase;
        if (timeBase.num == 0)
        {
            ChipLogError(Camera,
                         "Invalid timebase (num=0) for %s stream in sessionID: %" PRIu64 " Track name: %s - stopping recording",
                         mClipInfo.mHasVideo ? "video" : "audio", mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str());
            reason = ClipFinalizationReason::kErrorOccurred;
        }
        else
        {
            clipDuration = (remainingDuration * timeBase.den) / timeBase.num;
        }
    }

    // Pre-calculate common path components
    std::filesystem::path basePath = std::filesystem::path(mClipInfo.mOutputPath) /
        ("session_" + std::to_string(mClipInfo.mSessionNumber)) / mClipInfo.mTrackName;

    const char * reasonStr = "Unknown";
    switch (reason)
    {
    case ClipFinalizationReason::kErrorOccurred:
        reasonStr = "Error occurred";
        break;
    case ClipFinalizationReason::kSegmentUploadCheck:
        reasonStr = "Segment upload check";
        break;
    case ClipFinalizationReason::kCleanupUpload:
        reasonStr = "Cleanup upload";
        break;
    }

    bool shouldFinalize =
        (reason == ClipFinalizationReason::kErrorOccurred) || ((clipLengthInPTS >= clipDuration) && (mClipInfo.mTriggerType != 2));

    if (shouldFinalize)
    {
        ChipLogDetail(Camera, "Clip record completed, finalizing clip for sessionID: %" PRIu64 " Track name: %s, Reason: %s",
                      mClipInfo.mSessionNumber, mClipInfo.mTrackName.c_str(), reasonStr);
        Stop();
        mCurrentClipStartPts = AV_NOPTS_VALUE;
    }

    //  Helper function for safe path formatting using std::filesystem
    auto make_segment_path = [&](int stream, int number) -> std::filesystem::path {
        std::ostringstream oss;
        oss << "#__" << stream << "__#segment_" << std::setw(4) << std::setfill('0') << number << ".m4s";
        return mUploadFileBasePath / oss.str();
    };

    std::filesystem::path mpdPath = mUploadFileBasePath / "index.mpd";

    // Wait for the first segment (segment_0001.m4s) for any stream to be created before starting any uploads
    if (!firstSegmentReady)
    {
        for (size_t i = 0; i < mUploadSegmentID.size(); i++)
        {
            if (mUploadSegmentID[i] == 1 && IsFileReadyForUpload(make_segment_path(i, 1)))
            {
                firstSegmentReady = true;
                break;
            }
        }
    }
    if (!firstSegmentReady)
        return;

    if (mUploadMPD)
    {
        if (IsFileReadyForUpload(mpdPath))
        {
            UpdateMPDParams(mpdPath.string());
            CheckAndUploadFile(mpdPath.string());
            mUploadMPD = false; // Reset flag after successful upload
        }
        else
        {
            return; // Wait for MPD to be ready before proceeding
        }
    }

    for (size_t i = 0; i < mUploadSegmentID.size(); i++)
    {
        if (!mUploadedInitSegment[i])
        {
            const std::filesystem::path init_path = mUploadFileBasePath / ("#__" + std::to_string(i) + "__#" + ".init");
            if (IsFileReadyForUpload(init_path))
            {
                CheckAndUploadFile(init_path.string());
                mUploadedInitSegment[i] = true;
            }
            else
            {
                return; // Wait for init segment to be ready before proceeding
            }
        }
    }
    for (size_t i = 0; i < mUploadSegmentID.size(); i++)
    {
        std::filesystem::path segment_path = make_segment_path(i, mUploadSegmentID[i]);
        while (IsFileReadyForUpload(segment_path))
        {
            CheckAndUploadFile(segment_path.string());
            mUploadSegmentID[i]++;
            // For testing purpose
#ifdef TEST_UPLOAD_MPD_AFTER_EVERY_SEGMENT
            mUploadMPD = true;
#endif
            segment_path = make_segment_path(i, mUploadSegmentID[i]);
        }
    }

    // For testing purpose
#ifdef TEST_UPLOAD_MPD_AFTER_EVERY_SEGMENT
    if (IsFileReadyForUpload(mpdPath) && mUploadMPD)
    {
        UpdateMPDParams(mpdPath.string());
        CheckAndUploadFile(mpdPath.string());
        mUploadMPD = false;
    }
#endif
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
