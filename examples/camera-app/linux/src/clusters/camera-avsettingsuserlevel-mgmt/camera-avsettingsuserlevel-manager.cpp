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
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <camera-avsettingsuserlevel-manager.h>
#include <cmath>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using chip::Protocols::InteractionModel::Status;

void CameraAVSettingsUserLevelManager::SetCameraDeviceHAL(CameraDeviceInterface * aCameraDeviceHAL)
{
    mCameraDeviceHAL = aCameraDeviceHAL;
}

bool CameraAVSettingsUserLevelManager::CanChangeMPTZ()
{
    // The server needs to verify that the camera is in a state whereby changing the MPTZ is possible.
    // Device specific logic will be required.
    return true;
}

void CameraAVSettingsUserLevelManager::VideoStreamAllocated(uint16_t aStreamID)
{
    Globals::Structs::ViewportStruct::Type viewport = mCameraDeviceHAL->GetCameraHALInterface().GetViewport();
    this->GetServer()->AddMoveCapableVideoStream(aStreamID, viewport);
}

void CameraAVSettingsUserLevelManager::VideoStreamDeallocated(uint16_t aStreamID)
{
    this->GetServer()->RemoveMoveCapableVideoStream(aStreamID);
}

void CameraAVSettingsUserLevelManager::DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport)
{
    this->GetServer()->UpdateMoveCapableVideoStreams(aViewport);
}

Status CameraAVSettingsUserLevelManager::MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    if (aPan.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetPan(aPan.Value());
    }

    if (aTilt.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetTilt(aTilt.Value());
    }

    if (aZoom.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetZoom(aZoom.Value());
    }

    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    if (aPan.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetPan(aPan.Value());
    }

    if (aTilt.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetTilt(aTilt.Value());
    }

    if (aZoom.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetZoom(aZoom.Value());
    }

    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                          Optional<uint8_t> aZoom)
{
    // The Cluster implementation has validated the preset is valid, and provided the MPTZ values associated with that preset.
    // Do any needed hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response.
    //
    if (aPan.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetPan(aPan.Value());
    }

    if (aTilt.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetTilt(aTilt.Value());
    }

    if (aZoom.HasValue())
    {
        mCameraDeviceHAL->GetCameraHALInterface().SetZoom(aZoom.Value());
    }

    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZSavePreset(uint8_t aPreset)
{
    // The Cluster implementation has validated that there is space, and provided the new preset id.
    // The application needs to confirm that the current MPTZ values can be save in a new preset.
    //
    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZRemovePreset(uint8_t aPreset)
{
    // The Cluster implementation has validated that there is a saved preset associated with the provided id.
    // The application needs to confirm that this action is acceptable given the current state of the device.
    //
    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::DPTZSetViewport(uint16_t aVideoStreamID, Globals::Structs::ViewportStruct::Type aViewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with HAL to access the stream, validate the viewport
    // and set the new viewport value.
    //
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == aVideoStreamID && stream.isAllocated)
        {
            // Validate the received viewport dimensions
            //
            // Ensure pixel count is > min pixels
            // Esnure width does not exceed sensor width
            // Ensure height does not exceed sensor height
            //
            uint16_t requestedWidth             = aViewport.x2 - aViewport.x1;
            uint16_t requestedHeight            = aViewport.y2 - aViewport.y1;
            VideoResolutionStruct minResolution = mCameraDeviceHAL->GetCameraHALInterface().GetMinViewport();
            VideoSensorParamsStruct sensorParms = mCameraDeviceHAL->GetCameraHALInterface().GetVideoSensorParams();
            if ((requestedWidth < minResolution.width) || (requestedHeight < minResolution.height) ||
                (requestedWidth > sensorParms.sensorWidth) || (requestedHeight > sensorParms.sensorHeight))
            {
                ChipLogError(Camera, "CameraApp: DPTZSetViewport with invalid viewport dimensions");
                return Status::ConstraintError;
            }

            // Get the ARs to no more than 2DP.  Otherwise you get mismatches e.g. 16:9 ratio calculation for 480p isn't the same as
            // 1080p beyond 2DP.
            float requestedAR = floorf((static_cast<float>(requestedWidth) / requestedHeight) * 100) / 100;
            float streamAR    = floorf((static_cast<float>(stream.videoStreamParams.maxResolution.width) /
                                     stream.videoStreamParams.maxResolution.height) *
                                       100) /
                100;

            ChipLogDetail(Camera, "DPTZSetViewpoort. AR of viewport %f, AR of stream %f.", requestedAR, streamAR);
            // Ensure that the aspect ration of the viewport matches the aspect ratio of the stream
            if (requestedAR != streamAR)
            {
                ChipLogError(Camera, "CameraApp: DPTZSetViewport with mismatching aspect ratio.");
                return Status::ConstraintError;
            }
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, aViewport);
            return Status::Success;
        }
    }

    ChipLogError(Camera, "CameraApp: DPTZSetViewport with no matching video stream ID.");
    return Status::NotFound;
}

