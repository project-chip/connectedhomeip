/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements a chip-echo-requester, for the
 *      CHIP Echo Protocol.
 *
 *      The CHIP Echo Protocol implements two simple methods, in the
 *      style of ICMP ECHO REQUEST and ECHO REPLY, in which a sent
 *      payload is turned around by the responder and echoed back to
 *      the originator.
 *
 */

#include "common.h"

#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define ECHO_CLIENT_PORT (CHIP_PORT + 1)

namespace {

// Max value for the number of EchoRequests sent.
constexpr size_t kMaxEchoCount = 3;

// The CHIP Echo interval time in milliseconds.
constexpr int32_t gEchoInterval = 1000;

constexpr chip::Transport::AdminId gAdminId = 0;

// The EchoClient object.
chip::Protocols::Echo::EchoClient gEchoClient;

chip::TransportMgr<chip::Transport::UDP> gUDPManager;
chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
chip::Inet::IPAddress gDestAddr;

// The last time a CHIP Echo was attempted to be sent.
uint64_t gLastEchoTime = 0;

// Count of the number of EchoRequests sent.
uint64_t gEchoCount = 0;

// Count of the number of EchoResponses received.
uint64_t gEchoRespCount = 0;

bool gUseTCP = false;

CHIP_ERROR SendEchoRequest();
void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState, CHIP_ERROR error);

void Shutdown()
{
    chip::DeviceLayer::SystemLayer.CancelTimer(EchoTimerHandler, NULL);
    gEchoClient.Shutdown();
    ShutdownChip();
}

void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    if (gEchoRespCount != gEchoCount)
    {
        printf("No response received\n");

        // Set gEchoRespCount to gEchoCount to start next ping if there is any.
        gEchoRespCount = gEchoCount;
    }

    if (gEchoCount < kMaxEchoCount)
    {
        CHIP_ERROR err = SendEchoRequest();
        if (err != CHIP_NO_ERROR)
        {
            printf("Send request failed: %s\n", chip::ErrorStr(err));
            chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        }
    }
    else
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
}

CHIP_ERROR SendEchoRequest()
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    const char kRequestFormat[] = "Echo Message %" PRIu64 "\n";
    char requestData[(sizeof kRequestFormat) + 20 /* uint64_t decimal digits */];
    snprintf(requestData, sizeof requestData, kRequestFormat, gEchoCount);
    chip::System::PacketBufferHandle && payloadBuf = chip::MessagePacketBuffer::NewWithData(requestData, strlen(requestData));

    if (payloadBuf.IsNull())
    {
        printf("Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    gLastEchoTime = chip::System::Clock::GetMonotonicMilliseconds();

    err = chip::DeviceLayer::SystemLayer.StartTimer(gEchoInterval, EchoTimerHandler, NULL);
    if (err != CHIP_NO_ERROR)
    {
        printf("Unable to schedule timer\n");
        return CHIP_ERROR_INTERNAL;
    }

    printf("\nSend echo request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = gEchoClient.SendEchoRequest(std::move(payloadBuf));

    if (err == CHIP_NO_ERROR)
    {
        gEchoCount++;
    }
    else
    {
        printf("Send echo request failed, err: %s\n", chip::ErrorStr(err));
        chip::DeviceLayer::SystemLayer.CancelTimer(EchoTimerHandler, NULL);
    }

    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Optional<chip::Transport::PeerAddress> peerAddr;
    chip::SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<chip::SecurePairingUsingTestSecret>();
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (gUseTCP)
    {
        peerAddr = chip::Optional<chip::Transport::PeerAddress>::Value(chip::Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT));
    }
    else
    {
        peerAddr = chip::Optional<chip::Transport::PeerAddress>::Value(
            chip::Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID));
    }

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(peerAddr, chip::kTestDeviceNodeId, testSecurePairingSecret,
                                     chip::SecureSession::SessionRole::kInitiator, gAdminId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastEchoTime = chip::System::Clock::GetMonotonicMilliseconds();
    }
    else
    {
        printf("Establish secure session succeeded\n");
    }

    return err;
}

void HandleEchoResponseReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle && payload)
{
    uint32_t respTime    = chip::System::Clock::GetMonotonicMilliseconds();
    uint32_t transitTime = respTime - gLastEchoTime;

    gEchoRespCount++;

    printf("Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", gEchoRespCount, gEchoCount,
           static_cast<double>(gEchoRespCount) * 100 / gEchoCount, payload->DataLength(), static_cast<double>(transitTime) / 1000);
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = nullptr;

    if (argc <= 1)
    {
        printf("Missing Echo Server IP address\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (argc > 3)
    {
        printf("Too many arguments specified!\n");
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if ((argc == 3) && (strcmp(argv[2], "--tcp") == 0))
    {
        gUseTCP = true;
    }

    if (!chip::Inet::IPAddress::FromString(argv[1], gDestAddr))
    {
        printf("Invalid Echo Server IP address: %s\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    InitializeChip();

    adminInfo = admins.AssignAdminId(gAdminId, chip::kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (gUseTCP)
    {
        err = gTCPManager.Init(chip::Transport::TcpListenParameters(&chip::DeviceLayer::InetLayer)
                                   .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gTCPManager, &admins,
                                   &gMessageCounterManager);
        SuccessOrExit(err);
    }
    else
    {
        err = gUDPManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
                                   .SetAddressType(chip::Inet::kIPAddressType_IPv4)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(chip::kTestControllerNodeId, &chip::DeviceLayer::SystemLayer, &gUDPManager, &admins,
                                   &gMessageCounterManager);
        SuccessOrExit(err);
    }

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession();
    SuccessOrExit(err);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in PR:4451
    err = gEchoClient.Init(&gExchangeManager, { chip::kTestDeviceNodeId, 0, gAdminId });
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    err = chip::DeviceLayer::SystemLayer.StartTimer(0, EchoTimerHandler, NULL);
    SuccessOrExit(err);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    Shutdown();

exit:
    if ((err != CHIP_NO_ERROR) || (gEchoRespCount != kMaxEchoCount))
    {
        printf("ChipEchoClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
