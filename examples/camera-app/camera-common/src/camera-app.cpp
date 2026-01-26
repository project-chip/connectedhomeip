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
#include "data-model-providers/codegen/CodegenDataModelProvider.h"
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

    Clusters::PushAvStreamTransport::SetDelegate(mEndpoint, &(mCameraDevice->GetPushAVTransportDelegate()));

    Clusters::PushAvStreamTransport::SetTLSClientManagementDelegate(mEndpoint,
                                                                    &Clusters::TlsClientManagementCommandDelegate::GetInstance());

    Clusters::PushAvStreamTransport::SetTLSCertificateManagementDelegate(
        mEndpoint, &Clusters::TlsCertificateManagementCommandDelegate::GetInstance());

    // Fetch all initialization parameters for CameraAVSettingsUserLevelMgmt Server
    BitFlags<CameraAvSettingsUserLevelManagement::Feature, uint32_t> avsumFeatures(
        CameraAvSettingsUserLevelManagement::Feature::kDigitalPTZ, CameraAvSettingsUserLevelManagement::Feature::kMechanicalPan,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalTilt,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalZoom,
        CameraAvSettingsUserLevelManagement::Feature::kMechanicalPresets);

    const uint8_t appMaxPresets = 5;

    // Instantiate the CameraAVSettingsUserLevelMgmt Server
    mAVSettingsUserLevelMgmtServer.Create(mEndpoint, avsumFeatures, appMaxPresets);
    mAVSettingsUserLevelMgmtServer.Cluster().SetDelegate(&mCameraDevice->GetCameraAVSettingsUserLevelMgmtDelegate());
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mAVSettingsUserLevelMgmtServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to register CameraAvSettingsUserLevelManagement on endpoint %u: %" CHIP_ERROR_FORMAT,
                     mEndpoint, err.Format());
    }

    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().SetPanMin(mCameraDevice->GetCameraHALInterface().GetPanMin());
    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().SetPanMax(mCameraDevice->GetCameraHALInterface().GetPanMax());
    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().SetTiltMin(
        mCameraDevice->GetCameraHALInterface().GetTiltMin());
    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().SetTiltMax(
        mCameraDevice->GetCameraHALInterface().GetTiltMax());
    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().SetZoomMax(
        mCameraDevice->GetCameraHALInterface().GetZoomMax());

    // Fetch all initialization parameters for the ZoneManagement Server
    BitFlags<ZoneManagement::Feature, uint32_t> zoneMgmtFeatures(
        ZoneManagement::Feature::kTwoDimensionalCartesianZone, ZoneManagement::Feature::kPerZoneSensitivity,
        ZoneManagement::Feature::kUserDefined, ZoneManagement::Feature::kFocusZones);

    uint8_t appMaxZones                           = mCameraDevice->GetCameraHALInterface().GetMaxZones();
    uint8_t appMaxUserDefinedZones                = mCameraDevice->GetCameraHALInterface().GetMaxUserDefinedZones();
    uint8_t sensitivityMax                        = mCameraDevice->GetCameraHALInterface().GetSensitivityMax();
    VideoSensorParamsStruct sensorParams          = mCameraDevice->GetCameraHALInterface().GetVideoSensorParams();
    TwoDCartesianVertexStruct appTwoDCartesianMax = {};
    appTwoDCartesianMax.x                         = sensorParams.sensorWidth - 1;
    appTwoDCartesianMax.y                         = sensorParams.sensorHeight - 1;

    // Instantiate the ZoneManagement Server
    mZoneMgmtServerPtr = std::make_unique<ZoneMgmtServer>(mCameraDevice->GetZoneManagementDelegate(), mEndpoint, zoneMgmtFeatures,
                                                          appMaxUserDefinedZones, appMaxZones, sensitivityMax, appTwoDCartesianMax);

    TEMPORARY_RETURN_IGNORED mZoneMgmtServerPtr->SetSensitivity(mCameraDevice->GetCameraHALInterface().GetDetectionSensitivity());
}

