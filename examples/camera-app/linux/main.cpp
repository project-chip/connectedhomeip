/*
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

#include "CameraAppCommandDelegate.h"
#include "camera-app.h"
#include "camera-device.h"
#include "tls-client-management-instance.h"

#include <AppMain.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Camera;

namespace {
NamedPipeCommands sChipNamedPipeCommands;
CameraAppCommandDelegate sCameraAppCommandDelegate;
} // namespace

CameraDevice gCameraDevice;

void ApplicationInit()
{
    ChipLogProgress(Camera, "Matter Camera Linux App: ApplicationInit()");
    if (LinuxDeviceOptions::GetInstance().cameraVideoDevice.HasValue())
    {
        std::string videoDevicePath = LinuxDeviceOptions::GetInstance().cameraVideoDevice.Value();
        // If the path does not start with '/', assume it's a device name and prepend /dev/
        if (!videoDevicePath.empty() && videoDevicePath[0] != '/')
        {
            videoDevicePath = "/dev/" + videoDevicePath;
        }
        ChipLogDetail(Camera, "Using video device path from options: %s", videoDevicePath.c_str());
        gCameraDevice.SetVideoDevicePath(videoDevicePath);
    }
    else
    {
        ChipLogDetail(Camera, "Using default video device path: %s", Camera::kDefaultVideoDevicePath);
    }

    std::string appPipePath = std::string(LinuxDeviceOptions::GetInstance().app_pipe);
    if ((!appPipePath.empty()) && (sChipNamedPipeCommands.Start(appPipePath, &sCameraAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

    // Initialize TLS Client Management delegate before server starts
    // This must be called before ChipLinuxAppMainLoop() which initializes the server
    InitializeTlsClientManagement();

    gCameraDevice.Init();
    CameraAppInit(&gCameraDevice);

    sCameraAppCommandDelegate.SetCameraDevice(&gCameraDevice);
}

void ApplicationShutdown()
{
    CameraAppShutdown();

    TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    ChipLinuxAppMainLoop();

    return 0;
}
