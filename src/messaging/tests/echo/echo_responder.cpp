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

#include <lib/core/CHIPCore.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

namespace {

// The EchoServer object.
chip::Protocols::Echo::EchoServer gEchoServer;
chip::TransportMgr<chip::Transport::UDP> gUDPManager;
chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
chip::SessionHolder gSession;

// Callback handler when a CHIP EchoRequest is received.
void HandleEchoRequestReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle && payload)
{
    printf("Echo Request, len=%u ... sending response.\n", payload->DataLength());
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Transport::PeerAddress peer(chip::Transport::Type::kUndefined);
    bool useTCP      = false;
    bool disableEcho = false;

    const chip::FabricIndex gFabricIndex = 0;

    if (argc > 2)
    {
        printf("Too many arguments specified!\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if ((argc == 2) && (strcmp(argv[1], "--tcp") == 0))
    {
        useTCP = true;
    }

    if ((argc == 2) && (strcmp(argv[1], "--disable") == 0))
    {
        disableEcho = true;
    }

    InitializeChip();

    if (useTCP)
    {
        err = gTCPManager.Init(chip::Transport::TcpListenParameters(chip::DeviceLayer::TCPEndPointManager())
#if INET_CONFIG_ENABLE_IPV4
                                   .SetAddressType(chip::Inet::IPAddressType::kIPv4)
#else
                                   .SetAddressType(chip::Inet::IPAddressType::kIPv6)
#endif
        );
        SuccessOrExit(err);

        err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gTCPManager, &gMessageCounterManager, &gStorage,
                                   &gFabricTable);
        SuccessOrExit(err);
    }
    else
    {
        err = gUDPManager.Init(chip::Transport::UdpListenParameters(chip::DeviceLayer::UDPEndPointManager())
                                   .SetAddressType(chip::Inet::IPAddressType::kIPv6));
        SuccessOrExit(err);

        err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gUDPManager, &gMessageCounterManager, &gStorage,
                                   &gFabricTable);
        SuccessOrExit(err);
    }

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    if (!disableEcho)
    {
        err = gEchoServer.Init(&gExchangeManager);
        SuccessOrExit(err);
    }

    err = gSessionManager.InjectPaseSessionWithTestKey(gSession, 1, chip::kTestControllerNodeId, 1, gFabricIndex, peer,
                                                       chip::CryptoContext::SessionRole::kResponder);
    SuccessOrExit(err);

    if (!disableEcho)
    {
        // Arrange to get a callback whenever an Echo Request is received.
        gEchoServer.SetEchoRequestReceived(HandleEchoRequestReceived);
    }

    printf("Listening for Echo requests...\n");

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("EchoServer failed, err:%s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    if (!disableEcho)
    {
        gEchoServer.Shutdown();
    }

    gUDPManager.Close();

    ShutdownChip();

    return EXIT_SUCCESS;
}
