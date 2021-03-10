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

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <inttypes.h>
#include <stdarg.h>

#include "ChipShellMbedCollection.h"

#include <inet/InetError.h>
#include <inet/InetLayer.h>

#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Inet;

static chip::Shell::Shell sShellDateSubcommands;
static chip::Shell::Shell sShellNetworkSubcommands;
static chip::Shell::Shell sShellSocketSubcommands;

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr NodeId kSourceNodeId                = 123654;
constexpr NodeId kDestinationNodeId           = 111222333;
constexpr uint32_t kMessageId                 = 18;
static int SocketReceiveHandlerCallCount      = 0;

using TCPImpl = Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;

class SocketTransportMgrDelegate : public TransportMgrDelegate
{
public:
    SocketTransportMgrDelegate() {}
    ~SocketTransportMgrDelegate() override {}

    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                           System::PacketBufferHandle msgBuf) override
    {
        char info[Transport::PeerAddress::kMaxToStringSize];

        source.ToString(info, sizeof(info));
        streamer_printf(streamer_get(), "Received message from %s payload: %s\n\r", info, msgBuf->Start());

        SocketReceiveHandlerCallCount++;
    }
};

int cmd_date_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_date_help(int argc, char ** argv)
{
    sShellDateSubcommands.ForEachCommand(cmd_date_help_iterator, nullptr);
    return 0;
}

int cmd_date_dispatch(int argc, char ** argv)
{
    uint16_t year;
    uint8_t month, dayOfMonth;
    uint8_t hour, minute, second;

    uint64_t currTimeMS = 0;

    streamer_t * sout = streamer_get();

    if (0 == argc)
    {
        if (Layer::GetClock_RealTimeMS(currTimeMS) != CHIP_SYSTEM_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Date/Time was not set\r\n");
            return CHIP_ERROR_ACCESS_DENIED;
        }

        SecondsSinceEpochToCalendarTime(currTimeMS / 1000, year, month, dayOfMonth, hour, minute, second);
        streamer_printf(sout, "%04" PRIu16 "-%02" PRIu8 "-%02" PRIu8 " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8 "\n\r", year, month,
                        dayOfMonth, hour, minute, second);
        return CHIP_NO_ERROR;
    }

    return sShellDateSubcommands.ExecCommand(argc, argv);
}

