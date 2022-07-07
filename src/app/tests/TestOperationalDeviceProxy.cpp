/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <Credentials/GroupDataProviderImpl.h>
#include <app/OperationalDeviceProxy.h>
#include <inet/IPAddress.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <system/SystemLayerImpl.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/UDP.h>

using namespace chip;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Credentials;

#if INET_CONFIG_ENABLE_IPV4
namespace {

using TestTransportMgr = TransportMgr<Transport::UDP>;

void TestOperationalDeviceProxy_EstablishSessionDirectly(nlTestSuite * inSuite, void * inContext)
{
    // TODO: This test appears not to be workable since it does not init the fabric table!!!
    Platform::MemoryInit();
    TestTransportMgr transportMgr;
    SessionManager sessionManager;
    SimpleSessionResumptionStorage sessionResumptionStorage;
    ExchangeManager exchangeMgr;
    Inet::UDPEndPointManagerImpl udpEndPointManager;
    System::LayerImpl systemLayer;
    // Heap-allocate the fairly large FabricTable so we don't end up with a huge
    // stack.
    FabricTable * fabrics     = Platform::New<FabricTable>();
    const FabricInfo * fabric = fabrics->FindFabricWithIndex(1);
    VerifyOrDie(fabric != nullptr);
    secure_channel::MessageCounterManager messageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    GroupDataProviderImpl groupDataProvider;

    systemLayer.Init();
    udpEndPointManager.Init(systemLayer);
    transportMgr.Init(UdpListenParameters(udpEndPointManager).SetAddressType(Inet::IPAddressType::kIPv4).SetListenPort(CHIP_PORT));
    sessionManager.Init(&systemLayer, &transportMgr, &messageCounterManager, &deviceStorage);
    sessionResumptionStorage.Init(&deviceStorage);
    exchangeMgr.Init(&sessionManager);
    messageCounterManager.Init(&exchangeMgr);
    groupDataProvider.SetPersistentStorage(&deviceStorage);
    VerifyOrDie(groupDataProvider.Init() == CHIP_NO_ERROR);
    // TODO: Set IPK in groupDataProvider

    DeviceProxyInitParams params = {
        .sessionManager           = &sessionManager,
        .sessionResumptionStorage = &sessionResumptionStorage,
        .exchangeMgr              = &exchangeMgr,
        .fabricInfo               = fabric,
        .groupDataProvider        = &groupDataProvider,
    };
    NodeId mockNodeId = 1;
    OperationalDeviceProxy device(params, PeerId().SetNodeId(mockNodeId));
    Inet::IPAddress mockAddr;
    Inet::IPAddress::FromString("127.0.0.1", mockAddr);
    PeerAddress addr = PeerAddress::UDP(mockAddr, CHIP_PORT);
    device.UpdateAddress(addr);

    NL_TEST_ASSERT(inSuite, device.Connect(nullptr, nullptr) == CHIP_NO_ERROR);

    device.Clear();
    messageCounterManager.Shutdown();
    exchangeMgr.Shutdown();
    sessionManager.Shutdown();
    Platform::Delete(fabrics);
    transportMgr.Close();
    udpEndPointManager.Shutdown();
    systemLayer.Shutdown();
    Platform::MemoryShutdown();
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestOperationalDeviceProxy_EstablishSessionDirectly", TestOperationalDeviceProxy_EstablishSessionDirectly),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestOperationalDeviceProxy()
{
    nlTestSuite theSuite = { "OperationalDeviceProxy", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOperationalDeviceProxy)

#endif // INET_CONFIG_ENABLE_IPV4
