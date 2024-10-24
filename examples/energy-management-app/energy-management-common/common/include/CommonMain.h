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

void EvseApplicationInit(chip::EndpointId endpointId);
void EvseApplicationShutdown();

void WaterHeaterApplicationInit(chip::EndpointId endpointId);
void WaterHeaterApplicationShutdown();

// The DEM Delegate is used for the TestEventTriggers which are functions called outside of a class/init context
// thus have no simple way to inject this dependency, so use the Getter here.
chip::app::Clusters::DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate();
