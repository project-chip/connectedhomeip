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

#pragma once

#include <app/clusters/user-label-server/UserLabelCluster.h>
#include <app/TestEventTriggerDelegate.h>
#include <credentials/FabricTable.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/// Electrical Sensor child endpoint for a solar panel group or string, with User Label cluster.
class PanelElectricalSensor : public SimulatedElectricalSensor
{
public:
    PanelElectricalSensor(TimerDelegate & timerDelegate, TestEventTriggerDelegate & testEventTriggerDelegate,
                          FabricTable & fabricTable, Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList);

    Clusters::UserLabelCluster & UserLabelCluster() { return mUserLabelCluster.Cluster(); }

protected:
    CHIP_ERROR RegisterAdditionalServerClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalServerClusters(CodeDrivenDataModelProvider & provider) override;

private:
    FabricTable & mFabricTable;
    LazyRegisteredServerCluster<Clusters::UserLabelCluster> mUserLabelCluster;
};

} // namespace chip::app
