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

#include "push-av-stream-manager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;

PushAvStreamTransportManager::~PushAvStreamTransportManager()
{
    // Unregister all transports from Media Controller before deleting them. This will ensure that any ongoing streams are
    // stopped.
    if (mMediaController != nullptr)
    {
        for (auto & kv : mTransportMap)
        {
            mMediaController->UnregisterTransport(kv.second.get());
        }
    }
    mTransportMap.clear();
    mTransportOptionsMap.clear();
}

void PushAvStreamTransportManager::Init()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Initialized");
    return;
}

void PushAvStreamTransportManager::SetMediaController(MediaController * mediaController)
{
    mMediaController = mediaController;
}

void PushAvStreamTransportManager::SetCameraDevice(CameraDeviceInterface * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void PushAvStreamTransportManager::SetPushAvStreamTransportServer(PushAvStreamTransportServer * server)
{
    mPushAvStreamTransportServer = server;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized for AllocatePushTransport");
        return Status::Failure;
    }
    mTransportOptionsMap[connectionID] = transportOptions;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID] =
        std::make_unique<PushAVTransport>(transportOptions, connectionID, mAudioStreamParams, mVideoStreamParams);

    mTransportMap[connectionID]->SetPushAvStreamTransportServer(mPushAvStreamTransportServer);

    if (mMediaController == nullptr)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager: MediaController is not set");
        mTransportMap.erase(connectionID);
        return Status::NotFound;
    }

    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), transportOptions.videoStreamID.Value().Value(),
                                        transportOptions.audioStreamID.Value().Value());
    mMediaController->SetPreRollLength(mTransportMap[connectionID].get(), mTransportMap[connectionID].get()->GetPreRollLength());

    double newTransportBandwidthMbps = 0.0;
    GetBandwidthForStreams(transportOptions.videoStreamID, transportOptions.audioStreamID, newTransportBandwidthMbps);

    mTransportMap[connectionID].get()->SetCurrentlyUsedBandwidthMbps(newTransportBandwidthMbps);
    mTotalUsedBandwidthMbps += newTransportBandwidthMbps;
    ChipLogDetail(Camera,
                  "AllocatePushTransport: Transport for connection %u allocated successfully. "
                  "New transport bandwidth: %.2f Mbps. Total used bandwidth: %.2f Mbps.",
                  connectionID, newTransportBandwidthMbps, mTotalUsedBandwidthMbps);

    if (transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion &&
        transportOptions.triggerOptions.motionZones.HasValue())
    {
        std::vector<std::pair<chip::app::DataModel::Nullable<uint16_t>, uint8_t>> zoneSensitivityList;

        auto motionZones = transportOptions.triggerOptions.motionZones.Value().Value();
        for (const auto & zoneOption : motionZones)
        {
            if (zoneOption.sensitivity.HasValue())
            {
                zoneSensitivityList.push_back({ zoneOption.zone, zoneOption.sensitivity.Value() });
            }
            else
            {
                zoneSensitivityList.push_back(
                    { zoneOption.zone, transportOptions.triggerOptions.motionSensitivity.Value().Value() });
            }
        }

        if (!zoneSensitivityList.empty())
        {
            mTransportMap[connectionID].get()->SetZoneSensitivityList(zoneSensitivityList);
        }
    }

#ifndef TLS_CLUSTER_NOT_ENABLED
    ChipLogDetail(Camera, "PushAvStreamTransportManager: TLS Cluster enabled, using default certs");
    mTransportMap[connectionID].get()->SetTLSCert(mBufferRootCert, mBufferClientCert, mBufferClientCertKey,
                                                  mBufferIntermediateCerts);
#else
    // TODO: The else block is for testing purpose. It should be removed once the TLS cluster integration is stable.
    mTransportMap[connectionID].get()->SetTLSCertPath("/tmp/pavstest/certs/server/root.pem", "/tmp/pavstest/certs/device/dev.pem",
                                                      "/tmp/pavstest/certs/device/dev.key");
