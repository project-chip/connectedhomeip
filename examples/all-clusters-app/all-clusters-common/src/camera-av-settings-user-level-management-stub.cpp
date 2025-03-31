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
    return true;
}

bool AVSettingsUserLevelManagementDelegate::IsValidVideoStreamID(uint16_t videoStreamID)
{
    return true;
}

Status AVSettingsUserLevelManagementDelegate::PersistentAttributesLoadedCallback()
{
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZSetPosition(Optional<int16_t> pan, Optional<int16_t> tilt, Optional<int8_t> zoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRelativeMove(Optional<int16_t> pan, Optional<int16_t> tilt, Optional<int8_t> zoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZMoveToPreset(uint8_t preset, Optional<int16_t> pan, Optional<int16_t> tilt,
                                                               Optional<int8_t> zoom)
{
    // The Cluster implementation has validated the preset is value, and provided the MPTZ values associated with that preset.
    // Do any needed hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZSavePreset(uint8_t preset)
{
    // The Cluster implementation has validated that there is space, and provided the new preset id.
    // The application needs to confirm that the current MPTZ values can be save in a new preset.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::MPTZRemovePreset(uint8_t preset)
{
    // The Cluster implementation has validated that there is a saved preset associated with the provided id.
    // The application needs to confirm that this action is acceptable given the current state of the device.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZSetViewport(uint16_t videoStreamID, Structs::ViewportStruct::Type viewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new vieport value.
    //
    return Status::Success;
}

Status AVSettingsUserLevelManagementDelegate::DPTZRelativeMove(uint16_t videoStreamID, Optional<int16_t> deltaX,
                                                               Optional<int16_t> deltaY, Optional<int8_t> zoomDelta)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new values for the viewpoort based on the pixel movement requested
    //
    return Status::Success;
}

void emberAfCameraAvSettingsUserLevelManagementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gDelegate == nullptr && gAVSettingsUserLevelManagementCluster == nullptr);
    const int16_t appPanMin  = -90;
    const int16_t appPanMax  = 90;
    const int16_t appTiltMin = -45;
    const int16_t appTiltMax = 45;
    const int8_t appZoomMax  = 75;

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
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kZoomMax,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kTiltMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMin,
        CameraAvSettingsUserLevelManagement::OptionalAttributes::kPanMax);

    gAVSettingsUserLevelManagementCluster =
        new CameraAvSettingsUserLevelMgmtServer(kEndpointId, gDelegate, avsumFeatures, avsumAttrs);
    gAVSettingsUserLevelManagementCluster->Init();

    // Set app specific limits to pan, tilt, zoom
    gAVSettingsUserLevelManagementCluster->setPanMin(appPanMin);
    gAVSettingsUserLevelManagementCluster->setPanMax(appPanMax);
    gAVSettingsUserLevelManagementCluster->setTiltMin(appTiltMin);
    gAVSettingsUserLevelManagementCluster->setTiltMax(appTiltMax);
    gAVSettingsUserLevelManagementCluster->setZoomMax(appZoomMax);
}
