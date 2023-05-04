/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "init_OTARequestor.h"
#include "app/clusters/ota-requestor/DefaultOTARequestorStorage.h"
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/ASR/ASROTAImageProcessor.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
ExtendedOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
ASROTAImageProcessor gImageProcessor;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kGranted;
} // namespace

#define OTA_PERIODIC_TIMEOUT 86400 // 24 * 60 * 60

extern "C" void asrQueryImageCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling asrQueryImageCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
}

extern "C" void asrApplyUpdateCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling asrApplyUpdateCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
}

void OTAInitializer::InitOTARequestor(void)
{
    SetRequestorInstance(&gRequestorCore);
    ConfigurationMgr().StoreSoftwareVersion(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    // Set server instance used for session establishment
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    gRequestorUser.SetPeriodicQueryTimeout(OTA_PERIODIC_TIMEOUT);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);
    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetUserConsentState(gUserConsentState);
        gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
    }
    ChipLogProgress(DeviceLayer, "Current Software Version: %u", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    ChipLogProgress(DeviceLayer, "Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
}

void OTAInitializer::ReloadQueryTimeout(uint32_t timeout)
{
    if (timeout > 0)
    {
        gRequestorUser.SetPeriodicQueryTimeout(timeout);
        gRequestorUser.RekickPeriodicQueryTimer();
    }
}
