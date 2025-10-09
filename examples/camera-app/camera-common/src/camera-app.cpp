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
#include "tls-certificate-management-instance.h"
#include "tls-client-management-instance.h"
#include <app/clusters/push-av-stream-transport-server/CodegenIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::PushAvStreamTransport;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::ZoneManagement;

static constexpr uint32_t kBitsPerMegabit = 1000000;

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

    Clusters::PushAvStreamTransport::SetDelegate(mEndpoint, &(mCameraDevice->GetPushAVTransportDelegate()));

    Clusters::PushAvStreamTransport::SetTLSClientManagementDelegate(mEndpoint,
                                                                    &Clusters::TlsClientManagementCommandDelegate::GetInstance());

    Clusters::PushAvStreamTransport::SetTlsCertificateManagementDelegate(
        mEndpoint, &Clusters::TlsCertificateManagementCommandDelegate::GetInstance());
    // Fetch all initialization parameters for CameraAVStreamMgmt Server
    BitFlags<CameraAvStreamManagement::Feature> avsmFeatures;
    BitFlags<CameraAvStreamManagement::OptionalAttribute> avsmOptionalAttrs;
    avsmFeatures.Set(CameraAvStreamManagement::Feature::kSnapshot);
    avsmFeatures.Set(CameraAvStreamManagement::Feature::kVideo);

    // Enable the Watermark and OSD features if camera supports
    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsWatermark())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kWatermark);
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsOSD())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kOnScreenDisplay);
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsSoftPrivacy())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kPrivacy);
    }

    // Check microphone support to set Audio feature
    if (mCameraDevice->GetCameraHALInterface().HasMicrophone())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kAudio);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kMicrophoneAGCEnabled);
    }

    if (mCameraDevice->GetCameraHALInterface().HasLocalStorage())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kLocalStorage);
    }

    // Check if camera has speaker
    if (mCameraDevice->GetCameraHALInterface().HasSpeaker())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kSpeaker);
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsHDR())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kHighDynamicRange);
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsNightVision())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kNightVision);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum);
    }

    if (mCameraDevice->GetCameraHALInterface().HasHardPrivacySwitch())
    {
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kHardPrivacyModeOn);
    }

    if (mCameraDevice->GetCameraHALInterface().HasStatusLight())
    {
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kStatusLightEnabled);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kStatusLightBrightness);
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsImageControl())
    {
        avsmFeatures.Set(CameraAvStreamManagement::Feature::kImageControl);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kImageFlipVertical);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kImageFlipHorizontal);
        avsmOptionalAttrs.Set(CameraAvStreamManagement::OptionalAttribute::kImageRotation);
    }

    uint32_t maxConcurrentVideoEncoders  = mCameraDevice->GetCameraHALInterface().GetMaxConcurrentEncoders();
    uint32_t maxEncodedPixelRate         = mCameraDevice->GetCameraHALInterface().GetMaxEncodedPixelRate();
    VideoSensorParamsStruct sensorParams = mCameraDevice->GetCameraHALInterface().GetVideoSensorParams();
    bool nightVisionUsesInfrared         = mCameraDevice->GetCameraHALInterface().GetNightVisionUsesInfrared();
    VideoResolutionStruct minViewport    = mCameraDevice->GetCameraHALInterface().GetMinViewport();
    std::vector<RateDistortionTradeOffStruct> rateDistortionTradeOffPoints =
        mCameraDevice->GetCameraHALInterface().GetRateDistortionTradeOffPoints();

    uint32_t maxContentBufferSize            = mCameraDevice->GetCameraHALInterface().GetMaxContentBufferSize();
    AudioCapabilitiesStruct micCapabilities  = mCameraDevice->GetCameraHALInterface().GetMicrophoneCapabilities();
    AudioCapabilitiesStruct spkrCapabilities = mCameraDevice->GetCameraHALInterface().GetSpeakerCapabilities();
    TwoWayTalkSupportTypeEnum twowayTalkSupport =
        mCameraDevice->GetCameraHALInterface().HasMicrophone() && mCameraDevice->GetCameraHALInterface().HasSpeaker()
        ? TwoWayTalkSupportTypeEnum::kFullDuplex
        : TwoWayTalkSupportTypeEnum::kNotSupported;
    std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = mCameraDevice->GetCameraHALInterface().GetSnapshotCapabilities();
    uint32_t maxNetworkBandwidth = mCameraDevice->GetCameraHALInterface().GetMaxNetworkBandwidth() * kBitsPerMegabit;
    std::vector<StreamUsageEnum> supportedStreamUsages = mCameraDevice->GetCameraHALInterface().GetSupportedStreamUsages();
    std::vector<StreamUsageEnum> streamUsagePriorities = mCameraDevice->GetCameraHALInterface().GetStreamUsagePriorities();

    // Instantiate the CameraAVStreamMgmt Server
    mAVStreamMgmtServerPtr = std::make_unique<CameraAVStreamMgmtServer>(
        mCameraDevice->GetCameraAVStreamMgmtDelegate(), mEndpoint, avsmFeatures, avsmOptionalAttrs, maxConcurrentVideoEncoders,
        maxEncodedPixelRate, sensorParams, nightVisionUsesInfrared, minViewport, rateDistortionTradeOffPoints, maxContentBufferSize,
        micCapabilities, spkrCapabilities, twowayTalkSupport, snapshotCapabilities, maxNetworkBandwidth, supportedStreamUsages,
        streamUsagePriorities);

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
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kMovementState);
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

    uint8_t appMaxZones                           = mCameraDevice->GetCameraHALInterface().GetMaxZones();
    uint8_t appMaxUserDefinedZones                = mCameraDevice->GetCameraHALInterface().GetMaxUserDefinedZones();
    uint8_t sensitivityMax                        = mCameraDevice->GetCameraHALInterface().GetSensitivityMax();
    TwoDCartesianVertexStruct appTwoDCartesianMax = {};
    appTwoDCartesianMax.x                         = sensorParams.sensorWidth - 1;
    appTwoDCartesianMax.y                         = sensorParams.sensorHeight - 1;

    // Instantiate the ZoneManagement Server
    mZoneMgmtServerPtr = std::make_unique<ZoneMgmtServer>(mCameraDevice->GetZoneManagementDelegate(), mEndpoint, zoneMgmtFeatures,
                                                          appMaxUserDefinedZones, appMaxZones, sensitivityMax, appTwoDCartesianMax);

    mZoneMgmtServerPtr->SetSensitivity(mCameraDevice->GetCameraHALInterface().GetDetectionSensitivity());
}

