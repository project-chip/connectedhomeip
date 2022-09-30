/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTAHelper.h"

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/Beken/OTAImageProcessorImpl.h>
#include <system/SystemEvent.h>

#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::DeviceLayer;
using namespace chip;

class CustomOTARequestorDriver : public DeviceLayer::ExtendedOTARequestorDriver
{
public:
    bool CanConsent() override;
};

namespace {
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
CustomOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
chip::Optional<bool> gRequestorCanConsent;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;

} // namespace

bool CustomOTARequestorDriver::CanConsent()
{
    return gRequestorCanConsent.ValueOr(DeviceLayer::ExtendedOTARequestorDriver::CanConsent());
}

extern "C" void QueryImageCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling QueryImageCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
}

extern "C" void ApplyUpdateCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling ApplyUpdateCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
}

extern "C" void NotifyUpdateAppliedHandler(uint32_t version)
{
    ChipLogProgress(DeviceLayer, "NotifyUpdateApplied");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->NotifyUpdateApplied(); });
}

extern "C" void BkQueryImageCmdHandler(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    uint32_t dwLoop   = 0;
    uint32_t nodeId   = 0;
    uint32_t fabricId = 0;

    char cmd0 = 0;
    char cmd1 = 0;

    for (dwLoop = 0; dwLoop < argc; dwLoop++)
    {
        ChipLogProgress(DeviceLayer, "QueryImageArgument %d = %s\r\n", dwLoop + 1, argv[dwLoop]);
    }

    if (argc == 3)
    {
        cmd0   = argv[1][0] - 0x30;
        cmd1   = argv[1][1] - 0x30;
        nodeId = (uint32_t)(cmd0 * 10 + cmd1);

        cmd0     = argv[2][0] - 0x30;
        cmd1     = argv[2][1] - 0x30;
        fabricId = (uint32_t)(cmd0 * 10 + cmd1);
        ChipLogProgress(DeviceLayer, "nodeId %lu,fabricId %lu\r\n", nodeId, fabricId);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "cmd param error ");
        return;
    }

    QueryImageCmdHandler();
    ChipLogProgress(DeviceLayer, "QueryImageCmdHandler begin");
}

extern "C" void BkApplyUpdateCmdHandler(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    ApplyUpdateCmdHandler();
    ChipLogProgress(DeviceLayer, "ApplyUpdateCmdHandler send request");
}

extern "C" void BkNotifyUpdateApplied(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    uint32_t dwLoop  = 0;
    uint32_t version = 0;

    char cmd0 = 0;
    char cmd1 = 0;

    for (dwLoop = 0; dwLoop < argc; dwLoop++)
    {
        ChipLogProgress(DeviceLayer, "NotifyUpdateApplied %d = %s\r\n", dwLoop + 1, argv[dwLoop]);
    }

    if (argc == 2)
    {
        cmd0    = argv[1][0] - 0x30;
        cmd1    = argv[1][1] - 0x30;
        version = (uint32_t)(cmd0 * 10 + cmd1);

        ChipLogProgress(DeviceLayer, "version %lu \r\n", version);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "cmd param error ");
        return;
    }

    NotifyUpdateAppliedHandler(version);
    ChipLogProgress(DeviceLayer, "NotifyUpdateApplied send request");
}

static void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());

    // Set server instance used for session establishment
    gRequestorCore.Init(Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetUserConsentState(gUserConsentState);
        gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
    }

    // Initialize and interconnect the Requestor and Image Processor objects -- END
}

void OTAHelpers::InitOTARequestor()
{
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec), InitOTARequestorHandler,
                                                nullptr);
}
