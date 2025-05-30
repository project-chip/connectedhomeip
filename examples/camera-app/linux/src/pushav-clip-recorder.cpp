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
#include <sys/stat.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

#define IS_H264_FRAME_NALU_HEAD(frame)                                                                                             \
    (((frame)[0] == 0x00) && ((frame)[1] == 0x00) && (((frame)[2] == 0x01) || (((frame)[2] == 0x00) && ((frame)[3] == 0x01))))

AVDictionary * options = NULL;

PushAVClipRecorder::PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo) :
    mClipInfo(aClipInfo), mAudioInfo(aAudioInfo), mVideoInfo(aVideoInfo)
{

    mClipInfo.mRecorderId = std::to_string(mClipInfo.mVideoStreamId) + "-" + std::to_string(mClipInfo.mAudioStreamId);
    int streamIndex       = 0;
    mRunning              = false;
    mMetadataSet          = false;

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
    ChipLogProgress(Camera, "PushAVClipRecorder initialized with ID: %s, output path: %s", mClipInfo.mRecorderId.c_str(),
                    mClipInfo.mOutputPath.c_str());
}

PushAVClipRecorder::~PushAVClipRecorder()
{
    Stop();
}

static int read_packet(void * opaque, uint8_t * buf, int bufSize)
{
    struct BufferData * bd = (struct BufferData *) opaque;
    bufSize                = FFMIN(bufSize, bd->mSize);

    if (!bufSize)
        return AVERROR_EOF;

    /* copy internal buffer data to buf */
    memcpy(buf, bd->mPtr, bufSize);
    bd->mPtr += bufSize;
    bd->mSize -= bufSize;

    return bufSize;
}

