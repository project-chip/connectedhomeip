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

#include <stdio.h>
#include <stdlib.h>

#include <core/CHIPCore.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <shell/shell.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PASESession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#include <ChipShellCollection.h>

using namespace chip;
using namespace Shell;
using namespace Logging;

#define ECHO_CLIENT_PORT (CHIP_PORT + 1)

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr size_t kNetworkSleepTimeMsecs       = (100 * 1000);

namespace {

constexpr Transport::AdminId gAdminId = 0;

// The echo client object.
Protocols::Echo::EchoClient gEchoClient;

TransportMgr<Transport::UDP> gUDPManager;
TransportMgr<Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
Messaging::ExchangeManager gExchangeManager;
SecureSessionMgr gSessionManager;
Inet::IPAddress gDestAddr;

// Max value for the number of echo requests sent.
uint32_t gMaxEchoCount = 3;

// The CHIP Echo interval time in milliseconds.
uint32_t gEchoInterval = 1000;

// The last time a echo request was attempted to be sent.
uint64_t gLastEchoTime = 0;

// Count of the number of echo requests sent.
uint64_t gEchoCount = 0;

// Count of the number of echo responses received.
uint64_t gEchoRespCount = 0;

// True, if the echo client is waiting for an echo response
// after sending an echo request, false otherwise.
bool gWaitingForEchoResp = false;

bool gUseTCP = false;

bool gUseCRMP = true;

void ResetGlobalState()
{
    gMaxEchoCount       = 3;
    gEchoInterval       = 1000;
    gLastEchoTime       = 0;
    gEchoCount          = 0;
    gEchoRespCount      = 0;
    gWaitingForEchoResp = false;
    gUseTCP             = false;
    gUseCRMP            = true;
}

bool EchoIntervalExpired(void)
{
    uint64_t now = System::Timer::GetCurrentEpoch();

    return (now >= gLastEchoTime + gEchoInterval);
}

CHIP_ERROR SendEchoRequest(streamer_t * stream)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    const char kRequestFormat[] = "Echo Message %" PRIu64 "\n";
    char requestData[(sizeof kRequestFormat) + 20 /* uint64_t decimal digits */];
    snprintf(requestData, sizeof requestData, kRequestFormat, gEchoCount);
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(requestData, strlen(requestData));