#endif
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }
    mTotalUsedBandwidthMbps -= mTransportMap[connectionID].get()->GetCurrentlyUsedBandwidthMbps();
    mMediaController->UnregisterTransport(mTransportMap[connectionID].get());
    mTransportMap.erase(connectionID);
    mTransportOptionsMap.erase(connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ModifyPushTransport(const uint16_t connectionID, const TransportOptionsStorage transportOptions)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    double newTransportBandwidthMbps = 0.0;
    GetBandwidthForStreams(transportOptions.videoStreamID, transportOptions.audioStreamID, newTransportBandwidthMbps);

    mTotalUsedBandwidthMbps -= mTransportMap[connectionID].get()->GetCurrentlyUsedBandwidthMbps();

    mTransportMap[connectionID].get()->SetCurrentlyUsedBandwidthMbps(newTransportBandwidthMbps);
    mTotalUsedBandwidthMbps += newTransportBandwidthMbps;

    ChipLogDetail(Camera,
                  "ModifyPushTransport: Transport for connection %u allocated successfully. "
                  "New transport bandwidth: %.2f Mbps. Total used bandwidth: %.2f Mbps.",
                  connectionID, newTransportBandwidthMbps, mTotalUsedBandwidthMbps);

    mTransportOptionsMap[connectionID] = transportOptions;
    mTransportMap[connectionID].get()->ModifyPushTransport(transportOptions);
    ChipLogProgress(Camera, "PushAvStreamTransportManager, success to modify Connection :[%u]", connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                                                     TransportStatusEnum transportStatus)
{
    if (connectionIDList.empty())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, connectionIDList is empty");
        return Status::Failure;
    }

    if (transportStatus == TransportStatusEnum::kUnknownEnumValue)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, Invalid TransportStatus, transportStatus: [%u]",
                     (uint16_t) transportStatus);
        return Status::Failure;
    }

    if (transportStatus == TransportStatusEnum::kActive)
    {
        auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();
        bool isActive;
        CHIP_ERROR status = avsmController.IsPrivacyModeActive(isActive);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Camera,
                         "PushAvStreamTransportManager, Failed to retrieve Privacy Mode Status from AVStreamMgmtController.");
            return Status::Failure;
        }

        if (isActive)
        {
            ChipLogError(Camera, "PushAvStreamTransportManager, Cannot set transport status to Active as privacy mode is enabled.");
            return Status::InvalidInState;
        }
    }

    for (uint16_t connectionID : connectionIDList)
    {
        if (mTransportMap.find(connectionID) == mTransportMap.end())
        {
            ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
            continue;
        }
        mTransportMap[connectionID]->SetTransportStatus(transportStatus);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ManuallyTriggerTransport(
    const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
{
    if (activationReason == TriggerActivationReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, Manual Trigger failed for connection [%u], reason: [%u]", connectionID,
                     (uint16_t) activationReason);
        return Status::Failure;
    }

    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Trigger PushAV Transport for Connection: [%u]", connectionID);
    if (timeControl.HasValue())
    {
        mTransportMap[connectionID]->ConfigureRecorderTimeSetting(timeControl.Value());
    }
    mTransportMap[connectionID]->TriggerTransport(activationReason);

    return Status::Success;
}

void PushAvStreamTransportManager::GetBandwidthForStreams(const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                          const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                                          double & outBandwidthMbps)
{
    outBandwidthMbps = 0.0;

    if (videoStreamId.HasValue() && !videoStreamId.Value().IsNull())
    {
        uint16_t vStreamId = videoStreamId.Value().Value();

        auto & availableVideoStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedVideoStreams();
        for (const chip::app::Clusters::CameraAvStreamManagement::Structs::VideoStreamStruct::Type & stream : availableVideoStreams)
        {
            if (stream.videoStreamID == vStreamId)
            {
                outBandwidthMbps += (stream.maxBitRate / 1000000.0);
                ChipLogProgress(Camera, "GetBandwidthForStreams: VideoStream %u maxBitRate: %u bps (%.2f Mbps)", vStreamId,
                                stream.maxBitRate, (stream.maxBitRate / 1000000.0));
                break;
            }
        }
    }

    if (audioStreamId.HasValue() && !audioStreamId.Value().IsNull())
    {
        uint16_t aStreamId = audioStreamId.Value().Value();

        auto & availableAudioStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedAudioStreams();
        for (const chip::app::Clusters::CameraAvStreamManagement::Structs::AudioStreamStruct::Type & stream : availableAudioStreams)
        {
            if (stream.audioStreamID == aStreamId)
            {
                outBandwidthMbps += (stream.bitRate / 1000000.0);
                ChipLogProgress(Camera, "GetBandwidthForStreams: AudioStream %u bitRate: %u bps (%.2f Mbps)", aStreamId,
                                stream.bitRate, (stream.bitRate / 1000000.0));
                break;
            }
        }
    }

    return;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                                     const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                     const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized for ValidateBandwidthLimit");
        return Status::Failure;
    }

    double newStreamBandwidthMbps = 0.0;
    GetBandwidthForStreams(videoStreamId, audioStreamId, newStreamBandwidthMbps);
    uint32_t maxNetworkBandwidthMbps = mCameraDevice->GetCameraHALInterface().GetMaxNetworkBandwidth();

    double projectedTotalBandwidthMbps = mTotalUsedBandwidthMbps + newStreamBandwidthMbps;

    ChipLogProgress(Camera,
                    "ValidateBandwidthLimit: For streamUsage %u. New stream bandwidth: %.2f Mbps. "
                    "Currently used bandwidth: %.2f Mbps. Projected total: %.2f Mbps. Max allowed: %u Mbps.",
                    static_cast<uint16_t>(streamUsage), newStreamBandwidthMbps, mTotalUsedBandwidthMbps,
                    projectedTotalBandwidthMbps, maxNetworkBandwidthMbps);

    if (projectedTotalBandwidthMbps > maxNetworkBandwidthMbps)
    {
        ChipLogError(Camera,
                     "ValidateBandwidthLimit: ResourceExhausted for streamUsage %u. "
                     "Projected total bandwidth (%.2f Mbps) would exceed maximum network bandwidth (%u Mbps). "
                     "New stream requires %.2f Mbps, currently %.2f Mbps is in use.",
                     static_cast<uint16_t>(streamUsage), projectedTotalBandwidthMbps, maxNetworkBandwidthMbps,
                     newStreamBandwidthMbps, mTotalUsedBandwidthMbps);
        return Status::ResourceExhausted;
    }

    ChipLogProgress(Camera,
                    "ValidateBandwidthLimit: Success for streamUsage %u. "
                    "Allocating this stream would keep bandwidth usage within limits.",
                    static_cast<uint16_t>(streamUsage));
    return Status::Success;
}

