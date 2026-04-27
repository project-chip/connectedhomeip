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
#include <filesystem>
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

// #define TEST_UPLOAD_MPD_AFTER_EVERY_SEGMENT

/**
 * @enum ClipFinalizationReason
 *
 * Enumerates the reasons why a clip recording is finalized.
 */
enum class ClipFinalizationReason : uint8_t
{
    kErrorOccurred      = 0, ///< Clip finalized due to an error
    kSegmentUploadCheck = 1, ///< Normal packet processing - check for segment upload
    kCleanupUpload      = 2, ///< Cleanup time - skip finalization logic, just upload segments
};

/**
 * @enum RecorderStatus
 *
 * Status codes for recorder operations.
 */
enum class RecorderStatus : uint8_t
{
    kSuccess = 0, ///< Operation completed successfully
    kWarning = 1, ///< Operation completed with warnings
    kFail    = 2, ///< Operation failed
};

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
        int64_t mClipStartPTS;                                 ///< Clip start presentation timestamp
        uint64_t mSessionNumber;                               ///< Session number for unique clip identification
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
        uint16_t mMotionDetectedDurationS;                     ///< Current motion detected duration
        uint16_t mPreviousMotionDetectedDurationS;             ///< Previous duration before augmentation
    };

    /**
     * @struct AudioInfoStruct
     * @brief Audio stream configuration parameters
     */
    struct AudioInfoStruct
    {
        uint64_t mChannelLayout;      ///< Audio channel layout
        int mChannels;                ///< Number of audio channels
        AVCodecID mAudioCodecId;      ///< Audio codec identifier
        int mSampleRate;              ///< Sampling rate in Hz
        int mBitRate;                 ///< Audio bitrate in bps
        int mAudioInputStreamId;      ///< Input stream ID from camera
        int mAudioOutputStreamId;     ///< Output stream ID for ffmpeg recording
        AVRational mAudioTimeBase;    ///< Audio time base
        std::string mAudioStreamName; ///< Audio stream name
    };

    /**
     * @struct VideoInfoStruct
     * @brief Video stream configuration parameters
     */
    struct VideoInfoStruct
    {
        AVCodecID mVideoCodecId;      ///< Video codec identifier
        int mWidth;                   ///< Video frame width
        int mHeight;                  ///< Video frame height
        int mFrameRate;               ///< Video frame rate (fps)
        int mVideoInputStreamId;      ///< Input stream ID from camera
        int mVideoOutputStreamId;     ///< Output stream ID for ffmpeg recording
        uint32_t mBitRate;            ///< Video bitrate in bps
        AVRational mVideoTimeBase;    ///< Video time base
        std::string mVideoStreamName; ///< Video stream name
    };

    /// @name Construction/Destruction
    /// @{
    /**
     * @brief Constructs a PushAVClipRecorder instance
     * @param aClipInfo Reference to clip configuration structure
     * @param aAudioInfo Reference to audio stream configuration structure
     * @param aVideoInfo Reference to video stream configuration structure
     * @param aUploader Pointer to the uploader instance for file uploads
     */
    PushAVClipRecorder(ClipInfoStruct & aClipInfo, AudioInfoStruct & aAudioInfo, VideoInfoStruct & aVideoInfo,
                       PushAVUploader * aUploader);

    /**
     * @brief Destroys the PushAVClipRecorder instance
     *
     * Ensures proper cleanup of all resources including stopping the recording thread,
     * cleaning up FFmpeg contexts, and releasing memory allocations.
     */
    ~PushAVClipRecorder();
    /// @}

    /// @name Recording Control
    /// @{
    /**
     * @brief Starts the clip recording process
     *
     * Initializes the recording worker thread and begins processing media packets.
     * Sets up the output format context and starts the DASH/CMAF segmentation.
     */
    void Start();

    /**
     * @brief Stops the clip recording process
     *
     * Signals the worker thread to stop, finalizes the current clip,
     * and cleans up all recording resources. Waits for the worker thread
     * to complete before returning.
     */
    void Stop();
    /// @}

    /**
     * @brief Enqueues media data for processing
     * @param data Raw media data pointer containing encoded audio/video data
     * @param size Data size in bytes
     * @param timestampMs Timestamp in milliseconds
     * @param isVideo True for video data, false for audio data
     */
    void PushPacket(const uint8_t * data, size_t size, int64_t timestampMs, bool isVideo);

    /**
     * @brief Sets the callback function to be called when recording stops
     * @param cb The callback function to execute on recording stop
     */
    void SetOnStopCallback(std::function<void()> cb) { mOnStopCallback = std::move(cb); }

    /**
     * @brief Sets the PushAV stream transport server reference for direct API calls
     * @param server Pointer to the PushAV stream transport server instance
     */
    void SetPushAvStreamTransportServer(chip::app::Clusters::PushAvStreamTransportServer * server)
    {
        mPushAvStreamTransportServer = server;
    }

    /**
     * @brief Sets connection information for the recording session
     * @param connectionID The unique connection identifier
     * @param triggerType The type of transport trigger that initiated the recording
     * @param reasonType The optional reason for trigger activation
     */
    void SetConnectionInfo(uint16_t connectionID, chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                           chip::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum> reasonType)
    {
        mConnectionID = connectionID;
        mTriggerType  = triggerType;
        mReasonType   = reasonType;
    }

    std::atomic<bool> mDeinitializeRecorder{ false }; ///< Deinitialization flag
    ClipInfoStruct mClipInfo;                         ///< Clip configuration parameters

    /**
     * @brief Sets the recorder status
     * @param status The recorder status to set (true for active, false for inactive)
     */
    void SetRecorderStatus(bool status);

    /**
     * @brief Gets the current recorder status
     * @return true if recorder is active, false otherwise
     */
    bool GetRecorderStatus();

    /**
     * @brief Sets the fabric index for the recording session
     * @param fabricIndex The fabric index to associate with this recorder
     */
    void SetFabricIndex(chip::FabricIndex fabricIndex) { mFabricIndex = fabricIndex; }

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

    int64_t mCurrentClipStartPts = AV_NOPTS_VALUE;
    int64_t currentPts           = AV_NOPTS_VALUE;
    int64_t mLastVideoPts        = 0;
    int64_t mLastAudioPts        = 0;
    bool mMetadataSet            = false;
    bool mUploadMPD              = false;
    bool firstSegmentReady       = false;

    std::vector<int> mUploadSegmentID;
    std::vector<bool> mUploadedInitSegment;
    std::vector<std::string> mStreamIdNameMap;

    PushAVUploader * mUploader;

    // Cluster server reference for direct API calls
    uint16_t mConnectionID                                                          = 0;
    chip::FabricIndex mFabricIndex                                                  = 0;
    chip::app::Clusters::PushAvStreamTransportServer * mPushAvStreamTransportServer = nullptr;
    chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum mTriggerType;
    chip ::Optional<chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum> mReasonType;
    std::filesystem::path mUploadFileBasePath;

    /// @name Internal Methods
    /// @{

    /**
     * @brief Ensures a directory exists, creating it if necessary.
     * @param path The directory path to check/create.
     * @return true if the directory exists or was created successfully, false otherwise.
     */
    bool EnsureDirectoryExists(const std::string & path);

    /**
     * @brief Checks if a file exists and adds it to the upload queue.
     * @param path The file path to check and upload
     * @return true if the file was successfully added to the upload queue, false otherwise
     */
    bool CheckAndUploadFile(std::string filename);

    /**
     * @brief Checks if a file is ready for upload (exists and not being written to).
     * @param path The file path to check.
     * @return true if the file is ready for upload, false otherwise.
     */
    bool IsFileReadyForUpload(const std::filesystem::path & path) const;

    /**
     * @brief Updates MPD parameters including startNumber and stream paths.
     *
     * This function reads an MPD file, finds the startNumber attribute and stream paths,
     * and replaces them with appropriate values for the current streams.
     *
     * @param mpdPath Path to the MPD file to update
     */
    void UpdateMPDParams(const std::string & mpdPath);

    /**
     * @brief Determines if H.264 data contains an I-frame (IDR frame).
     * @param data Pointer to the H.264 NALU data.
     * @param length Length of the data in bytes.
     * @return true if the data contains an I-frame, false otherwise.
     */
    bool IsH264IFrame(const uint8_t * data, unsigned int length);

    /**
     * @brief Creates an AVPacket from raw media data with timestamp conversion
     * @param data Raw media data pointer
     * @param size Data size in bytes
     * @param timestampMs Timestamp in milliseconds
     * @param isVideo True for video data, false for audio data
     * @return Pointer to created AVPacket, or nullptr on failure
     */
    AVPacket * CreatePacket(const uint8_t * data, int size, int64_t timestampMs, bool isVideo);

    /**
     * @brief Processes queued packets and writes them to the output file.
     * @return RecorderStatus::kSuccess if processing was successful, RecorderStatus::kFail otherwise.
     */
    RecorderStatus ProcessBuffersAndWrite();

    /**
     * @brief Configures the output format context for DASH/CMAF VoD.
     *
     * @param output_prefix Base path for output files.
     * @param init_seg_pattern Pattern for initialization segments.
     * @param media_seg_pattern Pattern for media segments.
     * @return RecorderStatus::kSuccess on success, RecorderStatus::kFail otherwise.
     */
    RecorderStatus SetupOutput(const std::string & outputPrefix, const std::string & initSegPattern,
                               const std::string & mediaSegPattern);

    /**
     * @brief Starts the clip recording process.
     * @return RecorderStatus::kSuccess on success, RecorderStatus::kFail otherwise.
     */
    RecorderStatus StartClipRecording();

    /**
     * @brief Adds a video or audio stream to the output context.
     *
     * @param type The type of stream to add (AVMEDIA_TYPE_VIDEO or AVMEDIA_TYPE_AUDIO).
     * @return RecorderStatus::kSuccess on success, RecorderStatus::kFail otherwise.
     */
    RecorderStatus AddStreamToOutput(AVMediaType type);

    /**
     * @brief Cleans up the output context and associated resources.
     */
    void CleanupOutput();

    /**
     * @brief Finalizes the current clip and manages segment uploads.
     *
     * This function handles the complex logic of clip finalization and segment upload management.
     * It coordinates between multiple streams and ensures proper synchronization of uploads.
     *
     * Key responsibilities:
     * - Calculates clip duration and determines if finalization is needed
     * - Manages upload of MPD manifest, initialization segments, and media segments
     * - Handles different finalization reasons (error, segment check, cleanup)
     * - Coordinates upload timing between multiple streams
     *
     * @param reason The reason for clip finalization
     *              - kErrorOccurred: Finalize due to error condition
     *              - kSegmentUploadCheck: Normal processing, check for segment uploads
     *              - kCleanupUpload: Cleanup time, skip finalization logic
     */
    void FinalizeCurrentClip(ClipFinalizationReason reason);
    /// @}
};
