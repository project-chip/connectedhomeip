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

#include <inttypes.h>
#include <stdarg.h>

#include "ChipShellMbedCollection.h"

#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCP.h>
#include <transport/raw/UDP.h>

#include <messaging/ExchangeMgr.h>

#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>

#include <rtos/EventFlags.h>

#include <platform/mbed/MbedConfig.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::secure_channel;
using namespace chip::Protocols::Echo;
using namespace chip::System::Platform::Clock;
using namespace mbed;
using namespace rtos;

static Engine sShellDateSubcommands;
static Engine sShellNetworkSubcommands;
static Engine sShellServerSubcommands;

constexpr size_t kMaxTcpActiveConnectionCount  = 2;
constexpr size_t kMaxTcpPendingPackets         = 2;
constexpr uint32_t gResponseReceivedTimeoutlMs = 5000;

CHIP_ERROR cmd_common_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\r\n", command->cmd_name, command->cmd_help);
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_date_help(int argc, char ** argv)
{
    sShellDateSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_date_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint16_t year;
    uint8_t month, dayOfMonth;
    uint8_t hour, minute, second;

    uint64_t currTimeMS = 0;

    streamer_t * sout = streamer_get();

    if (0 == argc)
    {
        error = GetUnixTimeMicroseconds(currTimeMS);
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Get unix time ms failed\r\n");
            return error;
        }

        SecondsSinceUnixEpochToCalendarTime(currTimeMS / kMicrosecondsPerSecond, year, month, dayOfMonth, hour, minute, second);
        streamer_printf(sout, "%04" PRIu16 "-%02" PRIu8 "-%02" PRIu8 " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8 "\r\n", year, month,
                        dayOfMonth, hour, minute, second);
        return CHIP_NO_ERROR;
    }

    return sShellDateSubcommands.ExecCommand(argc, argv);
}

