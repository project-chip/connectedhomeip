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

#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/TCP.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/UDP.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define ECHO_CLIENT_PORT (CHIP_PORT + 1)

namespace {

// Max value for the number of EchoRequests sent.
constexpr size_t kMaxEchoCount = 3;

// The CHIP Echo interval time.
constexpr chip::System::Clock::Timeout gEchoInterval = chip::System::Clock::Seconds16(1);

constexpr chip::FabricIndex gFabricIndex = 0;

// The EchoClient object.
chip::Protocols::Echo::EchoClient gEchoClient;

chip::TransportMgr<chip::Transport::UDP> gUDPManager;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
// Max value for the number of tcp connect attempts.
constexpr size_t kMaxTCPConnectAttempts = 3;

chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;

chip::Transport::AppTCPConnectionCallbackCtxt gAppTCPConnCbCtxt;
chip::Transport::ActiveTCPConnectionState * gActiveTCPConnState = nullptr;

static void HandleConnectionAttemptComplete(chip::Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr);
static void HandleConnectionClosed(chip::Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

// True, if client is still connecting to the server, false otherwise.
static bool gClientConInProgress = false;

// True, once client connection to server is established.
static bool gClientConEstablished = false;

bool gUseTCP                  = false;
uint64_t gTCPConnAttemptCount = 0;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

chip::Inet::IPAddress gDestAddr;
chip::SessionHolder gSession;

// The last time a CHIP Echo was attempted to be sent.
chip::System::Clock::Timestamp gLastEchoTime = chip::System::Clock::kZero;

// The handle to the TCP connection to the peer.
// static chip::Transport::ActiveTCPConnectionState * gCon = nullptr;

// Count of the number of EchoRequests sent.
uint64_t gEchoCount = 0;

// Count of the number of EchoResponses received.
uint64_t gEchoRespCount = 0;

CHIP_ERROR SendEchoRequest();
void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState);

void Shutdown()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(EchoTimerHandler, nullptr);
    gEchoClient.Shutdown();
    ShutdownChip();
}

void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState)
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
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    static const char kRequestFormat[] = "Echo Message %" PRIu64 "\n";
    char requestData[(sizeof kRequestFormat) + 20 /* uint64_t decimal digits */];
    snprintf(requestData, sizeof requestData, kRequestFormat, gEchoCount);
    chip::System::PacketBufferHandle && payloadBuf = chip::MessagePacketBuffer::NewWithData(requestData, strlen(requestData));

    if (payloadBuf.IsNull())
    {
        printf("Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    gLastEchoTime = chip::System::SystemClock().GetMonotonicTimestamp();

    err = chip::DeviceLayer::SystemLayer().StartTimer(gEchoInterval, EchoTimerHandler, nullptr);
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
        chip::DeviceLayer::SystemLayer().CancelTimer(EchoTimerHandler, nullptr);
    }

    return err;
}

CHIP_ERROR EstablishSecureSession()
{
    char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
    chip::Transport::PeerAddress peerAddr;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gUseTCP)
    {
        peerAddr = chip::Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT);
    }
    else
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    {
        peerAddr = chip::Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, chip::Inet::InterfaceId::Null());
    }

    peerAddr.ToString(peerAddrBuf);

    // Establish secure session to the peer.
    CHIP_ERROR err = gSessionManager.InjectPaseSessionWithTestKey(gSession, 1, chip::kTestDeviceNodeId, 1, gFabricIndex, peerAddr,
                                                                  chip::CryptoContext::SessionRole::kInitiator);
    if (err != CHIP_NO_ERROR)
    {
        printf("Establish secure session failed, err: %s\n", chip::ErrorStr(err));
        gLastEchoTime = chip::System::SystemClock().GetMonotonicTimestamp();
    }
    else
    {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        if (gUseTCP)
        {
            printf("Associating secure session with connection %p\n", gActiveTCPConnState);
            gSession.Get().Value()->AsSecureSession()->SetTCPConnection(gActiveTCPConnState);
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

        printf("Successfully established secure session with peer at %s\n", peerAddrBuf);
    }

    return err;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
void CloseConnection()
{
    char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
    chip::Transport::PeerAddress peerAddr = chip::Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT);

    gSessionManager.TCPDisconnect(peerAddr);

    peerAddr.ToString(peerAddrBuf);
    printf("Connection closed to peer at %s\n", peerAddrBuf);

    gClientConEstablished = false;
    gClientConInProgress  = false;
}

