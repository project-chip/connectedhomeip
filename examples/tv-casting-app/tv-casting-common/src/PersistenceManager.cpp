/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PersistenceManager.h"

#include <lib/core/TLV.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;

CHIP_ERROR PersistenceManager::AddVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo)
{
    ChipLogProgress(AppServer, "PersistenceManager::AddVideoPlayer called");
    VerifyOrReturnError(targetVideoPlayerInfo != nullptr && targetVideoPlayerInfo->IsInitialized(), CHIP_ERROR_INVALID_ARGUMENT);

    // Read cache for video players targetted in previous runs
    TargetVideoPlayerInfo cachedVideoPlayers[kMaxCachedVideoPlayers];
    CHIP_ERROR err = ReadAllVideoPlayers(cachedVideoPlayers);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer,
                     "PersistenceManager::AddVideoPlayer status of reading previously cached video players %" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    // Add active video player to the list of video players from cache
    bool newVideoPlayer = true;
    size_t i;
    for (i = 0; i < kMaxCachedVideoPlayers && cachedVideoPlayers[i].IsInitialized(); i++)
    {
        // found the same video player, overwrite the data
        if (cachedVideoPlayers[i] == *targetVideoPlayerInfo)
        {
            cachedVideoPlayers[i] = *targetVideoPlayerInfo;
            newVideoPlayer        = false;
        }
    }
    if (newVideoPlayer)
    {
        VerifyOrReturnError(i < kMaxCachedVideoPlayers, CHIP_ERROR_BUFFER_TOO_SMALL);
        cachedVideoPlayers[i] = *targetVideoPlayerInfo;
    }

    return WriteAllVideoPlayers(cachedVideoPlayers);
}

