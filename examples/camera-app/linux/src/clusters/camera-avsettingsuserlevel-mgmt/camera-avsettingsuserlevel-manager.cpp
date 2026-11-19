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

#include <algorithm>
#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <camera-avsettingsuserlevel-manager.h>
#include <cmath>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using chip::Protocols::InteractionModel::Status;

static void onTimerExpiry(System::Layer * systemLayer, void * data);

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

Status CameraAVSettingsUserLevelManager::MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                                                         PhysicalPTZCallback * callback)
{
    mCallback = callback;

    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    mCameraDeviceHAL->GetCameraHALInterface().SetPhysicalPTZ(aPan, aTilt, aZoom);

    // For the purposes of the Camera App, run a timer equivalent to a typical physical elapsed time for PTZ. An actual HAL will
    // invoke OnPhysicalMoveCompleted method once it has determined via its own means that the move is completed.
    //
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(2), onTimerExpiry, this);
    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                                                          PhysicalPTZCallback * callback)
{
    mCallback = callback;

    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    mCameraDeviceHAL->GetCameraHALInterface().SetPhysicalPTZ(aPan, aTilt, aZoom);

    // For the purposes of the Camera App, run a timer equivalent to a typical physical elapsed time for PTZ. AAn actual HAL will
    // invoke OnPhysicalMoveCompleted method once it has determined via its own means that the move is completed.
    //
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(2), onTimerExpiry, this);
    return Status::Success;
}

Status CameraAVSettingsUserLevelManager::MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                          Optional<uint8_t> aZoom, PhysicalPTZCallback * callback)
{
    mCallback = callback;

    // The Cluster implementation has validated that the Feature Flags are set and the values themselves are in range. Do any needed
    // hardware interactions to actually set the camera to the new values of PTZ.  Then return a Status response. The server itself
    // will persist the new values.
    //
    mCameraDeviceHAL->GetCameraHALInterface().SetPhysicalPTZ(aPan, aTilt, aZoom);

    // For the purposes of the Camera App, run a timer equivalent to a typical physical elapsed time for PTZ. AAn actual HAL will
    // invoke OnPhysicalMoveCompleted method once it has determined via its own means that the move is completed.
    //
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(2), onTimerExpiry, this);
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

            // Track the position and size in int so the deltas and the zoom can be applied
            // without intermediate clamping, and the viewport is clamped against the sensor
            // once at the end.
            int x1     = static_cast<int>(viewport.x1);
            int y1     = static_cast<int>(viewport.y1);
            int width  = static_cast<int>(viewport.x2) - static_cast<int>(viewport.x1);
            int height = static_cast<int>(viewport.y2) - static_cast<int>(viewport.y1);

            if (aDeltaX.HasValue())
            {
                x1 += aDeltaX.Value();
            }
            if (aDeltaY.HasValue())
            {
                y1 += aDeltaY.Value();
            }

            if (aZoomDelta.HasValue() && aZoomDelta.Value() != 0)
            {
                int8_t zoomDelta = aZoomDelta.Value();

                // Scale the current size by the given zoom
                //
                uint32_t originalSize      = static_cast<uint32_t>(width) * static_cast<uint32_t>(height);
                uint32_t zoomDeltaInPixels = originalSize * static_cast<uint32_t>(abs(zoomDelta)) / 100;
                uint32_t newSize = (zoomDelta < 0) ? originalSize + zoomDeltaInPixels : originalSize - zoomDeltaInPixels;

                // If the new viewport after zoom would be less than the min, scale it to the min
                //
                if (newSize < (minResolution.width * minResolution.height))
                {
                    newSize = minResolution.width * minResolution.height;
                }

                // The new width of a rectangle with a defined aspect ratio is the square root of the new size
                // of that rectangle multiplied by the aspect ratio
                //
                double viewportAR = floorf((static_cast<float>(width) / height) * 100) / 100;

                int newWidth  = static_cast<int>(round(sqrt(newSize * viewportAR)));
                int newHeight = static_cast<int>(newWidth / viewportAR);

                // Cap the new size to the minimum and the sensor dimensions before positioning the
                // viewport, so the capping does not shift the viewport centre.
                newWidth  = std::max(newWidth, static_cast<int>(minResolution.width));
                newHeight = std::max(newHeight, static_cast<int>(minResolution.height));
                newWidth  = std::min(newWidth, static_cast<int>(sensorParms.sensorWidth));
                newHeight = std::min(newHeight, static_cast<int>(sensorParms.sensorHeight));

                // Scale the moved viewport about its centre
                int centerX = x1 + width / 2;
                int centerY = y1 + height / 2;
                x1     = centerX - newWidth / 2;
                y1     = centerY - newHeight / 2;
                width  = newWidth;
                height = newHeight;
            }

            // Move the upper-left corner inside the sensor. The size is capped to the sensor, so the
            // clamp limits are valid and the viewport keeps its size as it pins against an edge.
            x1 = std::clamp(x1, 0, static_cast<int>(sensorParms.sensorWidth) - width);
            y1 = std::clamp(y1, 0, static_cast<int>(sensorParms.sensorHeight) - height);

            viewport.x1 = static_cast<uint16_t>(x1);
            viewport.y1 = static_cast<uint16_t>(y1);
            viewport.x2 = static_cast<uint16_t>(x1 + width);
            viewport.y2 = static_cast<uint16_t>(y1 + height);
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, viewport);
            aViewport = viewport;
            return Status::Success;
        }
    }

    ChipLogError(Camera, "CameraApp: DPTZRelativeMove with no matching video stream ID.");
    return Status::NotFound;
}

CHIP_ERROR CameraAVSettingsUserLevelManager::PersistentAttributesLoadedCallback()
{
    ChipLogDetail(Camera, "CameraAvSettingsUserLevelManagement: Persistent attributes loaded");
    return CHIP_NO_ERROR;
}

// Timer expiration to mimic PTZ physical movememt
//
static void onTimerExpiry(System::Layer * systemLayer, void * data)
{
    CameraAVSettingsUserLevelManager * delegate = reinterpret_cast<CameraAVSettingsUserLevelManager *>(data);

    // All timers are cancelled on delegate shutdown, hence if this is invoked the delegate is alive
    delegate->OnPhysicalMoveCompleted(Protocols::InteractionModel::Status::Success);
}

void CameraAVSettingsUserLevelManager::CancelActiveTimers()
{
    // Cancel the PTZ mimic timer if it is active
    DeviceLayer::SystemLayer().CancelTimer(onTimerExpiry, this);
}

void CameraAVSettingsUserLevelManager::ShutdownApp()
{
    CancelActiveTimers();
}

// To be invoked by the camera once a physical PTZ action has completed. The callback method is realized by our cluster server,
// make sure that is still alive before trying to invoke methods thereon.
//
void CameraAVSettingsUserLevelManager::OnPhysicalMoveCompleted(Protocols::InteractionModel::Status status)
{
    if (GetServer() != nullptr)
    {
        if (mCallback != nullptr)
        {
            mCallback->OnPhysicalMovementComplete(status);
        }
    }
}
