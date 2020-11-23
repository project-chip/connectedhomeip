/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a chip-echo-responder, for the
 *      CHIP Echo Protocol.
 *
 *      The CHIP Echo Protocol implements two simple methods, in the
 *      style of ICMP ECHO REQUEST and ECHO REPLY, in which a sent
 *      payload is turned around by the responder and echoed back to
 *      the originator.
 *
 */

#include "common.h"

#include <protocols/echo/Echo.h>
#include <support/ErrorStr.h>

namespace {

// The EchoServer object.
Protocols::EchoServer gEchoServer;
TransportMgr<Transport::UDP> gTransportManager;
SecureSessionMgr gSessionManager;
SecurePairingUsingTestSecret gTestPairing;

// Callback handler when a CHIP EchoRequest is received.
void HandleEchoRequestReceived(NodeId nodeId, System::PacketBuffer * payload)
{
    printf("Echo Request from node %lu, len=%u ... sending response.\n", nodeId, payload->DataLength());
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    InitializeChip();

    err = gTransportManager.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(Inet::kIPAddressType_IPv4));
    SuccessOrExit(err);

    err = gSessionManager.Init(kServerDeviceId, &DeviceLayer::SystemLayer, &gTransportManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gEchoServer.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = gSessionManager.NewPairing(peer, kClientDeviceId, &gTestPairing);
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Request is received.
    gEchoServer.SetEchoRequestReceived(HandleEchoRequestReceived);

    printf("Listening for Echo requests...\n");

    DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("EchoServer failed, err:%s\n", ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    gEchoServer.Shutdown();

    ShutdownChip();

    return EXIT_SUCCESS;
}