bool PushAvStreamTransportManager::ValidateStreamUsage(StreamUsageEnum streamUsage)
{
    // TODO: if StreamUsage is present in the StreamUsagePriorities list, return true, false otherwise
    return true;
}

bool PushAvStreamTransportManager::ValidateSegmentDuration(uint16_t segmentDuration)
{
    // TODO: if Segment Duration is multiple of KeyFrameInterval, return true, false otherwise
    return true;
}

bool PushAvStreamTransportManager::ValidateUrl(const std::string & url)
{
    const std::string https = "https://";

    // Check minimum length and https prefix
    if (url.size() <= https.size() || url.substr(0, https.size()) != https)
    {
        return false;
    }

    // Check for non-empty host
    size_t hostStart = https.size();
    size_t hostEnd   = url.find('/', hostStart);
    std::string host = (hostEnd == std::string::npos) ? url.substr(hostStart) : url.substr(hostStart, hostEnd - hostStart);

    return !host.empty();
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SelectVideoStream(StreamUsageEnum streamUsage,
                                                                                    uint16_t & videoStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }
    auto & allocatedVideoStreams = mCameraDevice->GetCameraHALInterface().GetAvailableVideoStreams();

    if (allocatedVideoStreams.empty())
    {
        return Status::Failure;
    }
    for (VideoStream & stream : allocatedVideoStreams)
    {
        VideoStreamStruct & videoStreamParams = stream.videoStreamParams;
        if (videoStreamParams.streamUsage == streamUsage)
        {
            videoStreamId      = videoStreamParams.videoStreamID;
            mVideoStreamParams = videoStreamParams;
            return Status::Success;
        }
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SelectAudioStream(StreamUsageEnum streamUsage,
                                                                                    uint16_t & audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }

    auto & allocatedAudioStreams = mCameraDevice->GetCameraHALInterface().GetAvailableAudioStreams();
    if (allocatedAudioStreams.empty())
    {
        return Status::Failure;
    }
    for (AudioStream & stream : allocatedAudioStreams)
    {
        AudioStreamStruct & audioStreamParams = stream.audioStreamParams;
        if (audioStreamParams.streamUsage == streamUsage)
        {
            audioStreamId      = audioStreamParams.audioStreamID;
            mAudioStreamParams = audioStreamParams;
            return Status::Success;
        }
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateZoneId(uint16_t zoneId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }
    auto & zones = mCameraDevice->GetZoneManagementDelegate().GetZoneMgmtServer()->GetZones();

    for (const auto & zone : zones)
    {
        if (zone.zoneID == zoneId)
        {
            return Status::Success;
        }
    }
    return Status::Failure;
}

bool PushAvStreamTransportManager::ValidateMotionZoneListSize(size_t zoneListSize)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return false;
    }
    auto maxZones = mCameraDevice->GetZoneManagementDelegate().GetZoneMgmtServer()->GetMaxZones();
    if (zoneListSize >= maxZones)
    {
        return false;
    }
    return true;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateVideoStream(uint16_t videoStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();
    if (CHIP_NO_ERROR == avsmController.ValidateVideoStreamID(videoStreamId))
    {
        return Status::Success;
    }
    return Status::Failure;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateAudioStream(uint16_t audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    if (CHIP_NO_ERROR == avsmController.ValidateAudioStreamID(audioStreamId))
    {
        return Status::Success;
    }
    return Status::Failure;
}

PushAvStreamTransportStatusEnum PushAvStreamTransportManager::GetTransportBusyStatus(const uint16_t connectionID)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return PushAvStreamTransportStatusEnum::kUnknown;
    }

    if (mTransportMap[connectionID].get()->GetBusyStatus())
    {
        return PushAvStreamTransportStatusEnum::kBusy;
    }
    else
    {
        return PushAvStreamTransportStatusEnum::kIdle;
    }
}