CHIP_ERROR cmd_date_set(int argc, char ** argv)
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

    if (!CalendarTimeToSecondsSinceUnixEpoch(year, month, dayOfMonth, hour, minute, seconds, newCurrTime))
    {
        streamer_printf(sout, "ERROR: Wrong date and/or time values\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    error = SetUnixTimeMicroseconds(static_cast<uint64_t>(newCurrTime) * static_cast<uint64_t>(kMicrosecondsPerSecond));

exit:
    return error;
}

CHIP_ERROR cmd_device_test_config(int argc, char ** argv)
{
    chip::DeviceLayer::Internal::MbedConfig::RunConfigUnitTest();
    return CHIP_NO_ERROR;
}

/* Network commands */
CHIP_ERROR cmd_network_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellNetworkSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

CHIP_ERROR cmd_network_help(int argc, char ** argv)
{
    sShellNetworkSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_network_interface(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    InterfaceAddressIterator addrIterator;
    IPAddress addr;
    IPPrefix addrWithPrefix;
    char intName[IF_NAMESIZE];
    InterfaceId intId;
    uint8_t intCounter = 0;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(sout, "    Current interface:\r\n");
    for (; addrIterator.HasCurrent(); addrIterator.Next())
    {
        intId = addrIterator.GetInterface();
        if (intId == INET_NULL_INTERFACEID)
        {
            streamer_printf(sout, "ERROR: get interface failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }

        error = addrIterator.GetInterfaceName(intName, sizeof(intName));
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: get interface name failed\r\n");
            ExitNow();
        }
        streamer_printf(sout, "     interface id: %d, interface name: %s, interface state: %s\r\n", intId, intName,
                        addrIterator.IsUp() ? "UP" : "DOWN");

        addr = addrIterator.GetAddress();
        addrIterator.GetAddressWithPrefix(addrWithPrefix);
        char addrStr[80];
        addrWithPrefix.IPAddr.ToString(addrStr, sizeof(addrStr));
        streamer_printf(sout, "     interface address: %s/%d, %s broadcast addr, support multicast %s\r\n", addrStr,
                        addrWithPrefix.Length, addrIterator.HasBroadcastAddress() ? "has" : "no",
                        addrIterator.SupportsMulticast() ? "yes" : "no");

        intCounter++;
    }

    if (intCounter == 0)
    {
        streamer_printf(sout, "    no interface is available\r\n");
    }

exit:
    return error;
}

CHIP_ERROR cmd_network_idToName(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char intName[IF_NAMESIZE];
    InterfaceId intId;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    intId = (InterfaceId) atoi(argv[0]);
    if (intId == INET_NULL_INTERFACEID)
    {
        streamer_printf(sout, "ERROR: wrong interface ID\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    error = GetInterfaceName(intId, intName, sizeof(intName));
    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: get interface name failed\r\n");
        ExitNow();
    }

    streamer_printf(sout, "Name of %d interface: %s\r\n", intId, intName);

exit:
    return error;
}

CHIP_ERROR cmd_network_nameToId(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char intName[IF_NAMESIZE];
    InterfaceId intId;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    strncpy(intName, argv[0], IF_NAMESIZE);

    error = InterfaceNameToId(intName, intId);
    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: get interface ID failed\r\n");
        ExitNow();
    }

    if (intId == INET_NULL_INTERFACEID)
    {
        streamer_printf(sout, "ERROR: wrong interface name\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    streamer_printf(sout, "Interface %s has ID: %d\r\n", intName, intId);

exit:
    return error;
}

struct ChipClient
{
    ChipClient() {}
    ~ChipClient() {}

    union
    {
        TCPEndPoint * tcp;
        UDPEndPoint * udp;
    };
    Type type;
    PacketBufferHandle buffer;
    bool response;

    void Free()
    {

        if (type == Type::kTcp && tcp != nullptr)
        {
            tcp->Free();
            tcp = nullptr;
        }
        else if (type == Type::kUdp && udp != nullptr)
        {
            udp->Free();
            udp = nullptr;
        }

        type     = Type::kUndefined;
        response = false;
        buffer   = nullptr;
    }
};

static ChipClient gChipClient;

void HandleResponseTimerComplete(System::Layer * aSystemLayer, void * aAppState)
{
    streamer_printf(streamer_get(), "ERROR: Received client response failed\r\n");
    gChipClient.Free();
}

void HandleDNSResolveComplete(void * appState, CHIP_ERROR error, uint8_t addrCount, IPAddress * addrArray)
{
    if (addrCount > 0)
    {
        char destAddrStr[64];
        for (int i = 0; i < addrCount; i++)
        {
            addrArray[i].ToString(destAddrStr, sizeof(destAddrStr));
            streamer_printf(streamer_get(), "INFO: %d DNS name resolution complete: %s\r\n", i, destAddrStr);
        }
    }
    else
    {
        streamer_printf(streamer_get(), "ERROR: DNS name resolution return no addresses\r\n");
    }
}

CHIP_ERROR OnTcpMessageReceived(TCPEndPoint * endPoint, System::PacketBufferHandle && buffer)
{
    DeviceLayer::SystemLayer.CancelTimer(HandleResponseTimerComplete, nullptr);

    streamer_t * sout                  = streamer_get();
    System::PacketBufferHandle message = buffer.PopHead();
    streamer_printf(sout, "INFO: TCP message received\r\n");
    streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                    strstr((char *) message->Start(), "\n") - (char *) message->Start(), (char *) message->Start());
    gChipClient.Free();
    return CHIP_NO_ERROR;
}

void OnTcpConnectionCompleted(TCPEndPoint * endPoint, CHIP_ERROR error)
{
    streamer_t * sout = streamer_get();

    if (error == CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "INFO: TCP connection completed\r\n");
        streamer_printf(sout, "INFO: TCP client send message\r\n");
        auto err = endPoint->Send(std::move(gChipClient.buffer));
        if (err != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: TCP client send failed\r\n");
            gChipClient.Free();
        }

        if (gChipClient.response)
        {
            DeviceLayer::SystemLayer.StartTimer(gResponseReceivedTimeoutlMs, HandleResponseTimerComplete, nullptr);
        }
        else
        {
            gChipClient.Free();
        }
    }
    else
    {
        streamer_printf(streamer_get(), "ERROR: TCP client connection failed\r\n");
        gChipClient.Free();
    }
}

void OnUdpMessageReceived(IPEndPointBasis * endPoint, System::PacketBufferHandle && buffer, const IPPacketInfo * pktInfo)
{
    DeviceLayer::SystemLayer.CancelTimer(HandleResponseTimerComplete, nullptr);

    char peerAddrStr[PeerAddress::kMaxToStringSize];
    streamer_t * sout       = streamer_get();
    PeerAddress peerAddress = PeerAddress::UDP(pktInfo->SrcAddress, pktInfo->SrcPort);
    peerAddress.ToString(peerAddrStr, sizeof(peerAddrStr));
    System::PacketBufferHandle message = buffer.PopHead();

    streamer_printf(sout, "INFO: UDP message received from %s\r\n", peerAddrStr);
    streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                    strstr((char *) message->Start(), "\n") - (char *) message->Start(), (char *) message->Start());
    gChipClient.Free();
}

void OnUdpMReceiveError(IPEndPointBasis * endPoint, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    DeviceLayer::SystemLayer.CancelTimer(HandleResponseTimerComplete, nullptr);

    streamer_printf(streamer_get(), "ERROR: UDP receive failed\r\n");
    gChipClient.Free();
}

static CHIP_ERROR network_client_parse_args(char ** argv, Type & type, IPAddress & addr, uint16_t & port, char ** msg)
{
    if (strcmp(argv[0], "UDP") == 0)
    {
        type = Type::kUdp;
    }
    else if (strcmp(argv[0], "TCP") == 0)
    {
        type = Type::kTcp;
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

CHIP_ERROR cmd_network_client(int argc, char ** argv)
{
    InterfaceIterator networkInterface;
    CHIP_ERROR error = CHIP_NO_ERROR;
    char * msg;
    uint16_t msgLen;
    uint16_t port;
    char addrStr[16];
    Type type;
    IPAddress addr;

    streamer_t * sout = streamer_get();

    if (!networkInterface.IsUp())
    {
        streamer_printf(sout, "ERROR: no network connection\r\n");
        ExitNow(error = CHIP_ERROR_NOT_CONNECTED;);
    }

    VerifyOrExit(argc > 3, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = network_client_parse_args(argv, type, addr, port, &msg);
    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: wrong command arguments. Check network help\r\n");
        ExitNow();
    }

    gChipClient.Free();

    if ((argc == 5) && (strcmp(argv[4], "-r") == 0))
    {
        gChipClient.response = true;
    }

    msgLen = strlen(msg);

    gChipClient.buffer = PacketBufferHandle::NewWithData(msg, msgLen);
    if (gChipClient.buffer.IsNull())
    {
        streamer_printf(sout, "ERROR: create message buffer failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }

    gChipClient.type = type;

    if (type == Type::kTcp)
    {
        error = DeviceLayer::InetLayer.NewTCPEndPoint(&gChipClient.tcp);
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Create TCP endpoint failed\r\n");
            ExitNow();
        }

        gChipClient.tcp->OnConnectComplete = OnTcpConnectionCompleted;
        if (gChipClient.response)
        {
            gChipClient.tcp->OnDataReceived = OnTcpMessageReceived;
        }

        streamer_printf(sout, "INFO: connect to TCP server address: %s port: %d\r\n", addr.ToString(addrStr, sizeof(addrStr)),
                        port);
        error = gChipClient.tcp->Connect(addr, port, 0);
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: TCP client connect failed\r\n");
            ExitNow();
        }
    }
    else
    {
        error = DeviceLayer::InetLayer.NewUDPEndPoint(&gChipClient.udp);
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Create UDP endpoint failed\r\n");
            ExitNow();
        }

        error = gChipClient.udp->Bind(IPAddressType::kIPAddressType_IPv4, Inet::IPAddress::Any, CHIP_PORT,
                                      networkInterface.GetInterfaceId());
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: UDP endpoint bind failed\r\n");
            ExitNow();
        }

        if (gChipClient.response)
        {
            error = gChipClient.udp->Listen(OnUdpMessageReceived, OnUdpMReceiveError, nullptr);
            if (error != CHIP_NO_ERROR)
            {
                streamer_printf(sout, "ERROR: UDP endpoint listen failed\r\n");
                ExitNow();
            }
        }

        error = gChipClient.udp->SendTo(addr, port, std::move(gChipClient.buffer));
        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: UDP client send failed\r\n");
            ExitNow();
        }

        if (gChipClient.response)
        {
            DeviceLayer::SystemLayer.StartTimer(gResponseReceivedTimeoutlMs, HandleResponseTimerComplete, nullptr);
        }
        else
        {
            gChipClient.Free();
        }
    }

exit:
    if (error != CHIP_NO_ERROR)
    {
        gChipClient.Free();
    }

    return error;
}

CHIP_ERROR cmd_network_example(int argc, char ** argv)
{
    InterfaceIterator networkInterface;
    CHIP_ERROR error               = CHIP_NO_ERROR;
    const size_t maxIPAddress      = 5;
    const uint16_t port            = 80;
    IPAddress IPaddr[maxIPAddress] = { IPAddress::Any };
    char destAddrStr[64];

    const char hostname[] = "ifconfig.io";
    const char msg[]      = "GET / HTTP/1.1\r\n"
                       "Host: ifconfig.io\r\n"
                       "Connection: close\r\n"
                       "\r\n";
    uint16_t msgLen = sizeof(msg);

    streamer_t * sout = streamer_get();

    if (!networkInterface.IsUp())
    {
        streamer_printf(sout, "ERROR: no network connection\r\n");
        ExitNow(error = CHIP_ERROR_NOT_CONNECTED;);
    }

    streamer_printf(sout, "TCP test host : %s\r\n", hostname);

    DeviceLayer::InetLayer.ResolveHostAddress(hostname, maxIPAddress, IPaddr, HandleDNSResolveComplete, nullptr);

    gChipClient.Free();
    error = DeviceLayer::InetLayer.NewTCPEndPoint(&gChipClient.tcp);
    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: Create TCP endpoint failed\r\n");
        ExitNow();
    }

    gChipClient.type                   = Type::kTcp;
    gChipClient.tcp->OnDataReceived    = OnTcpMessageReceived;
    gChipClient.tcp->OnConnectComplete = OnTcpConnectionCompleted;
    gChipClient.buffer                 = PacketBufferHandle::NewWithData(msg, msgLen);
    if (gChipClient.buffer.IsNull())
    {
        streamer_printf(sout, "ERROR: create message buffer failed\r\n");
        ExitNow(error = CHIP_ERROR_INTERNAL;);
    }
    gChipClient.response = true;

    IPaddr[0].ToString(destAddrStr, sizeof(destAddrStr));

    streamer_printf(sout, "INFO: connect to TCP server address: %s port: %d\r\n", destAddrStr, port);
    error = gChipClient.tcp->Connect(IPaddr[0], port, 0);
    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(sout, "ERROR: TCP client connect failed\r\n");
        ExitNow();
    }

