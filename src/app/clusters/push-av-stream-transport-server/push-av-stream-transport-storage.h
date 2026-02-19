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
#include <lib/core/TLVTags.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

// Size calculation for CMAFContainerStorage
// CMAFInterface: 1 byte
// segmentDuration: 2 bytes
// chunkDuration: 2 bytes
// sessionGroup: 1 byte
// mTrackNameBuffer: 16 bytes
// mCENCKeyBuffer: 16 bytes
// mCENCKeyIDBuffer: 16 bytes
// metadataEnabled: ~2 bytes (EstimateStructOverhead)
static constexpr size_t kCMAFContainerStorageSize =
    TLV::EstimateStructOverhead(sizeof(uint8_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint8_t), kMaxTrackNameLength,
                                kMaxCENCKeyLength, kMaxCENCKeyIDLength, sizeof(bool));

// Size calculation for ContainerOptionsStorage
// containerType: 1 byte
// mCMAFContainerStorage: kCMAFContainerStorageSize
static constexpr size_t kContainerOptionsStorageSize = TLV::EstimateStructOverhead(sizeof(uint8_t), kCMAFContainerStorageSize);

// Size of MotionTimeControl struct
static constexpr size_t kMotionTimeControlSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(uint16_t));

// Size calculation for TransportZoneOptions
// Depends on CHIP_CONFIG_MAX_NUM_ZONES
static constexpr size_t kTransportZoneOptionsSize =
    2 /* Array Overhead */ + (CHIP_CONFIG_MAX_NUM_ZONES * TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint8_t)));

// Size calculation for TriggerOptionsStorage
// triggerType: 1 byte
// mTransportZoneOptions: kTransportZoneOptionsSize
// motionSensitivity: 1 byte
// motionTimeControl: kMotionTimeControlSize
// maxPreRollLen: 2 bytes
static constexpr size_t kTriggerOptionsStorageSize = TLV::EstimateStructOverhead(
    sizeof(uint8_t), kTransportZoneOptionsSize, sizeof(uint8_t), kMotionTimeControlSize, sizeof(uint16_t));

static constexpr size_t kTransportOptionsStorageSize =
    TLV::EstimateStructOverhead(sizeof(uint8_t),              // streamUsage
                                sizeof(uint16_t),             // videoStreamID
                                sizeof(uint16_t),             // audioStreamID
                                sizeof(uint16_t),             // TLSEndpointID
                                kMaxUrlLength,                // max url
                                sizeof(uint8_t),              // ingestMethod
                                sizeof(uint32_t),             // expiryTime
                                kTriggerOptionsStorageSize,   // triggerOptions
                                kContainerOptionsStorageSize, // containerOptions
                                2u /* Array Overhead */ +
                                    CHIP_CONFIG_MAX_NUM_CAMERA_VIDEO_STREAMS *
                                        TLV::EstimateStructOverhead(kMaxStreamNameLength /* stream name */, sizeof(uint16_t)),
                                2u /* Array Overhead */ +
                                    CHIP_CONFIG_MAX_NUM_CAMERA_AUDIO_STREAMS *
                                        TLV::EstimateStructOverhead(kMaxStreamNameLength /* stream name */, sizeof(uint16_t)));

static constexpr size_t kMaxOneCurrentConnectionSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t),             // connectionID
                                sizeof(uint8_t),              // transportStatus
                                kTransportOptionsStorageSize, // estimated transportOptions
                                sizeof(FabricIndex)           // fabricIndex
    );

// Max size for the TLV-encoded array of CurrentConnection structs
static constexpr size_t kMaxCurrentConnectionsSerializedSize = 2 /* ArrayTlvOverhead */ +
    (CHIP_CONFIG_MAX_FABRICS * CHIP_CONFIG_MAX_NUM_PUSH_TRANSPORTS * kMaxOneCurrentConnectionSerializedSize);

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
                motionZones.Value().SetNull();
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

        if (containerType == ContainerFormatEnum::kCmaf)
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
        if (!mVideoStreamsStorage.empty())
        {
            videoStreams.SetValue(
                DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
        }
        else
        {
            videoStreams.ClearValue();
        }

        // Copy audio streams storage
        mAudioStreamsStorage = aTransportOptionsStorage.mAudioStreamsStorage;
        if (!mAudioStreamsStorage.empty())
        {
            audioStreams.SetValue(
                DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
        }
        else
        {
            audioStreams.ClearValue();
        }

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

    void AddVideoStream(const Structs::VideoStreamStruct::Type & videoStream)
    {
        mVideoStreamsStorage.push_back(videoStream);
        videoStreams.SetValue(
            DataModel::List<const Structs::VideoStreamStruct::Type>(mVideoStreamsStorage.data(), mVideoStreamsStorage.size()));
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

    void AddAudioStream(const Structs::AudioStreamStruct::Type & audioStream)
    {
        mAudioStreamsStorage.push_back(audioStream);
        audioStreams.SetValue(
            DataModel::List<const Structs::AudioStreamStruct::Type>(mAudioStreamsStorage.data(), mAudioStreamsStorage.size()));
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
