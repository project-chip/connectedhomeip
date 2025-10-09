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

#include <DeviceEnergyManagementDelegateImpl.h>
#include <DeviceEnergyManagementManager.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <PowerTopologyDelegate.h>
#include <lib/core/CHIPError.h>

// This app is configured by default with EP1 for EVSE and EP2 for WaterHeater, with only one endpoint
// enabled. On linux, there's a command line argument (--application) to dynamically enable
// "evse|water-heater", i.e. EP1 or EP2. On other platforms, it's a build time definition (#define).
chip::EndpointId GetEnergyDeviceEndpointId();

// The DEM Delegate is used for the TestEventTriggers
chip::app::Clusters::DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate();

void EvseApplicationInit();
void EvseApplicationShutdown();

void WaterHeaterApplicationInit();
void WaterHeaterApplicationShutdown();
