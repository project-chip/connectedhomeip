#include "pushav-clip-recorder.h"
#include <lib/support/logging/CHIPLogging.h>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

PushAVClipRecorder::PushAVClipRecorder(const std::string& id, const std::string& outputPath, bool hasVideo, bool hasAudio)
    : mId(id), mOutputPath(outputPath), mHasVideo(hasVideo), mHasAudio(hasAudio), mRunning(false)
{
    ChipLogProgress(Camera, "PushAVClipRecorder initialized with ID: %s, output path: %s", mId.c_str(), mOutputPath.c_str());
}

PushAVClipRecorder::~PushAVClipRecorder()
{
    Stop();
}

static int read_packet(void * opaque, uint8_t * buf, int buf_size)
{
    struct buffer_data * bd = (struct buffer_data *) opaque;
    buf_size                = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

void PushAVClipRecorder::Start()
{
    if (mRunning == true)
    {
        ChipLogError(Camera, "Recording is already running. Stop before starting again.");
        return;
    }
    mRunning = true;
    mWorker  = std::thread(&PushAVClipRecorder::StartClipRecord, this);
    uploader.start();
    ChipLogProgress(Camera, "Recording started for ID: %s", mId.c_str());
}

void PushAVClipRecorder::Stop()
{
    if (mRunning)
    {
        mRunning = false;
        mCondition.notify_all();
        if (mWorker.joinable())
        {
            mWorker.join();
        }
        CleanupOutput();
        ChipLogProgress(Camera, "Recording stopped for ID: %s", mId.c_str());
    }
}
void PushAVClipRecorder::PushPacket(AVPacket * packet, bool isVideo)
{
    if (!packet) {
        ChipLogError(Camera, "Null packet provided");
        return;
    }
    AVPacket * clonedPacket = av_packet_clone(packet);
    if (!clonedPacket)
    {
        ChipLogError(Camera, "Failed to clone packet");
        return;
    }
    std::lock_guard<std::mutex> lock(mQueueMutex);
    std::queue<AVPacket*>& queue = isVideo ? videoQueue : audioQueue;
    if (queue.size() >= MAX_QUEUE_SIZE) {
        ChipLogProgress(Camera, "Queue full. Dropping packet.");
        av_packet_free(&clonedPacket);
        return;
    }
    queue.push(clonedPacket);
    mCondition.notify_one();
}

void PushAVClipRecorder::SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern, const std::string & mediaSegPattern)
{
    const std::string mpdFilename = outputPrefix + mId + ".mpd";
    if (avformat_alloc_output_context2(&mFmtCtx, nullptr, "dash", mpdFilename.c_str()) < 0)
    {
        ChipLogError(Camera, "Failed to allocate output context");
        return;
    }
    // Set DASH/CMAF options
    av_opt_set(mFmtCtx->priv_data, "seg_duration", "4", 0);
    av_opt_set(mFmtCtx->priv_data, "use_template", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "increment_tc", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "use_timeline", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "movflags", "+cmaf+dash+delay_moov+skip_sidx+skip_trailer+frag_custom", 0);
    av_opt_set(mFmtCtx->priv_data, "init_seg_pattern", initSegPattern.c_str(), 0);
    av_opt_set(mFmtCtx->priv_data, "media_seg_name", mediaSegPattern.c_str(), 0);
    av_opt_set_int(mFmtCtx->priv_data, "hls_playlist", 1, 0);
    av_opt_set(mFmtCtx->priv_data, "hls_segment_type", "fmp4", 0);
    av_opt_set(mFmtCtx->priv_data, "hls_base_url", outputPrefix.c_str(), 0);

    if (mHasVideo)
        AddStreamToOutput(AVMEDIA_TYPE_VIDEO);
    if (mHasAudio)
        AddStreamToOutput(AVMEDIA_TYPE_AUDIO);

    if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&mFmtCtx->pb, mpdFilename.c_str(), AVIO_FLAG_WRITE) < 0) {
            ChipLogError(Camera, "Failed to open output file: %s", mpdFilename.c_str());
            avformat_free_context(mFmtCtx);
            mFmtCtx = nullptr;
            return;
        }
    }

    if (avformat_write_header(mFmtCtx, nullptr) < 0)
    {
        ChipLogError(Camera, "Error writing output header");
        CleanupOutput();
        return;
    }
}

int PushAVClipRecorder::StartClipRecord()
{
    av_log_set_level(AV_LOG_DEBUG);

    if (!mHasVideo && !mHasAudio)
    {
        return 1;
    }

    while (mRunning)
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCondition.wait(lock, [this] { return !videoQueue.empty() || !audioQueue.empty(); });
        ProcessBuffersAndWrite();
    }
    return 0;
}

void PushAVClipRecorder::AddStreamToOutput(AVMediaType type)
{
    if (type == AVMEDIA_TYPE_VIDEO)
    {
        AVCodecParameters * video_in_codecpar = mInFmtCtx->streams[0]->codecpar;
        mVideoStream  = avformat_new_stream(mFmtCtx, nullptr);
        if (avcodec_parameters_copy(mVideoStream->codecpar, video_in_codecpar) < 0)
        {
            ChipLogError(Camera, "Failed to copy codec parameters for media type: %d", type);
            return;
        }
        mVideoStream ->codecpar->codec_tag = 0;
        mVideoStream ->avg_frame_rate      = (AVRational) { 20, 1 };
    }
    else if (type == AVMEDIA_TYPE_AUDIO)
    {
        AVCodecParameters * audio_in_codecpar = mInFmtCtx->streams[1]->codecpar;
        mAudioStream  = avformat_new_stream(mFmtCtx, nullptr);
        if (avcodec_parameters_copy(mAudioStream->codecpar, audio_in_codecpar) < 0)
        {
            ChipLogError(Camera, "Failed to copy codec parameters for media type: %d", type);
            return;
        }
    }
    else
    {
        ChipLogProgress(Camera, "Unknow media type");
    }
}

