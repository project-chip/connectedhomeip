/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CastingStore.h"

#include <lib/core/TLV.h>
#include <platform/KeyValueStoreManager.h>

namespace matter {
namespace casting {
namespace support {

CastingStore * CastingStore::_CastingStore = nullptr;

CastingStore::CastingStore() {}

CastingStore * CastingStore::GetInstance()
{
    if (_CastingStore == nullptr)
    {
        _CastingStore = new CastingStore();
    }
    return _CastingStore;
}

CHIP_ERROR CastingStore::AddOrUpdate(core::CastingPlayer castingPlayer)
{
    ChipLogProgress(AppServer, "CastingStore::AddOrUpdate() called with CastingPlayer deviceName: %s",
                    castingPlayer.GetDeviceName());

    // Read cache of CastingPlayers
    std::vector<core::CastingPlayer> castingPlayers = ReadAll();

    // search for castingPlayer in CastingStore cache and overwrite it, if found
    if (castingPlayers.size() != 0)
    {
        auto it = std::find_if(
            castingPlayers.begin(), castingPlayers.end(),
            [castingPlayer](const core::CastingPlayer & castingPlayerParam) { return castingPlayerParam == castingPlayer; });

        if (it != castingPlayers.end())
        {
            unsigned index        = (unsigned int) std::distance(castingPlayers.begin(), it);
            castingPlayers[index] = castingPlayer;
            ChipLogProgress(AppServer, "CastingStore::AddOrUpdate() updating CastingPlayer in CastingStore cache");
            return WriteAll(castingPlayers); // return early
        }
    }

    // add *new* castingPlayer to CastingStore cache
    castingPlayers.push_back(castingPlayer);
    ChipLogProgress(AppServer, "CastingStore::AddOrUpdate() adding new CastingPlayer in CastingStore cache");
    return WriteAll(castingPlayers);
}

std::vector<core::CastingPlayer> CastingStore::ReadAll()
{
    ChipLogProgress(AppServer, "CastingStore::ReadAll() called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::vector<core::CastingPlayer> castingPlayers;
    uint8_t castingStoreData[kCastingStoreDataMaxBytes];
    size_t castingStoreDataSize = 0;
    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kCastingStoreDataKey, castingStoreData,
                                                                      kCastingStoreDataMaxBytes, &castingStoreDataSize);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "KeyValueStoreMgr.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
    ChipLogProgress(AppServer, "CastingStore::ReadAll() Read TLV(CastingStoreData) from KVS store with size: %lu bytes",
                    static_cast<unsigned long>(castingStoreDataSize));

    chip::TLV::TLVReader reader;
    reader.Init(castingStoreData);

    // read the envelope (and version)
    err = reader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag());
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));

