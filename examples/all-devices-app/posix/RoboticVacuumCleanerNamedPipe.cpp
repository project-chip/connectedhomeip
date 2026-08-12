/*
 *
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

#include <RoboticVacuumCleanerNamedPipe.h>

#include <AllDevicesAppNamedPipeCommandHandler.h>
#include <AppCommandDelegate.h>
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationLogic.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationTopology.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::all_devices::rvc_simulation;

// Reset is composed entirely from RoboticVacuumCleaner's public accessors and the shared
// simulation helpers, so it needs no dedicated API on the device type itself.
void HandleReset(RoboticVacuumCleaner & rvcDevice)
{
    rvcDevice.RunMode().UpdateCurrentMode(Topology::kRunModeIdle);
    LogErrorOnFailure(rvcDevice.OperationalState().SetOperationalState(
        to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped)));
    rvcDevice.CleanMode().UpdateCurrentMode(Topology::kCleanModeQuick);

    rvcDevice.GetServiceAreaCluster().ClearSelectedAreas();
    rvcDevice.GetServiceAreaCluster().ClearProgress();
    rvcDevice.GetServiceAreaCluster().SetCurrentArea(DataModel::NullNullable);
    rvcDevice.GetServiceAreaCluster().SetEstimatedEndTime(DataModel::NullNullable);

    ApplyDefaultMapTopology(rvcDevice.GetServiceAreaCluster());
    rvcDevice.GetServiceAreaCluster().SetCurrentArea(DefaultCurrentAreaId());
}

class RvcNamedPipeCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    explicit RvcNamedPipeCommandHandler(std::string commandName) : mCommandName(std::move(commandName)) {}

    const char * GetName() const override { return mCommandName.c_str(); }

    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * rvcDevice = delegate->GetRvcDeviceByEndpoint(endpointId);
        if (rvcDevice == nullptr)
        {
            ChipLogError(AppServer, "RoboticVacuumCleaner not found on endpoint %d", endpointId);
            return;
        }

        auto & opStateDelegate = rvcDevice->OperationalStateDelegate();

        if (mCommandName == "Reset")
        {
            HandleReset(*rvcDevice);
        }
        else if (mCommandName == "Charged")
        {
            opStateDelegate.HandleCharged();
        }
        else if (mCommandName == "Charging")
        {
            opStateDelegate.HandleCharging();
        }
        else if (mCommandName == "Docked")
        {
            opStateDelegate.HandleDocked();
        }
        else if (mCommandName == "ChargerFound")
        {
            opStateDelegate.HandleChargerFound();
        }
        else if (mCommandName == "LowCharge")
        {
            opStateDelegate.HandleLowCharge();
        }
        else if (mCommandName == "ActivityComplete")
        {
            opStateDelegate.HandleActivityComplete();
        }
        else if (mCommandName == "AreaComplete")
        {
            opStateDelegate.HandleAreaComplete();
        }
        else if (mCommandName == "ClearError")
        {
            opStateDelegate.HandleClearError();
        }
        else if (mCommandName == "EmptyingDustBin")
        {
            LogErrorOnFailure(rvcDevice->OperationalState().SetOperationalState(
                to_underlying(RvcOperationalState::OperationalStateEnum::kEmptyingDustBin)));
        }
        else if (mCommandName == "CleaningMop")
        {
            LogErrorOnFailure(rvcDevice->OperationalState().SetOperationalState(
                to_underlying(RvcOperationalState::OperationalStateEnum::kCleaningMop)));
        }
        else if (mCommandName == "FillingWaterTank")
        {
            LogErrorOnFailure(rvcDevice->OperationalState().SetOperationalState(
                to_underlying(RvcOperationalState::OperationalStateEnum::kFillingWaterTank)));
        }
        else if (mCommandName == "UpdatingMaps")
        {
            LogErrorOnFailure(rvcDevice->OperationalState().SetOperationalState(
                to_underlying(RvcOperationalState::OperationalStateEnum::kUpdatingMaps)));
        }
        else if (mCommandName == "ErrorEvent")
        {
            if (json.isMember("Error") && json["Error"].isString())
            {
                opStateDelegate.HandleErrorEvent(json["Error"].asString());
            }
        }
        else if (mCommandName == "AddMap")
        {
            if (json.isMember("MapId") && json["MapId"].isUInt() && json.isMember("MapName") && json["MapName"].isString())
            {
                const uint32_t mapId = json["MapId"].asUInt();
                std::string mapName  = json["MapName"].asString();
                if (!rvcDevice->GetServiceAreaCluster().AddSupportedMap(mapId, CharSpan(mapName.data(), mapName.size())))
                {
                    ChipLogError(AppServer, "AddMap: failed to add map %u", static_cast<unsigned>(mapId));
                }
            }
        }
        else if (mCommandName == "RemoveMap")
        {
            if (json.isMember("MapId") && json["MapId"].isUInt())
            {
                const uint32_t mapId = json["MapId"].asUInt();
                if (!rvcDevice->GetServiceAreaCluster().RemoveSupportedMap(mapId))
                {
                    ChipLogError(AppServer, "RemoveMap: failed to remove map %u", static_cast<unsigned>(mapId));
                }
            }
        }
        else if (mCommandName == "AddArea")
        {
            if (!json.isMember("AreaId") || !json["AreaId"].isUInt())
            {
                return;
            }

            const uint32_t areaId = json["AreaId"].asUInt();
            ServiceArea::AreaStructureWrapper area;
            area.SetAreaId(areaId);
            if (json.isMember("MapId"))
            {
                if (!json["MapId"].isUInt())
                {
                    return;
                }
                area.SetMapId(json["MapId"].asUInt());
            }
            if (json.isMember("LocationName"))
            {
                if (!json["LocationName"].isString())
                {
                    return;
                }
                std::string locationName = json["LocationName"].asString();
                area.SetLocationInfo(CharSpan(locationName.data(), locationName.size()), DataModel::NullNullable,
                                     DataModel::NullNullable);
            }
            if (!rvcDevice->GetServiceAreaCluster().AddSupportedArea(area))
            {
                ChipLogError(AppServer, "AddArea: failed to add area %u", static_cast<unsigned>(areaId));
            }
        }
        else if (mCommandName == "RemoveArea")
        {
            if (json.isMember("AreaId") && json["AreaId"].isUInt())
            {
                const uint32_t areaId = json["AreaId"].asUInt();
                if (!rvcDevice->GetServiceAreaCluster().RemoveSupportedArea(areaId))
                {
                    ChipLogError(AppServer, "RemoveArea: failed to remove area %u", static_cast<unsigned>(areaId));
                }
            }
        }
        else
        {
            ChipLogError(AppServer, "Unknown RVC named-pipe command: %s", mCommandName.c_str());
        }
    }

private:
    std::string mCommandName;
};

} // namespace

void RegisterRvcNamedPipeCommandHandlers(AllDevicesAppCommandDelegate & delegate)
{
    static constexpr const char * kRvcNamedPipeCommands[] = {
        "Reset",        "Charged",    "Charging",        "Docked",      "ChargerFound",     "LowCharge",    "ActivityComplete",
        "AreaComplete", "ClearError", "EmptyingDustBin", "CleaningMop", "FillingWaterTank", "UpdatingMaps", "ErrorEvent",
        "AddMap",       "RemoveMap",  "AddArea",         "RemoveArea",
    };
    for (const char * commandName : kRvcNamedPipeCommands)
    {
        delegate.RegisterCommandHandler(std::make_unique<RvcNamedPipeCommandHandler>(commandName));
    }
}
