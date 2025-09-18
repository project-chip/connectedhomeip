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

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID)
{

    mTransportOptionsMap[connectionID] = transportOptions;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID] =
        std::make_unique<PushAVTransport>(transportOptions, connectionID, mAudioStreamParams, mVideoStreamParams);

    if (mMediaController == nullptr)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager: MediaController is not set");
        mTransportMap.erase(connectionID);
        return Status::NotFound;
    }

    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), transportOptions.videoStreamID.Value().Value(),
                                        transportOptions.audioStreamID.Value().Value());

    mMediaController->SetPreRollLength(mTransportMap[connectionID].get(), mTransportMap[connectionID].get()->GetPreRollLength());
#ifdef TLS_CLUSTER_ENABLED
    // TODO: get TLS endpointId from PAVST cluster
    auto & tlsClientManager = mCameraDevice->GetTLSClientMgmtDelegate();
    auto & tlsCertManager   = mCameraDevice->GetTLSCertMgmtDelegate();

    if (!tlsClientManager || !tlsClientManager)
    {
        ChipLogError(Camera, "Failed to get TLS cluster handlers");
        return;
    }

    // Get TLS endpoint information
    TLSClientManagement::Commands::FindEndpointResponse::Type endpointResponse;
    CHIP_ERROR err = tlsClientManager->FindEndpoint(mConnectionID, endpointResponse);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to find TLS endpoint for connection %u: %" CHIP_ERROR_FORMAT, mConnectionID, err.Format());
        return;
    }
    auto endpoint = endpointResponse.endpoint;

    // Get root certificate
    TLSCertificateManagement::Commands::FindRootCertificateResponse::Type rootCertResponse;
    rootCertResponse.CertificateDetails = DataModel::List<TLSCertificateManagement::Structs::TLSCertStruct::Type>();
    err                                 = tlsCertManager->FindRootCertificate(endpoint.CAID, rootCertResponse);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to find root certificate for CAID %u: %" CHIP_ERROR_FORMAT, endpoint.CAID, err.Format());
        return;
    }
    auto rootCert = rootCertResponse.CertificateDetails[0].Certificate;

    // Get client certificate details if configured
    if (endpoint.CCDID != NullOptional)
    {
        chip::app::Clusters::TLSCertificateManagement::Commands::FindClientCertificateResponse::Type clientCertResponse;
        err = tlsCertManager->FindClientCertificate(endpoint.CCDID.Value(), clientCertResponse);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Camera, "Failed to find client certificate for CCDID %u: %" CHIP_ERROR_FORMAT, endpoint.CCDID.Value(),
                         err.Format());
            return;
        }
        auto clientCert = clientCertResponse.CertificateDetails[0].ClientCertificate;

        // Get private key from secure storage
        auto mDevKey = GetPrivateKeyFromSecureStorage(endpoint.CCDID.Value());
        if (mCertPath.mDevKey.empty())
        {
            ChipLogError(Camera, "Failed to get private key for CCDID %u", endpoint.CCDID.Value());
            return;
        }
    }
    mTransportMap[connectionID].get()->SetTLSCertPath(rootCert, clientCert, mDevKey);
#else
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
    mTransportMap[connectionID]->TriggerTransport(activationReason);

    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                                     const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                     const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // TODO: Validates the requested stream usage against the camera's resource management.
    // Returning Status::Success to pass through checks in the Server Implementation.
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

bool PushAvStreamTransportManager::ValidateMotionZoneSize(uint16_t zoneSize)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return false;
    }
    auto maxZones = mCameraDevice->GetZoneManagementDelegate().GetZoneMgmtServer()->GetMaxZones();
    if (zoneSize >= maxZones)
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
