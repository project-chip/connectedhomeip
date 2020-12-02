/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file implements a process to effect a functional test for
 *      the InetLayer Internet Protocol stack abstraction interfaces.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <type_traits>
#include <unistd.h>

#include <CHIPVersion.h>

#include <support/CodeUtils.h>

#include <system/SystemTimer.h>

#include "TestInetCommon.h"
#include "TestInetLayerCommon.hpp"

using namespace chip;
using namespace chip::Inet;
using namespace chip::ArgParser;
using namespace chip::System;

/* Preprocessor Macros */

#define kToolName "TestInetLayer"

#define kToolOptTCPIP 't'

#define kToolOptExpectedRxSize (kToolOptBase + 0)
#define kToolOptExpectedTxSize (kToolOptBase + 1)

/* Type Definitions */

enum OptFlags
{
    kOptFlagExpectedRxSize = 0x00010000,
    kOptFlagExpectedTxSize = 0x00020000,

    kOptFlagUseTCPIP = 0x00040000
};

struct TestState
{
    TransferStats mStats;
    TestStatus mStatus;
};

/* Function Declarations */

static void HandleSignal(int aSignal);
static bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);
static bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[]);

static void StartTest();
static void CleanupTest();

/* Global Variables */

static const uint32_t kExpectedRxSizeDefault = 1523;
static const uint32_t kExpectedTxSizeDefault = kExpectedRxSizeDefault;

static const uint32_t kOptFlagsDefault = (kOptFlagUseIPv6 | kOptFlagUseUDPIP);

static RawEndPoint * sRawIPEndPoint       = nullptr;
static TCPEndPoint * sTCPIPEndPoint       = nullptr; // Used for connect/send/receive
static TCPEndPoint * sTCPIPListenEndPoint = nullptr; // Used for accept/listen
static UDPEndPoint * sUDPIPEndPoint       = nullptr;

static const uint16_t kTCPPort = kUDPPort;
// clang-format off
static TestState         sTestState             =
{
    { { 0, 0 }, { 0, 0 } },
    { false, false }
};
// clang-format on

static IPAddress sDestinationAddress   = IPAddress::Any;
static const char * sDestinationString = nullptr;

// clang-format off
static OptionDef         sToolOptionDefs[] =
{
    { "interface",                 kArgumentRequired,  kToolOptInterface              },
    { "expected-rx-size",          kArgumentRequired,  kToolOptExpectedRxSize         },
    { "expected-tx-size",          kArgumentRequired,  kToolOptExpectedTxSize         },
    { "interval",                  kArgumentRequired,  kToolOptInterval               },
#if INET_CONFIG_ENABLE_IPV4
    { "ipv4",                      kNoArgument,        kToolOptIPv4Only               },
#endif // INET_CONFIG_ENABLE_IPV4
    { "ipv6",                      kNoArgument,        kToolOptIPv6Only               },
    { "listen",                    kNoArgument,        kToolOptListen                 },
    { "raw",                       kNoArgument,        kToolOptRawIP                  },
    { "send-size",                 kArgumentRequired,  kToolOptSendSize               },
    { "tcp",                       kNoArgument,        kToolOptTCPIP                  },
    { "udp",                       kNoArgument,        kToolOptUDPIP                  },
    { }
};

static const char *      sToolOptionHelp =
    "  -I, --interface <interface>\n"
    "       The network interface to bind to and from which to send and receive all packets.\n"
    "\n"
    "  --expected-rx-size <size>\n"
    "       Expect to receive size bytes of user data (default 1523).\n"
    "\n"
    "  --expected-tx-size <size>\n"
    "       Expect to send size bytes of user data (default 1523).\n"
    "\n"
    "  -i, --interval <interval>\n"
    "       Wait interval milliseconds between sending each packet (default: 1000 ms).\n"
    "\n"
    "  -l, --listen\n"
    "       Act as a server (i.e., listen) for packets rather than send them.\n"
    "\n"
#if INET_CONFIG_ENABLE_IPV4
    "  -4, --ipv4\n"
    "       Use IPv4 only.\n"
    "\n"
#endif // INET_CONFIG_ENABLE_IPV4
    "  -6, --ipv6\n"
    "       Use IPv6 only (default).\n"
    "\n"
    "  -s, --send-size <size>\n"
    "       Send size bytes of user data (default: 59 bytes)\n"
    "\n"
    "  -r, --raw\n"
    "       Use raw IP (default).\n"
    "\n"
    "  -t, --tcp\n"
    "       Use TCP over IP.\n"
    "\n"
    "  -u, --udp\n"
    "       Use UDP over IP (default).\n"
    "\n";

