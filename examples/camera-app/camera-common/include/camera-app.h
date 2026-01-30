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

#include "camera-device-interface.h"
#include <app/util/config.h>
#include <cstring>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/interaction_model/StatusCode.h>
#include <utility>

// Camera App defines all the cluster servers needed for a particular device
class CameraApp
{

public:
    // This class is responsible for initialising all the camera clusters and managing the interactions between them
    CameraApp(chip::EndpointId aClustersEndpoint, CameraDeviceInterface * cameraDevice);

    // Initialize all the camera device clusters.
    void InitCameraDeviceClusters();

    // Shutdown all the camera device clusters
    void ShutdownCameraDeviceClusters();

private:
    chip::EndpointId mEndpoint;
    CameraDeviceInterface * mCameraDevice;

    // SDK cluster servers
    chip::app::LazyRegisteredServerCluster<chip::app::Clusters::WebRTCTransportProvider::WebRTCTransportProviderCluster>
        mWebRTCTransportProviderServer;
    std::unique_ptr<chip::app::Clusters::ChimeServer> mChimeServerPtr;
    chip::app::LazyRegisteredServerCluster<chip::app::Clusters::CameraAvSettingsUserLevelManagementCluster>
        mAVSettingsUserLevelMgmtServer;
    chip::app::LazyRegisteredServerCluster<chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamManagementCluster>
        mAVStreamMgmtServer;
    std::unique_ptr<chip::app::Clusters::ZoneManagement::ZoneMgmtServer> mZoneMgmtServerPtr;

    // Method to instantiate CameraAVStreamMgmt and set attribute defaults for initialization.
    void CreateAndInitializeCameraAVStreamMgmt();
    CHIP_ERROR InitializeCameraAVStreamMgmt();
};

void CameraAppInit(CameraDeviceInterface * cameraDevice);

void CameraAppShutdown();