int cmd_date_set(int argc, char ** argv)
{
    uint16_t year;
    uint8_t month, dayOfMonth;
    uint8_t hour, minute, seconds;

    uint32_t newCurrTime = 0;
    streamer_t * sout    = streamer_get();

    int ret          = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc == 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    ret = sscanf(argv[0], "%4" SCNu16 "-%2" SCNu8 "-%2" SCNu8, &year, &month, &dayOfMonth);
    if (3 != ret)
    {
        streamer_printf(sout, "ERROR: Date is in wrong format! Please use 'YYYY-MM-DD HH:MM:SS' format.\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    ret = sscanf(argv[1], "%2" SCNu8 ":%2" SCNu8 ":%2" SCNu8, &hour, &minute, &seconds);
    if (3 != ret)
    {
        streamer_printf(sout, "ERROR: Time is in wrong format! Please use 'YYYY-MM-DD HH:MM:SS' format.\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    if (!CalendarTimeToSecondsSinceEpoch(year, month, dayOfMonth, hour, minute, seconds, newCurrTime))
    {
        streamer_printf(sout, "ERROR: Wrond date and/or time values\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    error = Layer::SetClock_RealTime(static_cast<uint64_t>(newCurrTime) * UINT64_C(1000000));

exit:
    return error;
}

int cmd_device_test_config(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    chip::DeviceLayer::ConfigurationMgrImpl().RunConfigUnitTest();
exit:
    return error;
}

/* Network commands */
int cmd_network_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellNetworkSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

int cmd_network_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_network_help(int argc, char ** argv)
{
    sShellNetworkSubcommands.ForEachCommand(cmd_network_help_iterator, nullptr);
    return 0;
}

int cmd_network_interface(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    InterfaceIterator intIterator;
    InterfaceAddressIterator addrIterator;
    IPAddress addr;
    IPPrefix addrWithPrefix;
    char intName[IF_NAMESIZE];
    InterfaceId intId;
    INET_ERROR err;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(sout, "    Current interface:\n");
    for (; intIterator.HasCurrent(); intIterator.Next())
    {
        intId = intIterator.GetInterface();
        if (intId == INET_NULL_INTERFACEID)
        {
            streamer_printf(sout, "ERROR: get interface failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }

        err = intIterator.GetInterfaceName(intName, sizeof(intName));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: get interface name failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        printf("     interface id: %d, interface name: %s, interface state: %s, %s broadcast addr\n", intId, intName,
               intIterator.IsUp() ? "UP" : "DOWN", intIterator.HasBroadcastAddress() ? "has" : "no");
        if (addrIterator.HasCurrent())
        {
            addr = addrIterator.GetAddress();
            addrIterator.GetAddressWithPrefix(addrWithPrefix);
            char addrStr[80];
            addrWithPrefix.IPAddr.ToString(addrStr, sizeof(addrStr));
            printf("     interface address: %s/%d\n", addrStr, addrWithPrefix.Length);
            addrIterator.Next();
        }
    }

exit:
    return error;
}

int cmd_network_idToName(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char intName[IF_NAMESIZE];
    InterfaceId intId;
    INET_ERROR err;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    intId = (InterfaceId) atoi(argv[0]);
    if (intId == INET_NULL_INTERFACEID)
    {
        streamer_printf(sout, "ERROR: wrong interface ID\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    err = GetInterfaceName(intId, intName, sizeof(intName));
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: get interface name failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    streamer_printf(sout, "Name of %d interface: %s\n", intId, intName);

exit:
    return error;
}

int cmd_network_nameToId(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char intName[IF_NAMESIZE];
    InterfaceId intId;
    INET_ERROR err;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    strncpy(intName, argv[0], IF_NAMESIZE);

    err = InterfaceNameToId(intName, intId);
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: get interface ID failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    if (intId == INET_NULL_INTERFACEID)
    {
        streamer_printf(sout, "ERROR: wrong interface name\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    streamer_printf(sout, "Interface %s has ID: %d\n", intName, intId);

exit:
    return error;
}

/* Socket commands */
int cmd_socket_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellSocketSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

int cmd_socket_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_socket_help(int argc, char ** argv)
{
    sShellSocketSubcommands.ForEachCommand(cmd_socket_help_iterator, nullptr);
    return 0;
}

static void serviceEvents(struct ::timeval & aSleepTime)
{
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (DeviceLayer::SystemLayer.State() == System::kLayerState_Initialized)
        DeviceLayer::SystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    if (DeviceLayer::InetLayer.State == InetLayer::kState_Initialized)
        DeviceLayer::InetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        return;
    }

    if (DeviceLayer::SystemLayer.State() == System::kLayerState_Initialized)
    {
        DeviceLayer::SystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (DeviceLayer::InetLayer.State == InetLayer::kState_Initialized)
    {
        DeviceLayer::InetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
}

static void driveIOUntil(unsigned int timeoutMs, std::function<bool(void)> completionFunction)
{
    uint64_t mStartTime = DeviceLayer::SystemLayer.GetClock_MonotonicMS();
    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    while (true)
    {
        serviceEvents(aSleepTime); // at least one IO loop is guaranteed

        if (completionFunction() || ((DeviceLayer::SystemLayer.GetClock_MonotonicMS() - mStartTime) >= timeoutMs))
        {
            break;
        }
    }
}

struct ChipSocket
{
    ChipSocket() {}
    ~ChipSocket() {}

    union
    {
        TCPImpl tcpSocket;
        Transport::UDP udpSocket;
    };
    Transport::Type type;
};

int cmd_socket_echo(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    INET_ERROR err;
    char * payload;
    uint16_t payloadLen;
    ChipSocket sock;

    PacketBufferHandle buffer;
    PacketHeader header;

    TransportMgrBase gTransportMgrBase;
    SocketTransportMgrDelegate gSocketTransportMgrDelegate;

    streamer_t * sout = streamer_get();
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);

    VerifyOrExit(argc == 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(argv[0], "UDP") == 0)
    {
        new (&sock.udpSocket) Transport::UDP();
        sock.type = Transport::Type::kUdp;

        err = sock.udpSocket.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(addr.Type()));
    }
    else if (strcmp(argv[0], "TCP") == 0)
    {
        new (&sock.tcpSocket) TCPImpl;
        sock.type = Transport::Type::kTcp;

        err = sock.tcpSocket.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer).SetAddressType(addr.Type()));
    }
    else
    {
        streamer_printf(sout, "ERROR: Wrong socket type\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: create %s endpoint failed\r\n", argv[0]);
        ExitNow(error = err;);
    }

    payload    = argv[1];
    payloadLen = sizeof(payload);

    buffer = PacketBufferHandle::NewWithData(payload, payloadLen);
    if (buffer.IsNull())
    {
        streamer_printf(sout, "ERROR: create payload buffer failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    gTransportMgrBase.SetSecureSessionMgr((TransportMgrDelegate *) &gSocketTransportMgrDelegate);
    gTransportMgrBase.SetRendezvousSession((TransportMgrDelegate *) &gSocketTransportMgrDelegate);
    if (sock.type == Transport::Type::kUdp)
    {
        gTransportMgrBase.Init((Transport::Base *) &sock.udpSocket);
    }
    else
    {
        gTransportMgrBase.Init((Transport::Base *) &sock.tcpSocket);
    }

    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);

    SocketReceiveHandlerCallCount = 0;

    // Should be able to send a message to itself by just calling send.
    if (sock.type == Transport::Type::kUdp)
    {
        err = sock.udpSocket.SendMessage(header, Transport::PeerAddress::UDP(addr), std::move(buffer));
    }
    else
    {
        err = sock.tcpSocket.SendMessage(header, Transport::PeerAddress::TCP(addr), std::move(buffer));
    }

    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: send socket message failed\r\n");
        ExitNow(error = err;);
    }

    driveIOUntil(5000 /* ms */, []() { return SocketReceiveHandlerCallCount != 0; });

    if (sock.type == Transport::Type::kTcp)
    {
        // Disconnect and wait for seeing peer close
        sock.tcpSocket.Disconnect(Transport::PeerAddress::TCP(addr));
        driveIOUntil(5000 /* ms */, [&sock]() { return !sock.tcpSocket.HasActiveConnections(); });
    }

exit:
    return error;
}

static const shell_command_t cmds_date_root = { &cmd_date_dispatch, "date", "Display the current time, or set the system date." };

static const shell_command_t cmds_date[] = { { &cmd_date_set, "set", "Set date/time using 'YYYY-MM-DD HH:MM:SS' format" },
                                             { &cmd_date_help, "help", "Display help for each subcommand" } };

static const shell_command_t cmds_test_config = { &cmd_device_test_config, "testconfig",
                                                  "Test the configuration implementation. Usage: device testconfig" };

static const shell_command_t cmds_network_root = { &cmd_network_dispatch, "network", "Network interface layer commands" };

static const shell_command_t cmds_network[] = { { &cmd_network_interface, "interface",
                                                  "Display current network interface details" },
                                                { &cmd_network_idToName, "idToName", "Display interface name by id" },
                                                { &cmd_network_nameToId, "nameToId", "Display interface id by name" },
                                                { &cmd_network_help, "help", "Display help for each network subcommands" } };

static const shell_command_t cmds_socket_root = { &cmd_socket_dispatch, "socket", "Socket layer commands" };

static const shell_command_t cmds_socket[] = {
    { &cmd_socket_echo, "echo",
      "Echo IP communication test via specific socket. Send message in loopback. Usage: socket echo <type> <message>" },
    { &cmd_socket_help, "help", "Display help for each socket subcommands" }
};

void cmd_mbed_utils_init()
{
    sShellDateSubcommands.RegisterCommands(cmds_date, ArraySize(cmds_date));
    sShellNetworkSubcommands.RegisterCommands(cmds_network, ArraySize(cmds_network));
    sShellSocketSubcommands.RegisterCommands(cmds_socket, ArraySize(cmds_socket));
    shell_register(&cmds_date_root, 1);
    shell_register(&cmds_test_config, 1);
    shell_register(&cmds_network_root, 1);
    shell_register(&cmds_socket_root, 1);
}
