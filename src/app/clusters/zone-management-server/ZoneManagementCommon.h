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
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

// Shared storage and delegate types live in this header to avoid an include cycle:
// the code-driven cluster needs them, and the legacy ZoneMgmtServer compatibility
// wrapper also needs them. Keeping them here lets the wrapper include the cluster
// without making the cluster header depend on the wrapper header.

using TwoDCartesianZoneDecodableStruct = Structs::TwoDCartesianZoneStruct::DecodableType;
using TwoDCartesianZoneStruct          = Structs::TwoDCartesianZoneStruct::Type;
using TwoDCartesianVertexStruct        = Structs::TwoDCartesianVertexStruct::Type;
using ZoneInformationStruct            = Structs::ZoneInformationStruct::Type;
using ZoneTriggerControlStruct         = Structs::ZoneTriggerControlStruct::Type;

class ZoneMgmtServer;

struct TwoDCartesianZoneStorage : TwoDCartesianZoneStruct
{
    TwoDCartesianZoneStorage() = default;

    TwoDCartesianZoneStorage(const CharSpan & aName, ZoneUseEnum aUse, const std::vector<TwoDCartesianVertexStruct> & aVertices,
                             Optional<CharSpan> aColor)
    {
        Set(aName, aUse, aVertices, aColor);
    }

    TwoDCartesianZoneStorage(const TwoDCartesianZoneStorage & aTwoDCartZone) { *this = aTwoDCartZone; }

    TwoDCartesianZoneStorage & operator=(const TwoDCartesianZoneStorage & aTwoDCartZone)
    {
        Set(aTwoDCartZone.name, aTwoDCartZone.use, aTwoDCartZone.verticesVector, aTwoDCartZone.color);
        return *this;
    }

    void Set(const CharSpan & aName, ZoneUseEnum aUse, const std::vector<TwoDCartesianVertexStruct> & aVertices,
             Optional<CharSpan> aColor)
    {
        nameString     = std::string(aName.begin(), aName.end());
        name           = CharSpan(nameString.c_str(), nameString.size());
        use            = aUse;
        verticesVector = aVertices;
        vertices       = DataModel::List<TwoDCartesianVertexStruct>(verticesVector.data(), verticesVector.size());
        if (aColor.HasValue())
        {
            colorString = std::string(aColor.Value().begin(), aColor.Value().end());
            color       = MakeOptional(CharSpan(colorString.c_str(), colorString.size()));
        }
        else
        {
            colorString.clear();
            color = NullOptional;
        }
    }

    std::string nameString;
    std::string colorString;
    std::vector<TwoDCartesianVertexStruct> verticesVector;
};

struct ZoneInformationStorage : ZoneInformationStruct
{
    ZoneInformationStorage() = default;

    ZoneInformationStorage(const uint16_t & aZoneID, ZoneTypeEnum aZoneType, ZoneSourceEnum aZoneSource,
                           const Optional<TwoDCartesianZoneStorage> & aTwoDCartZoneStorage)
    {
        Set(aZoneID, aZoneType, aZoneSource, aTwoDCartZoneStorage);
    }

    ZoneInformationStorage(const ZoneInformationStorage & aZoneInfoStorage) { *this = aZoneInfoStorage; }

    ZoneInformationStorage & operator=(const ZoneInformationStorage & aZoneInfoStorage)
    {
        Set(aZoneInfoStorage.zoneID, aZoneInfoStorage.zoneType, aZoneInfoStorage.zoneSource, aZoneInfoStorage.twoDCartZoneStorage);
        return *this;
    }

    void Set(const uint16_t & aZoneID, ZoneTypeEnum aZoneType, ZoneSourceEnum aZoneSource,
             const Optional<TwoDCartesianZoneStorage> & aTwoDCartZoneStorage)
    {
        zoneID              = aZoneID;
        zoneType            = aZoneType;
        zoneSource          = aZoneSource;
        twoDCartZoneStorage = aTwoDCartZoneStorage;
        twoDCartesianZone   = twoDCartZoneStorage;
    }

    Optional<TwoDCartesianZoneStorage> twoDCartZoneStorage;
};

class Delegate
{
public:
    Delegate() = default;
    virtual ~Delegate() = default;

    virtual Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                        uint16_t & outZoneID) = 0;
    virtual Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone) = 0;
    virtual Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID) = 0;
    virtual Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct & zoneTrigger) = 0;
    virtual Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger) = 0;
    virtual Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID) = 0;
    virtual void OnAttributeChanged(AttributeId attributeId)                    = 0;
    virtual CHIP_ERROR PersistentAttributesLoadedCallback()                     = 0;
    virtual CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> & aZones) = 0;
    virtual CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> & aTriggers) = 0;

    // Legacy applications may still discover the compatibility wrapper via the
    // delegate. The wrapper owns this pointer so the code-driven cluster API
    // stays independent of ZoneMgmtServer.
    ZoneMgmtServer * GetZoneMgmtServer() const { return mZoneMgmtServer; }

private:
    friend class ZoneMgmtServer;

    ZoneMgmtServer * mZoneMgmtServer = nullptr;

    void SetZoneMgmtServer(ZoneMgmtServer * aZoneMgmtServer) { mZoneMgmtServer = aZoneMgmtServer; }
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
