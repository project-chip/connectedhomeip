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

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_LAYER_BLE
#include <controller/CommissioneeDeviceProxy.h>
#include <inet/IPAddress.h>
#include <inetInetLayer.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <system/SystemLayerImpl.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/UDP.h>

using namespace chip;
using namespace chip::Transport;
using namespace chip::Controller;
using namespace chip::Messaging;

namespace {

using TestTransportMgr = TransportMgr<Transport::UDP>;

void TestDevice_EstablishSessionDirectly(nlTestSuite * inSuite, void * inContext)
{
    Platform::MemoryInit();
    chip::DeviceLayer::SetConfigurationMgr(&chip::DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance());
    DeviceTransportMgr transportMgr;
    SessionManager sessionManager;
    ExchangeManager exchangeMgr;
    Inet::UDPEndPointManagerImpl udpEndPointManager;
    System::LayerImpl systemLayer;
    chip::TestPersistentStorageDelegate deviceStorage;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer blelayer;
#endif // CONFIG_NETWORK_LAYER_BLE
    // Heap-allocate the fairly large FabricTable so we don't end up with a huge
    // stack.
    FabricTable * fabrics = Platform::New<FabricTable>();
    secure_channel::MessageCounterManager messageCounterManager;
    SessionIDAllocator idAllocator;

    systemLayer.Init();
    udpEndPointManager.Init(systemLayer);
    transportMgr.Init(UdpListenParameters(udpEndPointManager).SetAddressType(Inet::IPAddressType::kIPv6).SetListenPort(CHIP_PORT)
#if INET_CONFIG_ENABLE_IPV4
                          ,
                      UdpListenParameters(udpEndPointManager).SetAddressType(Inet::IPAddressType::kIPv4).SetListenPort(CHIP_PORT)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                          ,
                      BleListenParameters(&blelayer)
#endif
    );
    sessionManager.Init(&systemLayer, &transportMgr, &messageCounterManager, &deviceStorage;);
    exchangeMgr.Init(&sessionManager);
    messageCounterManager.Init(&exchangeMgr);

    ControllerDeviceInitParams params = {
        .transportMgr       = &transportMgr,
        .sessionManager     = &sessionManager,
        .exchangeMgr        = &exchangeMgr,
        .udpEndPointManager = &udpEndPointManager,
        .storageDelegate    = nullptr,
        .idAllocator        = &idAllocator,
        .fabricsTable       = fabrics,
    };
    CommissioneeDeviceProxy device;
    NodeId mockNodeId           = 1;
    FabricIndex mockFabricIndex = 1;
    Inet::IPAddress mockAddr;
    Inet::IPAddress::FromString("::1", mockAddr);
    PeerAddress addr = PeerAddress::UDP(mockAddr, CHIP_PORT);
    device.Init(params, mockNodeId, addr, mockFabricIndex);

    device.OperationalCertProvisioned();
    NL_TEST_ASSERT(inSuite, device.EstablishConnectivity(nullptr, nullptr) == CHIP_NO_ERROR);

    device.Reset();
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
    NL_TEST_DEF("TestDevice_EstablishSessionDirectly", TestDevice_EstablishSessionDirectly),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestDevice()
{
    nlTestSuite theSuite = { "Device", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDevice)