    if (gUseCRMP)
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNone);
    }
    else
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    }

    if (payloadBuf.IsNull())
    {
        streamer_printf(stream, "Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    gLastEchoTime = System::Timer::GetCurrentEpoch();

    streamer_printf(stream, "\nSend echo request message to Node: %" PRIu64 "\n", kTestDeviceNodeId);

    err = gEchoClient.SendEchoRequest(std::move(payloadBuf), sendFlags);

    if (err == CHIP_NO_ERROR)
    {
        gWaitingForEchoResp = true;
        gEchoCount++;
    }
    else
    {
        streamer_printf(stream, "Send echo request failed, err: %s\n", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR EstablishSecureSession(streamer_t * stream)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Optional<Transport::PeerAddress> peerAddr;
    SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<SecurePairingUsingTestSecret>();
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (gUseTCP)
    {
        peerAddr = Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::TCP(gDestAddr, CHIP_PORT));
    }
    else
    {
        peerAddr =
            Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(gDestAddr, CHIP_PORT, INET_NULL_INTERFACEID));
    }

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(peerAddr, kTestDeviceNodeId, testSecurePairingSecret,
                                     SecureSessionMgr::PairingDirection::kInitiator, gAdminId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Establish secure session failed, err: %s\n", ErrorStr(err));
        gLastEchoTime = System::Timer::GetCurrentEpoch();
    }
    else
    {
        streamer_printf(stream, "Establish secure session succeeded\n");
    }

    return err;
}

void HandleEchoResponseReceived(Messaging::ExchangeContext * ec, System::PacketBufferHandle payload)
{
    uint32_t respTime    = System::Timer::GetCurrentEpoch();
    uint32_t transitTime = respTime - gLastEchoTime;
    streamer_t * sout    = streamer_get();

    gWaitingForEchoResp = false;
    gEchoRespCount++;

    streamer_printf(sout, "Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", gEchoRespCount, gEchoCount,
                    static_cast<double>(gEchoRespCount) * 100 / gEchoCount, payload->DataLength(),
                    static_cast<double>(transitTime) / 1000);
}

void DriveIO(streamer_t * stream)
{
    struct timeval sleepTime;
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;
    int selectRes;

    sleepTime.tv_sec  = 0;
    sleepTime.tv_usec = kNetworkSleepTimeMsecs;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (chip::DeviceLayer::SystemLayer.State() == chip::System::kLayerState_Initialized)
        chip::DeviceLayer::SystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

    if (chip::DeviceLayer::InetLayer.State == chip::Inet::InetLayer::kState_Initialized)
        chip::DeviceLayer::InetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

    selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &sleepTime);
    if (selectRes < 0)
    {
        streamer_printf(stream, "Select failed: %s\n", chip::ErrorStr(chip::System::MapErrorPOSIX(errno)));
        return;
    }

    if (chip::DeviceLayer::SystemLayer.State() == chip::System::kLayerState_Initialized)
    {
        chip::DeviceLayer::SystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (chip::DeviceLayer::InetLayer.State == chip::Inet::InetLayer::kState_Initialized)
    {
        chip::DeviceLayer::InetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
}

void StartPinging(streamer_t * stream, char * destination)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::AdminPairingTable admins;
    Transport::AdminPairingInfo * adminInfo = nullptr;

    if (!Inet::IPAddress::FromString(destination, gDestAddr))
    {
        streamer_printf(stream, "Invalid Echo Server IP address: %s\n", destination);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    adminInfo = admins.AssignAdminId(gAdminId, kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (gUseTCP)
    {
        err = gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gTCPManager, &admins);
        SuccessOrExit(err);
    }
    else
    {
        err = gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gUDPManager, &admins);
        SuccessOrExit(err);
    }

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession(stream);
    SuccessOrExit(err);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in PR:4451
    err = gEchoClient.Init(&gExchangeManager, { kTestDeviceNodeId, 0, gAdminId });
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    // Connection has been established. Now send the EchoRequests.
    for (unsigned int i = 0; i < gMaxEchoCount; i++)
    {
        err = SendEchoRequest(stream);
        if (err != CHIP_NO_ERROR)
        {
            streamer_printf(stream, "Send request failed: %s\n", ErrorStr(err));
            break;
        }

        // Wait for response until the Echo interval.
        while (!EchoIntervalExpired())
        {
            DriveIO(stream);
        }

        // Check if expected response was received.
        if (gWaitingForEchoResp)
        {
            streamer_printf(stream, "No response received\n");
            gWaitingForEchoResp = false;
        }
    }

    gEchoClient.Shutdown();
    gExchangeManager.Shutdown();
    gSessionManager.Shutdown();

    ResetGlobalState();

exit:
    if ((err != CHIP_NO_ERROR))
    {
        streamer_printf(stream, "Ping failed with error: %s\n", ErrorStr(err));
    }
}

void PrintUsage(streamer_t * stream)
{
    streamer_printf(stream, "Usage: ping [options] <destination>\n\nOptions:\n");

    streamer_printf(stream,
                    "  -h              print help information\n"
                    "  -u              use UDP (default)\n"
                    "  -t              use TCP\n"
                    "  -i  <interval>  ping interval time in seconds\n"
                    "  -c  <count>     stop after <count> replies\n"
                    "  -r  <1|0>       enalbe/disable CRMP\n");
}

int cmd_ping(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    int ret           = 0;
    int optIndex      = 0;

    while (optIndex < argc && argv[optIndex][0] == '-')
    {
        switch (argv[optIndex][1])
        {
        case 'h':
            PrintUsage(sout);
            return 0;
        case 'u':
            gUseTCP = false;
            break;
        case 't':
            gUseTCP = true;
            break;
        case 'i':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -i\n");
                return -1;
            }
            else
            {
                gEchoInterval = atol(argv[optIndex]) * 1000;
            }
            break;
        case 'c':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -c\n");
                return -1;
            }
            else
            {
                gMaxEchoCount = atol(argv[optIndex]);
            }
            break;
        case 'r':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -r\n");
                return -1;
            }
            else
            {
                int arg = atoi(argv[optIndex]);

                if (arg == 0)
                {
                    gUseCRMP = false;
                }
                else if (arg == 1)
                {
                    gUseCRMP = true;
                }
                else
                {
                    ret = -1;
                }
            }
            break;
        default:
            ret = -1;
        }

        optIndex++;
    }

    if (optIndex >= argc)
    {
        streamer_printf(sout, "Missing IP address\n");
        ret = -1;
    }

    if (ret == 0)
    {
        streamer_printf(sout, "IP address: %s\n", argv[optIndex]);
        StartPinging(sout, argv[optIndex]);
    }

    return ret;
}

} // namespace

static shell_command_t cmds_ping[] = {
    { &cmd_ping, "ping", "Using Echo Protocol to measure packet loss across network paths" },
};

void cmd_ping_init()
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    err = gTCPManager.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(Inet::kIPAddressType_IPv4)
                               .SetListenPort(ECHO_CLIENT_PORT));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(sout, "Failed to init TCP manager error: %s\r\n", ErrorStr(err)));

    err = gUDPManager.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(Inet::kIPAddressType_IPv4)
                               .SetListenPort(ECHO_CLIENT_PORT));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(sout, "Failed to init UDP manager error: %s\r\n", ErrorStr(err)));

    shell_register(cmds_ping, ArraySize(cmds_ping));

exit:
    return;
}