exit:
    if (error != CHIP_NO_ERROR)
    {
        gChipClient.Free();
    }

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

    EchoServer echoServer;
    SecurePairingUsingTestSecret testPairing;
    SecureSessionMgr sessionManager;
    Messaging::ExchangeManager exchangeManager;
    MessageCounterManager messageCounterManager;

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

CHIP_ERROR cmd_server_dispatch(int argc, char ** argv)
{
    if (0 == argc)
    {
        gChipServer.displayState();
        return CHIP_NO_ERROR;
    }

    return sShellServerSubcommands.ExecCommand(argc, argv);
}

CHIP_ERROR cmd_server_help(int argc, char ** argv)
{
    sShellServerSubcommands.ForEachCommand(cmd_common_help_iterator, nullptr);
    return CHIP_NO_ERROR;
}

// Callback handler when a CHIP EchoRequest is received.
void HandleEchoRequestReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle && payload)
{
    streamer_t * sout = streamer_get();

    char src_addr[PeerAddress::kMaxToStringSize];
    // auto state            = mgr->GetPeerConnectionState(session);
    const size_t data_len = payload->DataLength();

    // state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

    streamer_printf(sout, "INFO: received %d bytes\r\n", data_len);
    streamer_printf(sout, "INFO: received message: \r\n%.*s\r\n\r\n",
                    strstr((char *) payload->Start(), "\n") - (char *) payload->Start(), (char *) payload->Start());
}

