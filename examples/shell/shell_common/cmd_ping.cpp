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

#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#include <ChipShellCollection.h>
#include <Globals.h>

using namespace chip;
using namespace Shell;
using namespace Logging;
using chip::Inet::IPAddress;

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
        mPayloadSize        = 32;
        mWaitingForEchoResp = false;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        mUsingTCP = false;
#endif
        mUsingMRP = true;
        mEchoPort = CHIP_PORT;
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

    uint32_t GetPayloadSize() const { return mPayloadSize; }
    void SetPayloadSize(uint32_t value) { mPayloadSize = value; }

    uint16_t GetEchoPort() const { return mEchoPort; }
    void SetEchoPort(uint16_t value) { mEchoPort = value; }

    bool IsWaitingForEchoResp() const { return mWaitingForEchoResp; }
    void SetWaitingForEchoResp(bool value) { mWaitingForEchoResp = value; }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    bool IsUsingTCP() const { return mUsingTCP; }
    void SetUsingTCP(bool value) { mUsingTCP = value; }
#endif

    bool IsUsingMRP() const { return mUsingMRP; }
    void SetUsingMRP(bool value) { mUsingMRP = value; }

private:
    // The last time a echo request was attempted to be sent.
    uint64_t mLastEchoTime;

    // Count of the number of echo requests sent.
    uint64_t mEchoCount;

    // Count of the number of echo responses received.
    uint64_t mEchoRespCount;

    // The CHIP Echo request payload size in bytes.
    uint32_t mPayloadSize;

    // Max value for the number of echo requests sent.
    uint32_t mMaxEchoCount;

    // The CHIP Echo interval time in milliseconds.
    uint32_t mEchoInterval;

    uint16_t mEchoPort;

    // True, if the echo client is waiting for an echo response
    // after sending an echo request, false otherwise.
    bool mWaitingForEchoResp;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    bool mUsingTCP;
#endif

    bool mUsingMRP;
} gPingArguments;

Protocols::Echo::EchoClient gEchoClient;
Transport::AdminPairingTable gAdmins;

CHIP_ERROR SendEchoRequest(streamer_t * stream);
void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState, CHIP_ERROR error);

Transport::PeerAddress GetEchoPeerAddress()
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gPingArguments.IsUsingTCP())
    {
        return Transport::PeerAddress::TCP(gDestAddr, gPingArguments.GetEchoPort());
    }
    else
#endif
    {

        return Transport::PeerAddress::UDP(gDestAddr, gPingArguments.GetEchoPort(), INET_NULL_INTERFACEID);
    }
}

void Shutdown()
{
    chip::DeviceLayer::SystemLayer.CancelTimer(EchoTimerHandler, NULL);
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gPingArguments.IsUsingTCP())
    {
        gTCPManager.Disconnect(GetEchoPeerAddress());
    }
    gTCPManager.Close();
#endif
    gUDPManager.Close();

    gEchoClient.Shutdown();
    gExchangeManager.Shutdown();
    gSessionManager.Shutdown();
}

void EchoTimerHandler(chip::System::Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    if (gPingArguments.GetEchoRespCount() != gPingArguments.GetEchoCount())
    {
        streamer_printf(streamer_get(), "No response received\n");
        gPingArguments.SetEchoRespCount(gPingArguments.GetEchoCount());
    }
    if (gPingArguments.GetEchoCount() < gPingArguments.GetMaxEchoCount())
    {
        CHIP_ERROR err = SendEchoRequest(streamer_get());
        if (err != CHIP_NO_ERROR)
        {
            streamer_printf(streamer_get(), "Send request failed: %s\n", ErrorStr(err));
            Shutdown();
        }
    }
    else
    {
        Shutdown();
    }
}

CHIP_ERROR SendEchoRequest(streamer_t * stream)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    System::PacketBufferHandle payloadBuf;
    uint32_t payloadSize = gPingArguments.GetPayloadSize();

    payloadBuf = MessagePacketBuffer::New(payloadSize);
    VerifyOrExit(!payloadBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    memset(payloadBuf->Start(), 0, payloadSize);
    payloadBuf->SetDataLength(payloadSize);

    if (gPingArguments.IsUsingMRP())
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNone);
    }
    else
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    }

    gPingArguments.SetLastEchoTime(System::Clock::GetMonotonicMilliseconds());
    SuccessOrExit(chip::DeviceLayer::SystemLayer.StartTimer(gPingArguments.GetEchoInterval(), EchoTimerHandler, NULL));

    streamer_printf(stream, "\nSend echo request message with payload size: %d bytes to Node: %" PRIu64 "\n", payloadSize,
                    kTestDeviceNodeId);

    err = gEchoClient.SendEchoRequest(std::move(payloadBuf), sendFlags);

    if (err == CHIP_NO_ERROR)
    {
        gPingArguments.SetWaitingForEchoResp(true);
        gPingArguments.IncrementEchoCount();
    }
    else
    {
        chip::DeviceLayer::SystemLayer.CancelTimer(EchoTimerHandler, NULL);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Send echo request failed, err: %s\n", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR EstablishSecureSession(streamer_t * stream, const Transport::PeerAddress & peerAddress)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Optional<Transport::PeerAddress> peerAddr;
    SecurePairingUsingTestSecret * testSecurePairingSecret = chip::Platform::New<SecurePairingUsingTestSecret>();
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    peerAddr = Optional<Transport::PeerAddress>::Value(peerAddress);

    // Attempt to connect to the peer.
    err = gSessionManager.NewPairing(peerAddr, kTestDeviceNodeId, testSecurePairingSecret, SecureSession::SessionRole::kInitiator,
                                     gAdminId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Establish secure session failed, err: %s\n", ErrorStr(err));
        gPingArguments.SetLastEchoTime(System::Clock::GetMonotonicMilliseconds());
    }
    else
    {
        streamer_printf(stream, "Establish secure session succeeded\n");
    }

    return err;
}

