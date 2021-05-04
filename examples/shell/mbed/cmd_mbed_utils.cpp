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

#include <lib/shell/shell_core.h>

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

#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#include <protocols/secure_channel/PASESession.h>
#include <rtos/EventFlags.h>

#include "netdb.h"
#include <common.h>
#include <net_socket.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace mbed;
using namespace rtos;

static chip::Shell::Shell sShellDateSubcommands;
static chip::Shell::Shell sShellNetworkSubcommands;
static chip::Shell::Shell sShellSocketSubcommands;
static chip::Shell::Shell sShellServerSubcommands;

constexpr size_t kMaxTcpActiveConnectionCount = 2;
constexpr size_t kMaxTcpPendingPackets        = 2;

EventFlags socketEvent;
uint32_t socketMsgReceiveFlag           = 0x1;
uint32_t socketConnectionCompeletedFlag = 0x2;

void HandleDNSResolveComplete(void * appState, INET_ERROR err, uint8_t addrCount, IPAddress * addrArray)
{

    if (addrCount > 0)
    {
        char destAddrStr[64];
        for (int i = 0; i < addrCount; i++)
        {
            addrArray[i].ToString(destAddrStr, sizeof(destAddrStr));
            streamer_printf(streamer_get(), "%d   DNS name resolution complete: %s\r\n", i, destAddrStr);
        }
    }
    else
        streamer_printf(streamer_get(), "DNS name resolution return no addresses\r\n");
}

void OnTcpMessageSent(Inet::TCPEndPoint * endPoint, uint16_t Length)
{
    streamer_printf(streamer_get(), "INFO: TCP socket message sent\r\n");
}

INET_ERROR OnTcpMessageReceived(Inet::TCPEndPoint * endPoint, System::PacketBufferHandle buffer)
{
    streamer_t * sout = streamer_get();

    streamer_printf(sout, "INFO: TCP socket message received\r\n");
    streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                    strstr((char *) buffer->Start(), "\n") - (char *) buffer->Start(), (char *) buffer->Start());
    buffer.FreeHead();
    socketEvent.set(socketMsgReceiveFlag);
    return INET_NO_ERROR;
}

void OnConnectionCompleted(Inet::TCPEndPoint * endPoint, INET_ERROR error)
{
    streamer_printf(streamer_get(), "INFO: TCP socket connection completed\r\n");
    socketEvent.set(socketConnectionCompeletedFlag);
}

void OnUdpMessageReceived(Inet::IPEndPointBasis * endPoint, System::PacketBufferHandle buffer, const Inet::IPPacketInfo * pktInfo)
{
    char peerAddrStr[PeerAddress::kMaxToStringSize];
    streamer_t * sout       = streamer_get();
    PeerAddress peerAddress = PeerAddress::UDP(pktInfo->SrcAddress, pktInfo->SrcPort);
    peerAddress.ToString(peerAddrStr, sizeof(peerAddrStr));

    streamer_printf(sout, "INFO: UDP socket message received from %s\r\n", peerAddrStr);
    streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                    strstr((char *) buffer->Start(), "\n") - (char *) buffer->Start(), (char *) buffer->Start());
    buffer.FreeHead();
    socketEvent.set(socketMsgReceiveFlag);
}

