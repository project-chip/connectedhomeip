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
    // This class is responsible for initialising all the camera clusters and
    // managing the interactions between them
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
    std::unique_ptr<chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamManagementCluster> mAVStreamMgmtServerPtr;

    // Helper to set attribute defaults for CameraAVStreamMgmt
    void InitializeCameraAVStreamMgmt();
};

void CameraAppInit(CameraDeviceInterface * cameraDevice);

void CameraAppShutdown();
