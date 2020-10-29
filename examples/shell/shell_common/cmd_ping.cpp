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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/CHIPCore.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/mcsp/MessageCounterManager.h>
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

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr size_t kNetworkSleepTimeMsecs       = (100 * 1000);
constexpr size_t kDecimalDigitsForUint64      = 20;

namespace {

class PingArguments
{
public:
    void Reset()
    {
        mMaxEchoCount       = 3;
        mEchoInterval       = 1000;
        mLastEchoTime       = 0;
        mEchoCount          = 0;
        mEchoRespCount      = 0;
        mWaitingForEchoResp = false;
        mUsingTCP           = false;
        mUsingCRMP          = true;
        mEchoPort           = CHIP_PORT;
    }

    uint64_t GetLastEchoTime() const { return mLastEchoTime; }
    void SetLastEchoTime(uint64_t value) { mLastEchoTime = value; }

    uint64_t GetEchoCount() const { return mEchoCount; }
    void SetEchoCount(uint64_t value) { mEchoCount = value; }
    void IncrementEchoCount() { mEchoCount++; }

    uint64_t GetEchoRespCount() const { return mEchoRespCount; }
    void SetEchoRespCount(uint64_t value) { mEchoRespCount = value; }
    void IncrementEchoRespCount() { mEchoRespCount++; }

    uint32_t GetMaxEchoCount() const { return mMaxEchoCount; }
    void SetMaxEchoCount(uint32_t id) { mMaxEchoCount = id; }

    uint32_t GetEchoInterval() const { return mEchoInterval; }
    void SetEchoInterval(uint32_t value) { mEchoInterval = value; }

    uint16_t GetEchoPort() const { return mEchoPort; }
    void SetEchoPort(uint16_t value) { mEchoPort = value; }

    bool IsWaitingForEchoResp() const { return mWaitingForEchoResp; }
    void SetWaitingForEchoResp(bool value) { mWaitingForEchoResp = value; }

    bool IsUsingTCP() const { return mUsingTCP; }
    void SetUsingTCP(bool value) { mUsingTCP = value; }

    bool IsUsingCRMP() const { return mUsingCRMP; }
    void SetUsingCRMP(bool value) { mUsingCRMP = value; }

private:
    // The last time a echo request was attempted to be sent.
    uint64_t mLastEchoTime;

    // Count of the number of echo requests sent.
    uint64_t mEchoCount;

    // Count of the number of echo responses received.
    uint64_t mEchoRespCount;

    // Max value for the number of echo requests sent.
    uint32_t mMaxEchoCount;

    // The CHIP Echo interval time in milliseconds.
    uint32_t mEchoInterval;

    uint16_t mEchoPort;

    // True, if the echo client is waiting for an echo response
    // after sending an echo request, false otherwise.
    bool mWaitingForEchoResp;

    bool mUsingTCP;
    bool mUsingCRMP;
} gPingArguments;

constexpr Transport::AdminId gAdminId = 0;

Protocols::Echo::EchoClient gEchoClient;
TransportMgr<Transport::UDP> gUDPManager;
TransportMgr<Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> gTCPManager;
Messaging::ExchangeManager gExchangeManager;
SecureSessionMgr gSessionManager;
chip::mcsp::MessageCounterManager gMessageCounterManager;
Inet::IPAddress gDestAddr;

bool EchoIntervalExpired(void)
{
    uint64_t now = System::Timer::GetCurrentEpoch();

    return (now >= gPingArguments.GetLastEchoTime() + gPingArguments.GetEchoInterval());
}

CHIP_ERROR SendEchoRequest(streamer_t * stream)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    const char kRequestFormat[] = "Echo Message %" PRIu64 "\n";
    char requestData[(sizeof kRequestFormat) + kDecimalDigitsForUint64];
    snprintf(requestData, sizeof requestData, kRequestFormat, gPingArguments.GetEchoCount());
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(requestData, strlen(requestData));

    if (gPingArguments.IsUsingCRMP())
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

    gPingArguments.SetLastEchoTime(System::Timer::GetCurrentEpoch());

    streamer_printf(stream, "\nSend echo request message to Node: %" PRIu64 "\n", kTestDeviceNodeId);

    err = gEchoClient.SendEchoRequest(std::move(payloadBuf), sendFlags);

