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

#include <rtos/EventFlags.h>

#include <common.h>
#include <net_socket.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Inet;
using namespace mbed;
using namespace rtos;

static chip::Shell::Shell sShellDateSubcommands;
static chip::Shell::Shell sShellNetworkSubcommands;
static chip::Shell::Shell sShellSocketSubcommands;

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr NodeId kSourceNodeId                = 123654;
constexpr NodeId kDestinationNodeId           = 111222333;
constexpr uint32_t kMessageId                 = 18;
EventFlags socketEvent;
uint32_t socketMsgReceiveFlag = 1;

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

        socketEvent.set(socketMsgReceiveFlag);
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
        printf("     interface id: %d, interface name: %s, interface state: %s\n", intId, intName,
               intIterator.IsUp() ? "UP" : "DOWN");
        if (addrIterator.HasCurrent())
        {
            addr = addrIterator.GetAddress();
            addrIterator.GetAddressWithPrefix(addrWithPrefix);
            char addrStr[80];
            addrWithPrefix.IPAddr.ToString(addrStr, sizeof(addrStr));
            printf("     interface address: %s/%d, %s broadcast addr\n", addrStr, addrWithPrefix.Length,
                   addrIterator.HasBroadcastAddress() ? "has" : "no");
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

static int socket_echo_parse_args(char ** argv, ChipSocket & sock, IPAddress & addr, uint16_t & port, char ** payload)
{
    if (strcmp(argv[0], "UDP") == 0)
    {
        sock.type = Transport::Type::kUdp;
    }
    else if (strcmp(argv[0], "TCP") == 0)
    {
        sock.type = Transport::Type::kTcp;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (IPAddress::FromString(argv[1], addr) == false)
    {
        return CHIP_ERROR_INVALID_ADDRESS;
    }

    port     = atoi(argv[2]);
    *payload = argv[3];

    return CHIP_NO_ERROR;
}

int cmd_socket_echo(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    INET_ERROR err;
    char * payload;
    uint16_t payloadLen;
    ChipSocket sock;
    uint16_t port;
    char addrStr[16];

    PacketBufferHandle buffer;
    PacketHeader header;

    TransportMgrBase gTransportMgrBase;
    SocketTransportMgrDelegate gSocketTransportMgrDelegate;

    streamer_t * sout = streamer_get();
    IPAddress addr;

    VerifyOrExit(argc == 4, error = CHIP_ERROR_INVALID_ARGUMENT);

    err = socket_echo_parse_args(argv, sock, addr, port, &payload);
    if (err != INET_NO_ERROR)
    {
        sock.type = Transport::Type::kUndefined;
        streamer_printf(sout, "ERROR: wrong command arguments. Check socket help\r\n");
        ExitNow(error = err;);
    }

    if (sock.type == Transport::Type::kUdp)
    {
        new (&sock.udpSocket) Transport::UDP();
        err = sock.udpSocket.Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(addr.Type()));
    }
    else
    {
        new (&sock.tcpSocket) TCPImpl;
        err = sock.tcpSocket.Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer).SetAddressType(addr.Type()));
    }

    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: create %s endpoint failed\r\n", argv[0]);
        ExitNow(error = err;);
    }

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

    socketEvent.clear();

    streamer_printf(sout, "INFO: send %s message %s to address: %s port: %d\r\n", argv[0], payload,
                    addr.ToString(addrStr, sizeof(addrStr)), port);
    if (sock.type == Transport::Type::kUdp)
    {
        err = sock.udpSocket.SendMessage(header, Transport::PeerAddress::UDP(addr, port), std::move(buffer));
    }
    else
    {
        err = sock.tcpSocket.SendMessage(header, Transport::PeerAddress::TCP(addr, port), std::move(buffer));
    }

    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: send socket message failed\r\n");
        ExitNow(error = err;);
    }

    socketEvent.wait_all(socketMsgReceiveFlag, 5000);

exit:
    if (sock.type == Transport::Type::kTcp)
    {
        sock.tcpSocket.Disconnect(Transport::PeerAddress::TCP(addr));
    }
    else if (sock.type == Transport::Type::kUdp)
    {
        sock.udpSocket.Close();
    }

    return error;
}

int cmd_socket_bsd(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    streamer_t * sout     = streamer_get();
    const char hostname[] = "ifconfig.io";
    SocketAddress address;
    /* get the host address */
    printf("\nResolve hostname %s\r\n", hostname);
    WiFiInterface * _net = WiFiInterface::get_default_instance();
    int result           = _net->gethostbyname(hostname, &address);
    if (result != 0)
    {
        printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
        return false;
    }

    printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None"));
    address.set_port(htons(80));
    sockaddr addrs;
    convert_mbed_addr_to_bsd(&addrs, &address);

    int fd = mbed_socket(1, 1, 0);

    mbed_bind(fd, &addrs, sizeof(sockaddr));
    mbed_connect(fd, &addrs, sizeof(sockaddr));

    const char buff[] = "GET / HTTP/1.1\r\n"
                        "Host: ifconfig.io\r\n"
                        "Connection: close\r\n"
                        "\r\n";

    nsapi_size_t bytes_to_send       = strlen(buff);
    nsapi_size_or_error_t bytes_sent = 0;

    printf("\r\nSending message: \r\n%s", buff);

    while (bytes_to_send)
    {

        bytes_sent = mbed_send(fd, buff + bytes_sent, bytes_to_send, 0);
        if (bytes_sent < 0)
        {
            printf("Error! _socket.send() returned: %d\r\n", bytes_sent);
            return false;
        }
        else
        {
            printf("sent %d bytes\r\n", bytes_sent);
        }

        bytes_to_send -= bytes_sent;
    }

    printf("Complete message sent\r\n");

    char buf[100];
    int remaining_bytes = 100;
    int received_bytes  = 0;

    nsapi_size_or_error_t res = remaining_bytes;
    while (res > 0 && remaining_bytes > 0)
    {
        res = mbed_recv(fd, buf + received_bytes, remaining_bytes, 0);
        if (res < 0)
        {
            printf("Error! _socket.recv() returned: %d\r\n", res);
            return false;
        }

        received_bytes += res;
        remaining_bytes -= res;
    }

    printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buf, "\n") - buf, buf);

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
      "Connect and send test message to echo server via specific socket. Usage: socket echo <type> <ip> <port> <message>" },
    { &cmd_socket_bsd, "bsd",
      "BSD IP communication test via specific socket. Send message in loopback. Usage: socket echo <type> <message>" },
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
