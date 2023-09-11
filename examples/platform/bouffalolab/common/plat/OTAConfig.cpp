/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
