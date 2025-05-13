#include "pushav-clip-recorder.h"
#include <lib/support/logging/CHIPLogging.h>
#include <cstring>
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


//TODO update 
PushAVClipRecorder::PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo, std::string url):
    mClipInfo(aClipInfo),mAudioInfo(aAudioInfo),mVideoInfo(aVideoInfo),mServerUrl(url) {

    int streamIndex = 0;
    mRunning = false;
    mMetadataSet = false;
    if (mServerUrl.size() >= 3) {
        mServerUrl.substr(0, mServerUrl.size() - 3);
    }
    // TODO: Getting invalid url from matter, remove once it is fixed.

    if(mClipInfo.mHasVideo){
        mVideoInfo.vStreamIndex = streamIndex++;
    }
    if(mClipInfo.mHasAudio){
        mAudioInfo.aStreamIndex = streamIndex;
    }
    if(std::find(mClipInfo.mSupportedCodec.begin(), 
                    mClipInfo.mSupportedCodec.end(), 
                    mVideoInfo.mVideoCodecId) == mClipInfo.mSupportedCodec.end()){
        ChipLogError(Camera, "Unsupported codec: %s", avcodec_get_name(mVideoInfo.mVideoCodecId));
    }
    if(std::find(mClipInfo.mSupportedCodec.begin(), 
                    mClipInfo.mSupportedCodec.end(), 
                    mAudioInfo.mAudioCodecId) == mClipInfo.mSupportedCodec.end()){
        ChipLogError(Camera, "Unsupported codec: %s", avcodec_get_name(mAudioInfo.mAudioCodecId));
    }
    ChipLogProgress(Camera, "PushAVClipRecorder initialized with ID: %s, output path: %s", mClipInfo.mRecorderID.c_str(), mClipInfo.mOutputPath.c_str());
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

bool PushAVClipRecorder::isH264Iframe(const uint8_t * data_ptr, unsigned int data_len)
{
    unsigned int idx = 0;
    int frame_type   = 0;
    int found_sps    = 0;
    int found_pps    = 0;
    int found_idr    = 0;
    bool ret         = false;

    if (data_ptr == nullptr || (data_len < 5))
    {
        return ret;
    }

    do
    {
        if (IS_H264_FRAME_NALU_HEAD(data_ptr + idx))
        {
            if (data_ptr[idx + 2] == 0x01)
                frame_type = data_ptr[idx + 3] & 0x1f;
            else if ((data_ptr[idx + 2] == 0x00) && (data_ptr[idx + 3] == 0x01))
                frame_type = data_ptr[idx + 4] & 0x1f;

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
            if ((data_ptr[idx + 2] == 0x00) && (data_ptr[idx + 3] == 0x01))
                idx++;

            idx += 4;
        }
        else
        {
            idx++;
        }
    } while (idx < (data_len - 4));

    if (found_sps == 1 && found_pps == 1 && found_idr == 1)
    {
        ret = true;
    }

    return ret;
}

AVPacket * PushAVClipRecorder::createPacket(const uint8_t * data, int size, bool isVideo)
{
    
    AVPacket * packet      = av_packet_alloc();
    if (!packet)
    {
        ChipLogError(Camera,"AVPacket allocation failed!");
        return nullptr;
    }
    packet->data           = (uint8_t *) av_malloc(size);
    if(!packet->data) {
        ChipLogError(Camera,"AVPacket data allocation failed!");    
        av_packet_free(&packet);
    }
    memcpy(packet->data, data, size);
    packet->size = size;
    if (isVideo)
    {
        if (isH264Iframe(data, size))
        {
            mFoundFirstIFrame   = true;
            packet->flags = AV_PKT_FLAG_KEY;
        }
        else
        {
            if (mFoundFirstIFrame == false)
            {
                ChipLogError(Camera, "First frame is not an I-frame. Dropping packet.");
                av_packet_free(&packet);
                return nullptr;
            }
        }
        packet->pts          = mVideoInfo.mVPts;
        packet->dts          = mVideoInfo.mVDts;
        packet->stream_index = mVideoInfo.vStreamIndex;
        packet->duration     = mVideoInfo.mVideoFrameDuration;
        mVideoInfo.mVDts += mVideoInfo.mVideoFrameDuration;
        mVideoInfo.mVPts += mVideoInfo.mVideoFrameDuration;
    }
    else
    {
        if (mFoundFirstIFrame == false)
        {
            ChipLogError(Camera, "First frame not found yet. Dropping packet.");
            av_packet_free(&packet);
            return nullptr;
        }        
        packet->pts          = mAudioInfo.mAPts;
        packet->dts          = mAudioInfo.mADts;
        packet->stream_index = mAudioInfo.aStreamIndex;

        packet->duration = mAudioInfo.mAudioFrameDuration;
        mAudioInfo.mADts += mAudioInfo.mAudioFrameDuration;
        mAudioInfo.mAPts += mAudioInfo.mAudioFrameDuration;
    }

    return packet;
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
    uploader.Start();
    ChipLogProgress(Camera, "Recording started for ID: %s", mClipInfo.mRecorderID.c_str());
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
        ChipLogProgress(Camera, "Recording stopped for ID: %s", mClipInfo.mRecorderID.c_str());
    }
}
void PushAVClipRecorder::PushPacket(const char * data, size_t size, uint16_t streamRecorderID, bool isVideo)
{
    if (!mRunning) {
           ChipLogError(Camera, "push packet dropped as recorder is not running");
           return;
    }

    AVPacket * packet = createPacket((const uint8_t *) data, size, isVideo);
    if (!packet) {
        ChipLogError(Camera, "PACKET DROPPED!");
        return;
    }

    std::lock_guard<std::mutex> lock(mQueueMutex);
    std::queue<AVPacket*>& queue = isVideo ? videoQueue : audioQueue;
    if (queue.size() >= mMaxQueueSize) {
        ChipLogProgress(Camera, "Queue full. Dropping packet.");
        av_packet_free(&packet);
        return;
    }
    queue.push(packet);
    mCondition.notify_one();
}

