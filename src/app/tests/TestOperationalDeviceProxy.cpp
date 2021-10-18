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
#include <app/device/OperationalDeviceProxy.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <system/SystemLayerImpl.h>

using namespace chip;

namespace {

constexpr size_t kMaxBlePendingPackets = 1;

// using DeviceTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
// #if INET_CONFIG_ENABLE_IPV4
//                                         ,
//                                         Transport::UDP /* IPv4 */
// #endif
// #if CONFIG_NETWORK_LAYER_BLE
//                                         ,
//                                         Transport::BLE<kMaxBlePendingPackets> /* BLE */
// #endif
//                                         >;

// using DeviceIPTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
// #if INET_CONFIG_ENABLE_IPV4
//                                           ,
//                                           Transport::UDP /* IPv4 */
// #endif
//                                           >;

void TestOperationalDeviceProxy_Connect(nlTestSuite * inSuite, void * inContext)
{
    //     Platform::MemoryInit();

    //     // Setup params for operational device proxy
    //     DeviceTransportMgr transportMgr;
    //     SessionManager sessionManager;
    //     Messaging::ExchangeManager exchangeMgr;
    //     System::LayerImpl systemLayer;
    //     Inet::InetLayer inetLayer;
    // #if CONFIG_NETWORK_LAYER_BLE
    //     Ble::BleLayer blelayer;
    // #endif // CONFIG_NETWORK_LAYER_BLE
    //     // Heap-allocate the fairly large FabricTable so we don't end up with a huge
    //     // stack.
    //     FabricTable * fabricTable = Platform::New<FabricTable>();
    //     secure_channel::MessageCounterManager messageCounterManager;
    //     SessionIDAllocator idAllocator;

    //     systemLayer.Init();
    //     inetLayer.Init(systemLayer, nullptr);
    //     transportMgr.Init(
    //         Transport::UdpListenParameters(&inetLayer).SetAddressType(Inet::IPAddressType::kIPAddressType_IPv6).SetListenPort(CHIP_PORT)
    // #if INET_CONFIG_ENABLE_IPV4
    //             ,
    //         Transport::UdpListenParameters(&inetLayer).SetAddressType(Inet::kIPAddressType_IPv4).SetListenPort(CHIP_PORT)
    // #endif
    // #if CONFIG_NETWORK_LAYER_BLE
    //             ,
    //         Transport::BleListenParameters(&blelayer)
    // #endif
    //     );
    //     sessionManager.Init(&systemLayer, &transportMgr, &messageCounterManager);
    //     exchangeMgr.Init(&sessionManager);
    //     messageCounterManager.Init(&exchangeMgr);

    //     // Setup instance of operational device proxy
    //     app::device::OperationalDeviceProxy operationalDevice;
    //     NodeId peerNodeId           = 1;
    //     FabricIndex peerFabricIndex = 1;
    //     Inet::IPAddress mockAddr;

    //     // Update the address of peer device
    //     Inet::IPAddress::FromString("::1", mockAddr);
    //     Transport::PeerAddress peerAddr = Transport::PeerAddress::UDP(mockAddr, CHIP_PORT);
    //     operationalDevice.UpdateAddress(peerAddr);

    //     // Setup device params
    //     app::device::OperationalDeviceProxyInitParams initParams = {
    //         .sessionManager = &sessionManager,
    //         .exchangeMgr    = &exchangeMgr,
    //         .idAllocator    = &idAllocator,
    //         .fabricsTable   = fabricTable,
    //     };

    //     operationalDevice.Init(peerNodeId, peerFabricIndex, initParams);
    //     NL_TEST_ASSERT(inSuite, operationalDevice.Connect(nullptr, nullptr) == CHIP_NO_ERROR);

    //     operationalDevice.GetDevice().Reset();
    //     messageCounterManager.Shutdown();
    //     exchangeMgr.Shutdown();
    //     sessionManager.Shutdown();
    //     Platform::Delete(fabricTable);
    //     transportMgr.Close();
    //     inetLayer.Shutdown();
    //     systemLayer.Shutdown();
    //     Platform::MemoryShutdown();
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestOperationalDeviceProxy_Connect", TestOperationalDeviceProxy_Connect),
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
