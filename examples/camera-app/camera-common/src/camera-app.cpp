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
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::ZoneManagement;

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
    BitFlags<CameraAvStreamManagement::Feature> avsmFeatures;
    avsmFeatures.Set(CameraAvStreamManagement::Feature::kSnapshot);
    avsmFeatures.Set(CameraAvStreamManagement::Feature::kVideo);

    if (mCameraDevice->GetCameraHALInterface().HasMicrophone())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kAudio);
    }

    avsmFeatures.Set(CameraAvStreamManagement::Feature::kHighDynamicRange);

    BitFlags<CameraAvStreamManagement::OptionalAttribute> avsmOptionalAttrs;

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsNightVision())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kNightVision);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kNightVision);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum);
    }

    uint32_t maxConcurrentVideoEncoders  = mCameraDevice->GetCameraHALInterface().GetMaxConcurrentEncoders();
    uint32_t maxEncodedPixelRate         = mCameraDevice->GetCameraHALInterface().GetMaxEncodedPixelRate();
    VideoSensorParamsStruct sensorParams = mCameraDevice->GetCameraHALInterface().GetVideoSensorParams();
    bool nightVisionUsesInfrared         = mCameraDevice->GetCameraHALInterface().GetNightVisionUsesInfrared();
    VideoResolutionStruct minViewport    = mCameraDevice->GetCameraHALInterface().GetMinViewport();
    std::vector<RateDistortionTradeOffStruct> rateDistortionTradeOffPoints =
        mCameraDevice->GetCameraHALInterface().GetRateDistortionTradeOffPoints();

    uint32_t maxContentBufferSize               = mCameraDevice->GetCameraHALInterface().GetMaxContentBufferSize();
    AudioCapabilitiesStruct micCapabilities     = mCameraDevice->GetCameraHALInterface().GetMicrophoneCapabilities();
    AudioCapabilitiesStruct spkrCapabilities    = mCameraDevice->GetCameraHALInterface().GetSpeakerCapabilities();
    TwoWayTalkSupportTypeEnum twowayTalkSupport = TwoWayTalkSupportTypeEnum::kNotSupported;
    std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = mCameraDevice->GetCameraHALInterface().GetSnapshotCapabilities();
    uint32_t maxNetworkBandwidth                                 = mCameraDevice->GetCameraHALInterface().GetMaxNetworkBandwidth();
    std::vector<StreamUsageEnum> supportedStreamUsages  = mCameraDevice->GetCameraHALInterface().GetSupportedStreamUsages();
    std::vector<StreamUsageEnum> rankedStreamPriorities = mCameraDevice->GetCameraHALInterface().GetRankedStreamPriorities();

    // Instantiate the CameraAVStreamMgmt Server
    mAVStreamMgmtServerPtr = std::make_unique<CameraAVStreamMgmtServer>(
        mCameraDevice->GetCameraAVStreamMgmtDelegate(), mEndpoint, avsmFeatures, avsmOptionalAttrs, maxConcurrentVideoEncoders,
        maxEncodedPixelRate, sensorParams, nightVisionUsesInfrared, minViewport, rateDistortionTradeOffPoints, maxContentBufferSize,
        micCapabilities, spkrCapabilities, twowayTalkSupport, snapshotCapabilities, maxNetworkBandwidth, supportedStreamUsages,
        rankedStreamPriorities);

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
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kDptzStreams,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kZoomMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kTiltMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kTiltMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMax);
    const uint8_t appMaxPresets = 5;

    // Instantiate the CameraAVSettingsUserLevelMgmt Server
    mAVSettingsUserLevelMgmtServerPtr = std::make_unique<CameraAvSettingsUserLevelMgmtServer>(
        mEndpoint, mCameraDevice->GetCameraAVSettingsUserLevelMgmtDelegate(), avsumFeatures, avsumAttrs, appMaxPresets);

    mAVSettingsUserLevelMgmtServerPtr->SetPanMin(mCameraDevice->GetCameraHALInterface().GetPanMin());
    mAVSettingsUserLevelMgmtServerPtr->SetPanMax(mCameraDevice->GetCameraHALInterface().GetPanMax());
    mAVSettingsUserLevelMgmtServerPtr->SetTiltMin(mCameraDevice->GetCameraHALInterface().GetTiltMin());
    mAVSettingsUserLevelMgmtServerPtr->SetTiltMax(mCameraDevice->GetCameraHALInterface().GetTiltMax());
    mAVSettingsUserLevelMgmtServerPtr->SetZoomMax(mCameraDevice->GetCameraHALInterface().GetZoomMax());

    // Fetch all initialization parameters for the ZoneManagement Server
    BitFlags<ZoneManagement::Feature, uint32_t> zoneMgmtFeatures(
        ZoneManagement::Feature::kTwoDimensionalCartesianZone, ZoneManagement::Feature::kPerZoneSensitivity,
        ZoneManagement::Feature::kUserDefined, ZoneManagement::Feature::kFocusZones);

    BitFlags<ZoneManagement::OptionalAttribute> zoneMgmtOptionalAttrs;
    // Hardcode these initialization params for now. TODO: Define HAL interfaces
    uint8_t appMaxZones                           = 8;  // Spec: min 1
    uint8_t appMaxUserDefinedZones                = 5;  // Spec: min 5
    uint8_t sensitivityMax                        = 10; // Spec: 2 to 10
    TwoDCartesianVertexStruct appTwoDCartesianMax = {};
    appTwoDCartesianMax.x                         = sensorParams.sensorWidth - 1;
    appTwoDCartesianMax.y                         = sensorParams.sensorHeight - 1;

    // Instantiate the ZoneManagement Server
    mZoneMgmtServerPtr = std::make_unique<ZoneMgmtServer>(mCameraDevice->GetZoneManagementDelegate(), mEndpoint, zoneMgmtFeatures,
                                                          zoneMgmtOptionalAttrs, appMaxUserDefinedZones, appMaxZones,
                                                          sensitivityMax, appTwoDCartesianMax);
}

void CameraApp::InitializeCameraAVStreamMgmt()
{
    // Set the attribute defaults
    mAVStreamMgmtServerPtr->SetHDRModeEnabled(mCameraDevice->GetCameraHALInterface().GetHDRMode());
    mAVStreamMgmtServerPtr->SetViewport(mCameraDevice->GetCameraHALInterface().GetViewport());

    mAVStreamMgmtServerPtr->Init();
}

void CameraApp::InitCameraDeviceClusters()
{
    // Initialize Cluster Servers
    mWebRTCTransportProviderPtr->Init();

    mChimeServerPtr->Init();

    mAVSettingsUserLevelMgmtServerPtr->Init();

    InitializeCameraAVStreamMgmt();

    mZoneMgmtServerPtr->Init();
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
