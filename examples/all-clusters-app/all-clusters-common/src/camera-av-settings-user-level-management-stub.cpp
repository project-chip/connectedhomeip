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

#include <camera-av-settings-user-level-management-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using chip::Protocols::InteractionModel::Status;

static AVSettingsUserLevelManagementDelegate * gDelegate                           = nullptr;
static CameraAvSettingsUserLevelMgmtServer * gAVSettingsUserLevelManagementCluster = nullptr;
static constexpr EndpointId kEndpointId                                            = 1;

CameraAvSettingsUserLevelMgmtServer * GetInstance()
{
    return gAVSettingsUserLevelManagementCluster;
}

void Shutdown()
{
    if (gAVSettingsUserLevelManagementCluster != nullptr)
    {
        delete gAVSettingsUserLevelManagementCluster;
        gAVSettingsUserLevelManagementCluster = nullptr;
    }
}

bool AVSettingsUserLevelManagementDelegate::CanChangeMPTZ()
{
    // The server needs to verify that the camera is in a state whereby changing the MPTZ is possible.
    // Device specific logic will be required.
    return true;
}

bool AVSettingsUserLevelManagementDelegate::IsValidVideoStreamID(uint16_t aVideoStreamID)
{
    // The server needs to verify that the provided Video Stream ID is valid and known and subject to digital modification
    // The camera app needs to also have an instance of AV Stream Management, querying that to determine validity of the provided
    // id.
    return true;
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

Status AVSettingsUserLevelManagementDelegate::DPTZSetViewport(uint16_t aVideoStreamID, Structs::ViewportStruct::Type aViewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new vieport value.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                               Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new values for the viewpoort based on the pixel movement requested
    //
    return Status::Success;
}

void emberAfCameraAvSettingsUserLevelManagementClusterInitCallback(chip::EndpointId aEndpointId)
{
    VerifyOrDie(aEndpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gDelegate == nullptr && gAVSettingsUserLevelManagementCluster == nullptr);
    const int16_t appPanMin     = -90;
    const int16_t appPanMax     = 90;
    const int16_t appTiltMin    = -45;
    const int16_t appTiltMax    = 45;
    const uint8_t appZoomMax    = 75;
    const uint8_t appMaxPresets = 5;

    gDelegate = new AVSettingsUserLevelManagementDelegate;
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

    gAVSettingsUserLevelManagementCluster =
        new CameraAvSettingsUserLevelMgmtServer(kEndpointId, gDelegate, avsumFeatures, avsumAttrs, appMaxPresets);
    gAVSettingsUserLevelManagementCluster->Init();

    // Set app specific limits to pan, tilt, zoom
    gAVSettingsUserLevelManagementCluster->SetPanMin(appPanMin);
    gAVSettingsUserLevelManagementCluster->SetPanMax(appPanMax);
    gAVSettingsUserLevelManagementCluster->SetTiltMin(appTiltMin);
    gAVSettingsUserLevelManagementCluster->SetTiltMax(appTiltMax);
    gAVSettingsUserLevelManagementCluster->SetZoomMax(appZoomMax);
}
