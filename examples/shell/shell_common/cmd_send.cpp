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

namespace {

Messaging::ExchangeContext * gExchangeCtx = nullptr;

class SendArguments
{
public:
    void Reset()
    {
        mProtocolId   = 0x0002;
        mMessageType  = 1;
        mLastSendTime = 0;
        mPayloadSize  = 32;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        mUsingTCP = false;
#endif
        mUsingMRP = true;
        mPort     = CHIP_PORT;
    }

    uint64_t GetLastSendTime() const { return mLastSendTime; }
    void SetLastSendTime(uint64_t value) { mLastSendTime = value; }

    uint16_t GetProtocolId() const { return mProtocolId; }
    void SetProtocolId(uint16_t value) { mProtocolId = value; }

    uint32_t GetPayloadSize() const { return mPayloadSize; }
    void SetPayloadSize(uint32_t value) { mPayloadSize = value; }

    uint16_t GetPort() const { return mPort; }
    void SetPort(uint16_t value) { mPort = value; }

    uint8_t GetMessageType() const { return mMessageType; }
    void SetMessageType(uint8_t type) { mMessageType = type; }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    bool IsUsingTCP() const { return mUsingTCP; }
    void SetUsingTCP(bool value) { mUsingTCP = value; }
#endif

    bool IsUsingMRP() const { return mUsingMRP; }
    void SetUsingMRP(bool value) { mUsingMRP = value; }

private:
    // The last time a CHIP message was attempted to be sent.
    uint64_t mLastSendTime;

    uint32_t mPayloadSize;
    uint16_t mProtocolId;
    uint16_t mPort;
    uint8_t mMessageType;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    bool mUsingTCP;
#endif

    bool mUsingMRP;
} gSendArguments;

class MockAppDelegate : public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && buffer) override
    {
        uint32_t respTime    = System::Timer::GetCurrentEpoch();
        uint32_t transitTime = respTime - gSendArguments.GetLastSendTime();
        streamer_t * sout    = streamer_get();

        streamer_printf(sout, "Response received: len=%u time=%.3fms\n", buffer->DataLength(),
                        static_cast<double>(transitTime) / 1000);

        gExchangeCtx->Close();
        gExchangeCtx = nullptr;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override
    {
        streamer_t * sout = streamer_get();
        streamer_printf(sout, "No response received\n");

        gExchangeCtx->Close();
        gExchangeCtx = nullptr;
    }
} gMockAppDelegate;

CHIP_ERROR SendMessage(streamer_t * stream)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    System::PacketBufferHandle payloadBuf;
    uint32_t payloadSize = gSendArguments.GetPayloadSize();

    // Discard any existing exchange context. Effectively we can only have one exchange with
    // a single node at any one time.
    if (gExchangeCtx != nullptr)
    {
        gExchangeCtx->Abort();
        gExchangeCtx = nullptr;
    }

    // Create a new exchange context.
    gExchangeCtx = gExchangeManager.NewContext({ kTestDeviceNodeId, 0, gAdminId }, &gMockAppDelegate);
    VerifyOrExit(gExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    payloadBuf = MessagePacketBuffer::New(payloadSize);
    VerifyOrExit(!payloadBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    memset(payloadBuf->Start(), 0, payloadSize);
    payloadBuf->SetDataLength(payloadSize);

    if (gSendArguments.IsUsingMRP())
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNone);
    }
    else
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    }

    gExchangeCtx->SetResponseTimeout(kResponseTimeOut);
    sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);

    gSendArguments.SetLastSendTime(System::Timer::GetCurrentEpoch());

    streamer_printf(stream, "\nSend CHIP message with payload size: %d bytes to Node: %" PRIu64 "\n", payloadSize,
                    kTestDeviceNodeId);

    err = gExchangeCtx->SendMessage(Protocols::Id(VendorId::Common, gSendArguments.GetProtocolId()),
                                    gSendArguments.GetMessageType(), std::move(payloadBuf), sendFlags);

    if (err != CHIP_NO_ERROR)
    {
        gExchangeCtx->Abort();
        gExchangeCtx = nullptr;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(stream, "Send CHIP message failed, err: %s\n", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR EstablishSecureSession(streamer_t * stream, Transport::PeerAddress & peerAddress)
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
        gSendArguments.SetLastSendTime(System::Timer::GetCurrentEpoch());
    }
    else
    {
        streamer_printf(stream, "Establish secure session succeeded\n");
    }

    return err;
}

