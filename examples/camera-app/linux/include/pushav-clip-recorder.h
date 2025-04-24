#ifndef PUSHAV_CLIP_RECORDER_H
#define PUSHAV_CLIP_RECORDER_H

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
#include "pushav-uploader.h"

// FFmpeg headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

struct buffer_data
{
    uint8_t * ptr;
    size_t size; ///< size left in the buffer
};
/**
 * @class PushAVClipRecorder
 * @brief Manages the recording of video and/or audio clips into segmented files (e.g., for DASH/CMAF).
 *
 * This class handles pushing packets into queues, processing them to write to output files,
 * and managing clip segmentation based on duration.
 */
class PushAVClipRecorder
{
public:
    /**
     * @brief Constructs a PushAVClipRecorder instance.
     *
     * @param id A unique identifier for the recorder instance.
     * @param outputPath The base output directory for recorded clips.
     * @param hasVideo Whether video recording is enabled.
     * @param hasAudio Whether audio recording is enabled.
     */
    PushAVClipRecorder(const std::string& id, const std::string& outputPath, bool hasVideo = true, bool hasAudio = false);

    /**
     * @brief Destructor. Ensures all resources are properly released.
     */
    ~PushAVClipRecorder();

    /**
     * @brief Starts the recording process in a separate thread.
     */
    void Start();

    /**
     * @brief Stops the recording process and cleans up resources.
     */
    void Stop();

    /**
     * @brief Pushes a packet into the appropriate queue (video or audio).
     *
     * @param packet The packet to be added to the queue.
     * @param isVideo Whether the packet is a video packet.
     */
    void PushPacket(AVPacket * packet, bool isVideo);

private:
    bool mHasVideo;
    bool mHasAudio;
    int mclipId = 0;
    std::string mId;
    std::string mOutputPath;

    std::thread mWorker;
    std::mutex mQueueMutex;
    std::atomic<bool> mRunning;
    std::condition_variable mCondition;

    std::string mOutputPrefix;
    std::string mInitSegName;
    std::string mMediaSegName;
    bool mMetadataSet = false;

    AVFormatContext * mFmtCtx   = nullptr;
    AVFormatContext * mInFmtCtx = nullptr;
    AVStream * mVideoStream     = nullptr;
    AVStream * mAudioStream     = nullptr;
    int mLastFragmentId = 0;

    int64_t mCurrentClipStartPts     = AV_NOPTS_VALUE;
    int64_t mFragmentDuration = 4 * AV_TIME_BASE;
    const int64_t MAX_CLIP_DURATION = 50 * AV_TIME_BASE;
    const size_t MAX_QUEUE_SIZE = 150;

    std::queue<AVPacket *> audioQueue;
    std::queue<AVPacket *> videoQueue;
    bool started_writing        = false;
    PushAVUploader uploader;

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
    void SetupOutput(const std::string & output_prefix, const std::string & init_seg_pattern,
                      const std::string & media_seg_pattern, int video);

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
