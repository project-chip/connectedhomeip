/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

#include <ElectricalPowerMeasurementDelegate.h>
#include <PowerTopologyDelegate.h>

#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/power-topology-server/power-topology-server.h>

extern std::unique_ptr<chip::app::Clusters::PowerTopology::PowerTopologyDelegate> gPTDelegate;
extern std::unique_ptr<chip::app::Clusters::PowerTopology::PowerTopologyInstance> gPTInstance;


extern std::unique_ptr<chip::app::Clusters::ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate> gEPMDelegate;
extern std::unique_ptr<chip::app::Clusters::ElectricalPowerMeasurement::ElectricalPowerMeasurementInstance> gEPMInstance;

// Electrical Energy Measurement cluster uses ember to initialise
extern std::unique_ptr<chip::app::Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;


CHIP_ERROR PowerTopologyInit();
CHIP_ERROR PowerTopologyShutdown();

CHIP_ERROR EnergyMeterInit();
CHIP_ERROR EnergyMeterShutdown();
