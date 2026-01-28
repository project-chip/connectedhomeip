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
#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/clusters/tls-certificate-management-server/TLSCertificateManagementCluster.h>
#include <camera-device-interface.h>
#include <chrono>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <functional>
#include <iomanip>
#include <media-controller.h>
#include <mutex>
#include <pushav-transport.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

static constexpr int kMaxSessionDuration     = 5; // in minutes
static constexpr int kSessionMonitorInterval = 1; // in seconds

// Helper function to combine FabricIndex and sessionGroup into a single key
inline uint32_t CreateSessionKey(FabricIndex fabricIdx, uint8_t sessionGroup)
{
    return (static_cast<uint32_t>(fabricIdx) << 8) | sessionGroup;
}

struct PushAvStream
{
    uint16_t id;
    TransportOptionsStruct transportOptions;
    TransportStatusEnum transportStatus;
    PushAvStreamTransportStatusEnum connectionStatus;
};

struct SessionInfo
{
    uint64_t sessionNumber = 0;
    std::chrono::system_clock::time_point sessionStartedTimestamp;
    std::unordered_set<uint16_t> activeConnectionIDs;
};

/**
 * The application delegate to define the options & implement commands.
 */
class PushAvStreamTransportManager : public PushAvStreamTransportDelegate
{
public:
    PushAvStreamTransportManager() = default;
    ~PushAvStreamTransportManager();

    void Init();
    void SetMediaController(MediaController * mediaController);
    void SetCameraDevice(CameraDeviceInterface * cameraDevice);
    void SetPushAvStreamTransportServer(PushAvStreamTransportServer * server) override;

    // Add missing override keywords and fix signatures
    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                              const uint16_t connectionID,
                                                              FabricIndex accessingFabricIndex) override;

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) override;

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStorage transportOptions) override;

    Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                           TransportStatusEnum transportStatus) override;

    Protocols::InteractionModel::Status ManuallyTriggerTransport(
        const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
        const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl) override;

    void SetTLSCerts(Tls::CertificateTable::BufferedClientCert & clientCertEntry,
                     Tls::CertificateTable::BufferedRootCert & rootCertEntry) override;

    bool ValidateStreamUsage(StreamUsageEnum streamUsage) override;

    bool ValidateSegmentDuration(uint16_t segmentDuration, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId) override;

    bool ValidateMaxPreRollLength(uint16_t maxPreRollLength, const DataModel::Nullable<uint16_t> & videoStreamId) override;

    Protocols::InteractionModel::Status
    ValidateBandwidthLimit(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override;

    Protocols::InteractionModel::Status ValidateZoneId(uint16_t zoneId) override;

    bool ValidateMotionZoneListSize(size_t zoneListSize) override;

    Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId) override;

    Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId) override;

    Protocols::InteractionModel::Status SetVideoStream(uint16_t videoStreamId) override;

    Protocols::InteractionModel::Status SetAudioStream(uint16_t audioStreamId) override;

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID) override;

    void OnAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override;

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override;

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override;

    bool GetCMAFSessionNumber(const uint16_t connectionID, uint64_t & sessionNumber) override;

    void HandleZoneTrigger(uint16_t zoneId);

    void RecordingStreamPrivacyModeChanged(bool privacyModeEnabled);

    uint64_t OnTriggerActivated(uint8_t fabricIdx, uint8_t sessionGroup, uint16_t connectionID);

    void OnTriggerDeactivated(uint8_t fabricIdx, uint8_t sessionGroup, uint16_t connectionID);

private:
    MediaController * mMediaController                         = nullptr;
    CameraDeviceInterface * mCameraDevice                      = nullptr;
    PushAvStreamTransportServer * mPushAvStreamTransportServer = nullptr;

    AudioStreamStruct mAudioStreamParams;
    VideoStreamStruct mVideoStreamParams;

    std::atomic<bool> mStopMonitoring{ false };
    std::mutex mSessionMapMutex;
    std::thread mSessionMonitorThread;

    std::unordered_map<uint16_t, std::unique_ptr<PushAVTransport>> mTransportMap; // map for the transport objects
    std::unordered_map<uint16_t, TransportOptionsStruct> mTransportOptionsMap;    // map for the transport options
    std::unordered_map<uint32_t, SessionInfo> mSessionMap;                        // map for the session info
    uint32_t mTotalUsedBandwidthbps = 0; // Tracks the total bandwidth used by all active transports

    std::vector<uint8_t> mBufferRootCert;
    std::vector<uint8_t> mBufferClientCert;
    std::vector<uint8_t> mBufferClientCertKey;
    std::vector<std::vector<uint8_t>> mBufferIntermediateCerts;
    std::vector<PushAvStream> pushavStreams;

    CHIP_ERROR IsAnyPrivacyModeActive(bool & isActive);

    /**
     * @brief Checks if the specified CMAF interface type is supported.
     * @param cmafInterface The CMAF interface enum to validate.
     * @return true if the interface is supported, false otherwise.
     */
    bool IsCMAFInterfaceSupported(CMAFInterfaceEnum cmafInterface) const;

    /**
     * @brief Calculates the total bandwidth in bps for the given video and audio stream IDs.
     * @param videoStreamId Optional nullable video stream ID.
     * @param audioStreamId Optional nullable audio stream ID.
     * @param outBandwidthbps Output parameter for the calculated bandwidth in bps.
     */
    void GetBandwidthForStreams(const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                const Optional<DataModel::Nullable<uint16_t>> & audioStreamId, uint32_t & outBandwidthbps);

    void StartSessionMonitor();
    void StopSessionMonitor();
    void SessionMonitor();
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