void HandleConnectionAttemptComplete(chip::Transport::ActiveTCPConnectionState * conn, CHIP_ERROR err)
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

    if (err != CHIP_NO_ERROR || conn != gActiveTCPConnState)
    {
        printf("Connection FAILED with err: %s\n", chip::ErrorStr(err));

        gLastEchoTime = chip::System::SystemClock().GetMonotonicTimestamp();
        CloseConnection();
        gTCPConnAttemptCount++;
        return;
    }

    err = EstablishSecureSession();
    if (err != CHIP_NO_ERROR)
    {
        printf("Secure session FAILED with err: %s\n", chip::ErrorStr(err));

        gLastEchoTime = chip::System::SystemClock().GetMonotonicTimestamp();
        CloseConnection();
        return;
    }

    gClientConEstablished = true;
    gClientConInProgress  = false;
}

void HandleConnectionClosed(chip::Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
{
    CloseConnection();
}

void EstablishTCPConnection()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Previous connection attempt underway.
    if (gClientConInProgress)
    {
        return;
    }

    gClientConEstablished = false;

    chip::Transport::PeerAddress peerAddr = chip::Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT);

    // Connect to the peer
    err = gSessionManager.TCPConnect(peerAddr, &gAppTCPConnCbCtxt, &gActiveTCPConnState);
    if (err != CHIP_NO_ERROR)
    {
        printf("Connection FAILED with err: %s\n", chip::ErrorStr(err));

        gLastEchoTime = chip::System::SystemClock().GetMonotonicTimestamp();
        CloseConnection();
        gTCPConnAttemptCount++;
        return;
    }

    gClientConInProgress = true;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

void HandleEchoResponseReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle && payload)
{
    chip::System::Clock::Timestamp respTime  = chip::System::SystemClock().GetMonotonicTimestamp();
    chip::System::Clock::Timeout transitTime = respTime - gLastEchoTime;

    gEchoRespCount++;

    printf("Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%" PRIu32 "time=%.3fs\n", gEchoRespCount, gEchoCount,
           static_cast<double>(gEchoRespCount) * 100 / static_cast<double>(gEchoCount),
           static_cast<uint32_t>(payload->DataLength()),
           static_cast<double>(chip::System::Clock::Milliseconds32(transitTime).count()) / 1000);
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if ((argc == 3) && (strcmp(argv[2], "--tcp") == 0))
    {
        gUseTCP = true;
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    if (!chip::Inet::IPAddress::FromString(argv[1], gDestAddr))
    {
        printf("Invalid Echo Server IP address: %s\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (gDestAddr.Type() != chip::Inet::IPAddressType::kIPv6)
    {
        printf("Echo Server IP address: %s is not of type IPv6\n", argv[1]);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    InitializeChip();

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gUseTCP)
    {
        err = gTCPManager.Init(chip::Transport::TcpListenParameters(chip::DeviceLayer::TCPEndPointManager())
                                   .SetAddressType(chip::Inet::IPAddressType::kIPv6)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gTCPManager, &gMessageCounterManager, &gStorage,
                                   &gFabricTable, gSessionKeystore);
        SuccessOrExit(err);

        gAppTCPConnCbCtxt.appContext     = nullptr;
        gAppTCPConnCbCtxt.connCompleteCb = HandleConnectionAttemptComplete;
        gAppTCPConnCbCtxt.connClosedCb   = HandleConnectionClosed;
    }
    else
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    {
        err = gUDPManager.Init(chip::Transport::UdpListenParameters(chip::DeviceLayer::UDPEndPointManager())
                                   .SetAddressType(chip::Inet::IPAddressType::kIPv6)
                                   .SetListenPort(ECHO_CLIENT_PORT));
        SuccessOrExit(err);

        err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gUDPManager, &gMessageCounterManager, &gStorage,
                                   &gFabricTable, gSessionKeystore);
        SuccessOrExit(err);
    }

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gUseTCP)
    {

        while (!gClientConEstablished)
        {
            // For TCP transport, attempt to establish the connection to the CHIP echo responder.
            // On Connection completion, call EstablishSecureSession();
            EstablishTCPConnection();

            chip::DeviceLayer::PlatformMgr().RunEventLoop();

            if (gTCPConnAttemptCount > kMaxTCPConnectAttempts)
            {
                ExitNow();
            }
        }
    }
    else
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    {
        // Start the CHIP session to the CHIP echo responder.
        err = EstablishSecureSession();
        SuccessOrExit(err);
    }

    err = gEchoClient.Init(&gExchangeManager, gSession.Get().Value());
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::kZero, EchoTimerHandler, nullptr);
    SuccessOrExit(err);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    gUDPManager.Close();

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gUseTCP)
    {
        gTCPManager.TCPDisconnect(chip::Transport::PeerAddress::TCP(gDestAddr));
    }
    gTCPManager.Close();
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    Shutdown();

exit:
    if ((err != CHIP_NO_ERROR) || (gEchoRespCount != kMaxEchoCount)
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        || (gTCPConnAttemptCount > kMaxTCPConnectAttempts)
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    )
    {
        printf("ChipEchoClient failed: %s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
