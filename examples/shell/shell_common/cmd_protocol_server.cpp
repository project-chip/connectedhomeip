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

#include "shell/streamer.h"
#include <ChipShellCollection.h>

#include <platform/CHIPDeviceLayer.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/PASESession.h>
#include <shell/shell_core.h>
#include <support/ErrorStr.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;
using chip::Transport::TcpListenParameters;
using chip::Transport::UdpListenParameters;

namespace {

class EchoServerCmd
{
public:
    EchoServerCmd() = default;

    CHIP_ERROR Start(bool tcp, bool echoServer)
    {
        if (mConnected)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
        mAdminInfo = mAdmins.AssignAdminId(kAdminId, chip::kTestDeviceNodeId);
        if (mAdminInfo == NULL)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        if (tcp)
        {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
            mTcp = tcp;
#if INET_CONFIG_ENABLE_IPV4
            ReturnErrorOnFailure(mTCPManager.Init(
                TcpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4),
                TcpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6)));
#else  // INET_CONFIG_ENABLE_IPV4
            ReturnErrorOnFailure(mTCPManager.Init(
                TcpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6)));
#endif // INET_CONFIG_ENABLE_IPV4
            ReturnErrorOnFailure(
                mSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &mTCPManager, &mAdmins));
#else  // INET_CONFIG_ENABLE_TCP_ENDPOINT
            return CHIP_ERROR_INVALID_ARGUMENT;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
        }
        else
        {
#if INET_CONFIG_ENABLE_IPV4
            ReturnErrorOnFailure(mUDPManager.Init(
                UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4),
                UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6)));
#else
            ReturnErrorOnFailure(mUDPManager.Init(
                UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6)));
#endif // INET_CONFIG_ENABLE_IPV4
            ReturnErrorOnFailure(
                mSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &mUDPManager, &mAdmins));
        }
        ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));

        if (echoServer)
        {
            ReturnErrorOnFailure(mEchoServer.Init(&mExchangeManager));
            mEchoServer.SetEchoRequestReceived(EchoServerCmd::HandleEchoRequestReceived);
        }

        ReturnErrorOnFailure(mSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &mTestPairing,
                                                        chip::SecureSessionMgr::PairingDirection::kResponder, kAdminId));
        mConnected = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Stop()
    {
        if (!mConnected)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        if (mTcp)
        {
            mTCPManager.Close();
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
        mUDPManager.Close();
        mEchoServer.Shutdown();
        mExchangeManager.Shutdown();
        mSessionManager.Shutdown();
        mAdmins.ReleaseAdminId(kAdminId);
        mAdmins.Reset();
        mConnected = false;

        return CHIP_NO_ERROR;
    }

private:
    static void HandleEchoRequestReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle payload)
    {
        streamer_printf(streamer_get(), "Echo Request, len=%u ... sending response.\n", payload->DataLength());
    }

    EchoServerCmd(const EchoServerCmd &) = delete;
    EchoServerCmd & operator=(const EchoServerCmd &) = delete;

    bool mTcp       = false;
    bool mConnected = false;

    chip::Protocols::Echo::EchoServer mEchoServer;
#if INET_CONFIG_ENABLE_IPV4
    chip::TransportMgr<chip::Transport::UDP, chip::Transport::UDP> mUDPManager;
#else
    chip::TransportMgr<chip::Transport::UDP> mUDPManager;
#endif // INET_CONFIG_ENABLE_IPV4

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    static constexpr size_t kMaxTcpActiveConnectionCount = 4;
    static constexpr size_t kMaxTcpPendingPackets        = 4;
#if INET_CONFIG_ENABLE_IPV4
    chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>,
                       chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>>
        mTCPManager;
#else  // INET_CONFIG_ENABLE_IPV4
    chip::TransportMgr<chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> mTCPManager;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    chip::SecureSessionMgr mSessionManager;
    chip::SecurePairingUsingTestSecret mTestPairing;

    chip::Messaging::ExchangeManager mExchangeManager;

    chip::Transport::AdminPairingTable mAdmins;
    chip::Transport::AdminPairingInfo * mAdminInfo     = nullptr;
    static constexpr chip::Transport::AdminId kAdminId = 0;
};

EchoServerCmd gEchoServerCmd;

} // namespace

int cmd_protocol_server(int argc, char ** argv)
{
    bool tcp         = false;
    bool stop        = false;
    bool echoServer  = false;
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc >= 1 && strcmp(argv[0], "stop") == 0)
    {
        stop = true;
    }
    else if (argc <= 0 || strcmp(argv[0], "start") != 0)
    {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        streamer_printf(streamer_get(), "protocol-server start|stop [--tcp] [--echo]\n");
#else
        streamer_printf(streamer_get(), "ping-responder start|stop [--echo]\n");
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
        return 0;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--tcp") == 0)
        {
            tcp = true;
        }
        else if (strcmp(argv[i], "--echo") == 0)
        {
            echoServer = true;
        }
    }

    if (stop)
    {
        error = gEchoServerCmd.Stop();
    }
    else
    {
        error = gEchoServerCmd.Start(tcp, echoServer);
    }

    return error;
}

static shell_command_t cmds_protocol_server[] = {
    { &cmd_protocol_server, "protocol-server", "Setup CHIP protocol server" },
};

extern "C" void cmd_protocol_server_init()
{
    shell_register(cmds_protocol_server, 1);
}