CHIP_ERROR cmd_server_on(int argc, char ** argv)
{
    InterfaceIterator networkInterface;
    CHIP_ERROR error = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);
    FabricTable fabrics;
    const chip::FabricIndex gFabricIndex = 0;

    streamer_t * sout = streamer_get();

    if (!networkInterface.IsUp())
    {
        streamer_printf(sout, "ERROR: no network connection\r\n");
        ExitNow(error = CHIP_ERROR_NOT_CONNECTED;);
    }

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
        streamer_printf(sout, "ERROR: wrong command arguments. Check network help\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    if (argc > 1)
    {
        gChipServer.port = atoi(argv[1]);
    }

    if (gChipServer.type == Type::kUdp)
    {
        gChipServer.udp = new TransportMgr<UDP>();
        if (gChipServer.udp == nullptr)
        {
            streamer_printf(sout, "ERROR: UDP manager create failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        error = gChipServer.udp->Init(Transport::UdpListenParameters(&DeviceLayer::InetLayer)
                                          .SetAddressType(Inet::kIPAddressType_IPv4)
                                          .SetListenPort(gChipServer.port));
        SuccessOrExit(error);

        error = gChipServer.sessionManager.Init(&DeviceLayer::SystemLayer, gChipServer.udp, &fabrics,
                                                &gChipServer.messageCounterManager);
        SuccessOrExit(error);
    }
    else
    {
        gChipServer.tcp = new TransportMgr<TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>>();
        if (gChipServer.tcp == nullptr)
        {
            streamer_printf(sout, "ERROR: TCP manager create failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        error = gChipServer.tcp->Init(Transport::TcpListenParameters(&DeviceLayer::InetLayer)
                                          .SetAddressType(Inet::kIPAddressType_IPv4)
                                          .SetListenPort(gChipServer.port));
        SuccessOrExit(error);

        error = gChipServer.sessionManager.Init(&DeviceLayer::SystemLayer, gChipServer.tcp, &fabrics,
                                                &gChipServer.messageCounterManager);
        SuccessOrExit(error);
    }

    error = gChipServer.exchangeManager.Init(&gChipServer.sessionManager);
    SuccessOrExit(error);

    error = gChipServer.messageCounterManager.Init(&gChipServer.exchangeManager);
    SuccessOrExit(error);

    error = gChipServer.echoServer.Init(&gChipServer.exchangeManager);
    SuccessOrExit(error);

    error = gChipServer.sessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gChipServer.testPairing,
                                                  chip::SecureSession::SessionRole::kResponder, gFabricIndex);
    SuccessOrExit(error);

    gChipServer.echoServer.SetEchoRequestReceived(HandleEchoRequestReceived);

    gChipServer.state = ChipServer::CHIP_SERVER_ON;

    gChipServer.displayState();

exit:
    if (error != CHIP_NO_ERROR)
    {
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

CHIP_ERROR cmd_server_off(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();

    if (gChipServer.state != ChipServer::CHIP_SERVER_ON)
    {
        streamer_printf(sout, "ERROR: CHIP server already disabled\r\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gChipServer.echoServer.Shutdown();
    gChipServer.messageCounterManager.Shutdown();
    gChipServer.exchangeManager.Shutdown();
    gChipServer.sessionManager.Shutdown();

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

static const shell_command_t cmds_network_root = { &cmd_network_dispatch, "network", "Network commands" };

static const shell_command_t cmds_network[] = {
    { &cmd_network_interface, "interface", "Display current network interface details" },
    { &cmd_network_idToName, "idToName", "Display interface name by id" },
    { &cmd_network_nameToId, "nameToId", "Display interface id by name" },
    { &cmd_network_client, "client",
      "Create client and send test message to server via specific endpoint.\n"
      "\tUsage: network client <type>[UDP/TCP] <ip> <port> <message> <response flag>[optional]\n"
      "\tExample: network client TCP 127.0.0.1 7 Hello -r" },
    { &cmd_network_example, "example",
      "Network communication example which sends HTTP request to ifconfig.io and receives a response.\n"
      "\tUsage: network example" },
    { &cmd_network_help, "help", "Display help for each network subcommands" }
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

void cmd_mbed_init()
{
    sShellDateSubcommands.RegisterCommands(cmds_date, ArraySize(cmds_date));
    sShellNetworkSubcommands.RegisterCommands(cmds_network, ArraySize(cmds_network));
    sShellServerSubcommands.RegisterCommands(cmds_server, ArraySize(cmds_server));
    Engine::Root().RegisterCommands(&cmds_date_root, 1);
    Engine::Root().RegisterCommands(&cmds_test_config, 1);
    Engine::Root().RegisterCommands(&cmds_network_root, 1);
    Engine::Root().RegisterCommands(&cmds_server_root, 1);
}
