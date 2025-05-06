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
        bool mHasVideo;                             ///< Video recording enabled flag
        bool mHasAudio;                             ///< Audio recording enabled flag
        int mClipID;                                ///< Current clip identifier
        int mMaxClipDuration;                       ///< Maximum clip duration in seconds
        int mChunkDuration;                         ///< Segment duration in seconds
        std::string mRecorderID;                    ///< Unique recorder identifier
        std::string mOutputPath;                    ///< Base output directory path
        std::vector<AVCodecID> mSupportedCodec;
        AVRational mInputTb;                        ///< Input time base (microseconds)
        uint16_t mAudioStreamID;                    ///< Audio stream identifier
        uint16_t mVideoStreamID;                    ///< Video stream identifier

    };

    /**
     * @struct AudioInfoStruct
     * @brief Audio stream configuration parameters
     */
    struct AudioInfoStruct {
        uint64_t mChannelLayout;                    ///< Audio channel layout
        int mChannels;                              ///< Number of audio channels
        AVCodecID mAudioCodecId;                    ///< Audio codec identifier
        int mSampleRate;                            ///< Sampling rate in Hz
        int mBitRate;                               ///< Audio bitrate in bps
        int64_t mAPts;                              ///< Audio presentation timestamp
        int64_t mADts;                              ///< Audio decoding timestamp
        int aStreamIndex;                           ///< Audio stream index
        int mAudioFrameDuration;                    ///< Audio frame duration in samples
        AVRational mAudioTb;                        ///< Audio time base
    };

    /**
     * @struct VideoInfoStruct
     * @brief Video stream configuration parameters
     */
    struct VideoInfoStruct {
        AVCodecID mVideoCodecId;                     ///< Video codec identifier
        int64_t mVPts;                              ///< Video presentation timestamp
        int64_t mVDts;                              ///< Video decoding timestamp
        int mWidth;                                 ///< Video frame width
        int mHeight;                                ///< Video frame height
        int mFrameRate;                             ///< Video frame rate (fps)
        int mVideoFrameDuration;                    ///< Video frame duration (μs)
        AVRational mVideoTb;                        ///< Video time base
        int vStreamIndex;                           ///< Video stream index
        uint32_t mBitRate;                          ///< Video bitrate in bps
    };

    /// @name Construction/Destruction
    /// @{
    PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo, std::string url);
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
    bool fileExists(const std::string & path);
    bool CheckAndUploadFile(char * path);
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
    std::string mServerUrl;

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
    void FinalizeCurrentClip();
};

#endif // PUSHAV_CLIP_RECORDER_H
