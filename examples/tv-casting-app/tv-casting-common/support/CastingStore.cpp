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
    ChipLogProgress(AppServer, "CastingStore::AddOrUpdate");

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
            ChipLogProgress(AppServer, "CastingStore::AddOrUpdate updating CastingPlayer in CastingStore cache");
            return WriteAll(castingPlayers); // return early
        }
    }

    // add *new* castingPlayer to CastingStore cache
    castingPlayers.push_back(castingPlayer);
    ChipLogProgress(AppServer, "CastingStore::AddOrUpdate adding new CastingPlayer in CastingStore cache");
    return WriteAll(castingPlayers);
}

std::vector<core::CastingPlayer> CastingStore::ReadAll()
{
    ChipLogProgress(AppServer, "CastingStore::ReadAll called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::vector<core::CastingPlayer> castingPlayers;
    uint8_t castingStoreData[kCastingStoreDataMaxBytes];
    size_t castingStoreDataSize = 0;
    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kCastingStoreDataKey, castingStoreData,
                                                                      kCastingStoreDataMaxBytes, &castingStoreDataSize);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "KeyValueStoreMgr.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
    ChipLogProgress(AppServer, "CastingStore::ReadAll Read TLV(CastingStoreData) from KVS store with size: %lu bytes",
                    static_cast<unsigned long>(castingStoreDataSize));

    chip::TLV::TLVReader reader;
    reader.Init(castingStoreData);

    // read the envelope (and version)
    err = reader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag());
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));

    chip::TLV::TLVType outerContainerType = chip::TLV::kTLVType_Structure;
    err                                   = reader.EnterContainer(outerContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = reader.Next();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));
    chip::TLV::Tag outerContainerTag = reader.GetTag();
    uint8_t outerContainerTagTagNum  = static_cast<uint8_t>(chip::TLV::TagNumFromTag(outerContainerTag));
    VerifyOrReturnValue(outerContainerTagTagNum == kCastingStoreDataVersionTag, castingPlayers,
                        ChipLogError(AppServer, "CastingStoreDataVersionTag not found"));
    uint32_t version;
    err = reader.Get(version);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Get failed %" CHIP_ERROR_FORMAT, err.Format()));
    ChipLogProgress(AppServer, "CastingStore::ReadAll TLV(CastingStoreData) version: %d", version);

    // Entering CastingPlayers container
    chip::TLV::TLVType castingPlayersContainerType = chip::TLV::kTLVType_Array;
    err                                            = reader.Next();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.Next failed %" CHIP_ERROR_FORMAT, err.Format()));
    err = reader.EnterContainer(castingPlayersContainerType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                        ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // Entering CastingPlayer container
        chip::TLV::TLVType castingPlayerContainerType = chip::TLV::kTLVType_Structure;
        err                                           = reader.EnterContainer(castingPlayerContainerType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                            ChipLogError(AppServer, "TLVReader.EnterContainer failed %" CHIP_ERROR_FORMAT, err.Format()));

        core::CastingPlayerAttributes attributes;
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

            if (err == CHIP_END_OF_TLV)
            {
                // Exiting CastingPlayer container
                err = reader.ExitContainer(castingPlayerContainerType);
                VerifyOrReturnValue(err == CHIP_NO_ERROR, std::vector<core::CastingPlayer>(),
                                    ChipLogError(AppServer, "TLVReader.ExitContainer failed %" CHIP_ERROR_FORMAT, err.Format()));
                core::CastingPlayer castingPlayer(attributes);
                castingPlayers.push_back(castingPlayer);
                break;
            }
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

    ChipLogProgress(AppServer, "CastingStore::ReadAll CastingPlayers size: %lu", static_cast<unsigned long>(castingPlayers.size()));
    return castingPlayers;
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

CHIP_ERROR CastingStore::WriteAll(std::vector<core::CastingPlayer> castingPlayers)
{
    ChipLogProgress(AppServer, "CastingStore::WriteAll called");

    chip::TLV::TLVWriter tlvWriter;
    uint8_t castingStoreData[kCastingStoreDataMaxBytes];
    tlvWriter.Init(castingStoreData, kCastingStoreDataMaxBytes);

    chip::TLV::TLVType outerContainerType = chip::TLV::kTLVType_Structure;
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingStoreDataVersionTag), kCurrentCastingStoreDataVersion));

    chip::TLV::TLVType castingPlayersContainerType = chip::TLV::kTLVType_Array;
    // CastingPlayers container starts
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayersContainerTag), chip::TLV::kTLVType_Array,
                                                  castingPlayersContainerType));

    for (auto & castingPlayer : castingPlayers)
    {
        chip::TLV::TLVType castingPlayerContainerType = chip::TLV::kTLVType_Structure;
        // CastingPlayer container starts
        ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::ContextTag(kCastingPlayerContainerTag),
                                                      chip::TLV::kTLVType_Structure, castingPlayerContainerType));

        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerIdTag), castingPlayer.GetId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerNodeIdTag), castingPlayer.GetNodeId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerFabricIndexTag), castingPlayer.GetFabricIndex()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerVendorIdTag), castingPlayer.GetVendorId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerProductIdTag), castingPlayer.GetProductId()));
        ReturnErrorOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kCastingPlayerDeviceTypeIdTag), castingPlayer.GetDeviceType()));
        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerDeviceNameTag),
                                                (const uint8_t *) castingPlayer.GetDeviceName(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetDeviceName()) + 1)));
        ReturnErrorOnFailure(tlvWriter.PutBytes(chip::TLV::ContextTag(kCastingPlayerHostNameTag),
                                                (const uint8_t *) castingPlayer.GetHostName(),
                                                static_cast<uint32_t>(strlen(castingPlayer.GetHostName()) + 1)));
        // CastingPlayer container ends
        ReturnErrorOnFailure(tlvWriter.EndContainer(castingPlayerContainerType));
    }

    // CastingPlayers container ends
    ReturnErrorOnFailure(tlvWriter.EndContainer(castingPlayersContainerType));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));

    ReturnErrorOnFailure(tlvWriter.Finalize());
    ChipLogProgress(AppServer,
                    "CastingStore::WriteAll TLV(CastingStoreData).LengthWritten: %d bytes, CastingPlayers size: %lu "
                    "and version: %d",
                    tlvWriter.GetLengthWritten(), static_cast<unsigned long>(castingPlayers.size()),
                    kCurrentCastingStoreDataVersion);
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kCastingStoreDataKey, castingStoreData,
                                                                       tlvWriter.GetLengthWritten());
}

}; // namespace support
}; // namespace casting
}; // namespace matter
