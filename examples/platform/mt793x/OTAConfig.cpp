/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "OTAConfig.h"

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/server/Server.h>
#include <platform/mt793x/OTAImageProcessorImpl.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace OTAConfig {

constexpr uint32_t kInitOTARequestorDelaySec = 3;

// Global OTA objects
chip::DefaultOTARequestor gRequestorCore;
chip::DefaultOTARequestorStorage gRequestorStorage;
chip::DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
chip::BDXDownloader gDownloader;
chip::OTAImageProcessorImpl gImageProcessor;

static void PlatformEventHandler(const ChipDeviceEvent * event, intptr_t arg);
static void InitRequestor(void);

void Init()
{
    ChipLogProgress(DeviceLayer, "Register OTA Requestor init handler");
    PlatformMgrImpl().AddEventHandler(PlatformEventHandler, 0);
}

void PlatformEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{

    ChipLogProgress(SoftwareUpdate, "OTA Config handler enter !!");
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established ||
            event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
        {
            ChipLogProgress(SoftwareUpdate, "Internet connect established!!");
            InitRequestor();
            // SystemLayer().StartTimer(System::Clock::Seconds32(kInitOTARequestorDelaySec), InitRequestor, nullptr);
        }
        break;
    default:
        break;
    }
}

void InitRequestor(void)
{
    ChipLogProgress(SoftwareUpdate, "Init Requestor enter");
    if (GetRequestorInstance() != nullptr)
        return;

    ChipLogProgress(SoftwareUpdate, "Initializing requestor");

    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}

} // namespace OTAConfig
