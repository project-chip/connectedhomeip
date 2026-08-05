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

#include <RvcSimulationLogic.h>
#include <RvcSimulationTopology.h>
#include <clusters/RvcCleanMode/Metadata.h>
#include <clusters/RvcRunMode/Metadata.h>
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <json/json.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

using namespace Clusters::ServiceArea;
using namespace chip::examples::rvc_simulation;
using namespace chip::examples::rvc_simulation::Topology;

void RoboticVacuumCleaner::HandleReset()
{
    mRunModeCluster.Cluster().UpdateCurrentMode(kRunModeIdle);
    LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
        to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped)));
    mCleanModeCluster.Cluster().UpdateCurrentMode(kCleanModeQuick);

    mServiceAreaCluster.Cluster().ClearSelectedAreas();
    mServiceAreaCluster.Cluster().ClearProgress();
    mServiceAreaCluster.Cluster().SetCurrentArea(DataModel::NullNullable);
    mServiceAreaCluster.Cluster().SetEstimatedEndTime(DataModel::NullNullable);

    mServiceAreaDelegate.SetMapTopology();
    mServiceAreaCluster.Cluster().SetCurrentArea(DefaultCurrentAreaId());
}

void RoboticVacuumCleaner::HandleNamedPipeCommand(const Json::Value & json)
{
    if (!json.isMember("Name") || !json["Name"].isString())
    {
        return;
    }

    const std::string name = json["Name"].asString();

    if (name == "Reset")
    {
        HandleReset();
    }
    else if (name == "Charged")
    {
        mDelegate.HandleCharged();
    }
    else if (name == "Charging")
    {
        mDelegate.HandleCharging();
    }
    else if (name == "Docked")
    {
        mDelegate.HandleDocked();
    }
    else if (name == "ChargerFound")
    {
        mDelegate.HandleChargerFound();
    }
    else if (name == "LowCharge")
    {
        mDelegate.HandleLowCharge();
    }
    else if (name == "ActivityComplete")
    {
        mDelegate.HandleActivityComplete();
    }
    else if (name == "AreaComplete")
    {
        mDelegate.HandleAreaComplete();
    }
    else if (name == "ClearError")
    {
        mDelegate.HandleClearError();
    }
    else if (name == "EmptyingDustBin")
    {
        LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
            to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kEmptyingDustBin)));
    }
    else if (name == "CleaningMop")
    {
        LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
            to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kCleaningMop)));
    }
    else if (name == "FillingWaterTank")
    {
        LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
            to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kFillingWaterTank)));
    }
    else if (name == "UpdatingMaps")
    {
        LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
            to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kUpdatingMaps)));
    }
    else if (name == "ErrorEvent")
    {
        if (json.isMember("Error") && json["Error"].isString())
        {
            mDelegate.HandleErrorEvent(json["Error"].asString());
        }
    }
    else if (name == "AddMap")
    {
        if (json.isMember("MapId") && json.isMember("MapName"))
        {
            std::string mapName = json["MapName"].asString();
            mServiceAreaCluster.Cluster().AddSupportedMap(json["MapId"].asUInt(), CharSpan(mapName.data(), mapName.size()));
        }
    }
    else if (name == "RemoveMap")
    {
        if (json.isMember("MapId"))
        {
            mServiceAreaCluster.Cluster().RemoveSupportedMap(json["MapId"].asUInt());
        }
    }
    else if (name == "AddArea")
    {
        if (!json.isMember("AreaId"))
        {
            return;
        }

        Clusters::ServiceArea::AreaStructureWrapper area;
        area.SetAreaId(json["AreaId"].asUInt());
        if (json.isMember("MapId"))
        {
            area.SetMapId(json["MapId"].asUInt());
        }
        if (json.isMember("LocationName"))
        {
            std::string locationName = json["LocationName"].asString();
            area.SetLocationInfo(CharSpan(locationName.data(), locationName.size()), DataModel::NullNullable,
                                 DataModel::NullNullable);
        }
        mServiceAreaCluster.Cluster().AddSupportedArea(area);
    }
    else if (name == "RemoveArea")
    {
        if (json.isMember("AreaId"))
        {
            mServiceAreaCluster.Cluster().RemoveSupportedArea(json["AreaId"].asUInt());
        }
    }
    else
    {
        ChipLogError(AppServer, "Unknown RVC named-pipe command: %s", name.c_str());
    }
}

} // namespace chip::app
