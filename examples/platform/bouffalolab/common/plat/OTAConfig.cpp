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
#include <app/server/Server.h>

#include <platform/bouffalolab/common/OTAImageProcessorImpl.h>
// Global OTA objects
chip::DefaultOTARequestor gRequestorCore;
chip::DefaultOTARequestorStorage gRequestorStorage;
chip::DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
chip::BDXDownloader gDownloader;
chip::OTAImageProcessorImpl gImageProcessor;

void OTAConfig::Init()
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    // Periodic query timeout must be set prior to requestor being initialized
    gRequestorUser.SetPeriodicQueryTimeout(OTA_PERIODIC_TIMEOUT);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}

void OTAConfig::InitOTARequestorHandler(chip::System::Layer * systemLayer, void * appState)
{
    if (!chip::GetRequestorInstance())
    {
        ChipLogProgress(NotSpecified, "Init OTA Requestor");
        OTAConfig::Init();
    }
}