int cmd_common_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_date_help(int argc, char ** argv)
{
    sShellDateSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
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

int cmd_network_help(int argc, char ** argv)
{
    sShellNetworkSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
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
    uint8_t intCounter = 0;

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
        streamer_printf(sout, "     interface id: %d, interface name: %s, interface state: %s\n", intId, intName,
                        intIterator.IsUp() ? "UP" : "DOWN");
        if (addrIterator.HasCurrent())
        {
            addr = addrIterator.GetAddress();
            addrIterator.GetAddressWithPrefix(addrWithPrefix);
            char addrStr[80];
            addrWithPrefix.IPAddr.ToString(addrStr, sizeof(addrStr));
            streamer_printf(sout, "     interface address: %s/%d, %s broadcast addr\n", addrStr, addrWithPrefix.Length,
                            addrIterator.HasBroadcastAddress() ? "has" : "no");
            addrIterator.Next();
        }

        intCounter++;
    }

    if (intCounter == 0)
    {
        streamer_printf(sout, "    no interface is available\n");
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

int cmd_socket_help(int argc, char ** argv)
{
    sShellSocketSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
    return 0;
}

struct ChipSocket
{
    ChipSocket() {}
    ~ChipSocket() {}

    union
    {
        TCPEndPoint * tcpSocket;
        UDPEndPoint * udpSocket;
    };
    Type type;
};

static int socket_echo_parse_args(char ** argv, ChipSocket & sock, IPAddress & addr, uint16_t & port, char ** msg)
{
    if (strcmp(argv[0], "UDP") == 0)
    {
        sock.type = Type::kUdp;
    }
    else if (strcmp(argv[0], "TCP") == 0)
    {
        sock.type = Type::kTcp;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (IPAddress::FromString(argv[1], addr) == false)
    {
        return CHIP_ERROR_INVALID_ADDRESS;
    }

    port = atoi(argv[2]);
    *msg = argv[3];

    return CHIP_NO_ERROR;
}

int cmd_socket_client(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    INET_ERROR err;
    char * msg;
    uint16_t msgLen;
    ChipSocket sock;
    uint16_t port;
    char addrStr[16];
    bool waitForResponse = false;

    PacketBufferHandle buffer;

    streamer_t * sout = streamer_get();
    IPAddress addr;

    VerifyOrExit(argc > 3, error = CHIP_ERROR_INVALID_ARGUMENT);

    err = socket_echo_parse_args(argv, sock, addr, port, &msg);
    if (err != INET_NO_ERROR)
    {
        sock.type = Type::kUndefined;
        streamer_printf(sout, "ERROR: wrong command arguments. Check socket help\r\n");
        ExitNow(error = err;);
    }

    if ((argc == 5) && (strcmp(argv[4], "wait") == 0))
    {
        waitForResponse = true;
    }

    msgLen = strlen(msg);

    buffer = PacketBufferHandle::NewWithData(msg, msgLen);
    if (buffer.IsNull())
    {
        streamer_printf(sout, "ERROR: create message buffer failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    if (sock.type == Type::kTcp)
    {
        err = DeviceLayer::InetLayer.NewTCPEndPoint(&sock.tcpSocket);
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Create TCP socket failed\r\n");
            ExitNow(error = err;);
        }

        sock.tcpSocket->OnConnectComplete = OnConnectionCompleted;

        if (waitForResponse)
        {
            sock.tcpSocket->OnDataReceived = OnTcpMessageReceived;
        }

        socketEvent.clear();

        streamer_printf(sout, "INFO: connect to TCP server address: %s port: %d\r\n", addr.ToString(addrStr, sizeof(addrStr)),
                        port);
        err = sock.tcpSocket->Connect(addr, port, 0);
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: TCP socket connect failed\r\n");
            ExitNow(error = err;);
        }

        if (socketEvent.wait_all(socketConnectionCompeletedFlag, 5000) & osFlagsError)
        {
            streamer_printf(sout, "ERROR: TCP socket connection is not completed\r\n");
            ExitNow(error = CHIP_ERROR_DEVICE_CONNECT_TIMEOUT;);
        }

        socketEvent.clear();

        streamer_printf(sout, "INFO: TCP socket send message\r\n");
        error = sock.tcpSocket->Send(std::move(buffer));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: TCP socket send failed\r\n");
            ExitNow(error = err;);
        }
    }
    else
    {
        err = DeviceLayer::InetLayer.NewUDPEndPoint(&sock.udpSocket);
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Create UDP endpoint failed\r\n");
            ExitNow(error = err;);
        }

        if (waitForResponse)
        {
            sock.udpSocket->Listen(OnUdpMessageReceived, nullptr);
        }

        socketEvent.clear();

        err = sock.udpSocket->SendTo(addr, port, std::move(buffer));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: UDP socket send failed\r\n");
            ExitNow(error = err;);
        }
    }

    if (waitForResponse && (socketEvent.wait_all(socketMsgReceiveFlag, 5000) & osFlagsError))
    {
        streamer_printf(sout, "ERROR: socket message does not received\r\n");
        error = CHIP_ERROR_TIMEOUT;
    }

exit:
    if (sock.type == Type::kTcp && sock.tcpSocket)
    {
        sock.tcpSocket->Free();
    }
    else if (sock.type == Type::kUdp && sock.udpSocket)
    {
        sock.udpSocket->Free();
    }
    return error;
}

