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

#include <app/util/config.h>
#include <cstring>
#include <protocols/interaction_model/StatusCode.h>
#include <utility>

// Thermostat App defines all the cluster servers needed for a particular device
class ThermostatApp
{

public:
    // This class is responsible for initialising the thermostat related clusters
    ThermostatApp(chip::EndpointId endpoint);

    // Initialize all the camera device clusters.
    void InitThermostatClusters();

    // Shutdown all the camera device clusters
    void ShutdownThermostatClusters();

private:
    chip::EndpointId mEndpoint;

    // SDK cluster servers
    std::unique_ptr<chip::app::Clusters::Thermostat::ThermostatCluster> mThermostatCluster;

    // Helper to set attribute defaults for CameraAVStreamMgmt
    void InitializeCameraAVStreamMgmt();
};

void ThermostatAppInit();

void ThermostatAppShutdown();