Status CameraAVSettingsUserLevelManager::DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                          Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta,
                                                          Globals::Structs::ViewportStruct::Type & aViewport)
{
    // The Cluster implementation has ensured that the videoStreamID represents a valid stream.
    // The application needs to interact with its instance of AVStreamManagement to access the stream, validate the viewport
    // and set the new values for the viewpoort based on the pixel movement requested
    //
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == aVideoStreamID && stream.isAllocated)
        {
            Globals::Structs::ViewportStruct::Type viewport = stream.viewport;
            VideoResolutionStruct minResolution             = mCameraDeviceHAL->GetCameraHALInterface().GetMinViewport();
            VideoSensorParamsStruct sensorParms             = mCameraDeviceHAL->GetCameraHALInterface().GetVideoSensorParams();

            if (aDeltaX.HasValue())
            {
                int16_t deltaX = aDeltaX.Value();
                if (deltaX != 0)
                {
                    // if the delta would move us out of the cartesian plane of the sensor, limit to the left hand edge
                    int16_t x1Movement =
                        ((deltaX < 0) && (abs(deltaX) > viewport.x1)) ? static_cast<int16_t>(-viewport.x1) : deltaX;
                    viewport.x1 = static_cast<uint16_t>(viewport.x1 + x1Movement);

                    int16_t x2Movement =
                        ((deltaX < 0) && (abs(deltaX) > viewport.x2)) ? static_cast<int16_t>(-viewport.x2) : deltaX;
                    viewport.x2 = static_cast<uint16_t>(viewport.x2 + x2Movement);
                }
            }

            if (aDeltaY.HasValue())
            {
                int16_t deltaY = aDeltaY.Value();
                if (deltaY != 0)
                {
                    // if the delta would move us out of the cartesian plane of the sensor, limit to the top hand edge
                    //
                    int16_t y1Movement =
                        ((deltaY < 0) && (abs(deltaY) > viewport.y1)) ? static_cast<int16_t>(-viewport.y1) : deltaY;
                    viewport.y1 = static_cast<uint16_t>(viewport.y1 + y1Movement);

                    int16_t y2Movement =
                        ((deltaY < 0) && (abs(deltaY) > viewport.y2)) ? static_cast<int16_t>(-viewport.y2) : deltaY;
                    viewport.y2 = static_cast<uint16_t>(viewport.y2 + y2Movement);
                }
            }

            if (aZoomDelta.HasValue())
            {
                int8_t zoomDelta = aZoomDelta.Value();
                if (zoomDelta != 0)
                {
                    // Scale the current values by the given zoom
                    //
                    uint16_t originalWidth     = viewport.x2 - viewport.x1;
                    uint16_t originalHeight    = viewport.y2 - viewport.y1;
                    uint32_t originalSize      = originalWidth * originalHeight;
                    uint32_t zoomDeltaInPixels = originalSize * static_cast<uint32_t>(abs(zoomDelta)) / 100;
                    uint32_t newSize = (zoomDelta < 0) ? originalSize - zoomDeltaInPixels : originalSize + zoomDeltaInPixels;

                    // If the new viewport after zoom would be less than the min, scale it to the min
                    //
                    if (newSize < (minResolution.width * minResolution.height))
                    {
                        newSize = minResolution.width * minResolution.height;
                    }

                    // The new width of a rectangle with a defined aspect ratio is the square root of the new size
                    // of that rectangle multiplied by the aspect ratio
                    //
                    double viewportAR = floorf((static_cast<float>(originalWidth) / originalHeight) * 100) / 100;

                    uint16_t newWidth  = static_cast<uint16_t>(round(sqrt(newSize * viewportAR)));
                    uint16_t newHeight = static_cast<uint16_t>(newWidth / viewportAR);

                    viewport.x1 = static_cast<uint16_t>(viewport.x1 - (newWidth - originalWidth));
                    viewport.x2 = static_cast<uint16_t>(viewport.x2 + (newWidth - originalWidth));
                    viewport.y1 = static_cast<uint16_t>(viewport.y1 - (newHeight - originalHeight));
                    viewport.y2 = static_cast<uint16_t>(viewport.y2 + (newHeight - originalHeight));
                }
            }
            // Is the requested viewport smaller than the minimum, if yes scale to the minimum size, starting at x1, y1.
            //
            if (((viewport.x2 - viewport.x1) < minResolution.width) || ((viewport.y2 - viewport.y1) < minResolution.height))
            {
                viewport.x2 = viewport.x1 + minResolution.width;
                viewport.y2 = viewport.y1 + minResolution.height;
            }

            // Is the requested viewport greater than the sensor size, if yes, scale to the sensor
            //
            if (((viewport.x2 - viewport.x1) > sensorParms.sensorWidth) || ((viewport.y2 - viewport.y1) > sensorParms.sensorHeight))
            {
                viewport.x1 = 0;
                viewport.y1 = 0;
                viewport.x2 = sensorParms.sensorWidth - 1;
                viewport.y2 = sensorParms.sensorHeight - 1;
            }
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, viewport);
            aViewport = viewport;
            return Status::Success;
        }
    }

    ChipLogError(Camera, "CameraApp: DPTZRelativeMove with no matching video stream ID.");
    return Status::NotFound;
}

CHIP_ERROR CameraAVSettingsUserLevelManager::LoadMPTZPresets(std::vector<MPTZPresetHelper> & mptzPresetHelpers)
{
    mptzPresetHelpers.clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVSettingsUserLevelManager::LoadDPTZStreams(std::vector<DPTZStruct> & dptzStreams)
{
    dptzStreams.clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVSettingsUserLevelManager::PersistentAttributesLoadedCallback()
{
    ChipLogDetail(Camera, "CameraAvSettingsUserLevelManagement: Persistent attributes loaded");
    return CHIP_NO_ERROR;
}