CHIP_ERROR CameraApp::InitializeCameraAVStreamMgmt()
{
    VerifyOrReturnError(mAVStreamMgmtServer.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    // Set the attribute defaults
    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsHDR())
    {
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetHDRModeEnabled(mCameraDevice->GetCameraHALInterface().GetHDRMode()));
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsSoftPrivacy())
    {
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetSoftRecordingPrivacyModeEnabled(
            mCameraDevice->GetCameraHALInterface().GetSoftRecordingPrivacyModeEnabled()));
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetSoftLivestreamPrivacyModeEnabled(
            mCameraDevice->GetCameraHALInterface().GetSoftLivestreamPrivacyModeEnabled()));
    }

    if (mCameraDevice->GetCameraHALInterface().HasHardPrivacySwitch())
    {
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetHardPrivacyModeOn(mCameraDevice->GetCameraHALInterface().GetHardPrivacyMode()));
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsNightVision())
    {
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetNightVision(mCameraDevice->GetCameraHALInterface().GetNightVision()));
    }

    ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetViewport(mCameraDevice->GetCameraHALInterface().GetViewport()));

    if (mCameraDevice->GetCameraHALInterface().HasSpeaker())
    {
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetSpeakerMuted(mCameraDevice->GetCameraHALInterface().GetSpeakerMuted()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetSpeakerVolumeLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerVolume()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetSpeakerMaxLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerMaxLevel()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetSpeakerMinLevel(mCameraDevice->GetCameraHALInterface().GetSpeakerMinLevel()));
    }

    if (mCameraDevice->GetCameraHALInterface().HasMicrophone())
    {
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetMicrophoneMuted(mCameraDevice->GetCameraHALInterface().GetMicrophoneMuted()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetMicrophoneVolumeLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneVolume()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetMicrophoneMaxLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneMaxLevel()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetMicrophoneMinLevel(mCameraDevice->GetCameraHALInterface().GetMicrophoneMinLevel()));
    }

    // Video and Snapshot features are already enabled.
    if (mCameraDevice->GetCameraHALInterface().HasLocalStorage())
    {
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetLocalVideoRecordingEnabled(
            mCameraDevice->GetCameraHALInterface().GetLocalVideoRecordingEnabled()));
        ReturnErrorOnFailure(mAVStreamMgmtServer.Cluster().SetLocalSnapshotRecordingEnabled(
            mCameraDevice->GetCameraHALInterface().GetLocalSnapshotRecordingEnabled()));
    }

    if (mCameraDevice->GetCameraHALInterface().HasStatusLight())
    {
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetStatusLightEnabled(mCameraDevice->GetCameraHALInterface().GetStatusLightEnabled()));
    }

    if (mCameraDevice->GetCameraHALInterface().GetCameraSupportsImageControl())
    {
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetImageRotation(mCameraDevice->GetCameraHALInterface().GetImageRotation()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetImageFlipVertical(mCameraDevice->GetCameraHALInterface().GetImageFlipVertical()));
        ReturnErrorOnFailure(
            mAVStreamMgmtServer.Cluster().SetImageFlipHorizontal(mCameraDevice->GetCameraHALInterface().GetImageFlipHorizontal()));
    }

    return mAVStreamMgmtServer.Cluster().Init();
}

void CameraApp::CreateAndInitializeCameraAVStreamMgmt()
{

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
    mAVStreamMgmtServer.Create(mCameraDevice->GetCameraAVStreamMgmtDelegate(), mEndpoint, avsmFeatures, avsmOptionalAttrs,
                               maxConcurrentVideoEncoders, maxEncodedPixelRate, sensorParams, nightVisionUsesInfrared, minViewport,
                               rateDistortionTradeOffPoints, maxContentBufferSize, micCapabilities, spkrCapabilities,
                               twowayTalkSupport, snapshotCapabilities, maxNetworkBandwidth, supportedStreamUsages,
                               streamUsagePriorities);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mAVStreamMgmtServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to register CameraAVStreamManagementServer on endpoint %u: %" CHIP_ERROR_FORMAT, mEndpoint,
                     err.Format());
    }

    err = InitializeCameraAVStreamMgmt();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to initialize CameraAVStreamManagementServer on endpoint %u: %" CHIP_ERROR_FORMAT, mEndpoint,
                     err.Format());
    }
}

void CameraApp::InitCameraDeviceClusters()
{
    // Initialize Cluster Servers
    mWebRTCTransportProviderServer.Create(mEndpoint, mCameraDevice->GetWebRTCProviderDelegate());
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mWebRTCTransportProviderServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to register WebRTCTransportProvider on endpoint %u: %" CHIP_ERROR_FORMAT, mEndpoint,
                     err.Format());
    }

    CreateAndInitializeCameraAVStreamMgmt();

    // Set the WebRTCTransportProvider server in the manager
    mCameraDevice->SetWebRTCTransportProvider(&mWebRTCTransportProviderServer.Cluster());

    TEMPORARY_RETURN_IGNORED mChimeServerPtr->Init();

    TEMPORARY_RETURN_IGNORED mAVSettingsUserLevelMgmtServer.Cluster().Init();

    TEMPORARY_RETURN_IGNORED mZoneMgmtServerPtr->Init();
}

void CameraApp::ShutdownCameraDeviceClusters()
{
    ChipLogDetail(Camera, "CameraAppShutdown: Shutting down Camera device clusters");
    mAVSettingsUserLevelMgmtServer.Cluster().Shutdown(ClusterShutdownType::kClusterShutdown);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mWebRTCTransportProviderServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "WebRTCTransportProvider unregister error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    mWebRTCTransportProviderServer.Destroy();

    err = CodegenDataModelProvider::Instance().Registry().Unregister(&mAVStreamMgmtServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "CameraAVStreamMgmt Server unregister error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    mAVStreamMgmtServer.Destroy();
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