bool PushAVClipRecorder::IsH264IFrame(const uint8_t * data, unsigned int length)
{
    unsigned int idx = 0;
    int frame_type   = 0;
    int found_sps    = 0;
    int found_pps    = 0;
    int found_idr    = 0;
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
                frame_type = data[idx + 3] & 0x1f;
            else if ((data[idx + 2] == 0x00) && (data[idx + 3] == 0x01))
                frame_type = data[idx + 4] & 0x1f;

            if (frame_type == 7)
            {
                found_sps = 1;
            }
            else if (frame_type == 8)
            {
                found_pps = 1;
            }
            else if (frame_type == 5)
            {
                found_idr = 1;
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

    if (found_sps == 1 && found_pps == 1 && found_idr == 1)
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
    packet->data = (uint8_t *) av_malloc(size);
    if (!packet->data)
    {
        ChipLogError(Camera, "ERROR: AVPacket data allocation failed!");
        av_packet_free(&packet);
        return nullptr;
    }
    memcpy(packet->data, data, size);
    packet->size = size;
    if (isVideo)
    {
        if (IsH264IFrame(data, size))
        {
            printf("First I-frame detected! at pts : %ld\n", mVideoInfo.mVideoPts);
            mFoundFirstIFramePts = mVideoInfo.mVideoPts;
            packet->flags        = AV_PKT_FLAG_KEY;
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
        mAudioInfo.mAudioDts += mAudioInfo.mAudioFrameDuration;
        mAudioInfo.mAudioPts += mAudioInfo.mAudioFrameDuration;
        if (mFoundFirstIFramePts < 0 && mFoundFirstIFramePts <= mAudioInfo.mAudioPts)
        {
            ChipLogError(Camera, "ERROR: frames will be dropped till an Iframe is recived \n");
            av_packet_free(&packet);
            return nullptr;
        }
        packet->pts          = mAudioInfo.mAudioPts;
        packet->dts          = mAudioInfo.mAudioDts;
        packet->stream_index = mAudioInfo.mAudioStreamIndex;
        packet->duration     = mAudioInfo.mAudioFrameDuration;
    }
    return (mFoundFirstIFramePts < 0) ? nullptr : packet;
}

void PushAVClipRecorder::Start()
{
    if (mRunning == true)
    {
        ChipLogError(Camera, "ERROR: Recording is already running. Stop before starting again");
        return;
    }
    mRunning       = true;
    mWorkerThread  = std::thread(&PushAVClipRecorder::StartClipRecording, this);
    mWorkerRunning = true;
    mUploader.Start();
    ChipLogProgress(Camera, "Recording started for ID: %s", mClipInfo.mRecorderId.c_str());
}

void PushAVClipRecorder::Stop()
{
    if (mRunning)
    {
        mRunning = false;
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
        ChipLogProgress(Camera, "Recorder stopping for ID: %s", mClipInfo.mRecorderId.c_str());
    }
    else
    {
        printf("Error recording is not running");
    }
    CleanupOutput();
}

void PushAVClipRecorder::PushPacket(const char * data, size_t size, bool isVideo)
{
    if (!mWorkerRunning)
    {
        ChipLogError(Camera, "ERROR: Push packet dropped as recorder is not ready");
        return;
    }
    if (!mRunning)
    {
        ChipLogError(Camera, "ERROR: Push packet dropped as recorder is not active");
        return;
    }

    AVPacket * packet = CreatePacket((const uint8_t *) data, size, isVideo);
    if (!packet)
    {
        ChipLogError(Camera, "ERROR: PACKET DROPPED!");
        return;
    }

    std::lock_guard<std::mutex> lock(mQueueMutex);
    std::queue<AVPacket *> & queue = isVideo ? mVideoQueue : mAudioQueue;
    if (queue.size() >= kMaxQueueSize)
    {
        ChipLogProgress(Camera, "Queue full. Dropping packet");
        av_packet_free(&packet);
        return;
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
    // Set DASH/CMAF options
    av_opt_set(mFormatContext->priv_data, "increment_tc", "1", 0);
    av_opt_set(mFormatContext->priv_data, "movflags", "cmaf+dash+empty_moov+default_base_moof+frag_custom+frag_keyframe", 0);
    av_opt_set(mFormatContext->priv_data, "seg_duration", std::to_string(mClipInfo.mChunkDuration).c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "init_seg_name", initSegPattern.c_str(), 0);
    av_opt_set(mFormatContext->priv_data, "media_seg_name", mediaSegPattern.c_str(), 0);

    av_opt_set_int(mFormatContext->priv_data, "use_template", 1, 0);
    av_dict_set_int(&options, "frag_duration", mClipInfo.mChunkDuration * 1000000, 0);
    av_dict_set_int(&options, "dash_segment_type", 1, 0);
    av_dict_set_int(&options, "use_timeline", 1, 0);
    av_dict_set(&options, "strict", "experimental", 0);

    if (mClipInfo.mHasVideo && AddStreamToOutput(AVMEDIA_TYPE_VIDEO) < 0)
    {
        ChipLogError(Camera, "ERROR: adding video stream to output");
        return -1;
    }
    if (mClipInfo.mHasAudio && AddStreamToOutput(AVMEDIA_TYPE_AUDIO) < 0)
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
}

int PushAVClipRecorder::StartClipRecording()
{
    av_log_set_level(AV_LOG_DEBUG);

    if (!mClipInfo.mHasVideo)
    {
        ChipLogError(Camera, "ERROR: No video stream available. Stopping recording");
        return -1;
    }

    while (mRunning)
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCondition.wait(lock, [this] { return !mVideoQueue.empty() || !mAudioQueue.empty(); });
        ProcessBuffersAndWrite();
    }

    ChipLogProgress(Camera, "Recorder thread closing");
    mWorkerRunning = false;
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
        mVideoStream->avg_frame_rate      = (AVRational) { mVideoInfo.mFrameRate, 1 };
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
        mAudioEncoderContext->sample_rate           = mAudioInfo.mSampleRate;
        mAudioEncoderContext->channels              = mAudioInfo.mChannels;
        mAudioEncoderContext->channel_layout        = av_get_default_channel_layout(mAudioEncoderContext->channels);
        mAudioEncoderContext->bit_rate              = mAudioInfo.mBitRate;
        mAudioEncoderContext->sample_fmt            = AV_SAMPLE_FMT_FLTP;
        mAudioEncoderContext->time_base             = (AVRational) { 1, mAudioInfo.mSampleRate };
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
        return false;
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
        return false;
    }

    if (mMetadataSet == false)
    {
        std::string prefix           = mClipInfo.mRecorderId + "_clip_" + std::to_string(mClipInfo.mClipId);
        std::string initSegName      = prefix + "_init-stream$RepresentationID$.fmp4";
        std::string mediaSegName     = prefix + "_chunk-stream$RepresentationID$-$Number%05d$.cmfv";
        mInputFormatContext          = avformat_alloc_context();
        int avioCtxBufferSize        = 1048576; // 1MB
        uint8_t * mAvioContextBuffer = (uint8_t *) av_malloc(avioCtxBufferSize);
        struct BufferData data       = { 0 };
        data.mPtr                    = (uint8_t *) pkt->data;
        data.mSize                   = pkt->size;
        mAvioContext = avio_alloc_context(mAvioContextBuffer, avioCtxBufferSize, 0, &data, &read_packet, nullptr, nullptr);
        mInputFormatContext->pb    = mAvioContext;
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
        mMetadataSet = true;
    }
    if (!mFormatContext)
    {
        ChipLogError(Camera, "Error: Output context not initialized. Skipping packet");
        Stop();
        return -1;
    }

    AVStream * out_stream     = useVideo ? mVideoStream : mAudioStream;
    AVRational outputTimeBase = useVideo ? mVideoInfo.mVideoTimeBase : mAudioInfo.mAudioTimeBase;

    if (pkt->pts == AV_NOPTS_VALUE && pkt->dts == AV_NOPTS_VALUE)
    {
        printf("Warning packet has no valid timestamps\n");
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
        printf("Warning Negative PTS detected: %" PRId64 "\n", pkt->pts);
        pkt->pts = (pkt->dts != AV_NOPTS_VALUE) ? pkt->dts : 0;
    }
    if (av_interleaved_write_frame(mFormatContext, pkt) < 0)
    {
        printf("Error writing frame to output file");
        FinalizeCurrentClip(1);
        return -1;
    }

    if (useVideo)
    {
        mVideoQueue.pop();
    }
    else
    {
        mAudioQueue.pop();
    }

    av_packet_free(&pkt);
    FinalizeCurrentClip(0);

    return 0;
}

void PushAVClipRecorder::CleanupOutput()
{
    if (mFormatContext)
    {
        av_write_trailer(mFormatContext);
        if (!(mFormatContext->oformat->flags & AVFMT_NOFILE) && mFormatContext->pb)
        {
            avio_closep(&mFormatContext->pb);
        }
        avformat_free_context(mFormatContext);
        mFormatContext = nullptr;
    }
    if (mAvioContext)
    {
        avio_context_free(&mAvioContext);
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
    int64_t clipLengthInPTS        = currentPts - mCurrentClipStartPts;
    const int64_t chunkDurationPTS = mClipInfo.mChunkDuration * AV_TIME_BASE_Q.den;
    const int64_t maxClipDuration  = mClipInfo.mMaxClipDuration * AV_TIME_BASE_Q.den;
    int currentFragmentId          = (clipLengthInPTS + chunkDurationPTS - 1) / chunkDurationPTS;
    bool finalizing                = (clipLengthInPTS >= maxClipDuration);
    if (reason)
    {
        currentFragmentId++;
        finalizing = true;
    }
    if (finalizing)
    {
        ChipLogProgress(Camera, "Clip recording complete for ID: %d", mClipInfo.mClipId);
        if (mClipInfo.mTriggerType == 1)
        {
            mRunning = false;
        }
        Stop();
        mCurrentClipStartPts = AV_NOPTS_VALUE;
        mLastFragmentId      = 0;
        mClipInfo.mClipId++;
        mCurrentClipStartPts = currentPts;
    }

    if (currentFragmentId > mLastFragmentId || finalizing)
    {
        char buffer[512];
        const std::string prefix   = mClipInfo.mRecorderId + "_clip_" + std::to_string(mClipInfo.mClipId);
        const std::string basePath = mClipInfo.mOutputPath + prefix;
        bool videoUploaded         = false;
        bool audioUploaded         = false;

        sprintf(buffer, "%s_chunk-stream0-%05d.cmfv", basePath.c_str(), mLastFragmentId);
        videoUploaded = CheckAndUploadFile(buffer);
        audioUploaded = false;
        if (mClipInfo.mHasAudio)
        {
            sprintf(buffer, "%s_chunk-stream1-%05d.cmfv", basePath.c_str(), mLastFragmentId);
            audioUploaded = CheckAndUploadFile(buffer);
        }
        if (videoUploaded || audioUploaded)
            mLastFragmentId = currentFragmentId;

        if (finalizing)
        {
            sprintf(buffer, "%s_init-stream0.fmp4", basePath.c_str());
            CheckAndUploadFile(buffer);
            if (mClipInfo.mHasAudio)
            {
                sprintf(buffer, "%s_init-stream1.fmp4", basePath.c_str());
                CheckAndUploadFile(buffer);
            }
            sprintf(buffer, "%s.mpd", basePath.c_str());
            CheckAndUploadFile(buffer);
            mLastFragmentId = 0;
        }
    }
}

bool PushAVClipRecorder::CheckAndUploadFile(char * path)
{
    if (FileExists(path))
    {
        std::string filename(path);
        ChipLogProgress(Camera, "Uploading file %s to %s", filename.c_str(), mClipInfo.mUrl.c_str());
        mUploader.AddFileToUpload(filename, mClipInfo.mUrl);
        return true;
    }
    return false;
}

bool PushAVClipRecorder::FileExists(const std::string & path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