int cmd_socket_example(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    INET_ERROR err;
    TCPEndPoint * endPoint = nullptr;
    PacketBufferHandle buffer;
    const uint16_t port = 80;
    IPAddress IPaddr[5] = { IPAddress::Any };
    bool async_dummy    = false;
    char destAddrStr[64];

    const char hostname[] = "ifconfig.io";
    const char msg[]      = "GET / HTTP/1.1\r\n"
                       "Host: ifconfig.io\r\n"
                       "Connection: close\r\n"
                       "\r\n";
    uint16_t msgLen = sizeof(msg);

    streamer_t * sout = streamer_get();
    streamer_printf(sout, "TCP  test host : %s\r\n", hostname);

    TcpListenParameters params(&DeviceLayer::InetLayer);
    params.GetInetLayer()->ResolveHostAddress(hostname, 4, IPaddr, HandleDNSResolveComplete, &async_dummy);

    err = params.GetInetLayer()->NewTCPEndPoint(&endPoint);
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: Create socket failed\r\n");
        ExitNow(error = err;);
    }

    endPoint->OnDataSent        = OnTcpMessageSent;
    endPoint->OnDataReceived    = OnTcpMessageReceived;
    endPoint->OnConnectComplete = OnConnectionCompleted;

    socketEvent.clear();

    IPaddr[0].ToString(destAddrStr, sizeof(destAddrStr));

    streamer_printf(sout, "INFO: connect to TCP server address: %s port: %d\r\n", destAddrStr, port);
    err = endPoint->Connect(IPaddr[0], port, 0);
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: socket connect failed\r\n");
        ExitNow(error = err;);
    }

    if (socketEvent.wait_all(socketConnectionCompeletedFlag, 5000) & osFlagsError)
    {
        streamer_printf(sout, "ERROR: socket connection is not completed\r\n");
    }

    buffer = PacketBufferHandle::NewWithData(msg, msgLen);
    if (buffer.IsNull())
    {
        streamer_printf(sout, "ERROR: create message buffer failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    socketEvent.clear();

    streamer_printf(sout, "INFO: send HTTP message: \r\n%s", msg);
    error = endPoint->Send(std::move(buffer));
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: socket send failed\r\n");
        ExitNow(error = err;);
    }

    if (socketEvent.wait_all(socketMsgReceiveFlag, 5000) & osFlagsError)
    {
        streamer_printf(sout, "ERROR: socket message does not received\r\n");
        error = CHIP_ERROR_TIMEOUT;
    }

exit:
    if (endPoint != nullptr)
    {
        endPoint->Free();
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
    streamer_printf(sout, "\nResolve hostname %s\r\n", hostname);
    WiFiInterface * _net = WiFiInterface::get_default_instance();
    int result           = _net->gethostbyname(hostname, &address);
    if (result != 0)
    {
        streamer_printf(sout, "Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
        return false;
    }

    streamer_printf(sout, "%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None"));
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

    streamer_printf(sout, "\r\nSending message: \r\n%s", buff);

    while (bytes_to_send)
    {

        bytes_sent = mbed_send(fd, buff + bytes_sent, bytes_to_send, 0);
        if (bytes_sent < 0)
        {
            streamer_printf(sout, "Error! _socket.send() returned: %d\r\n", bytes_sent);
            return false;
        }
        else
        {
            streamer_printf(sout, "sent %d bytes\r\n", bytes_sent);
        }

        bytes_to_send -= bytes_sent;
    }

    streamer_printf(sout, "Complete message sent\r\n");

    char buf[100];
    int remaining_bytes = 100;
    int received_bytes  = 0;

    nsapi_size_or_error_t res = remaining_bytes;
    while (res > 0 && remaining_bytes > 0)
    {
        res = mbed_recv(fd, buf + received_bytes, remaining_bytes, 0);
        if (res < 0)
        {
            streamer_printf(sout, "Error! _socket.recv() returned: %d\r\n", res);
            return false;
        }

        received_bytes += res;
        remaining_bytes -= res;
    }

    streamer_printf(sout, "received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buf, "\n") - buf, buf);

    return error;
}

/* Server commands */
class ChipServer
{
public:
    ChipServer() {}
    ~ChipServer() {}

    enum
    {
        CHIP_SERVER_OFF = 0,
        CHIP_SERVER_ON
    };
    union
    {
        TransportMgr<UDP> * udp;
        TransportMgr<TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>> * tcp;
    };

    SecureSessionMgr * sessionManager;

    Type type     = Type::kUndefined;
    uint16_t port = CHIP_PORT;
    uint8_t state = CHIP_SERVER_OFF;

    void displayState()
    {
        InterfaceAddressIterator intIterator;
        char addrStr[16];
        streamer_t * sout = streamer_get();

        if (state == CHIP_SERVER_OFF)
        {
            streamer_printf(sout, "INFO: CHIP server is disabled\r\n");
        }
        else
        {
            streamer_printf(sout, "INFO: %s CHIP server is enabled and listen on port %d\r\n", type == Type::kUdp ? "UDP" : "TCP",
                            port);
            if (intIterator.IsUp())
            {
                streamer_printf(sout, "INFO: network interface IP address %s\r\n",
                                intIterator.GetAddress().ToString(addrStr, sizeof(addrStr)));
            }
            else
            {
                streamer_printf(sout, "WARN: Network interface is down\r\n");
            }
        }
    }
};

static ChipServer gChipServer;

int cmd_server_dispatch(int argc, char ** argv)
{
    if (0 == argc)
    {
        gChipServer.displayState();
        return CHIP_NO_ERROR;
    }

    return sShellServerSubcommands.ExecCommand(argc, argv);
}

int cmd_server_help(int argc, char ** argv)
{
    sShellServerSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
    return 0;
}

class ServerCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           const Transport::PeerAddress & source, System::PacketBufferHandle buffer,
                           SecureSessionMgr * mgr) override
    {
        char src_addr[PeerAddress::kMaxToStringSize];
        streamer_t * sout     = streamer_get();
        auto state            = mgr->GetPeerConnectionState(session);
        const size_t data_len = buffer->DataLength();

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));
        streamer_printf(sout, "INFO: received %d bytes from: %s\r\n", data_len, src_addr);
        streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                        strstr((char *) buffer->Start(), "\n") - (char *) buffer->Start(), (char *) buffer->Start());
    }

    void OnReceiveError(CHIP_ERROR error, const PeerAddress & source, SecureSessionMgr * mgr) override
    {
        streamer_printf(streamer_get(), "ERROR: packet received error: %s\r\n", ErrorStr(error));
    }

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override
    {
        streamer_printf(streamer_get(), "INFO: Received a new connection\r\n");
    }
};

