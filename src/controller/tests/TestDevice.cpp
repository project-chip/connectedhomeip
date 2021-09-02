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

#include <ble/BleLayer.h>
#include <controller/CHIPDevice.h>
#include <inet/IPAddress.h>
#include <inet/InetLayer.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <transport/SecureSessionMgr.h>
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
    DeviceTransportMgr transportMgr;
    SecureSessionMgr sessionMgr;
    ExchangeManager exchangeMgr;
    Inet::InetLayer inetLayer;
    System::Layer systemLayer;
    Ble::BleLayer blelayer;
    FabricTable fabrics;
    secure_channel::MessageCounterManager messageCounterManager;
    SessionIDAllocator idAllocator;

    systemLayer.Init();
    inetLayer.Init(systemLayer, nullptr);
    transportMgr.Init(
        UdpListenParameters(&inetLayer).SetAddressType(Inet::IPAddressType::kIPAddressType_IPv4).SetListenPort(CHIP_PORT)
#if INET_CONFIG_ENABLE_IPV4
            ,
        UdpListenParameters(&inetLayer).SetAddressType(Inet::kIPAddressType_IPv4).SetListenPort(CHIP_PORT)
#endif
#if CONFIG_NETWORK_LAYER_BLE
            ,
        BleListenParameters(&blelayer)
#endif
    );
    sessionMgr.Init(&systemLayer, &transportMgr, &fabrics, &messageCounterManager);
    exchangeMgr.Init(&sessionMgr);
    messageCounterManager.Init(&exchangeMgr);

    ControllerDeviceInitParams params = {
        .transportMgr    = &transportMgr,
        .sessionMgr      = &sessionMgr,
        .exchangeMgr     = &exchangeMgr,
        .inetLayer       = &inetLayer,
        .storageDelegate = nullptr,
        .idAllocator     = &idAllocator,
        .fabricsTable    = &fabrics,
    };
    Device device;
    NodeId mockNodeId           = 1;
    FabricIndex mockFabricIndex = 1;
    Inet::IPAddress mockAddr;
    Inet::IPAddress::FromString("127.0.0.1", mockAddr);
    PeerAddress addr = PeerAddress::UDP(mockAddr, CHIP_PORT);
    device.Init(params, CHIP_PORT, mockNodeId, addr, mockFabricIndex);

    device.OperationalCertProvisioned();
    NL_TEST_ASSERT(inSuite, device.EstablishConnectivity(nullptr, nullptr) == CHIP_NO_ERROR);
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
