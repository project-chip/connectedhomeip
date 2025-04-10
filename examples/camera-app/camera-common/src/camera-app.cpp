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
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::app::Clusters::CameraAvStreamManagement;

template <typename T>
using List   = chip::app::DataModel::List<T>;
using Status = Protocols::InteractionModel::Status;

CameraApp::CameraApp(chip::EndpointId aClustersEndpoint, CameraDeviceInterface * aCameraDevice)
{
    mEndpoint     = aClustersEndpoint;
    mCameraDevice = aCameraDevice;

    // Instantiate Chime Server
    mChimeServerPtr = std::make_unique<ChimeServer>(mEndpoint, mCameraDevice->GetChimeDelegate());

    // Instantiate WebRTCTransport Provider
    mWebRTCTransportProviderPtr =
        std::make_unique<WebRTCTransportProviderServer>(mCameraDevice->GetWebRTCProviderDelegate(), mEndpoint);

    // Fetch all initialization parameters for CameraAVStreamMgmt Server
    BitFlags<Feature> features;
    features.Set(Feature::kSnapshot);
    BitFlags<OptionalAttribute> optionalAttrs;
    optionalAttrs.Set(chip::app::Clusters::CameraAvStreamManagement::OptionalAttribute::kNightVision);
    optionalAttrs.Set(chip::app::Clusters::CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum);
    uint32_t maxConcurrentVideoEncoders  = mCameraDevice->GetCameraHALInterface().GetMaxConcurrentVideoEncoders();
    uint32_t maxEncodedPixelRate         = mCameraDevice->GetCameraHALInterface().GetMaxEncodedPixelRate();
    VideoSensorParamsStruct sensorParams = mCameraDevice->GetCameraHALInterface().GetVideoSensorParams();
    bool nightVisionCapable              = mCameraDevice->GetCameraHALInterface().GetNightVisionCapable();
    VideoResolutionStruct minViewport    = mCameraDevice->GetCameraHALInterface().GetMinViewport();
    std::vector<RateDistortionTradeOffStruct> rateDistortionTradeOffPoints = {};
    uint32_t maxContentBufferSize                                          = 1024;
    AudioCapabilitiesStruct micCapabilities{};
    AudioCapabilitiesStruct spkrCapabilities{};
    TwoWayTalkSupportTypeEnum twowayTalkSupport               = TwoWayTalkSupportTypeEnum::kNotSupported;
    std::vector<SnapshotParamsStruct> supportedSnapshotParams = {};
    uint32_t maxNetworkBandwidth                              = 64;
    std::vector<StreamUsageEnum> supportedStreamUsages        = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };

    // Instantiate the CameraAVStreamMgmt Server
    mAVStreamMgmtServerPtr = std::make_unique<CameraAVStreamMgmtServer>(
        mCameraDevice->GetCameraAVStreamMgmtDelegate(), mEndpoint, features, optionalAttrs, maxConcurrentVideoEncoders,
        maxEncodedPixelRate, sensorParams, nightVisionCapable, minViewport, rateDistortionTradeOffPoints, maxContentBufferSize,
        micCapabilities, spkrCapabilities, twowayTalkSupport, supportedSnapshotParams, maxNetworkBandwidth, supportedStreamUsages);

    // Fetch all initialization parameters for CameraAVSettingsUserLevelMgmt Server
    BitFlags<CameraAvSettingsUserLevelManagement::Feature, uint32_t> avsumFeatures(
        CameraAvSettingsUserLevelManagement::Feature::kDigitalPTZ, CameraAvSettingsUserLevelManagement::Feature::kMechanicalPan,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalTilt,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalZoom,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalPresets);
    BitFlags<CameraAvSettingsUserLevelManagement::OptionalAttributes, uint32_t> avsumAttrs(
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kMptzPosition,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kMaxPresets,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kMptzPresets,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kDptzRelativeMove,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kZoomMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kTiltMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kTiltMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMax);
    const uint8_t appMaxPresets = 5;

    // Instantiate the CameraAVSettingsUserLevelMgmt Server
    mAVSettingsUserLevelMgmtServerPtr = std::make_unique<CameraAvSettingsUserLevelMgmtServer>(
        mCameraDevice->GetCameraAVSettingsUserLevelMgmtDelegate(), mEndpoint, features, optionalAttrs, appMaxPresets);

    // Set app specific limits to pan, tilt, zoom
    mAVSettingsUserLevelMgmtServerPtr->SetPanMin(mCameraDevice->GetCameraHALInterface().GetPanMin());
    mAVSettingsUserLevelMgmtServerPtr->SetPanMax(mCameraDevice->GetCameraHALInterface().GetPanMax());
    mAVSettingsUserLevelMgmtServerPtr->SetTiltMin(mCameraDevice->GetCameraHALInterface().GetTiltMin());
    mAVSettingsUserLevelMgmtServerPtr->SetTiltMax(mCameraDevice->GetCameraHALInterface().GetTiltMax());
    mAVSettingsUserLevelMgmtServerPtr->SetZoomMax(mCameraDevice->GetCameraHALInterface().GetZoomMax());
}

void CameraApp::InitCameraDeviceClusters()
{
    // Initialize Cluster Servers
    mWebRTCTransportProviderPtr->Init();

    mChimeServerPtr->Init();

    mAVStreamMgmtServerPtr->Init();
}

static constexpr EndpointId kCameraEndpointId = 1;

Platform::UniquePtr<CameraApp> gCameraApp;

void CameraAppInit(CameraDeviceInterface * cameraDevice)
{
    gCameraApp = Platform::MakeUnique<CameraApp>(kCameraEndpointId, cameraDevice);
    gCameraApp.get()->InitCameraDeviceClusters();

    ChipLogDetail(Camera, "CameraAppInit: Initialized Camera clusters");
}

void CameraAppShutdown()
{
    ChipLogDetail(Camera, "CameraAppShutdown: Shutting down Camera app");
    gCameraApp = nullptr;
}
