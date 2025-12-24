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

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include "AppMain.h"
#include <app/server/Server.h>
#include <data-model-providers/codegen/Instance.h>

#include <CommissionableInit.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

using namespace fuzztest;
using namespace std;

namespace {

LinuxCommissionableDataProvider gCommissionableDataProvider;

}

// TODO #35369: move this to a test fixture once Test Fixture issues within FuzzTest are resolved.
void CleanShutdown()
{
    Server::GetInstance().Shutdown();
    PlatformMgr().Shutdown();
    // TODO: We don't Platform::MemoryShutdown because ~CASESessionManager calls
    // Dnssd::ResolverProxy::Shutdown, which starts doing Platform::Delete.
    // Platform::MemoryShutdown();
}

void AllClustersFuzzer(const std::vector<std::uint8_t> & Bytes)
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
        initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

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
    // Transport::PeerAddress peerAddr(Transport::Type::kTcp);
    // Transport::PeerAddress peerAddr(Transport::Type::kWiFiPAF);

    System::PacketBufferHandle buf =
        System::PacketBufferHandle::NewWithData(Bytes.data(), Bytes.size(), /* aAdditionalSize = */ 0, /* aReservedSize = */ 0);

    // Too big; we couldn't represent this as a packetbuffer to start with.
    ASSERT_FALSE(buf.IsNull());

    // Ignoring the return value from OnMessageReceived, because we might be
    // passing it all sorts of garbage that will cause it to fail.
    Server::GetInstance().GetSecureSessionManager().OnMessageReceived(peerAddr, std::move(buf));

    // Transport::MessageTransportContext msgContext;
    //  Server::GetInstance().GetSecureSessionManager().OnMessageReceived(peerAddr, std::move(buf), &msgContext);

    // Now process pending events until our sentinel is reached.
    PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().StopEventLoopTask(); });
    PlatformMgr().RunEventLoop();
}

uint8_t unauthenticated_example[] = { 0x4,  0x0,  0x0,  0x0,  0x4d, 0xb5, 0xfa, 0x2,  0xd1, 0x30, 0xe,  0xef, 0x63, 0x25,
                                      0x90, 0x9f, 0x5,  0x20, 0x3f, 0xb0, 0x0,  0x0,  0x15, 0x30, 0x1,  0x20, 0xf2, 0xdf,
                                      0xd4, 0xc9, 0x2d, 0x6c, 0xde, 0x4e, 0x2c, 0x2c, 0x90, 0xc0, 0xae, 0xe6, 0x76, 0x80,
                                      0xbe, 0x95, 0xba, 0x87, 0x4f, 0x59, 0x60, 0xb1, 0x9b, 0xd9, 0xaf, 0x87, 0x84, 0xf3,
                                      0xe5, 0x1d, 0x25, 0x2,  0x99, 0x85, 0x24, 0x3,  0x0,  0x28, 0x4,  0x35, 0x5,  0x25,
                                      0x1,  0xf4, 0x1,  0x25, 0x2,  0x2c, 0x1,  0x25, 0x3,  0xa0, 0xf,  0x24, 0x4,  0x13,
                                      0x24, 0x5,  0xc,  0x26, 0x6,  0x0,  0x2,  0x4,  0x1,  0x24, 0x7,  0x1,  0x18, 0x18 };

auto unauthenticatedSeed()
{
    vector<uint8_t> data(&unauthenticated_example[0], &unauthenticated_example[0] + sizeof(unauthenticated_example));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

uint8_t unauthenticated_example_SHUTDOWN[] = {
    0x4,  0x0,  0x0,  0x0, 0x0,  0x7f, 0x5f, 0xd, 0xa8, 0x62, 0x41, 0xdc, 0x30,
    0x3b, 0x13, 0xba, 0x3, 0x10, 0xf8, 0x32, 0x0, 0x0,  0x29, 0x1c, 0x8,  0xc,
};

auto unauthenticatedSeedSHUTDOWN()
{
    vector<uint8_t> data(&unauthenticated_example_SHUTDOWN[0],
                         &unauthenticated_example_SHUTDOWN[0] + sizeof(unauthenticated_example_SHUTDOWN));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

FUZZ_TEST(AllClustersApp, AllClustersFuzzer).WithDomains(unauthenticatedSeed());
