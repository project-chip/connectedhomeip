/*
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

#include "AppMain.h"
#include <app/server/Server.h>

#include <CommissionableInit.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

LinuxCommissionableDataProvider gCommissionableDataProvider;

}

void CleanShutdown()
{
    Server::GetInstance().Shutdown();
    PlatformMgr().Shutdown();
    // TODO: We don't Platform::MemoryShutdown because ~CASESessionManager calls
    // Dnssd::ResolverProxy::Shutdown, which starts doing Platform::Delete.
    // Platform::MemoryShutdown();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * aData, size_t aSize)
{
    static bool matterStackInitialized = false;
    if (!matterStackInitialized)
    {
        // Might be simpler to do ChipLinuxAppInit() with argc == 0, argv set to
        // just a fake executable name?
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        VerifyOrDie(PlatformMgr().InitChipStack() == CHIP_NO_ERROR);

        VerifyOrDie(chip::examples::InitCommissionableDataProvider(gCommissionableDataProvider,
                                                                   LinuxDeviceOptions::GetInstance()) == CHIP_NO_ERROR);
        SetCommissionableDataProvider(&gCommissionableDataProvider);

        // ChipLinuxAppMainLoop blocks, and we don't want that here.
        static chip::CommonCaseDeviceServerInitParams initParams;
        (void) initParams.InitializeStaticResourcesBeforeServerInit();
        VerifyOrDie(Server::GetInstance().Init(initParams) == CHIP_NO_ERROR);

        ApplicationInit();

        // We don't start the event loop task, because we don't plan to deliver
        // data on a separate thread.

        matterStackInitialized = true;

        // The fuzzer does not have a way to tell us when it's done, so just
        // shut down things on exit.
        atexit(CleanShutdown);
    }

    // For now, just dump the data as a UDP payload into the session manager.
    // But maybe we should try to separately extract a PeerAddress and data from
    // the incoming data?
    Transport::PeerAddress peerAddr;
    System::PacketBufferHandle buf =
        System::PacketBufferHandle::NewWithData(aData, aSize, /* aAdditionalSize = */ 0, /* aReservedSize = */ 0);
    if (buf.IsNull())
    {
        // Too big; we couldn't represent this as a packetbuffer to start with.
        return 0;
    }

    // Ignoring the return value from OnMessageReceived, because we might be
    // passing it all sorts of garbage that will cause it to fail.
    Server::GetInstance().GetSecureSessionManager().OnMessageReceived(peerAddr, std::move(buf));

    // Now process pending events until our sentinel is reached.
    PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().StopEventLoopTask(); });
    PlatformMgr().RunEventLoop();
    return 0;
}
