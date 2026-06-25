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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

/**
 * @brief Storage implementation for transport trigger options.
 * Provides deep copy functionality and internal storage for motion zones.
 * Must be used when trigger options need to persist beyond the original data scope.
 */
struct TransportTriggerOptionsStorage : public TransportTriggerOptionsStruct
{
    TransportTriggerOptionsStorage() {}

    TransportTriggerOptionsStorage(const TransportTriggerOptionsStorage & aTransportTriggerOptionsStorage)
    {
        *this = aTransportTriggerOptionsStorage;
    }

    TransportTriggerOptionsStorage & operator=(const TransportTriggerOptionsStorage & aTransportTriggerOptionsStorage)
    {
        if (this == &aTransportTriggerOptionsStorage)
        {
            return *this;
        }

        triggerType = aTransportTriggerOptionsStorage.triggerType;

        mTransportZoneOptions = aTransportTriggerOptionsStorage.mTransportZoneOptions;

        motionZones = aTransportTriggerOptionsStorage.motionZones;

        // Rebind motionZones to point to the copied vector if it was set
        if (motionZones.HasValue() && !motionZones.Value().IsNull())
        {
            motionZones.SetValue(DataModel::MakeNullable(
                DataModel::List<const TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
        }

        motionSensitivity = aTransportTriggerOptionsStorage.motionSensitivity;
        motionTimeControl = aTransportTriggerOptionsStorage.motionTimeControl;
        maxPreRollLen     = aTransportTriggerOptionsStorage.maxPreRollLen;

        return *this;
    }

    TransportTriggerOptionsStorage &
    operator=(const Structs::TransportTriggerOptionsStruct::DecodableType & aTransportTriggerOptions)
    {

        triggerType = aTransportTriggerOptions.triggerType;

        // Reset before repopulating, as done for video/audio streams.
        mTransportZoneOptions.clear();

        auto & motionZonesList = aTransportTriggerOptions.motionZones;

        if (triggerType == TransportTriggerTypeEnum::kMotion && motionZonesList.HasValue())
        {
            if (!motionZonesList.Value().IsNull())
            {
                auto iter = motionZonesList.Value().Value().begin();

                while (iter.Next())
                {
                    auto & transportZoneOption = iter.GetValue();
                    mTransportZoneOptions.push_back(transportZoneOption);
                }
                // GetStatus() check is not needed here, because the motionZonesList is already checked in the
                // ValidateIncomingTransportOptions() function

                motionZones.SetValue(DataModel::MakeNullable(
                    DataModel::List<const TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
            }
            else
            {
                // The incoming motionZones field is present and explicitly null; SetValue()
                // makes the stored Optional present with a null Nullable.
                motionZones.SetValue(DataModel::NullNullable);
            }
        }
        else
        {
            motionZones.ClearValue();
        }

        motionSensitivity = aTransportTriggerOptions.motionSensitivity;
        motionTimeControl = aTransportTriggerOptions.motionTimeControl;
        maxPreRollLen     = aTransportTriggerOptions.maxPreRollLen;

        return *this;
    }

    TransportTriggerOptionsStorage(const Structs::TransportTriggerOptionsStruct::DecodableType & triggerOptions)
    {
        *this = triggerOptions;
    }

private:
    std::vector<TransportZoneOptionsStruct> mTransportZoneOptions;
};

/**
 * @brief Storage implementation for CMAF container options.
 * Manages fixed-size buffers for CENC keys and IDs with bounds checking.
 * Must be used when CMAF container configurations need persistent key storage.
 */
struct CMAFContainerOptionsStorage : public CMAFContainerOptionsStruct
{
    CMAFContainerOptionsStorage() {}

    CMAFContainerOptionsStorage(const CMAFContainerOptionsStorage & aCMAFContainerOptionsStorage)
    {
        *this = aCMAFContainerOptionsStorage;
    }

    CMAFContainerOptionsStorage & operator=(const CMAFContainerOptionsStorage & aCMAFContainerOptionsStorage)
    {
        if (this == &aCMAFContainerOptionsStorage)
        {
            return *this;
        }

        *this = static_cast<const Structs::CMAFContainerOptionsStruct::Type &>(aCMAFContainerOptionsStorage);

        return *this;
    }

    CMAFContainerOptionsStorage & operator=(const Structs::CMAFContainerOptionsStruct::Type & aCMAFContainerOptions)
    {
        CMAFInterface   = aCMAFContainerOptions.CMAFInterface;
        segmentDuration = aCMAFContainerOptions.segmentDuration;
        chunkDuration   = aCMAFContainerOptions.chunkDuration;
        sessionGroup    = aCMAFContainerOptions.sessionGroup;

        MutableCharSpan trackNameBuffer(mTrackNameBuffer);
        // ValidateIncomingTransportOptions() function already checked the trackName length
        CopyCharSpanToMutableCharSpanWithTruncation(aCMAFContainerOptions.trackName, trackNameBuffer);
        trackName = trackNameBuffer;

        CENCKey = aCMAFContainerOptions.CENCKey;

        CENCKeyID = aCMAFContainerOptions.CENCKeyID;

        if (CENCKey.HasValue())
        {
            MutableByteSpan CENCKeyBuffer(mCENCKeyBuffer);
            // ValidateIncomingTransportOptions() function already checked the CENCKey length
            TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(aCMAFContainerOptions.CENCKey.Value(), CENCKeyBuffer);
            CENCKey.SetValue(CENCKeyBuffer);
        }
        else
        {
            CENCKey.ClearValue();
        }
        else
        {
            trackName.ClearValue();
        }

        metadataEnabled = aCMAFContainerOptions.metadataEnabled;

        if (CENCKeyID.HasValue())
        {
            MutableByteSpan CENCKeyIDBuffer(mCENCKeyIDBuffer);
            // ValidateIncomingTransportOptions() function already checked the CENCKeyID length
            TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(aCMAFContainerOptions.CENCKeyID.Value(), CENCKeyIDBuffer);
            CENCKeyID.SetValue(CENCKeyIDBuffer);
        }
        else
        {
            CENCKeyID.ClearValue();
        }

        return *this;
    }

    CMAFContainerOptionsStorage(const Structs::CMAFContainerOptionsStruct::Type & CMAFContainerOptions)
    {
        *this = CMAFContainerOptions;
    }

private:
    char mTrackNameBuffer[kMaxTrackNameLength];
    uint8_t mCENCKeyBuffer[kMaxCENCKeyLength];
    uint8_t mCENCKeyIDBuffer[kMaxCENCKeyIDLength];
};

/**
 * @brief Storage implementation for container format options.
 * Manages different container formats with type-specific storage handling.
 * Currently supports CMAF containers.
 */
struct ContainerOptionsStorage : public ContainerOptionsStruct
{
    ContainerOptionsStorage() {}

    ContainerOptionsStorage(const ContainerOptionsStorage & aContainerOptionsStorage) { *this = aContainerOptionsStorage; }

    ContainerOptionsStorage & operator=(const ContainerOptionsStorage & aContainerOptionsStorage)
    {
        if (this == &aContainerOptionsStorage)
        {
            return *this;
        }

        *this = static_cast<const Structs::ContainerOptionsStruct::DecodableType &>(aContainerOptionsStorage);

        return *this;
    }

    ContainerOptionsStorage & operator=(const Structs::ContainerOptionsStruct::DecodableType & aContainerOptions)
    {
        containerType = aContainerOptions.containerType;

        if (containerType == ContainerFormatEnum::kCmaf && aContainerOptions.CMAFContainerOptions.HasValue())
        {
            mCMAFContainerStorage = aContainerOptions.CMAFContainerOptions.Value();
            CMAFContainerOptions.SetValue(mCMAFContainerStorage);
        }
        else
        {
            CMAFContainerOptions.ClearValue();
        }

        return *this;
    }

    ContainerOptionsStorage(const Structs::ContainerOptionsStruct::DecodableType & containerOptions) { *this = containerOptions; }

private:
    CMAFContainerOptionsStorage mCMAFContainerStorage;
};

/**
 * @brief Storage implementation for transport options.
 * Manages URL storage, trigger options, container options, and stream configurations.
 * Must be used when transport configurations need to persist beyond request scope.
 */
struct TransportOptionsStorage : public TransportOptionsStruct
{
    TransportOptionsStorage() {}

    TransportOptionsStorage(const TransportOptionsStorage & aTransportOptionsStorage) { *this = aTransportOptionsStorage; }

    TransportOptionsStorage & operator=(const TransportOptionsStorage & aTransportOptionsStorage)
    {
        if (this == &aTransportOptionsStorage)
        {
            return *this;
        }
        streamUsage   = aTransportOptionsStorage.streamUsage;
        videoStreamID = aTransportOptionsStorage.videoStreamID;
        audioStreamID = aTransportOptionsStorage.audioStreamID;
        TLSEndpointID = aTransportOptionsStorage.TLSEndpointID;

        // Deep copy the URL buffer
        std::memcpy(mUrlBuffer, aTransportOptionsStorage.mUrlBuffer, kMaxUrlLength);
        url = MutableCharSpan(mUrlBuffer, aTransportOptionsStorage.url.size());

        // Copy internal storage objects
        mTriggerOptionsStorage = aTransportOptionsStorage.mTriggerOptionsStorage;
        triggerOptions         = mTriggerOptionsStorage;

        ingestMethod = aTransportOptionsStorage.ingestMethod;

        mContainerOptionsStorage = aTransportOptionsStorage.mContainerOptionsStorage;
        containerOptions         = mContainerOptionsStorage;

        expiryTime = aTransportOptionsStorage.expiryTime;

        // Copy video streams storage
        mVideoStreamsStorage = aTransportOptionsStorage.mVideoStreamsStorage;
<<<<<<< HEAD
        if (!mVideoStreamsStorage.empty())
        {
            videoStreams.SetValue(
                DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
        }
        else
        {
            videoStreams.ClearValue();
        }
=======

        // Rebind video stream name CharSpans to our own name buffer
        for (size_t i = 0; i < mVideoStreamsStorage.size(); i++)
        {
            mVideoStreamsStorage[i].videoStreamName =
                CharSpan(mVideoStreamNameBuffer.data() + i * kMaxStreamNameLength, mVideoStreamsStorage[i].videoStreamName.size());
        }

        // Rebind videoStreams list view to point to our storage
        UpdateVideoStreamsList();
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))

        // Copy audio streams storage
        mAudioStreamsStorage = aTransportOptionsStorage.mAudioStreamsStorage;
<<<<<<< HEAD
        if (!mAudioStreamsStorage.empty())
        {
            audioStreams.SetValue(
                DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
        }
        else
        {
            audioStreams.ClearValue();
        }
=======

        // Rebind audio stream name CharSpans to our own name buffer
        for (size_t i = 0; i < mAudioStreamsStorage.size(); i++)
        {
            mAudioStreamsStorage[i].audioStreamName =
                CharSpan(mAudioStreamNameBuffer.data() + i * kMaxStreamNameLength, mAudioStreamsStorage[i].audioStreamName.size());
        }

        // Rebind audioStreams list view to point to our storage
        UpdateAudioStreamsList();
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))

        return *this;
    }

    TransportOptionsStorage & operator=(const Structs::TransportOptionsStruct::DecodableType & transportOptions)
    {
        streamUsage   = transportOptions.streamUsage;
        videoStreamID = transportOptions.videoStreamID;
        audioStreamID = transportOptions.audioStreamID;
        TLSEndpointID = transportOptions.TLSEndpointID;

        MutableCharSpan urlBuffer(mUrlBuffer);
        // ValidateIncomingTransportOptions() function already checked the url length
        CopyCharSpanToMutableCharSpanWithTruncation(transportOptions.url, urlBuffer);
        url = urlBuffer;

        mTriggerOptionsStorage = transportOptions.triggerOptions;
        triggerOptions         = mTriggerOptionsStorage;

        ingestMethod = transportOptions.ingestMethod;

        mContainerOptionsStorage = transportOptions.containerOptions;
        containerOptions         = mContainerOptionsStorage;

        expiryTime = transportOptions.expiryTime;

<<<<<<< HEAD
        // Handle videoStreams from decodable type
        if (transportOptions.videoStreams.HasValue())
        {
            mVideoStreamsStorage.clear();
            auto iter = transportOptions.videoStreams.Value().begin();
            while (iter.Next())
            {
                auto & videoStream = iter.GetValue();
                mVideoStreamsStorage.push_back(videoStream);
            }
            videoStreams.SetValue(
                DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
        }
        else
        {
            mVideoStreamsStorage.clear();
            videoStreams.ClearValue();
        }

        // Handle audioStreams from decodable type
        if (transportOptions.audioStreams.HasValue())
        {
            mAudioStreamsStorage.clear();
            auto iter = transportOptions.audioStreams.Value().begin();
            while (iter.Next())
            {
                auto & audioStream = iter.GetValue();
                mAudioStreamsStorage.push_back(audioStream);
            }
            audioStreams.SetValue(
                DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
        }
        else
        {
            mAudioStreamsStorage.clear();
            audioStreams.ClearValue();
=======
        // Handle videoStreams from decodable type - perform deep copy into flat buffer
        ClearVideoStreams();
        if (transportOptions.videoStreams.HasValue())
        {
            auto iter = transportOptions.videoStreams.Value().begin();
            while (iter.Next())
            {
                auto err = AddVideoStream(iter.GetValue());
                if (!::chip::ChipError::IsSuccess(err))
                {
                    break;
                }
            }
        }

        // Handle audioStreams from decodable type - perform deep copy into flat buffer
        ClearAudioStreams();
        if (transportOptions.audioStreams.HasValue())
        {
            auto iter = transportOptions.audioStreams.Value().begin();
            while (iter.Next())
            {
                auto err = AddAudioStream(iter.GetValue());
                if (!::chip::ChipError::IsSuccess(err))
                {
                    break;
                }
            }
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))
        }

        return *this;
    }

    TransportOptionsStorage(const Structs::TransportOptionsStruct::DecodableType & transportOptions) { *this = transportOptions; }

    // Public methods to manage video streams without exposing internal storage
    void ClearVideoStreams()
    {
        mVideoStreamsStorage.clear();
        videoStreams.ClearValue();
    }

    CHIP_ERROR AddVideoStream(const Structs::VideoStreamStruct::Type & videoStream)
    {
<<<<<<< HEAD
        mVideoStreamsStorage.push_back(videoStream);
        videoStreams.SetValue(
            DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
=======
        // Defense-in-depth: reject if flat buffer is full
        VerifyOrReturnError(mVideoStreamNameBufferUsed + kMaxStreamNameLength <= mVideoStreamNameBuffer.size(),
                            CHIP_ERROR_BUFFER_TOO_SMALL,
                            ChipLogError(Zcl, "Push-AV: AddVideoStream rejected, video stream buffer full"));

        // Deep copy stream name into flat buffer
        size_t offset = mVideoStreamNameBufferUsed;
        MutableCharSpan nameBuffer(mVideoStreamNameBuffer.data() + offset, kMaxStreamNameLength);
        CopyCharSpanToMutableCharSpanWithTruncation(videoStream.videoStreamName, nameBuffer);

        Structs::VideoStreamStruct::Type newStream;
        newStream.videoStreamID   = videoStream.videoStreamID;
        newStream.videoStreamName = nameBuffer;

        mVideoStreamsStorage.push_back(newStream);
        mVideoStreamNameBufferUsed += kMaxStreamNameLength;

        UpdateVideoStreamsList();
        return CHIP_NO_ERROR;
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))
    }

    void UpdateVideoStreamsList()
    {
        if (!mVideoStreamsStorage.empty())
        {
            videoStreams.SetValue(
                DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
        }
        else
        {
            videoStreams.ClearValue();
        }
    }

    // Public methods to manage audio streams without exposing internal storage
    void ClearAudioStreams()
    {
        mAudioStreamsStorage.clear();
        audioStreams.ClearValue();
    }

    CHIP_ERROR AddAudioStream(const Structs::AudioStreamStruct::Type & audioStream)
    {
<<<<<<< HEAD
        mAudioStreamsStorage.push_back(audioStream);
        audioStreams.SetValue(
            DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
=======
        // Defense-in-depth: reject if flat buffer is full
        VerifyOrReturnError(mAudioStreamNameBufferUsed + kMaxStreamNameLength <= mAudioStreamNameBuffer.size(),
                            CHIP_ERROR_BUFFER_TOO_SMALL,
                            ChipLogError(Zcl, "Push-AV: AddAudioStream rejected, audio stream buffer full"));

        // Deep copy stream name into flat buffer
        size_t offset = mAudioStreamNameBufferUsed;
        MutableCharSpan nameBuffer(mAudioStreamNameBuffer.data() + offset, kMaxStreamNameLength);
        CopyCharSpanToMutableCharSpanWithTruncation(audioStream.audioStreamName, nameBuffer);

        Structs::AudioStreamStruct::Type newStream;
        newStream.audioStreamID   = audioStream.audioStreamID;
        newStream.audioStreamName = nameBuffer;

        mAudioStreamsStorage.push_back(newStream);
        mAudioStreamNameBufferUsed += kMaxStreamNameLength;

        UpdateAudioStreamsList();
        return CHIP_NO_ERROR;
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))
    }

    void UpdateAudioStreamsList()
    {
        if (!mAudioStreamsStorage.empty())
        {
            audioStreams.SetValue(
                DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
        }
        else
        {
            audioStreams.ClearValue();
        }
    }

private:
    char mUrlBuffer[kMaxUrlLength];
    TransportTriggerOptionsStorage mTriggerOptionsStorage;
    ContainerOptionsStorage mContainerOptionsStorage;
    std::vector<Structs::VideoStreamStruct::Type> mVideoStreamsStorage;
    std::vector<Structs::AudioStreamStruct::Type> mAudioStreamsStorage;
<<<<<<< HEAD
=======

    // Separate flat storage for stream names (deep copy buffers)
    std::array<char, kMaxVideoStreams * kMaxStreamNameLength> mVideoStreamNameBuffer;
    std::array<char, kMaxAudioStreams * kMaxStreamNameLength> mAudioStreamNameBuffer;

    size_t mVideoStreamNameBufferUsed = 0;
    size_t mAudioStreamNameBufferUsed = 0;
>>>>>>> e2077a4bcf ([Camera] Fix Heap Buffer Overflow in Push-AV Stream Transport Cluster (#72530))
};

/**
 * @brief Storage implementation for transport configurations.
 * Manages connection ID, status, fabric index and transport options.
 * Uses shared_ptr for transport options to ensure proper lifetime management.
 */
struct TransportConfigurationStorage : public TransportConfigurationStruct
{
    TransportConfigurationStorage() {}

    TransportConfigurationStorage(const TransportConfigurationStorage & aTransportConfigurationStorage)
    {
        *this = aTransportConfigurationStorage;
    }

    TransportConfigurationStorage & operator=(const TransportConfigurationStorage & aTransportConfigurationStorage)
    {
        if (this == &aTransportConfigurationStorage)
        {
            return *this;
        }
        connectionID    = aTransportConfigurationStorage.connectionID;
        transportStatus = aTransportConfigurationStorage.transportStatus;
        fabricIndex     = aTransportConfigurationStorage.fabricIndex;

        mTransportOptionsPtr = aTransportConfigurationStorage.mTransportOptionsPtr;

        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }

        return *this;
    }

    TransportConfigurationStorage(const uint16_t aConnectionID, std::shared_ptr<TransportOptionsStorage> aTransportOptionsPtr)
    {
        connectionID    = aConnectionID;
        transportStatus = TransportStatusEnum::kInactive;
        /*Store the pointer to keep buffer alive*/
        mTransportOptionsPtr = aTransportOptionsPtr;
        /*Convert Storage type to base type*/
        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }
    }
    std::shared_ptr<TransportOptionsStorage> GetTransportOptionsPtr() const { return mTransportOptionsPtr; }
    void SetTransportOptionsPtr(std::shared_ptr<TransportOptionsStorage> aTransportOptionsPtr)
    {
        mTransportOptionsPtr = aTransportOptionsPtr;
        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }
    }

private:
    std::shared_ptr<TransportOptionsStorage> mTransportOptionsPtr;
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