static OptionSet         sToolOptions =
{
    HandleOption,
    sToolOptionDefs,
    "GENERAL OPTIONS",
    sToolOptionHelp
};

static HelpOptions       sHelpOptions(
    kToolName,
    "Usage: " kToolName " [ <options> ] <dest-node-addr>\n"
    "       " kToolName " [ <options> ] --listen\n",
    CHIP_VERSION_STRING "\n" CHIP_TOOL_COPYRIGHT
);

static OptionSet *       sToolOptionSets[] =
{
    &sToolOptions,
    &gNetworkOptions,
    &gFaultInjectionOptions,
    &sHelpOptions,
    nullptr
};
// clang-format on

static void CheckSucceededOrFailed(TestState & aTestState, bool & aOutSucceeded, bool & aOutFailed)
{
    const TransferStats & lStats = aTestState.mStats;

#if DEBUG
    printf("%u/%u sent, %u/%u received\n", lStats.mTransmit.mActual, lStats.mTransmit.mExpected, lStats.mReceive.mActual,
           lStats.mReceive.mExpected);
#endif

    if (((lStats.mTransmit.mExpected > 0) && (lStats.mTransmit.mActual > lStats.mTransmit.mExpected)) ||
        ((lStats.mReceive.mExpected > 0) && (lStats.mReceive.mActual > lStats.mReceive.mExpected)))
    {
        aOutFailed = true;
    }
    else if (((lStats.mTransmit.mExpected > 0) && (lStats.mTransmit.mActual < lStats.mTransmit.mExpected)) ||
             ((lStats.mReceive.mExpected > 0) && (lStats.mReceive.mActual < lStats.mReceive.mExpected)))
    {
        aOutSucceeded = false;
    }

    if (aOutSucceeded || aOutFailed)
    {
        if (aOutSucceeded)
            aTestState.mStatus.mSucceeded = true;

        if (aOutFailed)
            SetStatusFailed(aTestState.mStatus);
    }
}

static void HandleSignal(int aSignal)
{
    switch (aSignal)
    {

    case SIGUSR1:
        SetStatusFailed(sTestState.mStatus);
        break;
    }
}

int main(int argc, char * argv[])
{
    bool lSuccessful = true;
    INET_ERROR lStatus;

    InitTestInetCommon();

    SetupFaultInjectionContext(argc, argv);

    SetSignalHandler(HandleSignal);

    if (argc == 1)
    {
        sHelpOptions.PrintBriefUsage(stderr);
        lSuccessful = false;
        goto exit;
    }

    if (!ParseArgsFromEnvVar(kToolName, TOOL_OPTIONS_ENV_VAR_NAME, sToolOptionSets, nullptr, true) ||
        !ParseArgs(kToolName, argc, argv, sToolOptionSets, HandleNonOptionArgs))
    {
        lSuccessful = false;
        goto exit;
    }

    InitSystemLayer();

    InitNetwork();

    // At this point, we should have valid network interfaces,
    // including LwIP TUN/TAP shim interfaces. Validate the
    // -I/--interface argument, if present.

    if (gInterfaceName != nullptr)
    {
        lStatus = InterfaceNameToId(gInterfaceName, gInterfaceId);
        if (lStatus != INET_NO_ERROR)
        {
            PrintArgError("%s: unknown network interface %s\n", kToolName, gInterfaceName);
            lSuccessful = false;
            goto shutdown;
        }
    }

    StartTest();

    while (Common::IsTesting(sTestState.mStatus))
    {
        struct timeval sleepTime;
        bool lSucceeded = true;
        bool lFailed    = false;

        sleepTime.tv_sec  = 0;
        sleepTime.tv_usec = 10000;

        ServiceNetwork(sleepTime);

        CheckSucceededOrFailed(sTestState, lSucceeded, lFailed);

#if DEBUG
        // clang-format off
        printf("%s %s number of expected bytes\n",
               ((lSucceeded) ? "successfully" :
                ((lFailed) ? "failed to" :
                 "has not yet")),
               ((lSucceeded) ? (Common::IsReceiver() ? "received" : "sent") :
                ((lFailed) ? (Common::IsReceiver() ? "receive" : "send") :
                 Common::IsReceiver() ? "received" : "sent"))
               );
        // clang-format on
#endif
    }

    CleanupTest();

shutdown:
    ShutdownNetwork();
    ShutdownSystemLayer();

    lSuccessful = Common::WasSuccessful(sTestState.mStatus);

exit:
    return (lSuccessful ? EXIT_SUCCESS : EXIT_FAILURE);
}

static bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {

    case kToolOptInterval:
        if (!ParseInt(aValue, gSendIntervalMs))
        {
            PrintArgError("%s: invalid value specified for send interval: %s\n", aProgram, aValue);
            retval = false;
        }
        break;

    case kToolOptListen:
        gOptFlags |= kOptFlagListen;
        break;

    case kToolOptExpectedRxSize:
        if (!ParseInt(aValue, sTestState.mStats.mReceive.mExpected) || sTestState.mStats.mReceive.mExpected > UINT32_MAX)
        {
            PrintArgError("%s: Invalid value specified for max receive: %s\n", aProgram, aValue);
            retval = false;
        }
        gOptFlags |= kOptFlagExpectedRxSize;
        break;

    case kToolOptExpectedTxSize:
        if (!ParseInt(aValue, sTestState.mStats.mTransmit.mExpected) || sTestState.mStats.mTransmit.mExpected > UINT32_MAX)
        {
            PrintArgError("%s: Invalid value specified for max send: %s\n", aProgram, aValue);
            retval = false;
        }
        gOptFlags |= kOptFlagExpectedTxSize;
        break;

#if INET_CONFIG_ENABLE_IPV4
    case kToolOptIPv4Only:
        if (gOptFlags & kOptFlagUseIPv6)
        {
            PrintArgError("%s: the use of --ipv4 is exclusive with --ipv6. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        gOptFlags |= kOptFlagUseIPv4;
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kToolOptIPv6Only:
        if (gOptFlags & kOptFlagUseIPv4)
        {
            PrintArgError("%s: the use of --ipv6 is exclusive with --ipv4. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        gOptFlags |= kOptFlagUseIPv6;
        break;

    case kToolOptInterface:

        // NOTE: When using LwIP on a hosted OS, the interface will
        // not actually be available until AFTER InitNetwork,
        // consequently, we cannot do any meaningful validation
        // here. Simply save the value off and we will validate it
        // later.

        gInterfaceName = aValue;
        break;

    case kToolOptRawIP:
        if (gOptFlags & kOptFlagUseUDPIP)
        {
            PrintArgError("%s: the use of --raw is exclusive with --udp. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        else if (gOptFlags & kOptFlagUseTCPIP)
        {
            PrintArgError("%s: the use of --raw is exclusive with --tcp. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        gOptFlags |= kOptFlagUseRawIP;
        break;

    case kToolOptTCPIP:
        if (gOptFlags & kOptFlagUseRawIP)
        {
            PrintArgError("%s: the use of --tcp is exclusive with --raw. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        else if (gOptFlags & kOptFlagUseUDPIP)
        {
            PrintArgError("%s: the use of --tcp is exclusive with --udp. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        gOptFlags |= kOptFlagUseTCPIP;
        break;

    case kToolOptSendSize:
        if (!ParseInt(aValue, gSendSize))
        {
            PrintArgError("%s: invalid value specified for send size: %s\n", aProgram, aValue);
            return false;
        }
        break;

    case kToolOptUDPIP:
        if (gOptFlags & kOptFlagUseRawIP)
        {
            PrintArgError("%s: the use of --udp is exclusive with --raw. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        else if (gOptFlags & kOptFlagUseTCPIP)
        {
            PrintArgError("%s: the use of --udp is exclusive with --tcp. Please select only one of the two options.\n", aProgram);
            retval = false;
        }
        gOptFlags |= kOptFlagUseUDPIP;
        break;

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[])
{
    bool retval = true;

    if (Common::IsSender())
    {
        if (argc == 0)
        {
            PrintArgError("%s: Please specify a destination address.\n", aProgram);
            retval = false;
            goto exit;
        }

        retval = IPAddress::FromString(argv[0], sDestinationAddress);
        VerifyOrExit(retval == true, PrintArgError("%s: Please specify a valid destination address: %s\n", aProgram, argv[0]));

        sDestinationString = argv[0];

        argc--;
        argv++;
    }

    if (argc > 0)
    {
        PrintArgError("%s: unexpected argument: %s\n", aProgram, argv[0]);
        retval = false;
        goto exit;
    }

    // If no IP version or transport flags were specified, use the defaults.

    if (!(gOptFlags & (kOptFlagUseIPv4 | kOptFlagUseIPv6 | kOptFlagUseRawIP | kOptFlagUseTCPIP | kOptFlagUseUDPIP)))
    {
        gOptFlags |= kOptFlagsDefault;
    }

    // If no expected send or receive lengths were specified, use the defaults.

    if (!(gOptFlags & kOptFlagExpectedRxSize))
    {
        sTestState.mStats.mReceive.mExpected = kExpectedRxSizeDefault;
    }

    if (!(gOptFlags & kOptFlagExpectedTxSize))
    {
        sTestState.mStats.mTransmit.mExpected = kExpectedTxSizeDefault;
    }

exit:
    return (retval);
}

static void PrintReceivedStats(const TransferStats & aStats)
{
    printf("%u/%u received\n", aStats.mReceive.mActual, aStats.mReceive.mExpected);
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, bool aCheckBuffer, uint8_t aFirstValue)
{
    const bool lStatsByPacket = true;
    bool lStatus              = true;

    lStatus = Common::HandleDataReceived(aBuffer, sTestState.mStats, !lStatsByPacket, aCheckBuffer, aFirstValue);
    VerifyOrExit(lStatus == true, );

    PrintReceivedStats(sTestState.mStats);

exit:
    return (lStatus);
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, bool aCheckBuffer)
{
    const uint8_t lFirstValue = 0;
    bool lStatus              = true;

    lStatus = HandleDataReceived(aBuffer, aCheckBuffer, lFirstValue);
    VerifyOrExit(lStatus == true, );

exit:
    return (lStatus);
}

// TCP Endpoint Callbacks

void HandleTCPConnectionComplete(TCPEndPoint * aEndPoint, INET_ERROR aError)
{
    INET_ERROR lStatus;

    if (aError == INET_NO_ERROR)
    {
        IPAddress lPeerAddress;
        uint16_t lPeerPort;
        char lPeerAddressBuffer[INET6_ADDRSTRLEN];

        lStatus = aEndPoint->GetPeerInfo(&lPeerAddress, &lPeerPort);
        INET_FAIL_ERROR(lStatus, "TCPEndPoint::GetPeerInfo failed");

        lPeerAddress.ToString(lPeerAddressBuffer, sizeof(lPeerAddressBuffer));

        printf("TCP connection established to %s:%u\n", lPeerAddressBuffer, lPeerPort);

        if (sTCPIPEndPoint->PendingReceiveLength() == 0)
            sTCPIPEndPoint->PutBackReceivedData(nullptr);

        sTCPIPEndPoint->DisableReceive();
        sTCPIPEndPoint->EnableKeepAlive(10, 100);
        sTCPIPEndPoint->DisableKeepAlive();
        sTCPIPEndPoint->EnableReceive();

        DriveSend();
    }
    else
    {
        printf("TCP connection FAILED: %s\n", ErrorStr(aError));

        aEndPoint->Free();
        aEndPoint = nullptr;

        gSendIntervalExpired = false;
        gSystemLayer.CancelTimer(Common::HandleSendTimerComplete, nullptr);
        gSystemLayer.StartTimer(gSendIntervalMs, Common::HandleSendTimerComplete, nullptr);

        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleTCPConnectionClosed(TCPEndPoint * aEndPoint, INET_ERROR aError)
{
    if (aError == INET_NO_ERROR)
    {
        printf("TCP connection closed\n");
    }
    else
    {
        printf("TCP connection closed with error: %s\n", ErrorStr(aError));

        SetStatusFailed(sTestState.mStatus);
    }

    aEndPoint->Free();

    if (aEndPoint == sTCPIPEndPoint)
    {
        sTCPIPEndPoint = nullptr;
    }
}

static void HandleTCPDataSent(TCPEndPoint * aEndPoint, uint16_t len) {}

static void HandleTCPDataReceived(TCPEndPoint * aEndPoint, PacketBufferHandle aBuffer)
{
    const uint32_t lFirstValueReceived = sTestState.mStats.mReceive.mActual;
    const uint8_t lFirstValue          = uint8_t(lFirstValueReceived);
    const bool lCheckBuffer            = true;
    IPAddress lPeerAddress;
    uint16_t lPeerPort;
    char lPeerAddressBuffer[INET6_ADDRSTRLEN];
    bool lCheckPassed;
    INET_ERROR lStatus = INET_NO_ERROR;

    // Check that we did not lose information in our narrowing cast.
    VerifyOrExit(lFirstValue == lFirstValueReceived, lStatus = INET_ERROR_UNEXPECTED_EVENT);

    VerifyOrExit(aEndPoint != nullptr, lStatus = INET_ERROR_BAD_ARGS);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = INET_ERROR_BAD_ARGS);

    if (aEndPoint->State != TCPEndPoint::kState_Connected)
    {
        lStatus = aEndPoint->PutBackReceivedData(std::move(aBuffer));
        INET_FAIL_ERROR(lStatus, "TCPEndPoint::PutBackReceivedData failed");
        goto exit;
    }

    lStatus = aEndPoint->GetPeerInfo(&lPeerAddress, &lPeerPort);
    INET_FAIL_ERROR(lStatus, "TCPEndPoint::GetPeerInfo failed");

    lPeerAddress.ToString(lPeerAddressBuffer, sizeof(lPeerAddressBuffer));

    printf("TCP message received from %s:%u (%zu bytes)\n", lPeerAddressBuffer, lPeerPort,
           static_cast<size_t>(aBuffer->DataLength()));

    lCheckPassed = HandleDataReceived(aBuffer, lCheckBuffer, lFirstValue);
    VerifyOrExit(lCheckPassed == true, lStatus = INET_ERROR_UNEXPECTED_EVENT);

    lStatus = aEndPoint->AckReceive(aBuffer->TotalLength());
    INET_FAIL_ERROR(lStatus, "TCPEndPoint::AckReceive failed");

exit:
    if (lStatus != INET_NO_ERROR)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleTCPAcceptError(TCPEndPoint * aEndPoint, INET_ERROR aError)
{
    printf("TCP accept error: %s\n", ErrorStr(aError));

    SetStatusFailed(sTestState.mStatus);
}

static void HandleTCPConnectionReceived(TCPEndPoint * aListenEndPoint, TCPEndPoint * aConnectEndPoint,
                                        const IPAddress & aPeerAddress, uint16_t aPeerPort)
{
    char lPeerAddressBuffer[INET6_ADDRSTRLEN];

    aPeerAddress.ToString(lPeerAddressBuffer, sizeof(lPeerAddressBuffer));

    printf("TCP connection accepted from %s:%u\n", lPeerAddressBuffer, aPeerPort);

    aConnectEndPoint->OnConnectComplete  = HandleTCPConnectionComplete;
    aConnectEndPoint->OnConnectionClosed = HandleTCPConnectionClosed;
    aConnectEndPoint->OnDataSent         = HandleTCPDataSent;
    aConnectEndPoint->OnDataReceived     = HandleTCPDataReceived;

    sTCPIPEndPoint = aConnectEndPoint;
}

// Raw Endpoint Callbacks

static void HandleRawMessageReceived(IPEndPointBasis * aEndPoint, PacketBufferHandle aBuffer, const IPPacketInfo * aPacketInfo)
{
    const bool lCheckBuffer   = true;
    const bool lStatsByPacket = true;
    IPAddressType lAddressType;
    bool lStatus;

    VerifyOrExit(aEndPoint != nullptr, lStatus = false);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = false);
    VerifyOrExit(aPacketInfo != nullptr, lStatus = false);

    Common::HandleRawMessageReceived(aEndPoint, aBuffer, aPacketInfo);

    lAddressType = aPacketInfo->DestAddress.Type();

    if (lAddressType == kIPAddressType_IPv4)
    {
        const uint16_t kIPv4HeaderSize = 20;

        aBuffer->ConsumeHead(kIPv4HeaderSize);

        lStatus = Common::HandleICMPv4DataReceived(std::move(aBuffer), sTestState.mStats, !lStatsByPacket, lCheckBuffer);
    }
    else if (lAddressType == kIPAddressType_IPv6)
    {
        lStatus = Common::HandleICMPv6DataReceived(std::move(aBuffer), sTestState.mStats, !lStatsByPacket, lCheckBuffer);
    }
    else
    {
        lStatus = false;
    }

    if (lStatus)
    {
        PrintReceivedStats(sTestState.mStats);
    }

exit:
    if (!lStatus)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleRawReceiveError(IPEndPointBasis * aEndPoint, INET_ERROR aError, const IPPacketInfo * aPacketInfo)
{
    Common::HandleRawReceiveError(aEndPoint, aError, aPacketInfo);

    SetStatusFailed(sTestState.mStatus);
}

// UDP Endpoint Callbacks

static void HandleUDPMessageReceived(IPEndPointBasis * aEndPoint, PacketBufferHandle aBuffer, const IPPacketInfo * aPacketInfo)
{
    const bool lCheckBuffer = true;
    bool lStatus;

    VerifyOrExit(aEndPoint != nullptr, lStatus = false);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = false);
    VerifyOrExit(aPacketInfo != nullptr, lStatus = false);

    Common::HandleUDPMessageReceived(aEndPoint, aBuffer, aPacketInfo);

    lStatus = HandleDataReceived(aBuffer, lCheckBuffer);

exit:
    if (!lStatus)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleUDPReceiveError(IPEndPointBasis * aEndPoint, INET_ERROR aError, const IPPacketInfo * aPacketInfo)
{
    Common::HandleUDPReceiveError(aEndPoint, aError, aPacketInfo);

    SetStatusFailed(sTestState.mStatus);
}

static bool IsTransportReadyForSend()
{
    bool lStatus = false;

    if ((gOptFlags & (kOptFlagUseRawIP)) == (kOptFlagUseRawIP))
    {
        lStatus = (sRawIPEndPoint != nullptr);
    }
    else if ((gOptFlags & kOptFlagUseUDPIP) == kOptFlagUseUDPIP)
    {
        lStatus = (sUDPIPEndPoint != nullptr);
    }
    else if ((gOptFlags & kOptFlagUseTCPIP) == kOptFlagUseTCPIP)
    {
        if (sTCPIPEndPoint != nullptr)
        {
            if (sTCPIPEndPoint->PendingSendLength() == 0)
            {
                switch (sTCPIPEndPoint->State)
                {
                case TCPEndPoint::kState_Connected:
                case TCPEndPoint::kState_ReceiveShutdown:
                    lStatus = true;
                    break;

                default:
                    break;
                }
            }
        }
    }

    return (lStatus);
}

static INET_ERROR PrepareTransportForSend()
{
    INET_ERROR lStatus = INET_NO_ERROR;

    if (gOptFlags & kOptFlagUseTCPIP)
    {
        if (sTCPIPEndPoint == nullptr)
        {
            lStatus = gInet.NewTCPEndPoint(&sTCPIPEndPoint);
            INET_FAIL_ERROR(lStatus, "InetLayer::NewTCPEndPoint failed");

            sTCPIPEndPoint->OnConnectComplete  = HandleTCPConnectionComplete;
            sTCPIPEndPoint->OnConnectionClosed = HandleTCPConnectionClosed;
            sTCPIPEndPoint->OnDataSent         = HandleTCPDataSent;
            sTCPIPEndPoint->OnDataReceived     = HandleTCPDataReceived;

            lStatus = sTCPIPEndPoint->Connect(sDestinationAddress, kTCPPort, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Connect failed");
        }
    }

    return (lStatus);
}

static INET_ERROR DriveSendForDestination(const IPAddress & aAddress, uint16_t aSize)
{
    PacketBufferHandle lBuffer;
    INET_ERROR lStatus = INET_NO_ERROR;

    if ((gOptFlags & (kOptFlagUseRawIP)) == (kOptFlagUseRawIP))
    {
        // For ICMP (v4 or v6), we'll send n aSize or smaller
        // datagrams (with overhead for the ICMP header), each
        // patterned from zero to aSize - 1, following the ICMP
        // header.

        if ((gOptFlags & kOptFlagUseIPv6) == (kOptFlagUseIPv6))
        {
            lBuffer = Common::MakeICMPv6DataBuffer(aSize);
            VerifyOrExit(!lBuffer.IsNull(), lStatus = INET_ERROR_NO_MEMORY);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if ((gOptFlags & kOptFlagUseIPv4) == (kOptFlagUseIPv4))
        {
            lBuffer = Common::MakeICMPv4DataBuffer(aSize);
            VerifyOrExit(!lBuffer.IsNull(), lStatus = INET_ERROR_NO_MEMORY);
        }
#endif // INET_CONFIG_ENABLE_IPV4

        lStatus = sRawIPEndPoint->SendTo(aAddress, lBuffer.Release_ForNow());
        SuccessOrExit(lStatus);
    }
    else
    {
        if ((gOptFlags & kOptFlagUseUDPIP) == kOptFlagUseUDPIP)
        {
            const uint8_t lFirstValue = 0;

            // For UDP, we'll send n aSize or smaller datagrams, each
            // patterned from zero to aSize - 1.

            lBuffer = Common::MakeDataBuffer(aSize, lFirstValue);
            VerifyOrExit(!lBuffer.IsNull(), lStatus = INET_ERROR_NO_MEMORY);

            lStatus = sUDPIPEndPoint->SendTo(aAddress, kUDPPort, lBuffer.Release_ForNow());
            SuccessOrExit(lStatus);
        }
        else if ((gOptFlags & kOptFlagUseTCPIP) == kOptFlagUseTCPIP)
        {
            const uint32_t lFirstValue = sTestState.mStats.mTransmit.mActual;
            VerifyOrExit(lFirstValue < 256u, lStatus = INET_ERROR_UNEXPECTED_EVENT);

            // For TCP, we'll send one byte stream of
            // sTestState.mStats.mTransmit.mExpected in n aSize or
            // smaller transactions, patterned from zero to
            // sTestState.mStats.mTransmit.mExpected - 1.

            lBuffer = Common::MakeDataBuffer(aSize, uint8_t(lFirstValue));
            VerifyOrExit(!lBuffer.IsNull(), lStatus = INET_ERROR_NO_MEMORY);

            lStatus = sTCPIPEndPoint->Send(lBuffer.Release_ForNow());
            SuccessOrExit(lStatus);
        }
    }

exit:
    return (lStatus);
}

void DriveSend()
{
    INET_ERROR lStatus = INET_NO_ERROR;

    if (!Common::IsSender())
        goto exit;

    if (!gSendIntervalExpired)
        goto exit;

    if (!IsTransportReadyForSend())
    {
        lStatus = PrepareTransportForSend();
        SuccessOrExit(lStatus);
    }
    else
    {
        gSendIntervalExpired = false;
        gSystemLayer.StartTimer(gSendIntervalMs, Common::HandleSendTimerComplete, nullptr);

        if (sTestState.mStats.mTransmit.mActual < sTestState.mStats.mTransmit.mExpected)
        {
            const uint32_t lRemaining = (sTestState.mStats.mTransmit.mExpected - sTestState.mStats.mTransmit.mActual);
            const uint32_t lSendSize  = chip::min(lRemaining, static_cast<uint32_t>(gSendSize));

            // gSendSize is uint16_t, so this cast is safe: the value has to be
            // in the uint16_t range.
            static_assert(std::is_same<decltype(gSendSize), uint16_t>::value, "Unexpected type for gSendSize");
            lStatus = DriveSendForDestination(sDestinationAddress, uint16_t(lSendSize));
            SuccessOrExit(lStatus);

            sTestState.mStats.mTransmit.mActual += lSendSize;

            printf("%u/%u transmitted to %s\n", sTestState.mStats.mTransmit.mActual, sTestState.mStats.mTransmit.mExpected,
                   sDestinationString);
        }
    }

exit:
    if (lStatus != INET_NO_ERROR)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void StartTest()
{
    IPAddressType lIPAddressType = kIPAddressType_IPv6;
    IPProtocol lIPProtocol       = kIPProtocol_ICMPv6;
    IPVersion lIPVersion         = kIPVersion_6;
    IPAddress lAddress           = chip::Inet::IPAddress::Any;
    INET_ERROR lStatus;

    if (!gNetworkOptions.LocalIPv6Addr.empty())
        lAddress = gNetworkOptions.LocalIPv6Addr[0];

#if INET_CONFIG_ENABLE_IPV4
    if (gOptFlags & kOptFlagUseIPv4)
    {
        lIPAddressType = kIPAddressType_IPv4;
        lIPProtocol    = kIPProtocol_ICMPv4;
        lIPVersion     = kIPVersion_4;
        if (!gNetworkOptions.LocalIPv6Addr.empty())
            lAddress = gNetworkOptions.LocalIPv4Addr[0];
        else
            lAddress = chip::Inet::IPAddress::Any;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    // clang-format off
    printf("Using %sIP%s, device interface: %s (w/%c LwIP)\n",
           ((gOptFlags & kOptFlagUseRawIP) ? "" : ((gOptFlags & kOptFlagUseTCPIP) ? "TCP/" : "UDP/")),
           ((gOptFlags & kOptFlagUseIPv4) ? "v4" : "v6"),
           ((gInterfaceName) ? gInterfaceName : "<none>"),
           (CHIP_SYSTEM_CONFIG_USE_LWIP ? '\0' : 'o'));
    // clang-format on

    // Allocate the endpoints for sending or receiving.

    if (gOptFlags & kOptFlagUseRawIP)
    {
        lStatus = gInet.NewRawEndPoint(lIPVersion, lIPProtocol, &sRawIPEndPoint);
        INET_FAIL_ERROR(lStatus, "InetLayer::NewRawEndPoint failed");

        sRawIPEndPoint->OnMessageReceived = HandleRawMessageReceived;
        sRawIPEndPoint->OnReceiveError    = HandleRawReceiveError;

        if (IsInterfaceIdPresent(gInterfaceId))
        {
            lStatus = sRawIPEndPoint->BindInterface(lIPAddressType, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "RawEndPoint::BindInterface failed");
        }
    }
    else if (gOptFlags & kOptFlagUseUDPIP)
    {
        lStatus = gInet.NewUDPEndPoint(&sUDPIPEndPoint);
        INET_FAIL_ERROR(lStatus, "InetLayer::NewUDPEndPoint failed");

        sUDPIPEndPoint->OnMessageReceived = HandleUDPMessageReceived;
        sUDPIPEndPoint->OnReceiveError    = HandleUDPReceiveError;

        if (IsInterfaceIdPresent(gInterfaceId))
        {
            lStatus = sUDPIPEndPoint->BindInterface(lIPAddressType, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::BindInterface failed");
        }
    }

    if (Common::IsReceiver())
    {
        if (gOptFlags & kOptFlagUseRawIP)
        {
            lStatus = sRawIPEndPoint->Bind(lIPAddressType, lAddress);
            INET_FAIL_ERROR(lStatus, "RawEndPoint::Bind failed");

            if (gOptFlags & kOptFlagUseIPv6)
            {
                lStatus = sRawIPEndPoint->SetICMPFilter(kICMPv6_FilterTypes, gICMPv6Types);
                INET_FAIL_ERROR(lStatus, "RawEndPoint::SetICMPFilter failed");
            }

            lStatus = sRawIPEndPoint->Listen();
            INET_FAIL_ERROR(lStatus, "RawEndPoint::Listen failed");
        }
        else if (gOptFlags & kOptFlagUseUDPIP)
        {
            lStatus = sUDPIPEndPoint->Bind(lIPAddressType, IPAddress::Any, kUDPPort);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::Bind failed");

            lStatus = sUDPIPEndPoint->Listen();
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::Listen failed");
        }
        else if (gOptFlags & kOptFlagUseTCPIP)
        {
            const uint16_t lConnectionBacklogMax = 1;
            const bool lReuseAddress             = true;

            lStatus = gInet.NewTCPEndPoint(&sTCPIPListenEndPoint);
            INET_FAIL_ERROR(lStatus, "InetLayer::NewTCPEndPoint failed");

            sTCPIPListenEndPoint->OnConnectionReceived = HandleTCPConnectionReceived;
            sTCPIPListenEndPoint->OnAcceptError        = HandleTCPAcceptError;

            lStatus = sTCPIPListenEndPoint->Bind(lIPAddressType, IPAddress::Any, kTCPPort, lReuseAddress);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Bind failed");

            lStatus = sTCPIPListenEndPoint->Listen(lConnectionBacklogMax);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Listen failed");
        }
    }

    if (Common::IsReceiver())
        printf("Listening...\n");
    else
        DriveSend();
}

static void CleanupTest()
{
    INET_ERROR lStatus;

    gSendIntervalExpired = false;
    gSystemLayer.CancelTimer(Common::HandleSendTimerComplete, nullptr);

    // Release the resources associated with the allocated end points.

    if (sRawIPEndPoint != nullptr)
    {
        sRawIPEndPoint->Free();
    }

    if (sTCPIPEndPoint != nullptr)
    {
        lStatus = sTCPIPEndPoint->Close();
        INET_FAIL_ERROR(lStatus, "TCPEndPoint::Close failed");

        sTCPIPEndPoint->Free();
    }

    if (sTCPIPListenEndPoint != nullptr)
    {
        sTCPIPListenEndPoint->Shutdown();
        sTCPIPListenEndPoint->Free();
    }

    if (sUDPIPEndPoint != nullptr)
    {
        sUDPIPEndPoint->Free();
    }
}
