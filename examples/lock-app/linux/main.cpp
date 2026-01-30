/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppMain.h"
#include <app-common/zap-generated/ids/Clusters.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/Linux/OTAImageProcessorImpl.h>
#endif

#include "Identify.h"
#include "LockAppCommandDelegate.h"
#include "LockManager.h"

using namespace chip;
using namespace chip::app;

namespace {
// Variables for handling named pipe commands
NamedPipeCommands sChipNamedPipeCommands;
LockAppCommandDelegate sLockAppCommandDelegate;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DeviceLayer::ExtendedOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static char gOtaDownloadPath[] = "/tmp/test.bin";

void InitOTARequestor()
{
    // Set the global instance of the OTA requestor core component
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    SuccessOrDie(gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader));
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTAImageFile(gOtaDownloadPath);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Set the image processor instance used for handling image being downloaded
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
}
#endif

} // anonymous namespace

void ApplicationInit()
{
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sLockAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    InitOTARequestor();
#endif
    TEMPORARY_RETURN_IGNORED IdentifyInit();
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // TODO: Watch for LockState, DoorState, Mode, etc changes and trigger appropriate action
    if (attributePath.mClusterId == Clusters::DoorLock::Id)
    {
        ChipLogProgress(Zcl, "Door Lock attribute changed");
    }
}