void HandleEchoResponseReceived(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload)
{
    uint32_t respTime    = System::Clock::GetMonotonicMilliseconds();
    uint32_t transitTime = respTime - gPingArguments.GetLastEchoTime();
    streamer_t * sout    = streamer_get();

    gPingArguments.SetWaitingForEchoResp(false);
    gPingArguments.IncrementEchoRespCount();

    streamer_printf(sout, "Echo Response: %" PRIu64 "/%" PRIu64 "(%.2f%%) len=%u time=%.3fms\n", gPingArguments.GetEchoRespCount(),
                    gPingArguments.GetEchoCount(),
                    static_cast<double>(gPingArguments.GetEchoRespCount()) * 100 / gPingArguments.GetEchoCount(),
                    payload->DataLength(), static_cast<double>(transitTime) / 1000);
}

void StartPinging(streamer_t * stream, char * destination)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::AdminPairingInfo * adminInfo = nullptr;

    if (!IPAddress::FromString(destination, gDestAddr))
    {
        streamer_printf(stream, "Invalid Echo Server IP address: %s\n", destination);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    adminInfo = gAdmins.AssignAdminId(gAdminId, kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = gTCPManager.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(gDestAddr.Type())
                               .SetListenPort(gPingArguments.GetEchoPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init TCP manager error: %s\n", ErrorStr(err)));
#endif

    err = gUDPManager.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(gDestAddr.Type())
                               .SetListenPort(gPingArguments.GetEchoPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init UDP manager error: %s\n", ErrorStr(err)));

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gPingArguments.IsUsingTCP())
    {
        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gTCPManager, &gAdmins, &gMessageCounterManager);
        SuccessOrExit(err);

        err = gExchangeManager.Init(&gSessionManager);
        SuccessOrExit(err);
    }
    else
#endif
    {
        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gUDPManager, &gAdmins, &gMessageCounterManager);
        SuccessOrExit(err);

        err = gExchangeManager.Init(&gSessionManager);
        SuccessOrExit(err);
    }

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP echo responder.
    err = EstablishSecureSession(stream, GetEchoPeerAddress());
    SuccessOrExit(err);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in PR:4451
    err = gEchoClient.Init(&gExchangeManager, { kTestDeviceNodeId, 0, gAdminId });
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Response is received.
    gEchoClient.SetEchoResponseReceived(HandleEchoResponseReceived);

    err = SendEchoRequest(stream);
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Send request failed: %s\n", ErrorStr(err));
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Ping failed with error: %s\n", ErrorStr(err));
        Shutdown();
    }
}

void PrintUsage(streamer_t * stream)
{
    streamer_printf(stream, "Usage: ping [options] <destination>\n\nOptions:\n");

    // Need to split the help info to prevent overflowing the streamer_printf
    // buffer (CONSOLE_DEFAULT_MAX_LINE 256)
    streamer_printf(stream, "  -h              print help information\n");
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    streamer_printf(stream, "  -u              use UDP (default)\n");
    streamer_printf(stream, "  -t              use TCP\n");
#endif
    streamer_printf(stream, "  -p  <port>      echo server port\n");
    streamer_printf(stream, "  -i  <interval>  ping interval time in seconds\n");
    streamer_printf(stream, "  -c  <count>     stop after <count> replies\n");
    streamer_printf(stream, "  -r  <1|0>       enable or disable MRP\n");
    streamer_printf(stream, "  -s  <size>      application payload size in bytes\n");
}

CHIP_ERROR cmd_ping(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    int optIndex      = 0;

    gPingArguments.Reset();

    while (optIndex < argc && argv[optIndex][0] == '-')
    {
        switch (argv[optIndex][1])
        {
        case 'h':
            PrintUsage(sout);
            return CHIP_NO_ERROR;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        case 'u':
            gPingArguments.SetUsingTCP(false);
            break;
        case 't':
            gPingArguments.SetUsingTCP(true);
            break;
#endif
        case 'i':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -i\n");
                return CHIP_ERROR_INVALID_ARGUMENT;
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
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                gPingArguments.SetMaxEchoCount(atol(argv[optIndex]));
            }
            break;
        case 'p':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -p\n");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                gPingArguments.SetEchoPort(atol(argv[optIndex]));
            }
            break;
        case 's':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -s\n");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                gPingArguments.SetPayloadSize(atol(argv[optIndex]));
            }
            break;
        case 'r':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -r\n");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            else
            {
                int arg = atoi(argv[optIndex]);

                if (arg == 0)
                {
                    gPingArguments.SetUsingMRP(false);
                }
                else if (arg == 1)
                {
                    gPingArguments.SetUsingMRP(true);
                }
                else
                {
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }
            }
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        optIndex++;
    }

    if (optIndex >= argc)
    {
        streamer_printf(sout, "Missing IP address\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(sout, "IP address: %s\n", argv[optIndex]);
    StartPinging(sout, argv[optIndex]);

    return CHIP_NO_ERROR;
}

} // namespace

static shell_command_t cmds_ping[] = {
    { &cmd_ping, "ping", "Using Echo Protocol to measure packet loss across network paths" },
};

void cmd_ping_init()
{
    Engine::Root().RegisterCommands(cmds_ping, ArraySize(cmds_ping));
}