// Below API may not be needed
void PushAvStreamTransportManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

CHIP_ERROR PushAvStreamTransportManager::LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections)
{
    ChipLogProgress(Zcl, "Push AV Current Connections loaded");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PushAvStreamTransportManager::PersistentAttributesLoadedCallback()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Persistent attributes loaded");

    return CHIP_NO_ERROR;
}

void PushAvStreamTransportManager::OnZoneTriggeredEvent(uint16_t zoneId)
{
    for (auto & pavst : mTransportMap)
    {
        int connectionId = pavst.first;
        ChipLogError(Camera, "PushAV sending trigger to connection ID %d", connectionId);

        if (mTransportOptionsMap[connectionId].triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
        {
            pavst.second->TriggerTransport(TriggerActivationReasonEnum::kAutomation, zoneId, 10);
        }
    }
}

void PushAvStreamTransportManager::SetTLSCerts(Tls::CertificateTable::BufferedClientCert & clientCertEntry,
                                               Tls::CertificateTable::BufferedRootCert & rootCertEntry)
{
    auto rootSpan = rootCertEntry.GetCert().certificate.Value();
    mBufferRootCert.assign(rootSpan.data(), rootSpan.data() + rootSpan.size());

    auto clientSpan = clientCertEntry.GetCert().clientCertificate.Value().Value();
    mBufferClientCert.assign(clientSpan.data(), clientSpan.data() + clientSpan.size());

    mBufferIntermediateCerts.clear();
    if (clientCertEntry.mCertWithKey.detail.intermediateCertificates.HasValue())
    {
        auto intermediateList = clientCertEntry.mCertWithKey.detail.intermediateCertificates.Value();
        auto iter             = intermediateList.begin();
        while (iter.Next())
        {
            auto certSpan = iter.GetValue();
            std::vector<uint8_t> intermediateCert;
            intermediateCert.assign(certSpan.data(), certSpan.data() + certSpan.size());
            mBufferIntermediateCerts.push_back(intermediateCert);
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            ChipLogError(Camera, "Error iterating intermediate certificates: %" CHIP_ERROR_FORMAT, iter.GetStatus().Format());
            mBufferIntermediateCerts.clear();
        }
        else
        {
            ChipLogProgress(Camera, "Intermediate certificates fetched and stored. Size: %ld", mBufferIntermediateCerts.size());
        }
    }
    else
    {
        ChipLogProgress(Camera, "No intermediate certificates found.");
    }

    const ByteSpan rawKeySpan = clientCertEntry.mCertWithKey.key.Span();
    if (rawKeySpan.size() != Crypto::kP256_PublicKey_Length + Crypto::kP256_PrivateKey_Length)
    {
        ChipLogError(Camera, "Raw key pair has incorrect size: %ld (expected %ld)", rawKeySpan.size(),
                     static_cast<size_t>(Crypto::kP256_PublicKey_Length + Crypto::kP256_PrivateKey_Length));
        return;
    }

    Crypto::P256SerializedKeypair rawSerializedKeypair;
    if (rawSerializedKeypair.SetLength(rawKeySpan.size()) != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to set length for serialized keypair");
        return;
    }
    memcpy(rawSerializedKeypair.Bytes(), rawKeySpan.data(), rawKeySpan.size());

    uint8_t derBuffer[Credentials::kP256ECPrivateKeyDERLength];
    MutableByteSpan keypairDer(derBuffer);

    CHIP_ERROR err = Credentials::ConvertECDSAKeypairRawToDER(rawSerializedKeypair, keypairDer);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to convert raw keypair to DER: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    mBufferClientCertKey.assign(keypairDer.data(), keypairDer.data() + keypairDer.size());
}