bool PushAVClipRecorder::ProcessBuffersAndWrite()
{
    if (videoQueue.empty() && audioQueue.empty())
    {
        return false;
    }

    bool useVideo = false;
    AVPacket* pkt = nullptr;

    if (!videoQueue.empty() && !audioQueue.empty()) {
        AVPacket* videoPkt = videoQueue.front();
        AVPacket* audioPkt = audioQueue.front();

        if (videoPkt->pts != AV_NOPTS_VALUE && audioPkt->pts != AV_NOPTS_VALUE) {
            useVideo = (videoPkt->pts < audioPkt->pts);
        } else {
            useVideo = !audioQueue.empty();
        }
        pkt = useVideo ? videoPkt : audioPkt;
    }
    else {
        pkt = videoQueue.empty() ? audioQueue.front() : videoQueue.front();
        useVideo = !videoQueue.empty();
    }

    if (!pkt) {
        ChipLogError(Camera, "No valid packet to process");
        return false;
    }

    if (mMetadataSet == false)
    {
        std::string prefix = mId + "_clip_" + std::to_string(mclipId);
        mOutputPrefix      = mOutputPath + prefix;
        mInitSegName      = prefix + "_init-stream$RepresentationID$.fmp4";
        mMediaSegName     = prefix + "_chunk-stream$RepresentationID$-$Number%05d$.cmfv";
        mInFmtCtx                 = avformat_alloc_context();
        int avio_ctx_buffer_size  = 4096;
        uint8_t * avio_ctx_buffer = (uint8_t *) av_malloc(avio_ctx_buffer_size);
        struct buffer_data data   = { 0 };
        data.ptr                  = (uint8_t *) pkt->data;
        data.size                 = pkt->size;
        AVIOContext * avio_ctx    = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &data, &read_packet, nullptr, nullptr);
        mInFmtCtx->pb             = avio_ctx;
        mInFmtCtx->flags          = AVFMT_FLAG_CUSTOM_IO;
        int ret                   = avformat_open_input(&mInFmtCtx, "", nullptr, nullptr);
        ret                       = avformat_find_stream_info(mInFmtCtx, nullptr);
        SetupOutput(mOutputPrefix, mInitSegName, mMediaSegName);
        if (!mFmtCtx)
            return AVERROR(ENOENT);
        mMetadataSet = true;
    }

    AVStream * out_stream = useVideo ? mVideoStream  : mAudioStream;
    AVRational inputTb    = (AVRational) { 1, 1000000 };
    AVRational outputTb   = useVideo ? (AVRational) { 1, 90000 } : (AVRational) { 1, 48000 };
    int64_t currentPts    = AV_NOPTS_VALUE;

    if (pkt->pts != AV_NOPTS_VALUE)
    {
        currentPts = av_rescale_q(pkt->pts, inputTb, AV_TIME_BASE_Q);
    }
    else if (pkt->dts != AV_NOPTS_VALUE)
    {
        currentPts = av_rescale_q(pkt->dts, inputTb, AV_TIME_BASE_Q);
    }

    if (mCurrentClipStartPts  != AV_NOPTS_VALUE && currentPts != AV_NOPTS_VALUE)
    {
        if (currentPts - mCurrentClipStartPts >= MAX_CLIP_DURATION)
        {
            finalize_current_clip();
            mCurrentClipStartPts = currentPts;
        }
    }

    if (mCurrentClipStartPts == AV_NOPTS_VALUE && currentPts != AV_NOPTS_VALUE)
    {
        mCurrentClipStartPts = currentPts;
    }


    pkt->pts = av_rescale_q(pkt->pts, inputTb, outputTb);
    pkt->dts = av_rescale_q(pkt->dts, inputTb, outputTb);
    pkt->duration = av_rescale_q(pkt->duration, inputTb, outputTb);

    pkt->pos = -1;
    if (!mFmtCtx) {
        ChipLogError(Camera, "Output context not initialized. Skipping packet.");
    }
    pkt->pts = (pkt->pts < 0) ? 0 : pkt->pts;
    out_stream->time_base = outputTb;
    av_write_frame(mFmtCtx, pkt);
    int fragmentId = (pkt->pts / mFragmentDuration);
    if(fragmentId > mLastFragmentId){
        mLastFragmentId = fragmentId;
        char buffer[256];
        sprintf(buffer, "%s_chunk-stream%d-%05d.cmfv", mOutputPrefix.c_str(),
                    mVideoStream->id, fragmentId);
	std::string filename(buffer);
	std::string url = "https://localhost:1234/streams/1";
        uploader.add_uploadData(filename, url);	
    }

    // Free the clone
    av_packet_free(&pkt);
    if (useVideo) {
        videoQueue.pop();
    } else {
        audioQueue.pop();
    }
    return true;
}

void PushAVClipRecorder::CleanupOutput()
{
    if (mFmtCtx) {
        av_write_trailer(mFmtCtx);
        if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE) && mFmtCtx->pb)
        {
            avio_closep(&mFmtCtx->pb);
        }
        avformat_free_context(mFmtCtx);
        mFmtCtx = nullptr;
    }
    mVideoStream = nullptr;
    mAudioStream = nullptr;
    mMetadataSet = false;
    ChipLogProgress(Camera, "Cleanup completed.");
}

/**
 * @brief Finalizes the current clip and starts a new one.
 *
 * Writes the trailer of the current clip and initializes a new output file.
 */
void PushAVClipRecorder::finalize_current_clip()
{
    CleanupOutput();
    mclipId++;
}