void ProcessCommand(streamer_t * stream, char * destination)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::AdminPairingTable admins;
    Transport::PeerAddress peerAddress;
    Transport::AdminPairingInfo * adminInfo = nullptr;

    if (!chip::Inet::IPAddress::FromString(destination, gDestAddr))
    {
        streamer_printf(stream, "Invalid CHIP Server IP address: %s\n", destination);
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    adminInfo = admins.AssignAdminId(gAdminId, kTestControllerNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = gTCPManager.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(gDestAddr.Type())
                               .SetListenPort(gSendArguments.GetPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init TCP manager error: %s\n", ErrorStr(err)));
#endif

    err = gUDPManager.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(gDestAddr.Type())
                               .SetListenPort(gSendArguments.GetPort() + 1));
    VerifyOrExit(err == CHIP_NO_ERROR, streamer_printf(stream, "Failed to init UDP manager error: %s\n", ErrorStr(err)));

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gSendArguments.IsUsingTCP())
    {
        peerAddress = Transport::PeerAddress::TCP(gDestAddr, gSendArguments.GetPort());

        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gTCPManager, &admins, &gMessageCounterManager);
        SuccessOrExit(err);
    }
    else
#endif
    {
        peerAddress = Transport::PeerAddress::UDP(gDestAddr, gSendArguments.GetPort(), INET_NULL_INTERFACEID);

        err =
            gSessionManager.Init(kTestControllerNodeId, &DeviceLayer::SystemLayer, &gUDPManager, &admins, &gMessageCounterManager);
        SuccessOrExit(err);
    }

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    // Start the CHIP connection to the CHIP server.
    err = EstablishSecureSession(stream, peerAddress);
    SuccessOrExit(err);

    err = SendMessage(stream);
    SuccessOrExit(err);

    // TODO:#5496: Use condition_varible to suspend the current thread and wake it up when response arrive.
    sleep(2);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    gTCPManager.Disconnect(peerAddress);
    gTCPManager.Close();
#endif
    gUDPManager.Close();

    gExchangeManager.Shutdown();
    gSessionManager.Shutdown();

exit:
    if ((err != CHIP_NO_ERROR))
    {
        streamer_printf(stream, "Send failed with error: %s\n", ErrorStr(err));
    }
}

void PrintUsage(streamer_t * stream)
{
    streamer_printf(stream, "Usage: send [options] <destination>\n\nOptions:\n");

    // Need to split the help info to prevent overflowing the streamer_printf
    // buffer (CONSOLE_DEFAULT_MAX_LINE 256)
    streamer_printf(stream, "  -h              print help information\n");
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    streamer_printf(stream, "  -u              use UDP (default)\n");
    streamer_printf(stream, "  -t              use TCP\n");
#endif
    streamer_printf(stream, "  -P  <protocol>  protocol ID\n");
    streamer_printf(stream, "  -T  <type>      message type\n");
    streamer_printf(stream, "  -p  <port>      server port number\n");
    streamer_printf(stream, "  -r  <1|0>       enable or disable MRP\n");
    streamer_printf(stream, "  -s  <size>      application payload size in bytes\n");
}

int cmd_send(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    int ret           = 0;
    int optIndex      = 0;

    gSendArguments.Reset();

    while (optIndex < argc && argv[optIndex][0] == '-')
    {
        switch (argv[optIndex][1])
        {
        case 'h':
            PrintUsage(sout);
            return 0;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        case 'u':
            gSendArguments.SetUsingTCP(false);
            break;
        case 't':
            gSendArguments.SetUsingTCP(true);
            break;
#endif
        case 'P':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -P\n");
                return -1;
            }
            else
            {
                gSendArguments.SetProtocolId(atol(argv[optIndex]));
            }
            break;
        case 'T':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -T\n");
                return -1;
            }
            else
            {
                gSendArguments.SetMessageType(atoi(argv[optIndex]));
            }
            break;
        case 'p':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -p\n");
                return -1;
            }
            else
            {
                gSendArguments.SetPort(atol(argv[optIndex]));
            }
            break;
        case 's':
            if (++optIndex >= argc || argv[optIndex][0] == '-')
            {
                streamer_printf(sout, "Invalid argument specified for -s\n");
                return -1;
            }
            else
            {
                gSendArguments.SetPayloadSize(atol(argv[optIndex]));
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
                    gSendArguments.SetUsingMRP(false);
                }
                else if (arg == 1)
                {
                    gSendArguments.SetUsingMRP(true);
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
        ProcessCommand(sout, argv[optIndex]);
    }

    return ret;
}

} // namespace

static shell_command_t cmds_send[] = {
    { &cmd_send, "send", "Send raw CHIP message" },
};

void cmd_send_init()
{
    Engine::Root().RegisterCommands(cmds_send, ArraySize(cmds_send));
}