    chip::TLV::TLVType outerContainerType;
    err = reader.EnterContainer(outerContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = reader.Next();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));
    chip::TLV::Tag outerContainerTag = reader.GetTag();
    uint8_t outerContainerTagNum     = static_cast<uint8_t>(chip::TLV::TagNumFromTag(outerContainerTag));
    VerifyOrReturnValue(outerContainerTagNum == kCastingStoreDataVersionTag, castingPlayers,
                        ChipLogError(AppServer, "CastingStoreDataVersionTag not found"));
    uint32_t version;
    err = reader.Get(version);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
    ChipLogProgress(AppServer, "CastingStore::ReadAll() TLV(CastingStoreData) version: %d", version);

    // Entering CastingPlayers container
    err = reader.Next();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));
    chip::TLV::TLVType castingPlayersContainerType;
    err = reader.EnterContainer(castingPlayersContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // Entering CastingPlayer container
        chip::TLV::TLVType castingPlayerContainerType;
        err = reader.EnterContainer(castingPlayerContainerType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                            ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

        core::CastingPlayerAttributes attributes;
        std::vector<core::EndpointAttributes> endpointAttributesList;
        std::map<chip::EndpointId, std::vector<chip::ClusterId>> endpointServerListMap;
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            chip::TLV::Tag castingPlayerContainerTag = reader.GetTag();
            VerifyOrReturnValue(chip::TLV::IsContextTag(castingPlayerContainerTag), std::vector<core::CastingPlayer>(),
                                ChipLogError(AppServer, "Unexpected non-context TLV tag"));

            uint8_t castingPlayerContainerTagNum = static_cast<uint8_t>(chip::TLV::TagNumFromTag(castingPlayerContainerTag));
            if (castingPlayerContainerTagNum == kCastingPlayerIdTag)
            {
                err = reader.GetBytes(reinterpret_cast<uint8_t *>(attributes.id), core::kIdMaxLength + 1);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.GetBytes failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerNodeIdTag)
            {
                err = reader.Get(attributes.nodeId);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerFabricIndexTag)
            {
                err = reader.Get(attributes.fabricIndex);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerVendorIdTag)
            {
                err = reader.Get(attributes.vendorId);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerProductIdTag)
            {
                err = reader.Get(attributes.productId);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerDeviceTypeIdTag)
            {
                err = reader.Get(attributes.deviceType);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerSupportsCommissionerGeneratedPasscodeTag)
            {
                err = reader.Get(attributes.supportsCommissionerGeneratedPasscode);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerPortTag)
            {
                err = reader.Get(attributes.port);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerInstanceNameTag)
            {
                err = reader.GetBytes(reinterpret_cast<uint8_t *>(attributes.instanceName),
                                      chip::Dnssd::Commission::kInstanceNameMaxLength + 1);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.GetBytes failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerDeviceNameTag)
            {
                err = reader.GetBytes(reinterpret_cast<uint8_t *>(attributes.deviceName), chip::Dnssd::kMaxDeviceNameLen + 1);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.GetBytes failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerHostNameTag)
            {
                err = reader.GetBytes(reinterpret_cast<uint8_t *>(attributes.hostName), chip::Dnssd::kHostNameMaxLength + 1);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.GetBytes failed %" CHIP_ERROR_FORMAT, err.Format()));
                continue;
            }

            if (castingPlayerContainerTagNum == kCastingPlayerEndpointsContainerTag)
            {
                // Entering Endpoints container
                chip::TLV::TLVType endpointsContainerType;
                err = reader.EnterContainer(endpointsContainerType);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
                core::EndpointAttributes endpointAttributes;
                std::vector<chip::ClusterId> serverList;
                while ((err = reader.Next()) == CHIP_NO_ERROR)
                {
                    // Entering Endpoint container
                    chip::TLV::TLVType endpointContainerType;
                    err = reader.EnterContainer(endpointContainerType);
                    VerifyOrReturnValue(
                        err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                    while ((err = reader.Next()) == CHIP_NO_ERROR)
                    {
                        chip::TLV::Tag endpointContainerTag = reader.GetTag();
                        VerifyOrReturnValue(chip::TLV::IsContextTag(endpointContainerTag), std::vector<core::CastingPlayer>(),
                                            ChipLogError(AppServer, "Unexpected non-context TLV tag"));

                        uint8_t endpointContainerTagNum = static_cast<uint8_t>(chip::TLV::TagNumFromTag(endpointContainerTag));
                        if (endpointContainerTagNum == kCastingPlayerEndpointIdTag)
                        {
                            err = reader.Get(endpointAttributes.mId);
                            VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                                ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                            continue;
                        }

                        if (endpointContainerTagNum == kCastingPlayerEndpointVendorIdTag)
                        {
                            err = reader.Get(endpointAttributes.mVendorId);
                            VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                                ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                            continue;
                        }

                        if (endpointContainerTagNum == kCastingPlayerEndpointProductIdTag)
                        {
                            err = reader.Get(endpointAttributes.mProductId);
                            VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                                ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                            continue;
                        }

                        std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> deviceTypeList;
                        if (endpointContainerTagNum == kCastingPlayerEndpointDeviceTypeListContainerTag)
                        {
                            // Entering DeviceTypeList container
                            chip::TLV::TLVType deviceTypeListContainerType;
                            err = reader.EnterContainer(deviceTypeListContainerType);
                            VerifyOrReturnValue(
                                err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                            while ((err = reader.Next()) == CHIP_NO_ERROR)
                            {
                                // Entering DeviceTypeStruct container
                                chip::TLV::TLVType deviceTypeStructContainerType;
                                err = reader.EnterContainer(deviceTypeStructContainerType);
                                VerifyOrReturnValue(
                                    err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                                chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType deviceTypeStruct;
                                while ((err = reader.Next()) == CHIP_NO_ERROR)
                                {
                                    chip::TLV::Tag deviceTypeStructContainerTag = reader.GetTag();
                                    VerifyOrReturnValue(chip::TLV::IsContextTag(deviceTypeStructContainerTag),
                                                        std::vector<core::CastingPlayer>(),
                                                        ChipLogError(AppServer, "Unexpected non-context TLV tag"));

                                    uint8_t deviceTypeStructContainerTagNum =
                                        static_cast<uint8_t>(chip::TLV::TagNumFromTag(deviceTypeStructContainerTag));
                                    if (deviceTypeStructContainerTagNum == kCastingPlayerEndpointDeviceTypeTag)
                                    {
                                        err = reader.Get(deviceTypeStruct.deviceType);
                                        VerifyOrReturnValue(
                                            err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                            ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                                        continue;
                                    }

                                    if (deviceTypeStructContainerTagNum == kCastingPlayerEndpointDeviceTypeRevisionTag)
                                    {
                                        err = reader.Get(deviceTypeStruct.revision);
                                        VerifyOrReturnValue(
                                            err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                            ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                                        continue;
                                    }
                                }

                                if (err == CHIP_END_OF_TLV)
                                {
                                    // Exiting DeviceTypeStruct container
                                    err = reader.ExitContainer(deviceTypeStructContainerType);
                                    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                                        ChipLogError(AppServer,
                                                                     "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT,
                                                                     err.Format()));

                                    deviceTypeList.push_back(deviceTypeStruct);
                                    continue;
                                }
                            }
                            if (err == CHIP_END_OF_TLV)
                            {
                                // Exiting DeviceTypeList container
                                err = reader.ExitContainer(deviceTypeListContainerType);
                                VerifyOrReturnValue(
                                    err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                                endpointAttributes.mDeviceTypeList = deviceTypeList;
                                continue;
                            }
                        }

                        if (endpointContainerTagNum == kCastingPlayerEndpointServerListContainerTag)
                        {
                            // Entering ServerList container
                            chip::TLV::TLVType serverListContainerType;
                            err = reader.EnterContainer(serverListContainerType);
                            VerifyOrReturnValue(
                                err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                            while ((err = reader.Next()) == CHIP_NO_ERROR)
                            {
                                chip::TLV::Tag serverListContainerTag = reader.GetTag();
                                VerifyOrReturnValue(chip::TLV::IsContextTag(serverListContainerTag),
                                                    std::vector<core::CastingPlayer>(),
                                                    ChipLogError(AppServer, "Unexpected non-context TLV tag"));

                                uint8_t serverListContainerTagNum =
                                    static_cast<uint8_t>(chip::TLV::TagNumFromTag(serverListContainerTag));
                                if (serverListContainerTagNum == kCastingPlayerEndpointServerClusterIdTag)
                                {
                                    chip::ClusterId clusterId;
                                    err = reader.Get(clusterId);
                                    VerifyOrReturnValue(
                                        err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                        ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
                                    serverList.push_back(clusterId);
                                    continue;
                                }
                            }

                            if (err == CHIP_END_OF_TLV)
                            {
                                // Exiting ServerList container
                                err = reader.ExitContainer(serverListContainerType);
                                VerifyOrReturnValue(
                                    err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
                                continue;
                            }
                        }
                    }

                    if (err == CHIP_END_OF_TLV)
                    {
                        // Exiting Endpoint container
                        err = reader.ExitContainer(endpointContainerType);
                        VerifyOrReturnValue(
                            err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                            ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

                        endpointAttributesList.push_back(endpointAttributes);
                        endpointServerListMap[endpointAttributes.mId] = serverList;
                        serverList.clear();
                        continue;
                    }
                }

                if (err == CHIP_END_OF_TLV)
                {
                    // Exiting Endpoints container
                    err = reader.ExitContainer(endpointsContainerType);
                    VerifyOrReturnValue(
                        err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
                    continue;
                }
            }
        }
        if (err == CHIP_END_OF_TLV)
        {
            // Exiting CastingPlayer container
            err = reader.ExitContainer(castingPlayerContainerType);
            VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

            // create a castingPlayer with Endpoints and add it to the castingPlayers to be returned
            core::CastingPlayer * castingPlayer = new core::CastingPlayer(attributes);
            for (auto & endpointAttributes : endpointAttributesList)
            {
                std::shared_ptr<core::Endpoint> endpoint(new core::Endpoint(castingPlayer, endpointAttributes));
                ChipLogProgress(AppServer, "CastingStore::ReadAll() endpointServerListMap[endpointAttributes.mId].size(): %d",
                                static_cast<int>(endpointServerListMap[endpointAttributes.mId].size()));
                endpoint->RegisterClusters(endpointServerListMap[endpointAttributes.mId]);
                castingPlayer->RegisterEndpoint(endpoint);
                ChipLogProgress(AppServer, "CastingStore::ReadAll() Registered endpointID: %d", endpoint->GetId());
            }
            ChipLogProgress(AppServer, "CastingStore::ReadAll() Created CastingPlayer with deviceName: %s",
                            castingPlayer->GetDeviceName());
            castingPlayers.push_back(*castingPlayer);
            continue;
        }
    }

    VerifyOrReturnValue(err == CHIP_END_OF_TLV, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLV parsing failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = reader.ExitContainer(castingPlayersContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = reader.ExitContainer(outerContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

    ChipLogProgress(AppServer, "CastingStore::ReadAll() CastingPlayers size: %lu",
                    static_cast<unsigned long>(castingPlayers.size()));
    return castingPlayers;
}

CHIP_ERROR CastingStore::WriteAll(std::vector<core::CastingPlayer> castingPlayers)
{
    ChipLogProgress(AppServer, "CastingStore::WriteAll() called");

    chip::TLV::TLVWriter tlvWriter;
    uint8_t castingStoreData[kCastingStoreDataMaxBytes];
    tlvWriter.Init(castingStoreData, kCastingStoreDataMaxBytes);

    chip::TLV::TLVType outerContainerType;
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingStoreDataVersionTag), kCurrentCastingStoreDataVersion));

    chip::TLV::TLVType castingPlayersContainerType;
    // CastingPlayers container starts
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayersContainerTag), chip::TLV::kTLVType_Array,
                                                  castingPlayersContainerType));

    for (auto & castingPlayer : castingPlayers)
    {
        ChipLogProgress(AppServer, "CastingStore::WriteAll() writing castingPlayer:");
        chip::TLV::TLVType castingPlayerContainerType;
        // CastingPlayer container starts
        ReturnErrorOnFailure(
            tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, castingPlayerContainerType));

        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerIdTag), (const uint8_t *) castingPlayer.GetId(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetId()) + 1)));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerNodeIdTag), castingPlayer.GetNodeId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerFabricIndexTag), castingPlayer.GetFabricIndex()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerVendorIdTag), castingPlayer.GetVendorId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerProductIdTag), castingPlayer.GetProductId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerDeviceTypeIdTag), castingPlayer.GetDeviceType()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerSupportsCommissionerGeneratedPasscodeTag),
                                           castingPlayer.GetSupportsCommissionerGeneratedPasscode()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerPortTag), castingPlayer.GetPort()));
        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerInstanceNameTag),
                                                (const uint8_t *) castingPlayer.GetInstanceName(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetInstanceName()) + 1)));
        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerDeviceNameTag),
                                                (const uint8_t *) castingPlayer.GetDeviceName(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetDeviceName()) + 1)));
        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerHostNameTag),
                                                (const uint8_t *) castingPlayer.GetHostName(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetHostName()) + 1)));

        // Endpoints container starts
        chip::TLV::TLVType endpointsContainerType;
        ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayerEndpointsContainerTag),
                                                      chip::TLV::kTLVType_Array, endpointsContainerType));
        std::vector<memory::Strong<core::Endpoint>> endpoints = core::CastingPlayer::GetTargetCastingPlayer()->GetEndpoints();
        for (auto & endpoint : endpoints)
        {
            ChipLogProgress(AppServer, "CastingStore::WriteAll() writing CastingPlayer Endpoint with endpointId: %d",
                            endpoint->GetId());
            chip::TLV::TLVType endpointContainerType;
            // Endpoint container starts
            ReturnErrorOnFailure(
                tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, endpointContainerType));
            ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointIdTag), endpoint->GetId()));
            ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointVendorIdTag), endpoint->GetVendorId()));
            ReturnErrorOnFailure(
                tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointProductIdTag), endpoint->GetProductId()));

            // DeviceTypeList container starts
            chip::TLV::TLVType deviceTypeListContainerType;
            ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayerEndpointDeviceTypeListContainerTag),
                                                          chip::TLV::kTLVType_Array, deviceTypeListContainerType));
            std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> deviceTypeList =
                endpoint->GetDeviceTypeList();
            for (chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType deviceTypeStruct : deviceTypeList)
            {
                chip::TLV::TLVType deviceTypeStructContainerType;
                // DeviceTypeStruct container starts
                ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure,
                                                              deviceTypeStructContainerType));
                ReturnErrorOnFailure(
                    tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointDeviceTypeTag), deviceTypeStruct.deviceType));
                ReturnErrorOnFailure(
                    tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointDeviceTypeRevisionTag), deviceTypeStruct.revision));

                // DeviceTypeStruct container ends
                ReturnErrorOnFailure(tlvWriter.EndContainer(deviceTypeStructContainerType));
            }
            // DeviceTypeList container ends
            ReturnErrorOnFailure(tlvWriter.EndContainer(deviceTypeListContainerType));

            // ServerList container starts
            chip::TLV::TLVType serverListContainerType;
            ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayerEndpointServerListContainerTag),
                                                          chip::TLV::kTLVType_Structure, serverListContainerType));
            std::vector<chip::ClusterId> serverList = endpoint->GetServerList();
            ChipLogProgress(AppServer, "CastingStore::WriteAll() writing CastingPlayer Endpoint ServerList:");
            for (chip::ClusterId clusterId : serverList)
            {
                ChipLogProgress(AppServer, "CastingStore::WriteAll() clusterId: %d", clusterId);
                ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerEndpointServerClusterIdTag), clusterId));
            }
            // ServerList container ends
            ReturnErrorOnFailure(tlvWriter.EndContainer(serverListContainerType));

            // Endpoint container ends
            ReturnErrorOnFailure(tlvWriter.EndContainer(endpointContainerType));
        }
        // Endpoints container ends
        ReturnErrorOnFailure(tlvWriter.EndContainer(endpointsContainerType));

        // CastingPlayer container ends
        ReturnErrorOnFailure(tlvWriter.EndContainer(castingPlayerContainerType));
    }

    // CastingPlayers container ends
    ReturnErrorOnFailure(tlvWriter.EndContainer(castingPlayersContainerType));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));

    ReturnErrorOnFailure(tlvWriter.Finalize());
    ChipLogProgress(AppServer,
                    "CastingStore::WriteAll() TLV(CastingStoreData).LengthWritten: %d bytes, CastingPlayers size: %lu "
                    "and version: %d",
                    tlvWriter.GetLengthWritten(), static_cast<unsigned long>(castingPlayers.size()),
                    kCurrentCastingStoreDataVersion);
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kCastingStoreDataKey, castingStoreData,
                                                                       tlvWriter.GetLengthWritten());
}

