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
#include "camera-app.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::CameraAvStreamManagement;

template <typename T>
using List   = chip::app::DataModel::List<T>;
using Status = Protocols::InteractionModel::Status;

CameraApp::CameraApp(chip::EndpointId aClustersEndpoint, CameraDeviceInterface * aCameraDevice)
{
    mEndpoint = aClustersEndpoint;
    mCameraDevice = aCameraDevice;
    mChimeServerPtr = std::make_unique<ChimeServer>(mEndpoint, mCameraDevice->GetChimeDelegate());

    BitFlags<Feature> features;
    features.Set(Feature::kSnapshot);
    BitFlags<OptionalAttribute> optionalAttrs;
    optionalAttrs.Set(chip::app::Clusters::CameraAvStreamManagement::OptionalAttribute::kNightVision);
    optionalAttrs.Set(chip::app::Clusters::CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum);
    uint32_t maxConcurrentVideoEncoders  = 1;
    uint32_t maxEncodedPixelRate         = 10000;
    VideoSensorParamsStruct sensorParams = { 4608, 2592, 120, chip::Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
    bool nightVisionCapable              = false;
    VideoResolutionStruct minViewport    = { 854, 480 }; // Assuming 480p resolution.
    std::vector<RateDistortionTradeOffStruct> rateDistortionTradeOffPoints = {};
    uint32_t maxContentBufferSize                                          = 1024;
    AudioCapabilitiesStruct micCapabilities{};
    AudioCapabilitiesStruct spkrCapabilities{};
    TwoWayTalkSupportTypeEnum twowayTalkSupport               = TwoWayTalkSupportTypeEnum::kNotSupported;
    std::vector<SnapshotParamsStruct> supportedSnapshotParams = {};
    uint32_t maxNetworkBandwidth                              = 64;
    std::vector<StreamUsageEnum> supportedStreamUsages        = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };

    mAVStreamMgmtServerPtr = std::make_unique<CameraAVStreamMgmtServer>(
        mCameraDevice->GetCameraAVStreamMgmtDelegate(), mEndpoint, features, optionalAttrs, maxConcurrentVideoEncoders, maxEncodedPixelRate,
        sensorParams, nightVisionCapable, minViewport, rateDistortionTradeOffPoints, maxContentBufferSize, micCapabilities,
        spkrCapabilities, twowayTalkSupport, supportedSnapshotParams, maxNetworkBandwidth, supportedStreamUsages);
}

void CameraApp::InitCameraDeviceClusters()
{
    // Initialize Cluster Servers
    mWebRTCTransportProvider.Init();

    mChimeServerPtr->Init();

    mAVStreamMgmtServerPtr->Init();
}

static constexpr EndpointId kCameraEndpointId = 1;

Platform::UniquePtr<CameraApp> gCameraApp;

void CameraAppInit(CameraDeviceInterface * cameraDevice)
{
    gCameraApp = Platform::MakeUnique<CameraApp>(kCameraEndpointId, cameraDevice);
    gCameraApp.get()->InitCameraDeviceClusters();
}

void CameraAppShutdown()
{
    gCameraApp = nullptr;
}
