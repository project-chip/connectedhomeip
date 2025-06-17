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

#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-server.h>
#include <camera-av-settings-user-level-management-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using chip::Protocols::InteractionModel::Status;

std::unique_ptr<AVSettingsUserLevelManagementDelegate> gDelegate;
std::unique_ptr<CameraAvSettingsUserLevelMgmtServer> gAVSettingsUserLevelManagementCluster;
static constexpr EndpointId kEndpointId = 1;

void Shutdown()
{
    if (gAVSettingsUserLevelManagementCluster != nullptr)
    {
        gDelegate                             = nullptr;
        gAVSettingsUserLevelManagementCluster = nullptr;
    }
}

bool AVSettingsUserLevelManagementDelegate::CanChangeMPTZ()
{
    // The server needs to verify that the camera is in a state whereby changing the MPTZ is possible.
    // Device specific logic will be required.
    return true;
}

void AVSettingsUserLevelManagementDelegate::VideoStreamAllocated(uint16_t aStreamID)
{
    // The app needs to invoke this whenever the AV Stream Manager allocates a video stream; this informs the server of the
    // id that is now subject to DPTZ, and the default viewport of the device
    Globals::Structs::ViewportStruct::Type viewport = { 0, 0, 1920, 1080 };
    this->GetServer()->AddMoveCapableVideoStream(aStreamID, viewport);
}

void AVSettingsUserLevelManagementDelegate::VideoStreamDeallocated(uint16_t aStreamID)
{
    // The app needs to invoke this whenever the AV Stream Manager deallocates a video stream; this informs the server of the
    // deallocated id that is now not subject to DPTZ
    this->GetServer()->RemoveMoveCapableVideoStream(aStreamID);
}

void AVSettingsUserLevelManagementDelegate::DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport)
{
    // The app needs to invoke this whenever the AV Stream Manager updates the device level default Viewport.  This informs
    // the server of the new viewport that shall be appled to all known streams.
    this->GetServer()->UpdateMoveCapableVideoStreams(aViewport);
}

Status AVSettingsUserLevelManagementDelegate::MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                              Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                               Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                               Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated the preset is valid, and provided the MPTZ values associated with that preset.
    // Do any needed hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZSavePreset(uint8_t aPreset)
{
    // The Cluster implementation has validated that there is space, and provided the new preset id.
    // The application needs to confirm that the current MPTZ values can be save in a new preset.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRemovePreset(uint8_t aPreset)
{
    // The Cluster implementation has validated that there is a saved preset associated with the provided id.
    // The application needs to confirm that this action is acceptable given the current state of the device.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZSetViewport(uint16_t aVideoStreamID,
                                                              Globals::Structs::ViewportStruct::Type aViewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new vieport value.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                               Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta,
                                                               Globals::Structs::ViewportStruct::Type & aViewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate
    // new dimensions after application of the deltas, and set the new values for the viewport based on the pixel movement
    // requested
    // The passed in viewport is empty, and needs to be populated by the delegate with the value of the viewport after
    // applying all deltas within the constraints of the sensor.
    //
    aViewport = { 0, 0, 1920, 1080 };
    return Status::Success;
}

CHIP_ERROR AVSettingsUserLevelManagementDelegate::LoadMPTZPresets(std::vector<MPTZPresetHelper> & mptzPresetHelpers)
{
    mptzPresetHelpers.clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AVSettingsUserLevelManagementDelegate::LoadDPTZStreams(std::vector<DPTZStruct> dptzStreams)
{
    dptzStreams.clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AVSettingsUserLevelManagementDelegate::PersistentAttributesLoadedCallback()
{
    return CHIP_NO_ERROR;
}

void emberAfCameraAvSettingsUserLevelManagementClusterInitCallback(chip::EndpointId aEndpointId)
{
    VerifyOrDie(aEndpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gDelegate && !gAVSettingsUserLevelManagementCluster);
    const int16_t appPanMin     = -90;
    const int16_t appPanMax     = 90;
    const int16_t appTiltMin    = -45;
    const int16_t appTiltMax    = 45;
    const uint8_t appZoomMax    = 75;
    const uint8_t appMaxPresets = 5;

    gDelegate = std::make_unique<AVSettingsUserLevelManagementDelegate>();
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

    gAVSettingsUserLevelManagementCluster = std::make_unique<CameraAvSettingsUserLevelMgmtServer>(
        kEndpointId, *gDelegate.get(), avsumFeatures, avsumAttrs, appMaxPresets);
    gAVSettingsUserLevelManagementCluster->Init();

    // Set app specific limits to pan, tilt, zoom
    gAVSettingsUserLevelManagementCluster->SetPanMin(appPanMin);
    gAVSettingsUserLevelManagementCluster->SetPanMax(appPanMax);
    gAVSettingsUserLevelManagementCluster->SetTiltMin(appTiltMin);
    gAVSettingsUserLevelManagementCluster->SetTiltMax(appTiltMax);
    gAVSettingsUserLevelManagementCluster->SetZoomMax(appZoomMax);
}