void PushAVClipRecorder::SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern, const std::string & mediaSegPattern)
{
    const std::string mpdFilename = outputPrefix + ".mpd";
    if (avformat_alloc_output_context2(&mFmtCtx, nullptr, "dash", mpdFilename.c_str()) < 0)
    {
        ChipLogError(Camera, "Failed to allocate output context");
        return;
    }
    // Set DASH/CMAF options
    av_opt_set(mFmtCtx->priv_data, "seg_duration", std::to_string(mClipInfo.mChunkDuration).c_str(), 0);
    av_opt_set(mFmtCtx->priv_data, "use_template", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "increment_tc", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "use_timeline", "1", 0);
    av_opt_set(mFmtCtx->priv_data, "movflags", "+cmaf+dash+delay_moov+skip_sidx+skip_trailer+frag_custom", 0);
    av_opt_set(mFmtCtx->priv_data, "init_seg_name", initSegPattern.c_str(), 0);
    av_opt_set(mFmtCtx->priv_data, "media_seg_name", mediaSegPattern.c_str(), 0);
    if (mClipInfo.mHasVideo)
        AddStreamToOutput(AVMEDIA_TYPE_VIDEO);
    if (mClipInfo.mHasAudio)
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

    if (!mClipInfo.mHasVideo && !mClipInfo.mHasAudio)
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
        mVideoStream->codecpar->width     = mVideoInfo.mWidth;
        mVideoStream->codecpar->height    = mVideoInfo.mHeight;

        mVideoStream->avg_frame_rate = (AVRational) { mVideoInfo.mFrameRate, 1 };
    }
    else if (type == AVMEDIA_TYPE_AUDIO)
    {
        mAudioStream                 = avformat_new_stream(mFmtCtx, nullptr);
		const AVCodec* audioCodec    = avcodec_find_encoder(mAudioInfo.mAudioCodecId);
		AVCodecContext * audioEncCtx = avcodec_alloc_context3(audioCodec);
		audioEncCtx->sample_rate     = mAudioInfo.mSampleRate;
		audioEncCtx->channel_layout  = mAudioInfo.mChannelLayout;
		audioEncCtx->channels        = mClipInfo.mAudioStreamID;
		audioEncCtx->bit_rate        = mAudioInfo.mBitRate;
		audioEncCtx->sample_fmt      = audioCodec->sample_fmts[0];
		audioEncCtx->time_base       = (AVRational) { 1, mAudioInfo.mSampleRate };
		avcodec_open2(audioEncCtx, audioCodec, nullptr);
		avcodec_parameters_from_context(mAudioStream->codecpar, audioEncCtx);
		if (mFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
		   audioEncCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    else
    {
        ChipLogProgress(Camera, "Unknow media type");
    }
}

bool PushAVClipRecorder::ProcessBuffersAndWrite() {
    if (videoQueue.empty() && audioQueue.empty()) {
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

    if (mMetadataSet == false) {
        if (mClipInfo.mHasVideo && !useVideo) {
            return false;
        }
        std::string prefix = mClipInfo.mRecorderID + "_clip_" + std::to_string(mClipInfo.mClipID);
        std::string initSegName      = prefix + "_init-stream$RepresentationID$.fmp4";
        std::string mediaSegName     = prefix + "_chunk-stream$RepresentationID$-$Number%05d$.cmfv";
        mInFmtCtx                 = avformat_alloc_context();
        int avio_ctx_buffer_size  = 1048576;
        uint8_t * avio_ctx_buffer = (uint8_t *) av_malloc(avio_ctx_buffer_size);
        struct buffer_data data   = { 0 }; 
        data.ptr                  = (uint8_t *) pkt->data;
        data.size                 = pkt->size;
        AVIOContext * avio_ctx    = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &data, &read_packet, nullptr, nullptr);
        mInFmtCtx->pb             = avio_ctx;
        mInFmtCtx->flags          = AVFMT_FLAG_CUSTOM_IO;
        //TODO: Update when multiple codecs are supported
        if (useVideo) {
            const AVInputFormat *ifmt = av_find_input_format("h264");
            int ret                   = avformat_open_input(&mInFmtCtx, "", (AVInputFormat*)ifmt, nullptr);
            if(!ret) {
                ChipLogError(Camera, "Failed to open input format for video");
            }
            ret                       = avformat_find_stream_info(mInFmtCtx, nullptr);
            if(!ret) {
                ChipLogError(Camera, "Failed to find stream info for video");
            }
        }
        SetupOutput(mClipInfo.mOutputPath + prefix, initSegName, mediaSegName);
        mMetadataSet = true;
    }
    if (!mFmtCtx) {
        ChipLogError(Camera, "Output context not initialized. Skipping packet.");
        return false;
    }

    AVStream * out_stream = useVideo ? mVideoStream  : mAudioStream;
    AVRational outputTb   = useVideo ? mVideoInfo.mVideoTb : mAudioInfo.mAudioTb;
    int64_t currentPts    = AV_NOPTS_VALUE;

    if (pkt->pts != AV_NOPTS_VALUE)
    {
        currentPts = av_rescale_q(pkt->pts, mClipInfo.mInputTb, AV_TIME_BASE_Q);
    }
    else if (pkt->dts != AV_NOPTS_VALUE)
    {
        currentPts = av_rescale_q(pkt->dts, mClipInfo.mInputTb, AV_TIME_BASE_Q);
    }

    if (mCurrentClipStartPts == AV_NOPTS_VALUE && currentPts != AV_NOPTS_VALUE)
    {
        mCurrentClipStartPts = currentPts;
    }


    pkt->pts = av_rescale_q(pkt->pts, mClipInfo.mInputTb, outputTb);
    pkt->dts = av_rescale_q(pkt->dts, mClipInfo.mInputTb, outputTb);
    pkt->duration = av_rescale_q(pkt->duration, mClipInfo.mInputTb, outputTb);

    pkt->pos = -1;
    pkt->pts = (pkt->pts < 0) ? 0 : pkt->pts;
    out_stream->time_base = outputTb;
    av_write_frame(mFmtCtx, pkt);

    // Free the clone
    av_packet_free(&pkt);
    if (useVideo) {
        videoQueue.pop();
    } else {
        audioQueue.pop();
    }

    int64_t clipLengthInPTS = currentPts - mCurrentClipStartPts;
    int currentFragmentId   = (clipLengthInPTS / (mClipInfo.mChunkDuration*AV_TIME_BASE));

    bool finalizing = false;
    if (clipLengthInPTS >= (mClipInfo.mMaxClipDuration*AV_TIME_BASE))
    {
        ChipLogProgress(Camera, "Clip recording complete for ID: %d", mClipInfo.mClipID);

        currentFragmentId++;
        finalizing = true;
        FinalizeCurrentClip();
        mCurrentClipStartPts = currentPts;
    }

    if (currentFragmentId > mLastFragmentId || finalizing/* && check is frag exists*/)
    {
        char buffer[256];
        std::string prefix = mClipInfo.mRecorderID + "_clip_" + std::to_string(mClipInfo.mClipID);
	if (currentFragmentId <= 1)
        {
            sprintf(buffer, "%s_init-stream0.fmp4", (mClipInfo.mOutputPath + prefix).c_str());
            CheckAndUploadFile(buffer);
            sprintf(buffer, "%s_init-stream1.fmp4", (mClipInfo.mOutputPath + prefix).c_str());
            CheckAndUploadFile(buffer);
            mLastFragmentId = currentFragmentId;
        }
        else
        {
            sprintf(buffer, "%s_chunk-stream0-%05d.cmfv", (mClipInfo.mOutputPath + prefix).c_str(), mLastFragmentId);
            bool video = CheckAndUploadFile(buffer);
            sprintf(buffer, "%s_chunk-stream1-%05d.cmfv", (mClipInfo.mOutputPath + prefix).c_str(), mLastFragmentId);
            bool audio = CheckAndUploadFile(buffer);
            if (audio || video)
                mLastFragmentId = currentFragmentId;

        }

        if(finalizing) {
            mLastFragmentId = 0;
            sprintf(buffer, "%s_chunk-stream0-%05d.cmfv", (mClipInfo.mOutputPath + prefix).c_str(), mLastFragmentId);
            CheckAndUploadFile(buffer);
            sprintf(buffer, "%s_chunk-stream1-%05d.cmfv", (mClipInfo.mOutputPath + prefix).c_str(), mLastFragmentId);
            CheckAndUploadFile(buffer);
            sprintf(buffer, "%s.mpd", (mClipInfo.mOutputPath + prefix).c_str());
            CheckAndUploadFile(buffer);
        }
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
void PushAVClipRecorder::FinalizeCurrentClip()
{
    CleanupOutput();
    while (!audioQueue.empty())
        audioQueue.pop();
    while (!videoQueue.empty())
        videoQueue.pop();
    mClipInfo.mClipID++;
}

bool PushAVClipRecorder::CheckAndUploadFile(char * path)
{
    ChipLogProgress(Camera, "CheckAndUploadFile file %s", path);
    if (fileExists(path))
    {
        std::string filename(path);
        ChipLogProgress(Camera, "Uploading file %s to %s", filename, mServerUrl.c_str());
        uploader.AddFileToUpload(filename, mServerUrl);
        return true;
    }
    return false;
}

bool PushAVClipRecorder::fileExists(const std::string & path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
