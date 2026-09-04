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

#include <device/types/robotic-vacuum-cleaner/impl/RvcNamedPipeSimulation.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::NamedPipe {

CHIP_ERROR RvcTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json, OOBAccessorRegistry & registry)
{
    if (!json.isObject() || !json.isMember("Name") || !json["Name"].isString())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::string action                  = json["Name"].asString();
    RvcNamedPipeSimulation * simulation = GetRvcNamedPipeSimulation(endpointId);
    if (simulation == nullptr)
    {
        ChipLogError(AppServer, "RvcNamedPipeSimulation not found on endpoint %u", static_cast<unsigned>(endpointId));
        return CHIP_ERROR_NOT_FOUND;
    }

    if (action == "Reset")
    {
        simulation->HandleReset();
        return CHIP_NO_ERROR;
    }
    if (action == "Charged")
    {
        simulation->HandleCharged();
        return CHIP_NO_ERROR;
    }
    if (action == "Charging")
    {
        simulation->HandleCharging();
        return CHIP_NO_ERROR;
    }
    if (action == "Docked")
    {
        simulation->HandleDocked();
        return CHIP_NO_ERROR;
    }
    if (action == "ChargerFound")
    {
        simulation->HandleChargerFound();
        return CHIP_NO_ERROR;
    }
    if (action == "LowCharge")
    {
        simulation->HandleLowCharge();
        return CHIP_NO_ERROR;
    }
    if (action == "ActivityComplete")
    {
        simulation->HandleActivityComplete();
        return CHIP_NO_ERROR;
    }
    if (action == "AreaComplete")
    {
        simulation->HandleAreaComplete();
        return CHIP_NO_ERROR;
    }
    if (action == "ClearError")
    {
        simulation->HandleClearError();
        return CHIP_NO_ERROR;
    }
    if (action == "EmptyingDustBin")
    {
        simulation->HandleEmptyingDustBin();
        return CHIP_NO_ERROR;
    }
    if (action == "CleaningMop")
    {
        simulation->HandleCleaningMop();
        return CHIP_NO_ERROR;
    }
    if (action == "FillingWaterTank")
    {
        simulation->HandleFillingWaterTank();
        return CHIP_NO_ERROR;
    }
    if (action == "UpdatingMaps")
    {
        simulation->HandleUpdatingMaps();
        return CHIP_NO_ERROR;
    }
    if (action == "ErrorEvent")
    {
        if (!json.isMember("Error") || !json["Error"].isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        simulation->HandleErrorEvent(json["Error"].asString());
        return CHIP_NO_ERROR;
    }
    if (action == "AddMap")
    {
        auto mapId = ExtractUInt<uint32_t>(json, "MapId");
        if (!mapId.has_value() || !json.isMember("MapName") || !json["MapName"].isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        std::string mapName = json["MapName"].asString();
        if (!simulation->HandleAddMap(mapId.value(), CharSpan::fromCharString(mapName.c_str())))
        {
            ChipLogError(AppServer, "AddMap: failed to add map %" PRIu32, mapId.value());
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }
    if (action == "RemoveMap")
    {
        auto mapId = ExtractUInt<uint32_t>(json, "MapId");
        if (!mapId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (!simulation->HandleRemoveMap(mapId.value()))
        {
            ChipLogError(AppServer, "RemoveMap: failed to remove map %" PRIu32, mapId.value());
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }
    if (action == "AddArea")
    {
        auto areaId = ExtractUInt<uint32_t>(json, "AreaId");
        if (!areaId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        auto mapId = ExtractUInt<uint32_t>(json, "MapId");
        std::optional<std::string> locName;
        std::optional<CharSpan> locNameSpan;
        if (json.isMember("LocationName"))
        {
            if (!json["LocationName"].isString())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            locName     = json["LocationName"].asString();
            locNameSpan = CharSpan::fromCharString(locName->c_str());
        }
        if (!simulation->HandleAddArea(areaId.value(), mapId, locNameSpan))
        {
            ChipLogError(AppServer, "AddArea: failed to add area %" PRIu32, areaId.value());
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }
    if (action == "RemoveArea")
    {
        auto areaId = ExtractUInt<uint32_t>(json, "AreaId");
        if (!areaId.has_value())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (!simulation->HandleRemoveArea(areaId.value()))
        {
            ChipLogError(AppServer, "RemoveArea: failed to remove area %" PRIu32, areaId.value());
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip::app::NamedPipe