    if (err == CHIP_NO_ERROR)
    {
        gPingArguments.SetWaitingForEchoResp(true);
        gPingArguments.IncrementEchoCount();
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

    if (gPingArguments.IsUsingTCP())
    {
        peerAddr = Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::TCP(gDestAddr, gPingArguments.GetEchoPort()));
    }
    else
    {
        peerAddr = Optional<Transport::PeerAddress>::Value(
            Transport::PeerAddress::UDP(gDestAddr, gPingArguments.GetEchoPort(), INET_NULL_INTERFACEID));
    }

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(peerAddr, kTestDeviceNodeId, testSecurePairingSecret,
                                     SecureSessionMgr::PairingDirection::kInitiator, gAdminId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Establish secure session failed, err: %s\n", ErrorStr(err));
        gPingArguments.SetLastEchoTime(System::Timer::GetCurrentEpoch());
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
    uint32_t transitTime = respTime - gPingArguments.GetLastEchoTime();
    streamer_t * sout    = streamer_get();

    gPingArguments.SetWaitingForEchoResp(false);
    gPingArguments.IncrementEchoRespCount();

    streamer_printf(sout, "Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", gPingArguments.GetEchoRespCount(),
                    gPingArguments.GetEchoCount(),
                    static_cast<double>(gPingArguments.GetEchoRespCount()) * 100 / gPingArguments.GetEchoCount(),
                    payload->DataLength(), static_cast<double>(transitTime) / 1000);
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
    uint32_t maxEchoCount                   = 0;

    if (!Inet::IPAddress::FromString(destination, gDestAddr))
    {
        streamer_printf(stream, "Invalid Echo Server IP address: %s\n", destination);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    adminInfo = admins.AssignAdminId(gAdminId, kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = gTCPManager.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(Inet::kIPAddressType_IPv4)
                               .SetListenPort(gPingArguments.GetEchoPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init TCP manager error: %s\r\n", ErrorStr(err)));

    err = gUDPManager.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(Inet::kIPAddressType_IPv4)
                               .SetListenPort(gPingArguments.GetEchoPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init UDP manager error: %s\r\n", ErrorStr(err)));

    if (gPingArguments.IsUsingTCP())
    {
        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gTCPManager, &admins, &gMessageCounterManager);
        SuccessOrExit(err);

        err = gExchangeManager.Init(kTestControllerNodeId, &gTCPManager, &gSessionManager);
        SuccessOrExit(err);
    }
    else
    {
        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gUDPManager, &admins, &gMessageCounterManager);
        SuccessOrExit(err);

        err = gExchangeManager.Init(kTestControllerNodeId, &gUDPManager, &gSessionManager);
        SuccessOrExit(err);
    }

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession(stream);
    SuccessOrExit(err);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in PR:4451
    err = gEchoClient.Init(&gExchangeManager, { kTestDeviceNodeId, 0, gAdminId });
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    maxEchoCount = gPingArguments.GetMaxEchoCount();

    // Connection has been established. Now send the EchoRequests.
    for (unsigned int i = 0; i < maxEchoCount; i++)
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
        if (gPingArguments.IsWaitingForEchoResp())
        {
            streamer_printf(stream, "No response received\n");
            gPingArguments.SetWaitingForEchoResp(false);
        }
    }

    gEchoClient.Shutdown();
    gExchangeManager.Shutdown();
    gSessionManager.Shutdown();

exit:
    if ((err != CHIP_NO_ERROR))
    {
        streamer_printf(stream, "Ping failed with error: %s\n", ErrorStr(err));
    }
}

void PrintUsage(streamer_t * stream)
{
    streamer_printf(stream, "Usage: ping [options] <destination>\n\nOptions:\n");

    // Need to split the help info to prevent overflowing the streamer_printf
    // buffer (CONSOLE_DEFAULT_MAX_LINE 256)
    streamer_printf(stream, "  -h              print help information\n");
    streamer_printf(stream, "  -u              use UDP (default)\n");
    streamer_printf(stream, "  -t              use TCP\n");
    streamer_printf(stream, "  -p  <port>      echo server port\n");
    streamer_printf(stream, "  -i  <interval>  ping interval time in seconds\n");
    streamer_printf(stream, "  -c  <count>     stop after <count> replies\n");
    streamer_printf(stream, "  -r  <1|0>       enable or disable CRMP\n");
}

int cmd_ping(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    int ret           = 0;
    int optIndex      = 0;

    gPingArguments.Reset();

    while (optIndex < argc && argv[optIndex][0] == '-')
    {
        switch (argv[optIndex][1])
        {
        case 'h':
            PrintUsage(sout);
            return 0;
        case 'u':
            gPingArguments.SetUsingTCP(false);
            break;
        case 't':
            gPingArguments.SetUsingTCP(true);
            break;
        case 'i':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -i\n");
                return -1;
            }
            else
            {
                gPingArguments.SetEchoInterval(atol(argv[optIndex]) * 1000);
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
                gPingArguments.SetMaxEchoCount(atol(argv[optIndex]));
            }
            break;
        case 'p':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -c\n");
                return -1;
            }
            else
            {
                gPingArguments.SetEchoPort(atol(argv[optIndex]));
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
                    gPingArguments.SetUsingCRMP(false);
                }
                else if (arg == 1)
                {
                    gPingArguments.SetUsingCRMP(true);
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
    shell_register(cmds_ping, ArraySize(cmds_ping));
}