static ServerCallback gCallbacks;
static SecurePairingUsingTestSecret gTestPairing;
static AdminPairingTable gAdmin;

int cmd_server_on(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    INET_ERROR err;
    const AdminId gAdminId       = 0;
    AdminPairingInfo * adminInfo = nullptr;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (gChipServer.state != ChipServer::CHIP_SERVER_OFF)
    {
        streamer_printf(sout, "ERROR: CHIP server already enabled\r\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (strcmp(argv[0], "UDP") == 0)
    {
        gChipServer.type = Type::kUdp;
    }
    else if (strcmp(argv[0], "TCP") == 0)
    {
        gChipServer.type = Type::kTcp;
    }
    else
    {
        streamer_printf(sout, "ERROR: wrong command arguments. Check socket help\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    if (argc > 1)
    {
        gChipServer.port = atoi(argv[1]);
    }

    adminInfo = gAdmin.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    gChipServer.sessionManager = new SecureSessionMgr();
    if (gChipServer.sessionManager == nullptr)
    {
        streamer_printf(sout, "ERROR: Sesion manager create failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    if (gChipServer.type == Type::kUdp)
    {
        gChipServer.udp = new TransportMgr<UDP>();
        if (gChipServer.udp == nullptr)
        {
            streamer_printf(sout, "ERROR: UDP manager create failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        err = gChipServer.udp->Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                                        .SetAddressType(Inet::kIPAddressType_IPv4)
                                        .SetListenPort(gChipServer.port));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: UDP manager intialization failed\r\n");
            ExitNow(error = err;);
        }

        err = gChipServer.sessionManager->Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, gChipServer.udp, &gAdmin);
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Session manager intialization failed\r\n");
            ExitNow(error = err;);
        }
    }
    else
    {
        gChipServer.tcp = new TransportMgr<TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>>();
        if (gChipServer.tcp == nullptr)
        {
            streamer_printf(sout, "ERROR: TCP manager create failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        err = gChipServer.tcp->Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                                        .SetAddressType(Inet::kIPAddressType_IPv4)
                                        .SetListenPort(gChipServer.port));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: TCP manager intialization failed\r\n");
            ExitNow(error = err;);
        }

        err = gChipServer.sessionManager->Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, gChipServer.tcp, &gAdmin);
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Session manager intialization failed\r\n");
            ExitNow(error = err;);
        }
    }

    gChipServer.sessionManager->SetDelegate(&gCallbacks);

    err = gChipServer.sessionManager->NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing,
                                                 chip::SecureSessionMgr::PairingDirection::kResponder, gAdminId);
    if (err != INET_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: set new pairing failed\r\n");
        ExitNow(error = err;);
    }

    gChipServer.state = ChipServer::CHIP_SERVER_ON;

    gChipServer.displayState();

exit:
    if (error != CHIP_NO_ERROR)
    {
        if (gChipServer.sessionManager)
        {
            delete gChipServer.sessionManager;
        }

        if (gChipServer.type == Type::kUdp && gChipServer.udp)
        {
            delete gChipServer.udp;
        }

        if (gChipServer.type == Type::kTcp && gChipServer.tcp)
        {
            delete gChipServer.tcp;
        }
    }
    return error;
}

int cmd_server_off(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();

    if (gChipServer.state != ChipServer::CHIP_SERVER_ON)
    {
        streamer_printf(sout, "ERROR: CHIP server already disabled\r\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (gChipServer.sessionManager)
    {
        delete gChipServer.sessionManager;
    }

    if (gChipServer.type == Type::kUdp && gChipServer.udp)
    {
        delete gChipServer.udp;
    }

    if (gChipServer.type == Type::kTcp && gChipServer.tcp)
    {
        delete gChipServer.tcp;
    }

    gChipServer.state = ChipServer::CHIP_SERVER_OFF;

    gChipServer.type = Type::kUndefined;
    gChipServer.port = CHIP_PORT;

    gChipServer.displayState();

    return CHIP_NO_ERROR;
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
    { &cmd_socket_client, "client",
      "Create client and send test message to server via specific socket.\n"
      "\tUsage: socket client <type>[UDP/TCP] <ip> <port> <message> <wait for response flag>[wait - optional]\n"
      "\tExample: socket client TCP 127.0.0.1 7 Hello wait" },
    { &cmd_socket_bsd, "bsd",
      "BSD IP communication test via specific socket. Send message in loopback.\n"
      "\tUsage: socket echo <type> <message>" },
    { &cmd_socket_example, "example",
      "Socket example which sends HTTP request to ifconfig.io and receives a response.\n"
      "\tUsage: socket example" },
    { &cmd_socket_help, "help", "Display help for each socket subcommands" }
};

static const shell_command_t cmds_server_root = { &cmd_server_dispatch, "server", "Enable/disable CHIP server" };

static const shell_command_t cmds_server[] = { { &cmd_server_on, "on",
                                                 "Create CHIP server for communication testing.\n"
                                                 "\tUsage: server on <type> <port>[optional - default 11097]\n"
                                                 "\tExample: server on UDP 7" },
                                               { &cmd_server_off, "off",
                                                 "Shutdown current CHIP server instance.\n"
                                                 "\tUsage: server off" },
                                               { &cmd_server_help, "help", "Display help for each server subcommands" } };

void cmd_mbed_utils_init()
{
    sShellDateSubcommands.RegisterCommands(cmds_date, ArraySize(cmds_date));
    sShellNetworkSubcommands.RegisterCommands(cmds_network, ArraySize(cmds_network));
    sShellSocketSubcommands.RegisterCommands(cmds_socket, ArraySize(cmds_socket));
    sShellServerSubcommands.RegisterCommands(cmds_server, ArraySize(cmds_server));
    shell_register(&cmds_date_root, 1);
    shell_register(&cmds_test_config, 1);
    shell_register(&cmds_network_root, 1);
    shell_register(&cmds_socket_root, 1);
    shell_register(&cmds_server_root, 1);
}
