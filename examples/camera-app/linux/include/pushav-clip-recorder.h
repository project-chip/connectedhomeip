#ifndef PUSHAV_CLIP_RECORDER_H
#define PUSHAV_CLIP_RECORDER_H

#include "pushav-uploader.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// FFmpeg headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

/**
 * @struct buffer_data
 * @brief Contains buffer information for custom IO operations
 */
struct buffer_data
{
    uint8_t * ptr;
    size_t size; ///< size left in the buffer
};

/**
 * @class PushAVClipRecorder
 * @brief Manages multimedia clip recording with DASH/CMAF segmentation
 * 
 * Handles video/audio stream processing, packet queueing, and segmented file output
 */
class PushAVClipRecorder
{
public:

    /**
     * @struct ClipInfoStruct
     * @brief Contains clip configuration and runtime state
     */
    struct ClipInfoStruct {
        bool mHasVideo = true;                          ///< Video recording enabled flag
        bool mHasAudio = false;                         ///< Audio recording enabled flag
        int mclipId = 0;                                ///< Current clip identifier
        int mMaxClipDuration = 10;                      ///< Maximum clip duration in seconds
        int mSegDuration = 4;                           ///< Segment duration in seconds
        std::string mRecorderId;                        ///< Unique recorder identifier
        std::string mOutputPath;                        ///< Base output directory path
        const std::vector<AVCodecID> mSupportedCodec = {///< Supported codecs
            AV_CODEC_ID_OPUS,
            AV_CODEC_ID_H264
        };
        AVRational mInputTb = { 1, 1000000 };           ///< Input time base (microseconds)
    };

    /**
     * @struct AudioInfoStruct
     * @brief Audio stream configuration parameters
     */
    struct AudioInfoStruct {
    uint64_t mChannelLayout = AV_CH_LAYOUT_STEREO;      ///< Audio channel layout
    uint16_t mAudioStreamID = 1;                        ///< Audio stream identifier
    int mChannels = 2;                                  ///< Number of audio channels
    AVCodecID mAudioCodecId = AV_CODEC_ID_OPUS;         ///< Audio codec identifier
    int mSampleRate = 48000;                            ///< Sampling rate in Hz
    int mBitRate = 20000;                               ///< Audio bitrate in bps
    int64_t a_pts = 0;                                  ///< Audio presentation timestamp
    int64_t a_dts = 0;                                  ///< Audio decoding timestamp
    int aStreamIndex = -1;                              ///< Audio stream index
    int mAudioFrameDuration = 960;                      ///< Audio frame duration in samples
    AVRational mAudioTb = { 1, 48000 };                 ///< Audio time base
    };

    /**
     * @struct VideoInfoStruct
     * @brief Video stream configuration parameters
     */
    struct VideoInfoStruct {
        AVCodecID mVideoCodecId = AV_CODEC_ID_H264;     ///< Video codec identifier
        uint16_t mVideoStreamID = 0;                    ///< Video stream identifier
        int64_t v_pts = 3000;                           ///< Video presentation timestamp
        int64_t v_dts = 3000;                           ///< Video decoding timestamp
        int mWidth = 320;                               ///< Video frame width
        int mHeight = 240;                              ///< Video frame height
        int mFrameRate = 15;                            ///< Video frame rate (fps)
        int mVideoFrameDuration = 66667;                ///< Video frame duration (μs)
        AVRational mVideoTb = { 1, 90000 };             ///< Video time base
        int vStreamIndex = -1;                          ///< Video stream index
    };

    /// @name Construction/Destruction
    /// @{
    PushAVClipRecorder();
    ~PushAVClipRecorder();
    /// @}

    /// @name Recording Control
    /// @{
    void Start();
    void Stop();
    /// @}

    /**
     * @brief Enqueues media data for processing
     * @param data Raw media data pointer
     * @param size Data size in bytes
     * @param streamRecorderId Stream identifier
     * @param isVideo True for video data, false for audio
     */
    void PushPacket(const char* data, size_t size, uint16_t streamRecorderId, bool isVideo);
    std::atomic<bool> mRunning; ///< Recording activity flag

private:
    /// @name Stream Configuration
    /// @{
    ClipInfoStruct mClipInfo;    ///< Clip configuration parameters
    AudioInfoStruct mAudioInfo;  ///< Audio stream parameters
    VideoInfoStruct mVideoInfo;  ///< Video stream parameters
    /// @}
    std::thread mWorker;
    std::mutex mQueueMutex;
    std::condition_variable mCondition;

    bool mMetadataSet = false;
    int mMaxQueueSize = 150;
    AVFormatContext * mFmtCtx   = nullptr;
    AVFormatContext * mInFmtCtx = nullptr;
    AVStream * mVideoStream     = nullptr;
    AVStream * mAudioStream     = nullptr;
    int mLastFragmentId         = 0;

    int64_t mCurrentClipStartPts    = AV_NOPTS_VALUE;
    std::queue<AVPacket *> audioQueue;
    std::queue<AVPacket *> videoQueue;
    PushAVUploader uploader;
    bool mFoundFirstIFrame = false;

    bool isH264Iframe(const uint8_t * data_ptr, unsigned int data_len);

    AVPacket * createPacket(const uint8_t * data, int size, bool isVideo);

    /**
     * @brief Processes queued packets and writes them to the output file.
     * @return True if processing was successful, false otherwise.
     */
    bool ProcessBuffersAndWrite();

    /**
     * @brief Configures the output format context for DASH/CMAF VoD.
     *
     * @param output_prefix Base path for output files.
     * @param init_seg_pattern Pattern for initialization segments.
     * @param media_seg_pattern Pattern for media segments.
     */
    void SetupOutput(const std::string & output_prefix, const std::string & init_seg_pattern, const std::string & media_seg_pattern);

    /**
     * @brief Starts the clip recording process.
     * @return 0 on success, error code otherwise.
     */
    int StartClipRecord();

    /**
     * @brief Adds a video or audio stream to the output context.
     *
     * @param type The type of stream to add (AVMEDIA_TYPE_VIDEO or AVMEDIA_TYPE_AUDIO).
     */
    void AddStreamToOutput(AVMediaType type);

    /**
     * @brief Cleans up the output context and associated resources.
     */
    void CleanupOutput();

    /**
     * @brief Finalizes the current clip and prepares for a new one.
     */
    void finalize_current_clip();
};

#endif // PUSHAV_CLIP_RECORDER_H