void CameraApp::InitializeCameraAVStreamMgmt()
{
    // Set the attribute defaults
    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsHDR())
    {
        mAVStreamMgmtServerPtr->SetHDRModeEnabled(mCameraDevice->GetCameraHALInterface().GetHDRMode());
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsSoftPrivacy())
    {
        mAVStreamMgmtServerPtr->SetSoftRecordingPrivacyModeEnabled(
            mCameraDevice->GetCameraHALInterface().GetSoftRecordingPrivacyModeEnabled());
        mAVStreamMgmtServerPtr->SetSoftLivestreamPrivacyModeEnabled(
            mCameraDevice->GetCameraHALInterface().GetSoftLivestreamPrivacyModeEnabled());
    }

    if (mCameraDevice->GetCameraHALInterface().HasHardPrivacySwitch())
    {
        mAVStreamMgmtServerPtr->SetHardPrivacyModeOn(mCameraDevice->GetCameraHALInterface().GetHardPrivacyMode());
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsNightVision())
    {
        mAVStreamMgmtServerPtr->SetNightVision(mCameraDevice->GetCameraHALInterface().GetNightVision());
    }

    mAVStreamMgmtServerPtr->SetViewport(mCameraDevice->GetCameraHALInterface().GetViewport());

    if (mCameraDevice->GetCameraHALInterface().HasSpeaker())
    {
        mAVStreamMgmtServerPtr->SetSpeakerMuted(mCameraDevice->GetCameraHALInterface().GetSpeakerMuted());
        mAVStreamMgmtServerPtr->SetSpeakerVolumeLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerVolume());
        mAVStreamMgmtServerPtr->SetSpeakerMaxLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerMaxLevel());
        mAVStreamMgmtServerPtr->SetSpeakerMinLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerMinLevel());
    }

    if (mCameraDevice->GetCameraHALInterface().HasMicrophone())
    {
        mAVStreamMgmtServerPtr->SetMicrophoneMuted(mCameraDevice->GetCameraHALInterface().GetMicrophoneMuted());
        mAVStreamMgmtServerPtr->SetMicrophoneVolumeLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneVolume());
        mAVStreamMgmtServerPtr->SetMicrophoneMaxLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneMaxLevel());
        mAVStreamMgmtServerPtr->SetMicrophoneMinLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneMinLevel());
    }

    // Video and Snapshot features are already enabled.
    if (mCameraDevice->GetCameraHALInterface().HasLocalStorage())
    {
        mAVStreamMgmtServerPtr->SetLocalVideoRecordingEnabled(
            mCameraDevice->GetCameraHALInterface().GetLocalVideoRecordingEnabled());
        mAVStreamMgmtServerPtr->SetLocalSnapshotRecordingEnabled(
            mCameraDevice->GetCameraHALInterface().GetLocalSnapshotRecordingEnabled());
    }

    if (mCameraDevice->GetCameraHALInterface().HasStatusLight())
    {
        mAVStreamMgmtServerPtr->SetStatusLightEnabled(mCameraDevice->GetCameraHALInterface().GetStatusLightEnabled());
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsImageControl())
    {
        mAVStreamMgmtServerPtr->SetImageRotation(mCameraDevice->GetCameraHALInterface().GetImageRotation());
        mAVStreamMgmtServerPtr->SetImageFlipVertical(mCameraDevice->GetCameraHALInterface().GetImageFlipVertical());
        mAVStreamMgmtServerPtr->SetImageFlipHorizontal(mCameraDevice->GetCameraHALInterface().GetImageFlipHorizontal());
    }

    mAVStreamMgmtServerPtr->Init();
}

void CameraApp::InitCameraDeviceClusters()
{
    // Initialize Cluster Servers
    mWebRTCTransportProviderPtr->Init();
    mCameraDevice->GetWebRTCProviderController().SetWebRTCTransportProvider(std::move(mWebRTCTransportProviderPtr));

    mChimeServerPtr->Init();

    mAVSettingsUserLevelMgmtServerPtr->Init();

    InitializeCameraAVStreamMgmt();

    mZoneMgmtServerPtr->Init();
}

void CameraApp::ShutdownCameraDeviceClusters()
{
    ChipLogDetail(Camera, "CameraAppShutdown: Shutting down Camera device clusters");
    mAVSettingsUserLevelMgmtServerPtr->Shutdown();
    mWebRTCTransportProviderPtr->Shutdown();
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
    gCameraApp.get()->ShutdownCameraDeviceClusters();
    gCameraApp = nullptr;
}