CHIP_ERROR PersistenceManager::WriteAllVideoPlayers(TargetVideoPlayerInfo videoPlayers[])
{
    ChipLogProgress(AppServer, "PersistenceManager::WriteAllVideoPlayers called");

    VerifyOrReturnError(videoPlayers != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    TLV::TLVWriter tlvWriter;
    uint8_t castingData[kCastingDataMaxBytes];
    tlvWriter.Init(castingData, kCastingDataMaxBytes);

    TLV::TLVType outerContainerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kCurrentCastingDataVersionTag), kCurrentCastingDataVersion));

    TLV::TLVType videoPlayersContainerType = TLV::kTLVType_Array;
    // Video Players container starts
    ReturnErrorOnFailure(
        tlvWriter.StartContainer(TLV::ContextTag(kVideoPlayersContainerTag), TLV::kTLVType_Structure, videoPlayersContainerType));

    for (size_t videoPlayerIndex = 0; videoPlayerIndex < kMaxCachedVideoPlayers && videoPlayers[videoPlayerIndex].IsInitialized();
         videoPlayerIndex++)
    {
        TargetVideoPlayerInfo * videoPlayer = &videoPlayers[videoPlayerIndex];
        if (videoPlayer->IsInitialized())
        {
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kNodeIdTag), videoPlayer->GetNodeId()));
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kFabricIndexTag), videoPlayer->GetFabricIndex()));
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kVideoPlayerVendorIdTag), videoPlayer->GetVendorId()));
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kVideoPlayerProductIdTag), videoPlayer->GetProductId()));
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kVideoPlayerDeviceTypeIdTag), videoPlayer->GetDeviceType()));
            ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(kVideoPlayerDeviceNameTag),
                                                    (const uint8_t *) videoPlayer->GetDeviceName(),
                                                    static_cast<uint32_t>(strlen(videoPlayer->GetDeviceName()) + 1)));
            ReturnErrorOnFailure(
                tlvWriter.Put(TLV::ContextTag(kVideoPlayerNumIPsTag), static_cast<uint64_t>(videoPlayer->GetNumIPs())));
            const Inet::IPAddress * ipAddress = videoPlayer->GetIpAddresses();
            if (ipAddress != nullptr && videoPlayer->GetNumIPs() > 0)
            {
                TLV::TLVType ipAddressesContainerType = TLV::kTLVType_Array;
                // IP Addresses container starts
                ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::ContextTag(kIpAddressesContainerTag), TLV::kTLVType_Structure,
                                                              ipAddressesContainerType));
                for (size_t i = 0; i < videoPlayer->GetNumIPs() && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
                {
                    char ipAddressStr[Inet::IPAddress::kMaxStringLength];
                    ipAddress[i].ToString(ipAddressStr, Inet::IPAddress::kMaxStringLength);
                    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(kVideoPlayerIPAddressTag),
                                                            (const uint8_t *) ipAddressStr,
                                                            static_cast<uint32_t>(strlen(ipAddressStr) + 1)));
                }
                // IP Addresses container ends
                ReturnErrorOnFailure(tlvWriter.EndContainer(ipAddressesContainerType));
            }

            TargetEndpointInfo * endpoints = videoPlayer->GetEndpoints();
            if (endpoints != nullptr)
            {
                TLV::TLVType contentAppEndpointsContainerType = TLV::kTLVType_Array;
                // Content app endpoints container starts
                ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::ContextTag(kContentAppEndpointsContainerTag),
                                                              TLV::kTLVType_Structure, contentAppEndpointsContainerType));
                for (size_t endpointIndex = 0; endpointIndex < kMaxNumberOfEndpoints && endpoints[endpointIndex].IsInitialized();
                     endpointIndex++)
                {
                    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kEndpointIdTag), endpoints[endpointIndex].GetEndpointId()));

                    chip::ClusterId * clusterIds = endpoints[endpointIndex].GetClusters();
                    if (clusterIds != nullptr)
                    {
                        TLV::TLVType clusterIdsContainerType = TLV::kTLVType_Array;
                        // ClusterIds container starts
                        ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::ContextTag(kClusterIdsContainerTag),
                                                                      TLV::kTLVType_Structure, clusterIdsContainerType));
                        for (size_t clusterIndex = 0; clusterIndex < kMaxNumberOfClustersPerEndpoint; clusterIndex++)
                        {
                            if (clusterIds[clusterIndex] != kInvalidClusterId)
                            {
                                ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kClusterIdTag), clusterIds[clusterIndex]));
                            }
                        }
                        // ClusterIds container ends
                        ReturnErrorOnFailure(tlvWriter.EndContainer(clusterIdsContainerType));
                    }
                }
                // Content app endpoints container ends
                ReturnErrorOnFailure(tlvWriter.EndContainer(contentAppEndpointsContainerType));
            }
            // Video Players container ends
            ReturnErrorOnFailure(tlvWriter.EndContainer(videoPlayersContainerType));
            ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));

            ReturnErrorOnFailure(tlvWriter.Finalize());
            ChipLogProgress(AppServer,
                            "PersistenceManager::WriteAllVideoPlayers TLV(CastingData).LengthWritten: %d bytes and version: %d",
                            tlvWriter.GetLengthWritten(), kCurrentCastingDataVersion);
            return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kCastingDataKey, castingData,
                                                                               tlvWriter.GetLengthWritten());
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistenceManager::ReadAllVideoPlayers(TargetVideoPlayerInfo outVideoPlayers[])
{
    ChipLogProgress(AppServer, "PersistenceManager::ReadAllVideoPlayers called");
    VerifyOrReturnError(outVideoPlayers != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t castingData[kCastingDataMaxBytes];
    size_t castingDataSize = 0;
    ReturnErrorOnFailure(chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kCastingDataKey, castingData,
                                                                                     kCastingDataMaxBytes, &castingDataSize));
    ChipLogProgress(AppServer, "PersistenceManager::ReadAllVideoPlayers Read TLV(CastingData) from KVS store with size: %lu bytes",
                    static_cast<unsigned long>(castingDataSize));

    TLV::TLVReader reader;
    reader.Init(castingData);

    // read the envelope (and version)
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType outerContainerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.EnterContainer(outerContainerType));
    ReturnErrorOnFailure(reader.Next());
    TLV::Tag outerContainerTag      = reader.GetTag();
    uint8_t outerContainerTagTagNum = static_cast<uint8_t>(TLV::TagNumFromTag(outerContainerTag));
    VerifyOrReturnError(outerContainerTagTagNum == kCurrentCastingDataVersionTag, CHIP_ERROR_INVALID_TLV_TAG);
    uint32_t version;
    ReturnErrorOnFailure(reader.Get(version));
    ChipLogProgress(AppServer, "PersistenceManager::ReadAllVideoPlayers TLV(CastingData) version: %d", version);

    // Entering Video Players container
    TLV::TLVType videoPlayersContainerType = TLV::kTLVType_Array;
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(videoPlayersContainerType));
    size_t videoPlayerIndex                             = 0;
    chip::NodeId nodeId                                 = 0;
    chip::FabricIndex fabricIndex                       = 0;
    uint16_t vendorId                                   = 0;
    uint16_t productId                                  = 0;
    uint16_t deviceType                                 = 0;
    char deviceName[chip::Dnssd::kMaxDeviceNameLen + 1] = {};
    size_t numIPs                                       = 0;
    Inet::IPAddress ipAddress[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];
    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag videoPlayersContainerTag = reader.GetTag();
        if (!TLV::IsContextTag(videoPlayersContainerTag))
        {
            ChipLogError(AppServer, "Unexpected non-context TLV tag.");
            return CHIP_ERROR_INVALID_TLV_TAG;
        }

        uint8_t videoPlayersContainerTagNum = static_cast<uint8_t>(TLV::TagNumFromTag(videoPlayersContainerTag));
        if (videoPlayersContainerTagNum == kNodeIdTag)
        {
            ReturnErrorOnFailure(reader.Get(nodeId));
            continue;
        }

        if (videoPlayersContainerTagNum == kFabricIndexTag)
        {
            ReturnErrorOnFailure(reader.Get(fabricIndex));
            continue;
        }

        if (videoPlayersContainerTagNum == kVideoPlayerVendorIdTag)
        {
            ReturnErrorOnFailure(reader.Get(vendorId));
            continue;
        }

        if (videoPlayersContainerTagNum == kVideoPlayerProductIdTag)
        {
            ReturnErrorOnFailure(reader.Get(productId));
            continue;
        }

        if (videoPlayersContainerTagNum == kVideoPlayerDeviceTypeIdTag)
        {
            ReturnErrorOnFailure(reader.Get(deviceType));
            continue;
        }

        if (videoPlayersContainerTagNum == kVideoPlayerDeviceNameTag)
        {
            ReturnErrorOnFailure(reader.GetBytes(reinterpret_cast<uint8_t *>(deviceName), chip::Dnssd::kMaxDeviceNameLen + 1));
            continue;
        }

        if (videoPlayersContainerTagNum == kVideoPlayerNumIPsTag)
        {
            ReturnErrorOnFailure(reader.Get(reinterpret_cast<uint64_t &>(numIPs)));
            continue;
        }

        if (videoPlayersContainerTagNum == kIpAddressesContainerTag)
        {
            // Entering IP Addresses container
            TLV::TLVType ipAddressesContainerType = TLV::kTLVType_Array;
            ReturnErrorOnFailure(reader.EnterContainer(ipAddressesContainerType));

            size_t ipCount = 0;
            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                TLV::Tag ipAddressesContainerTag = reader.GetTag();
                if (!TLV::IsContextTag(ipAddressesContainerTag))
                {
                    ChipLogError(AppServer, "Unexpected non-context TLV tag.");
                    return CHIP_ERROR_INVALID_TLV_TAG;
                }

                uint8_t ipAddressesContainerTagNum = static_cast<uint8_t>(TLV::TagNumFromTag(ipAddressesContainerTag));
                if (ipAddressesContainerTagNum == kVideoPlayerIPAddressTag)
                {
                    char ipAddressStr[Inet::IPAddress::kMaxStringLength];
                    ReturnErrorOnFailure(
                        reader.GetBytes(reinterpret_cast<uint8_t *>(ipAddressStr), Inet::IPAddress::kMaxStringLength));

                    Inet::IPAddress addressInet;
                    VerifyOrReturnError(Inet::IPAddress::FromString(ipAddressStr, addressInet), CHIP_ERROR_INVALID_TLV_ELEMENT);
                    ipAddress[ipCount] = addressInet;
                    ipCount++;
                    continue;
                }
            }
            if (err == CHIP_END_OF_TLV)
            {
                // Exiting IP Addresses container
                ReturnErrorOnFailure(reader.ExitContainer(ipAddressesContainerType));
                continue;
            }
        }

        if (videoPlayersContainerTagNum == kContentAppEndpointsContainerTag)
        {
            outVideoPlayers[videoPlayerIndex].Initialize(nodeId, fabricIndex, nullptr, nullptr, vendorId, productId, deviceType,
                                                         deviceName, numIPs, ipAddress);
            // Entering Content App Endpoints container
            TLV::TLVType contentAppEndpointArrayContainerType = TLV::kTLVType_Array;
            ReturnErrorOnFailure(reader.EnterContainer(contentAppEndpointArrayContainerType));

            // reset all endpoints
            TargetEndpointInfo * endpoints = outVideoPlayers[videoPlayerIndex].GetEndpoints();
            if (endpoints != nullptr)
            {
                for (size_t i = 0; i < kMaxNumberOfEndpoints; i++)
                {
                    endpoints[i].Reset();
                }
            }
            TargetEndpointInfo * endpoint = nullptr;
            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                TLV::Tag contentAppsContainerTag = reader.GetTag();
                if (!TLV::IsContextTag(contentAppsContainerTag))
                {
                    ChipLogError(AppServer, "Unexpected non-context TLV tag.");
                    return CHIP_ERROR_INVALID_TLV_TAG;
                }

                uint8_t contentAppsContainerTagNum = static_cast<uint8_t>(TLV::TagNumFromTag(contentAppsContainerTag));
                if (contentAppsContainerTagNum == kEndpointIdTag)
                {
                    chip::EndpointId endpointId;
                    ReturnErrorOnFailure(reader.Get(endpointId));
                    endpoint = outVideoPlayers[videoPlayerIndex].GetOrAddEndpoint(endpointId);
                    continue;
                }

                if (endpoint != nullptr && contentAppsContainerTagNum == kClusterIdsContainerTag)
                {
                    // Entering ClusterIds container
                    TLV::TLVType clusterIdArrayContainerType = TLV::kTLVType_Array;
                    ReturnErrorOnFailure(reader.EnterContainer(clusterIdArrayContainerType));
                    while ((err = reader.Next()) == CHIP_NO_ERROR)
                    {
                        TLV::Tag clusterIdsContainerTag = reader.GetTag();
                        if (!TLV::IsContextTag(clusterIdsContainerTag))
                        {
                            ChipLogError(AppServer, "Unexpected non-context TLV tag.");
                            return CHIP_ERROR_INVALID_TLV_TAG;
                        }

                        uint8_t clusterIdsContainerTagNum = static_cast<uint8_t>(TLV::TagNumFromTag(clusterIdsContainerTag));
                        if (clusterIdsContainerTagNum == kClusterIdTag)
                        {
                            chip::ClusterId clusterId;
                            ReturnErrorOnFailure(reader.Get(clusterId));
                            if (clusterId != kInvalidClusterId)
                            {
                                endpoint->AddCluster(clusterId);
                            }
                            else
                            {
                                ChipLogError(AppServer, "PersistenceManager: ReadAllVideoPlayers ignoring invalid clusterId");
                            }
                            continue;
                        }
                    }
                    if (err == CHIP_END_OF_TLV)
                    {
                        // Exiting ClusterIds container
                        ReturnErrorOnFailure(reader.ExitContainer(clusterIdArrayContainerType));
                        continue;
                    }
                }
            }
            if (err == CHIP_END_OF_TLV)
            {
                // Exiting Content App Endpoints container
                ReturnErrorOnFailure(reader.ExitContainer(contentAppEndpointArrayContainerType));
                videoPlayerIndex++;
                continue;
            }
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(videoPlayersContainerType));
    ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistenceManager::PurgeVideoPlayerCache()
{
    ChipLogProgress(AppServer, "PersistenceManager::PurgeVideoPlayerCache called");
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kCastingDataKey);
}
