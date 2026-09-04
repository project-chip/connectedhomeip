/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "RvcTranslator.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::NamedPipe {

CHIP_ERROR RvcTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry) const
{
    std::string action = json["Name"].asString();

    if (action == "Reset" || action == "Charged" || action == "Charging" || action == "Docked" || action == "ChargerFound" ||
        action == "LowCharge" || action == "ActivityComplete" || action == "AreaComplete" || action == "ClearError" ||
        action == "EmptyingDustBin" || action == "CleaningMop" || action == "FillingWaterTank" || action == "UpdatingMaps")
    {
        return DispatchAction(registry, CharSpan(action.data(), action.size()), endpointId);
    }
    if (action == "ErrorEvent")
    {
        if (!json.isMember("Error") || !json["Error"].isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        std::string err = json["Error"].asString();
        return DispatchStringAction(registry, "ErrorEvent"_span, endpointId, CharSpan(err.data(), err.size()));
    }
    if (action == "AddMap")
    {
        auto mapId = ExtractUInt<uint32_t>(json, "MapId");
        if (!mapId.has_value() || !json.isMember("MapName") || !json["MapName"].isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        std::string mapName = json["MapName"].asString();

        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *mapId));
        ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(3), CharSpan(mapName.data(), mapName.size())));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction("AddMap"_span, ByteSpan(buffer, writer.GetLengthWritten()));
    }
    if (action == "RemoveMap")
    {
        auto mapId = ExtractUInt<uint32_t>(json, "MapId");
        if (!mapId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return DispatchAction(registry, "RemoveMap"_span, endpointId, *mapId);
    }
    if (action == "AddArea")
    {
        auto areaId = ExtractUInt<uint32_t>(json, "AreaId");
        if (!areaId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        std::optional<uint32_t> mapId;
        if (json.isMember("MapId"))
        {
            mapId = ExtractUInt<uint32_t>(json, "MapId");
            if (!mapId.has_value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        std::optional<std::string> locName;
        if (json.isMember("LocationName"))
        {
            if (!json["LocationName"].isString())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            locName = json["LocationName"].asString();
        }

        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *areaId));
        if (mapId.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *mapId));
        }
        if (locName.has_value())
        {
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(4), CharSpan(locName->data(), locName->size())));
        }
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());

        return registry.HandleAction("AddArea"_span, ByteSpan(buffer, writer.GetLengthWritten()));
    }
    if (action == "RemoveArea")
    {
        auto areaId = ExtractUInt<uint32_t>(json, "AreaId");
        if (!areaId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return DispatchAction(registry, "RemoveArea"_span, endpointId, *areaId);
    }

    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::NamedPipe
