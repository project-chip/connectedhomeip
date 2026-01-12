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

#include "pushav-uploader.h"
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
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

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {
class PushAvStreamTransportManager; // Forward declaration
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip

/**
 * @struct BufferData
 * @brief Contains buffer information for custom IO operations
 */
struct BufferData
{
    uint8_t * mPtr; ///< Pointer to buffer data
    size_t mSize;   ///< Size left in the buffer
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
    struct ClipInfoStruct
    {
        bool mHasVideo;                                        ///< Video recording enabled flag
        bool mHasAudio;                                        ///< Audio recording enabled flag
        uint64_t mSessionNumber;                               ///< Session number for unique clip identification
        uint8_t mSessionGroup;                                 ///< Session group for grouping multiple transports
        uint32_t mMaxClipDurationS;                            ///< Maximum clip duration in seconds
        uint16_t mInitialDurationS;                            ///< Initial clip duration in seconds
        uint16_t mAugmentationDurationS;                       ///< Duration increment on motion detect
        uint16_t mChunkDurationMs;                             ///< Chunk duration  milliseconds
        uint16_t mSegmentDurationMs;                           ///< Segment duration in milliseconds
        uint16_t mBlindDurationS;                              ///< Duration without recording after motion stop
        uint16_t mPreRollLengthMs;                             ///< Pre-roll length in milliseconds
        uint16_t mElapsedTimeS;                                ///< Elapsed time since recording start in seconds
        std::string mOutputPath;                               ///< Base output directory path
        std::string mTrackName;                                ///< Track name for segmented files
        std::string mUrl;                                      ///< URL for uploading clips;
        int mTriggerType;                                      ///< Recording trigger type
        std::chrono::steady_clock::time_point mActivationTime; ///< Time when the recording started
    };

    /**
     * @struct AudioInfoStruct
     * @brief Audio stream configuration parameters
     */
    struct AudioInfoStruct
    {
        uint64_t mChannelLayout;   ///< Audio channel layout
        int mChannels;             ///< Number of audio channels
        AVCodecID mAudioCodecId;   ///< Audio codec identifier
        int mSampleRate;           ///< Sampling rate in Hz
        int mBitRate;              ///< Audio bitrate in bps
        int64_t mAudioPts;         ///< Audio presentation timestamp
        int64_t mAudioDts;         ///< Audio decoding timestamp
        int mAudioStreamIndex;     ///< Audio stream index
        int mAudioFrameDuration;   ///< Audio frame duration in samples
        AVRational mAudioTimeBase; ///< Audio time base
    };

    /**
     * @struct VideoInfoStruct
     * @brief Video stream configuration parameters
     */
    struct VideoInfoStruct
    {
        AVCodecID mVideoCodecId;   ///< Video codec identifier
        int64_t mVideoPts;         ///< Video presentation timestamp
        int64_t mVideoDts;         ///< Video decoding timestamp
        int mWidth;                ///< Video frame width
        int mHeight;               ///< Video frame height
        int mFrameRate;            ///< Video frame rate (fps)
        int mVideoFrameDuration;   ///< Video frame duration (μs)
        AVRational mVideoTimeBase; ///< Video time base
        int mVideoStreamIndex;     ///< Video stream index
        uint32_t mBitRate;         ///< Video bitrate in bps
    };

    /// @name Construction/Destruction
    /// @{
    PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo,
                       PushAVUploader * aUploader);
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
     * @param isVideo True for video data, false for audio
     */
    void PushPacket(const uint8_t * data, size_t size, bool isVideo);

    void SetOnStopCallback(std::function<void()> cb) { mOnStopCallback = std::move(cb); }

    // Set the cluster server reference for direct API calls
    void SetPushAvStreamTransportServer(chip::app::Clusters::PushAvStreamTransportServer * server)
    {
        mPushAvStreamTransportServer = server;
    }

    void SetConnectionInfo(uint16_t connectionID, chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                           chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum> reasonType)
    {
        mConnectionID = connectionID;
        mTriggerType  = triggerType;
        mReasonType   = reasonType;
    }

    std::atomic<bool> mDeinitializeRecorder{ false }; ///< Deinitialization flag
    ClipInfoStruct mClipInfo;                         ///< Clip configuration parameters
    void SetRecorderStatus(bool status);              ///< Sets the recorder status
    bool GetRecorderStatus();                         ///< Gets the recorder status
    void SetFabricIndex(chip::FabricIndex fabricIndex) { mFabricIndex = fabricIndex; }
    void SetPushAvStreamTransportManager(chip::app::Clusters::PushAvStreamTransport::PushAvStreamTransportManager * manager)
    {
        mPushAvStreamTransportManager = manager;
    }

private:
    long unsigned int kMaxQueueSize = 500; ///< Maximum queue size for media packets
    std::atomic<bool> mRunning{ false };   ///< Recording activity flag

    /// @name Stream Configuration
    /// @{
    AudioInfoStruct mAudioInfo; ///< Audio stream parameters
    VideoInfoStruct mVideoInfo; ///< Video stream parameters
    /// @}

    std::function<void()> mOnStopCallback;

    AVFormatContext * mFormatContext;
    AVFormatContext * mInputFormatContext;
    AVStream * mVideoStream;
    AVStream * mAudioStream;
    AVCodecContext * mAudioEncoderContext;
    std::thread mWorkerThread;
    std::mutex mQueueMutex;
    std::condition_variable mCondition;

    std::queue<AVPacket *> mAudioQueue;
    std::queue<AVPacket *> mVideoQueue;

    int mUploadSegmentID;
    int64_t mCurrentClipStartPts = AV_NOPTS_VALUE;
    int64_t mFoundFirstIFramePts = -1;
    int64_t currentPts           = AV_NOPTS_VALUE;
    bool mMetadataSet            = false;
    bool mUploadedInitSegment    = false;
    bool mUploadMPD              = false;

    PushAVUploader * mUploader;

    // Cluster server reference for direct API calls
    uint16_t mConnectionID                                                                                   = 0;
    chip::FabricIndex mFabricIndex                                                                           = 0;
    chip::app::Clusters::PushAvStreamTransportServer * mPushAvStreamTransportServer                          = nullptr;
    chip::app::Clusters::PushAvStreamTransport::PushAvStreamTransportManager * mPushAvStreamTransportManager = nullptr;
    chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum mTriggerType;
    chip ::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum> mReasonType;

    /// @name Internal Methods
    /// @{

    /**
     * @brief Ensures a directory exists, creating it if necessary.
     * @param path The directory path to check/create.
     * @return true if the directory exists or was created successfully, false otherwise.
     */
    bool EnsureDirectoryExists(const std::string & path);

    bool CheckAndUploadFile(std::string path);

    bool IsH264IFrame(const uint8_t * data, unsigned int length);

    AVPacket * CreatePacket(const uint8_t * data, int size, bool isVideo);

    /**
     * @brief Processes queued packets and writes them to the output file.
     * @return Zero if processing was successful, negative otherwise, positive for warnings.
     */
    int ProcessBuffersAndWrite();

    /**
     * @brief Configures the output format context for DASH/CMAF VoD.
     *
     * @param output_prefix Base path for output files.
     * @param init_seg_pattern Pattern for initialization segments.
     * @param media_seg_pattern Pattern for media segments.
     */
    int SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern, const std::string & mediaSegPattern);

    /**
     * @brief Starts the clip recording process.
     * @return 0 on success, error code otherwise.
     */
    int StartClipRecording();

    /**
     * @brief Adds a video or audio stream to the output context.
     *
     * @param type The type of stream to add (AVMEDIA_TYPE_VIDEO or AVMEDIA_TYPE_AUDIO).
     */
    int AddStreamToOutput(AVMediaType type);

    /**
     * @brief Cleans up the output context and associated resources.
     */
    void CleanupOutput();

    /**
     * @brief Finalizes the current clip and prepares for a new one.
     *
     * @param reason Zero for normal clip finalization or Positive number for abrupt finalization
     */
    void FinalizeCurrentClip(int reason);
    /// @}
};
