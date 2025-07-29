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

#include "camera-app.h"
#include "camera-device.h"

#include <AppMain.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Camera;

CameraDevice gCameraDevice;

void ApplicationInit()
{
    ChipLogProgress(Camera, "Matter Camera Linux App: ApplicationInit()");
    if (LinuxDeviceOptions::GetInstance().cameraVideoDevice.HasValue())
    {
        std::string videoDevicePath = LinuxDeviceOptions::GetInstance().cameraVideoDevice.Value();
        ChipLogDetail(Camera, "Using video device path from options: %s", videoDevicePath.c_str());
        gCameraDevice.SetVideoDevicePath(videoDevicePath);
    }
    else
    {
        ChipLogDetail(Camera, "Using default video device path: %s", Camera::kDefaultVideoDevicePath);
    }
    gCameraDevice.Init();
    CameraAppInit(&gCameraDevice);
}

void ApplicationShutdown()
{
    CameraAppShutdown();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    ChipLinuxAppMainLoop();

    return 0;
}
