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

#include <algorithm>
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
    StopSessionMonitor();
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
    StartSessionMonitor();
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
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID,
                                                    FabricIndex accessingFabricIndex)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized for AllocatePushTransport");
        return Status::Failure;
    }

    if (transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        const auto cmafInterface = transportOptions.containerOptions.CMAFContainerOptions.Value().CMAFInterface;

        if (!IsCMAFInterfaceSupported(cmafInterface))
        {
            ChipLogError(Camera, "PushAvStreamTransportManager, Unsupported CMAF interface type: %u",
                         static_cast<uint16_t>(cmafInterface));
            return Status::Failure;
        }

        ChipLogDetail(Camera, "PushAvStreamTransportManager, CMAF interface selected: %u", static_cast<uint16_t>(cmafInterface));
    }

    mTransportOptionsMap[connectionID] = transportOptions;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);

    auto transport = std::make_unique<PushAVTransport>(transportOptions, connectionID, mAudioStreamParams, mVideoStreamParams);

    CHIP_ERROR err = transport->ConfigureRecorderSettings(transportOptions, mAudioStreamParams, mVideoStreamParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to configure recorder settings for Connection: [%u], error: %s",
                     connectionID, ErrorStr(err));
        return Status::Failure;
    }
    mTransportMap[connectionID] = std::move(transport);
    mTransportMap[connectionID]->SetPushAvStreamTransportServer(mPushAvStreamTransportServer);
    mTransportMap[connectionID]->SetPushAvStreamTransportManager(this);
    mTransportMap[connectionID]->SetFabricIndex(accessingFabricIndex);

    if (mMediaController == nullptr)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager: MediaController is not set");
        mTransportMap.erase(connectionID);
        return Status::NotFound;
    }

    /*
    Initialize video, audio stream ids with default invalid value (UINT16_MAX = 65535)
    This is necessary because the MediaController API expects these values to be set.
    If any of video/audio stream id is absent in the transport options,UINT16_MAX max is used as default value.
    */
    uint16_t videoStreamID = -1;
    uint16_t audioStreamID = -1;

    // TODO: Supporting single video stream and single audio stream. This logic need to be updated for all the streams
    //  in future for application as part of 1.5.1
    if (transportOptions.videoStreamID.HasValue() && !transportOptions.videoStreamID.Value().IsNull())
    {
        videoStreamID = transportOptions.videoStreamID.Value().Value();
    }

    if (transportOptions.audioStreamID.HasValue() && !transportOptions.audioStreamID.Value().IsNull())
    {
        audioStreamID = transportOptions.audioStreamID.Value().Value();
    }

    ChipLogProgress(
        Camera, "PushAvStreamTransportManager, RegisterTransport for connectionID: [%u], videoStreamID: [%u], audioStreamID: [%u]",
        connectionID, videoStreamID, audioStreamID);
    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), videoStreamID, audioStreamID);
    mMediaController->SetPreRollLength(mTransportMap[connectionID].get(), mTransportMap[connectionID].get()->GetPreRollLength());

    uint32_t newTransportBandwidthbps = 0;
    GetBandwidthForStreams(transportOptions.videoStreamID, transportOptions.audioStreamID, newTransportBandwidthbps);

    mTransportMap[connectionID].get()->SetCurrentlyUsedBandwidthbps(newTransportBandwidthbps);
    mTotalUsedBandwidthbps += newTransportBandwidthbps;
    ChipLogDetail(Camera,
                  "AllocatePushTransport: Transport for connection %u allocated successfully. "
                  "New transport bandwidth: %u bps. Total used bandwidth: %u bps.",
                  connectionID, newTransportBandwidthbps, mTotalUsedBandwidthbps);

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
    mTotalUsedBandwidthbps -= mTransportMap[connectionID].get()->GetCurrentlyUsedBandwidthbps();
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

    CHIP_ERROR err = mTransportMap[connectionID].get()->ModifyPushTransport(transportOptions);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to modify Connection :[%u], error: %s", connectionID,
                     ErrorStr(err));
        return Status::Failure;
    }

    uint32_t newTransportBandwidthbps = 0;
    GetBandwidthForStreams(transportOptions.videoStreamID, transportOptions.audioStreamID, newTransportBandwidthbps);

    mTotalUsedBandwidthbps -= mTransportMap[connectionID].get()->GetCurrentlyUsedBandwidthbps();

    mTransportMap[connectionID].get()->SetCurrentlyUsedBandwidthbps(newTransportBandwidthbps);
    mTotalUsedBandwidthbps += newTransportBandwidthbps;

    ChipLogDetail(Camera,
                  "ModifyPushTransport: Transport for connection %u allocated successfully. "
                  "New transport bandwidth: %u bps. Total used bandwidth: %u bps.",
                  connectionID, newTransportBandwidthbps, mTotalUsedBandwidthbps);

    mTransportOptionsMap[connectionID] = transportOptions;

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
        bool isActive;
        CHIP_ERROR status = IsAnyPrivacyModeActive(isActive);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Camera,
                         "PushAvStreamTransportManager, Failed to retrieve Privacy Mode Status from AVStreamMgmtController.");
            return Status::Failure;
        }

        if (isActive)
        {
            ChipLogError(Camera,
                         "PushAvStreamTransportManager, Cannot set transport status to Active as a privacy mode is enabled.");
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

bool PushAvStreamTransportManager::IsCMAFInterfaceSupported(CMAFInterfaceEnum cmafInterface) const
{
    // CMAF interface 1 and DASH (interface 2) are supported in the current implementation
    switch (cmafInterface)
    {
    case CMAFInterfaceEnum::kInterface1:
        return true;
    case CMAFInterfaceEnum::kInterface2DASH:
        ChipLogDetail(Camera,
                      "DASH interface enabled - WARNING: This is still provisional and full compliance to CMAF ingest "
                      "interface-2: DASH may not be available");
        return true;
    case CMAFInterfaceEnum::kInterface2HLS:
        return false;
    case CMAFInterfaceEnum::kUnknownEnumValue:
        return false;
    default:
        return false;
    }
}

void PushAvStreamTransportManager::GetBandwidthForStreams(const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                          const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                                          uint32_t & outBandwidthbps)
{
    mCameraDevice->GetCameraAVStreamMgmtController().GetBandwidthForStreams(videoStreamId, audioStreamId, outBandwidthbps);
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

    uint32_t newStreamBandwidthbps = 0;
    GetBandwidthForStreams(videoStreamId, audioStreamId, newStreamBandwidthbps);
    uint32_t maxNetworkBandwidthbps = mCameraDevice->GetCameraHALInterface().GetMaxNetworkBandwidth();

    uint32_t projectedTotalBandwidthbps = mTotalUsedBandwidthbps + newStreamBandwidthbps;

    ChipLogProgress(Camera,
                    "ValidateBandwidthLimit: For streamUsage %u. New stream bandwidth: %u bps. "
                    "Currently used bandwidth: %u bps. Projected total: %u bps. Max allowed: %u bps.",
                    static_cast<uint16_t>(streamUsage), newStreamBandwidthbps, mTotalUsedBandwidthbps, projectedTotalBandwidthbps,
                    maxNetworkBandwidthbps);

    if (projectedTotalBandwidthbps > maxNetworkBandwidthbps)
    {
        ChipLogError(Camera,
                     "ValidateBandwidthLimit: ResourceExhausted for streamUsage %u. "
                     "Projected total bandwidth (%u bps) would exceed maximum network bandwidth (%u bps). "
                     "New stream requires %u bps, currently %u bps is in use.",
                     static_cast<uint16_t>(streamUsage), projectedTotalBandwidthbps, maxNetworkBandwidthbps, newStreamBandwidthbps,
                     mTotalUsedBandwidthbps);
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
    std::vector<StreamUsageEnum> supportedStreamUsages = mCameraDevice->GetCameraHALInterface().GetSupportedStreamUsages();
    auto it = std::find(supportedStreamUsages.begin(), supportedStreamUsages.end(), streamUsage);
    if (it == supportedStreamUsages.end())
    {
        ChipLogError(Camera, "Requested stream usage not found in supported stream usages");
        return false;
    }
    return true;
}

bool PushAvStreamTransportManager::ValidateSegmentDuration(uint16_t segmentDuration,
                                                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId)
{
    // If the video stream ID is missing or null, error log and return false
    if (!videoStreamId.HasValue())
    {
        ChipLogError(Camera, "Segment validation requested with no provided stream") return false;
    }
    else
    {
        if (videoStreamId.Value().IsNull())
        {
            ChipLogError(Camera, "Segment validation requested against a Null stream") return false;
        }
    }

    auto & videoStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedVideoStreams();

    if (videoStreams.empty())
    {
        return false;
    }

    for (const VideoStreamStruct & stream : videoStreams)
    {
        if (stream.videoStreamID == videoStreamId.Value().Value())
        {
            // Segment duration must be an exact multiple of the key frame interval
            if ((segmentDuration % stream.keyFrameInterval) == 0)
            {
                return true;
            }
            break;
        }
    }

    return false;
}

bool PushAvStreamTransportManager::ValidateMaxPreRollLength(uint16_t maxPreRollLength,
                                                            const DataModel::Nullable<uint16_t> & videoStreamId)
{
    // If the video stream ID is null, log error and return false
    if (videoStreamId.IsNull())
    {
        ChipLogError(Camera, "MaxPreRollLength validation requested with null video stream ID");
        return false;
    }

    auto & videoStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedVideoStreams();

    if (videoStreams.empty())
    {
        ChipLogError(Camera, "Attempt to validate max pre-roll length when no video streams are allocated.");
        return false;
    }

    for (const VideoStreamStruct & stream : videoStreams)
    {
        if (stream.videoStreamID == videoStreamId.Value())
        {
            // If non-zero, Max pre roll length must be greater than or equal to key frame interval
            if (maxPreRollLength >= stream.keyFrameInterval)
            {
                return true;
            }
            ChipLogError(Camera,
                         "Max pre-roll length validation failed for video stream id [%u], max pre-roll length [%u], key frame "
                         "interval [%u] ",
                         stream.videoStreamID, maxPreRollLength, stream.keyFrameInterval);
            break;
        }
    }

    return false;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SelectVideoStream(StreamUsageEnum streamUsage,
                                                                                    uint16_t & videoStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }
    auto & allocatedVideoStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedVideoStreams();

    if (allocatedVideoStreams.empty())
    {
        ChipLogError(Camera, "Attempt to select a video stream when none are allocated.");
        return Status::InvalidInState;
    }
    for (const VideoStreamStruct & stream : allocatedVideoStreams)
    {
        if (stream.streamUsage == streamUsage)
        {
            videoStreamId      = stream.videoStreamID;
            mVideoStreamParams = stream;
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

    auto & allocatedAudioStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedAudioStreams();
    if (allocatedAudioStreams.empty())
    {
        ChipLogError(Camera, "Attempt to select an audio stream when none are allocated.");
        return Status::InvalidInState;
    }
    for (const AudioStreamStruct & stream : allocatedAudioStreams)
    {
        if (stream.streamUsage == streamUsage)
        {
            audioStreamId      = stream.audioStreamID;
            mAudioStreamParams = stream;
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

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetVideoStream(uint16_t videoStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }

    auto & allocatedVideoStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedVideoStreams();
    for (const auto & stream : allocatedVideoStreams)
    {
        if (videoStreamId == stream.videoStreamID)
        {
            ChipLogProgress(Camera, "Selecting validated video stream ID %u", videoStreamId);
            mVideoStreamParams = stream;
            return Status::Success;
        }
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetAudioStream(uint16_t audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return Status::Failure;
    }

    auto & allocatedAudioStreams = mCameraDevice->GetCameraAVStreamMgmtDelegate().GetAllocatedAudioStreams();
    for (const auto & stream : allocatedAudioStreams)
    {
        if (audioStreamId == stream.audioStreamID)
        {
            ChipLogProgress(Camera, "Selecting validated audio stream ID %u", audioStreamId);
            mAudioStreamParams = stream;
            return Status::Success;
        }
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

void PushAvStreamTransportManager::HandleZoneTrigger(uint16_t zoneId)
{
    for (auto & pavst : mTransportMap)
    {
        int connectionId = pavst.first;
        ChipLogError(Camera, "PushAV sending trigger to connection ID %d", connectionId);

        if (mTransportOptionsMap[connectionId].triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
        {
            pavst.second->TriggerTransport(TriggerActivationReasonEnum::kAutomation, zoneId, 10, true);
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
            ChipLogProgress(Camera, "Intermediate certificates fetched and stored. Size: %zu", mBufferIntermediateCerts.size());
        }
    }
    else
    {
        ChipLogProgress(Camera, "No intermediate certificates found.");
    }

    const ByteSpan rawKeySpan = clientCertEntry.mCertWithKey.key.Span();
    if (rawKeySpan.size() != Crypto::kP256_PublicKey_Length + Crypto::kP256_PrivateKey_Length)
    {
        ChipLogError(Camera, "Raw key pair has incorrect size: %zu (expected %zu)", rawKeySpan.size(),
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

void PushAvStreamTransportManager::RecordingStreamPrivacyModeChanged(bool privacyModeEnabled)
{
    // To Do:
    // Depending on the change delegate should set transport status for each known connection to either active or inactive, plus
    // any other needed work.
}

CHIP_ERROR PushAvStreamTransportManager::IsHardPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsHardPrivacyModeActive(isActive);
}

CHIP_ERROR PushAvStreamTransportManager::IsSoftRecordingPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsSoftRecordingPrivacyModeActive(isActive);
}

CHIP_ERROR PushAvStreamTransportManager::IsSoftLivestreamPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsSoftLivestreamPrivacyModeActive(isActive);
}

CHIP_ERROR PushAvStreamTransportManager::IsAnyPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController                  = mCameraDevice->GetCameraAVStreamMgmtController();
    bool isHardPrivacyModeActive           = false;
    bool isSoftRecordingPrivacyModeActve   = false;
    bool isSoftLivestreamPrivacyModeActive = false;

    CHIP_ERROR status = avsmController.IsHardPrivacyModeActive(isHardPrivacyModeActive);
    status            = avsmController.IsSoftRecordingPrivacyModeActive(isSoftRecordingPrivacyModeActve);
    status            = avsmController.IsSoftLivestreamPrivacyModeActive(isSoftLivestreamPrivacyModeActive);

    isActive = isHardPrivacyModeActive || isSoftRecordingPrivacyModeActve || isSoftLivestreamPrivacyModeActive;
    return CHIP_NO_ERROR;
}

uint64_t PushAvStreamTransportManager::OnTriggerActivated(uint8_t fabricIdx, uint8_t sessionGroup, uint16_t connectionID)
{
    std::lock_guard<std::mutex> lock(mSessionMapMutex);
    auto sessionKey = CreateSessionKey(fabricIdx, sessionGroup);
    if (mSessionMap.find(sessionKey) == mSessionMap.end())
    {
        mSessionMap[sessionKey] = SessionInfo();
    }
    auto & sessionInfo = mSessionMap[sessionKey];
    auto now           = std::chrono::system_clock::now();
    if (sessionInfo.activeConnectionIDs.size() == 0)
    {
        // This case is a new trigger activation.
        sessionInfo.sessionNumber++;
        sessionInfo.sessionStartedTimestamp = now;
    }
    sessionInfo.activeConnectionIDs.insert(connectionID);
    return sessionInfo.sessionNumber;
}

void PushAvStreamTransportManager::OnTriggerDeactivated(uint8_t fabricIdx, uint8_t sessionGroup, uint16_t connectionID)
{
    std::lock_guard<std::mutex> lock(mSessionMapMutex);
    auto sessionKey    = CreateSessionKey(fabricIdx, sessionGroup);
    auto & sessionInfo = mSessionMap[sessionKey];
    sessionInfo.activeConnectionIDs.erase(connectionID);
}

void PushAvStreamTransportManager::StartSessionMonitor()
{
    mStopMonitoring       = false;
    mSessionMonitorThread = std::thread(&PushAvStreamTransportManager::SessionMonitor, this);
}

void PushAvStreamTransportManager::StopSessionMonitor()
{
    mStopMonitoring = true;
    if (mSessionMonitorThread.joinable())
    {
        mSessionMonitorThread.join();
    }
}

void PushAvStreamTransportManager::SessionMonitor()
{
    while (!mStopMonitoring)
    {
        std::vector<std::pair<uint16_t, uint64_t>> sessionsToRestart;
        {
            std::lock_guard<std::mutex> lock(mSessionMapMutex);
            for (auto & session : mSessionMap)
            {
                auto & sessionInfo = session.second;
                auto now           = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - sessionInfo.sessionStartedTimestamp).count();
                if (!sessionInfo.activeConnectionIDs.empty() && elapsed >= kMaxSessionDuration)
                {
                    sessionInfo.sessionNumber++;
                    sessionInfo.sessionStartedTimestamp = now;
                    for (auto connectionID : sessionInfo.activeConnectionIDs)
                    {
                        sessionsToRestart.push_back({ connectionID, sessionInfo.sessionNumber });
                    }
                }
            }
        }

        for (auto & [connectionID, newSessionNumber] : sessionsToRestart)
        {
            auto it = mTransportMap.find(connectionID);
            if (it != mTransportMap.end())
            {
                it->second->StartNewSession(newSessionNumber);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(kSessionMonitorInterval));
    }
}

bool PushAvStreamTransportManager::GetCMAFSessionNumber(const uint16_t connectionID, uint64_t & sessionNumber)
{
    std::lock_guard<std::mutex> lock(mSessionMapMutex);

    // Look for the connection in any session
    for (const auto & sessionPair : mSessionMap)
    {
        const auto & sessionInfo = sessionPair.second;
        if (sessionInfo.activeConnectionIDs.find(connectionID) != sessionInfo.activeConnectionIDs.end())
        {
            sessionNumber = sessionInfo.sessionNumber;
            return true;
        }
    }

    // If not found in active sessions, check if we have transport options for this connection
    auto transportIt = mTransportOptionsMap.find(connectionID);
    if (transportIt != mTransportOptionsMap.end())
    {
        // For connections not in active sessions, return a default session number
        sessionNumber = 0;
        return true;
    }

    return false;
}