CHIP_ERROR CastingStore::DeleteAll()
{
    ChipLogProgress(AppServer, "CastingStore::DeleteAll called");
    CHIP_ERROR err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kCastingStoreDataKey);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) // no error, if the key-value pair was not stored
    {
        ChipLogProgress(AppServer, "CastingStore::DeleteAll ignoring error %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR CastingStore::Delete(core::CastingPlayer castingPlayer)
{
    ChipLogProgress(AppServer, "CastingStore::Delete");

    // Read cache of CastingPlayers
    std::vector<core::CastingPlayer> castingPlayers = ReadAll();

    // search for castingPlayer in CastingStore cache and delete it, if found
    if (castingPlayers.size() != 0)
    {
        auto it = std::find_if(
            castingPlayers.begin(), castingPlayers.end(),
            [castingPlayer](const core::CastingPlayer & castingPlayerParam) { return castingPlayerParam == castingPlayer; });

        if (it != castingPlayers.end())
        {
            ChipLogProgress(AppServer, "CastingStore::Delete deleting CastingPlayer %s from CastingStore cache", it->GetId());
            castingPlayers.erase(it);
            return WriteAll(castingPlayers);
        }
    }
    return CHIP_NO_ERROR;
}

void CastingStore::OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex)
{
    ChipLogProgress(AppServer, "CastingStore::OnFabricRemoved");

    // Read cache of CastingPlayers
    std::vector<core::CastingPlayer> castingPlayers = ReadAll();

    // search for castingPlayer in CastingStore cache and delete it, if found
    if (castingPlayers.size() != 0)
    {
        auto it = std::find_if(castingPlayers.begin(), castingPlayers.end(),
                               [fabricIndex](const core::CastingPlayer & castingPlayerParam) {
                                   return castingPlayerParam.GetFabricIndex() == fabricIndex;
                               });

        if (it != castingPlayers.end())
        {
            ChipLogProgress(AppServer, "CastingStore::OnFabricRemoved deleting CastingPlayer %s from CastingStore cache",
                            it->GetId());
            castingPlayers.erase(it);
            WriteAll(castingPlayers);
        }
    }
}

}; // namespace support
}; // namespace casting
}; // namespace matter
