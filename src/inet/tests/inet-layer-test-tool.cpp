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

#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <type_traits>
#include <unistd.h>

#include <CHIPVersion.h>

#include <inet/InetArgParser.h>
#include <lib/support/CodeUtils.h>

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"
#include "TestInetLayerCommon.hpp"
#include "TestSetupFaultInjection.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::ArgParser;
using namespace chip::System;

/* Preprocessor Macros */

#define kToolName "inet-layer-test-tool"

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
static bool HandleNonOptionArgs(const char * aProgram, int argc, char * const argv[]);

static void StartTest();
static void CleanupTest();

/* Global Variables */

static const uint32_t kExpectedRxSizeDefault = 1523;
static const uint32_t kExpectedTxSizeDefault = kExpectedRxSizeDefault;

static const uint32_t kOptFlagsDefault = (kOptFlagUseIPv6 | kOptFlagUseUDPIP);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
static TCPEndPoint * sTCPIPEndPoint       = nullptr; // Used for connect/send/receive
static TCPEndPoint * sTCPIPListenEndPoint = nullptr; // Used for accept/listen
static const uint16_t kTCPPort            = kUDPPort;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

static UDPEndPoint * sUDPIPEndPoint = nullptr;

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

namespace chip {
namespace Inet {

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
class TCPTest
{
public:
    static bool StateIsConnected(const TCPEndPoint * endPoint) { return endPoint->mState == TCPEndPoint::State::kConnected; }
    static bool StateIsConnectedOrReceiveShutdown(const TCPEndPoint * endPoint)
    {
        return endPoint->mState == TCPEndPoint::State::kConnected || endPoint->mState == TCPEndPoint::State::kReceiveShutdown;
    }
};
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

} // namespace Inet
} // namespace chip

static void CheckSucceededOrFailed(TestState & aTestState, bool & aOutSucceeded, bool & aOutFailed)
{
    const TransferStats & lStats = aTestState.mStats;

#ifdef DEBUG_TCP_TEST
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
    CHIP_ERROR lStatus;

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
        lStatus = InterfaceId::InterfaceNameToId(gInterfaceName, gInterfaceId);
        if (lStatus != CHIP_NO_ERROR)
        {
            PrintArgError("%s: unknown network interface %s\n", kToolName, gInterfaceName);
            lSuccessful = false;
            goto shutdown;
        }
    }

    StartTest();

    while (Common::IsTesting(sTestState.mStatus))
    {
        bool lSucceeded = true;
        bool lFailed    = false;

        constexpr uint32_t kSleepTimeMilliseconds = 10;
        ServiceNetwork(kSleepTimeMilliseconds);

        CheckSucceededOrFailed(sTestState, lSucceeded, lFailed);

#ifdef DEBUG_TCP_TEST
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

    ShutdownTestInetCommon();

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

    case kToolOptTCPIP:
        if (gOptFlags & kOptFlagUseUDPIP)
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
        if (gOptFlags & kOptFlagUseTCPIP)
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

bool HandleNonOptionArgs(const char * aProgram, int argc, char * const argv[])
{
    if (Common::IsSender())
    {
        if (argc == 0)
        {
            PrintArgError("%s: Please specify a destination address.\n", aProgram);
            return false;
        }

        if (!IPAddress::FromString(argv[0], sDestinationAddress))
        {
            PrintArgError("%s: Please specify a valid destination address: %s\n", aProgram, argv[0]);
            return false;
        }

        sDestinationString = argv[0];

        argc--;
        argv++;
    }

    if (argc > 0)
    {
        PrintArgError("%s: unexpected argument: %s\n", aProgram, argv[0]);
        return false;
    }

    // If no IP version or transport flags were specified, use the defaults.

    if (!(gOptFlags & (kOptFlagUseIPv4 | kOptFlagUseIPv6 | kOptFlagUseTCPIP | kOptFlagUseUDPIP)))
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

    return true;
}

static void PrintReceivedStats(const TransferStats & aStats)
{
    printf("%" PRIu32 "/%" PRIu32 "received\n", aStats.mReceive.mActual, aStats.mReceive.mExpected);
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, bool aCheckBuffer, uint8_t aFirstValue)
{
    constexpr bool lStatsByPacket = true;

    if (!Common::HandleDataReceived(aBuffer, sTestState.mStats, !lStatsByPacket, aCheckBuffer, aFirstValue))
    {
        return false;
    }

    PrintReceivedStats(sTestState.mStats);

    return true;
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, bool aCheckBuffer)
{
    constexpr uint8_t lFirstValue = 0;
    return HandleDataReceived(aBuffer, aCheckBuffer, lFirstValue);
}

// TCP Endpoint Callbacks

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
void HandleTCPConnectionComplete(TCPEndPoint * aEndPoint, CHIP_ERROR aError)
{
    CHIP_ERROR lStatus;

    if (aError == CHIP_NO_ERROR)
    {
        IPAddress lPeerAddress;
        uint16_t lPeerPort;
        char lPeerAddressBuffer[INET6_ADDRSTRLEN];

        lStatus = aEndPoint->GetPeerInfo(&lPeerAddress, &lPeerPort);
        INET_FAIL_ERROR(lStatus, "TCPEndPoint::GetPeerInfo failed");

        lPeerAddress.ToString(lPeerAddressBuffer);

        printf("TCP connection established to %s:%u\n", lPeerAddressBuffer, lPeerPort);

        if (sTCPIPEndPoint->PendingReceiveLength() == 0)
            sTCPIPEndPoint->SetReceivedDataForTesting(nullptr);

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
        gSystemLayer.StartTimer(System::Clock::Milliseconds32(gSendIntervalMs), Common::HandleSendTimerComplete, nullptr);

        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleTCPConnectionClosed(TCPEndPoint * aEndPoint, CHIP_ERROR aError)
{
    if (aError == CHIP_NO_ERROR)
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

static void HandleTCPDataSent(TCPEndPoint * aEndPoint, size_t len) {}

static CHIP_ERROR HandleTCPDataReceived(TCPEndPoint * aEndPoint, PacketBufferHandle && aBuffer)
{
    const uint32_t lFirstValueReceived = sTestState.mStats.mReceive.mActual;
    const uint8_t lFirstValue          = uint8_t(lFirstValueReceived);
    const bool lCheckBuffer            = true;
    IPAddress lPeerAddress;
    uint16_t lPeerPort;
    char lPeerAddressBuffer[INET6_ADDRSTRLEN];
    bool lCheckPassed;
    CHIP_ERROR lStatus = CHIP_NO_ERROR;

    // Check that we did not lose information in our narrowing cast.
    VerifyOrExit(lFirstValue == lFirstValueReceived, lStatus = CHIP_ERROR_UNEXPECTED_EVENT);

    VerifyOrExit(aEndPoint != nullptr, lStatus = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = CHIP_ERROR_INVALID_ARGUMENT);

    if (!TCPTest::StateIsConnected(aEndPoint))
    {
        lStatus = aEndPoint->SetReceivedDataForTesting(std::move(aBuffer));
        INET_FAIL_ERROR(lStatus, "TCPEndPoint::PutBackReceivedData failed");
        goto exit;
    }

    lStatus = aEndPoint->GetPeerInfo(&lPeerAddress, &lPeerPort);
    INET_FAIL_ERROR(lStatus, "TCPEndPoint::GetPeerInfo failed");

    lPeerAddress.ToString(lPeerAddressBuffer);

    printf("TCP message received from %s:%u (%u bytes)\n", lPeerAddressBuffer, lPeerPort,
           static_cast<unsigned int>(aBuffer->DataLength()));

    lCheckPassed = HandleDataReceived(aBuffer, lCheckBuffer, lFirstValue);
    VerifyOrExit(lCheckPassed == true, lStatus = CHIP_ERROR_UNEXPECTED_EVENT);

    lStatus = aEndPoint->AckReceive(aBuffer->TotalLength());
    INET_FAIL_ERROR(lStatus, "TCPEndPoint::AckReceive failed");

exit:
    if (lStatus != CHIP_NO_ERROR)
    {
        SetStatusFailed(sTestState.mStatus);
    }
    return lStatus;
}

static void HandleTCPAcceptError(TCPEndPoint * aEndPoint, CHIP_ERROR aError)
{
    printf("TCP accept error: %s\n", ErrorStr(aError));

    SetStatusFailed(sTestState.mStatus);
}

static void HandleTCPConnectionReceived(TCPEndPoint * aListenEndPoint, TCPEndPoint * aConnectEndPoint,
                                        const IPAddress & aPeerAddress, uint16_t aPeerPort)
{
    char lPeerAddressBuffer[INET6_ADDRSTRLEN];

    aPeerAddress.ToString(lPeerAddressBuffer);

    printf("TCP connection accepted from %s:%u\n", lPeerAddressBuffer, aPeerPort);

    aConnectEndPoint->OnConnectComplete  = HandleTCPConnectionComplete;
    aConnectEndPoint->OnConnectionClosed = HandleTCPConnectionClosed;
    aConnectEndPoint->OnDataSent         = HandleTCPDataSent;
    aConnectEndPoint->OnDataReceived     = HandleTCPDataReceived;

    sTCPIPEndPoint = aConnectEndPoint;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

// UDP Endpoint Callbacks

static void HandleUDPMessageReceived(UDPEndPoint * aEndPoint, PacketBufferHandle && aBuffer, const IPPacketInfo * aPacketInfo)
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

static void HandleUDPReceiveError(UDPEndPoint * aEndPoint, CHIP_ERROR aError, const IPPacketInfo * aPacketInfo)
{
    Common::HandleUDPReceiveError(aEndPoint, aError, aPacketInfo);

    SetStatusFailed(sTestState.mStatus);
}

static bool IsTransportReadyForSend()
{
    if ((gOptFlags & kOptFlagUseUDPIP) == kOptFlagUseUDPIP)
    {
        return (sUDPIPEndPoint != nullptr);
    }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if ((gOptFlags & kOptFlagUseTCPIP) == kOptFlagUseTCPIP)
    {
        return (sTCPIPEndPoint != nullptr) && (sTCPIPEndPoint->PendingSendLength() == 0) &&
            TCPTest::StateIsConnectedOrReceiveShutdown(sTCPIPEndPoint);
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    return false;
}

static CHIP_ERROR PrepareTransportForSend()
{
    CHIP_ERROR lStatus = CHIP_NO_ERROR;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (gOptFlags & kOptFlagUseTCPIP)
    {
        if (sTCPIPEndPoint == nullptr)
        {
            lStatus = gTCP.NewEndPoint(&sTCPIPEndPoint);
            INET_FAIL_ERROR(lStatus, "TCP NewEndPoint failed");

            sTCPIPEndPoint->OnConnectComplete  = HandleTCPConnectionComplete;
            sTCPIPEndPoint->OnConnectionClosed = HandleTCPConnectionClosed;
            sTCPIPEndPoint->OnDataSent         = HandleTCPDataSent;
            sTCPIPEndPoint->OnDataReceived     = HandleTCPDataReceived;

            lStatus = sTCPIPEndPoint->Connect(sDestinationAddress, kTCPPort, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Connect failed");
        }
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    return (lStatus);
}

static CHIP_ERROR DriveSendForDestination(const IPAddress & aAddress, uint16_t aSize)
{
    PacketBufferHandle lBuffer;

    if ((gOptFlags & kOptFlagUseUDPIP) == kOptFlagUseUDPIP)
    {
        const uint8_t lFirstValue = 0;

        // For UDP, we'll send n aSize or smaller datagrams, each
        // patterned from zero to aSize - 1.

        lBuffer = Common::MakeDataBuffer(aSize, lFirstValue);
        VerifyOrReturnError(!lBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(sUDPIPEndPoint->SendTo(aAddress, kUDPPort, std::move(lBuffer)));
    }
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    else if ((gOptFlags & kOptFlagUseTCPIP) == kOptFlagUseTCPIP)
    {
        const uint32_t lFirstValue = sTestState.mStats.mTransmit.mActual;
        VerifyOrReturnError(lFirstValue < 256u, CHIP_ERROR_UNEXPECTED_EVENT);

        // For TCP, we'll send one byte stream of
        // sTestState.mStats.mTransmit.mExpected in n aSize or
        // smaller transactions, patterned from zero to
        // sTestState.mStats.mTransmit.mExpected - 1.

        lBuffer = Common::MakeDataBuffer(aSize, uint8_t(lFirstValue));
        VerifyOrReturnError(!lBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(sTCPIPEndPoint->Send(std::move(lBuffer)));
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    return CHIP_NO_ERROR;
}

void DriveSend()
{
    CHIP_ERROR lStatus = CHIP_NO_ERROR;

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
        gSystemLayer.StartTimer(System::Clock::Milliseconds32(gSendIntervalMs), Common::HandleSendTimerComplete, nullptr);

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

            printf("%" PRIu32 "/%" PRIu32 "transmitted to %s\n", sTestState.mStats.mTransmit.mActual,
                   sTestState.mStats.mTransmit.mExpected, sDestinationString);
        }
    }

exit:
    if (lStatus != CHIP_NO_ERROR)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void StartTest()
{
    IPAddressType lIPAddressType = IPAddressType::kIPv6;
    IPAddress lAddress           = chip::Inet::IPAddress::Any;
    CHIP_ERROR lStatus;

    if (!gNetworkOptions.LocalIPv6Addr.empty())
        lAddress = gNetworkOptions.LocalIPv6Addr[0];

#if INET_CONFIG_ENABLE_IPV4
    if (gOptFlags & kOptFlagUseIPv4)
    {
        lIPAddressType = IPAddressType::kIPv4;
        if (!gNetworkOptions.LocalIPv6Addr.empty())
            lAddress = gNetworkOptions.LocalIPv4Addr[0];
        else
            lAddress = chip::Inet::IPAddress::Any;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    // clang-format off
    printf("Using %sIP%s, device interface: %s (w/%c LwIP)\n",
           ((gOptFlags & kOptFlagUseTCPIP) ? "TCP/" : "UDP/"),
           ((gOptFlags & kOptFlagUseIPv4) ? "v4" : "v6"),
           ((gInterfaceName) ? gInterfaceName : "<none>"),
           (CHIP_SYSTEM_CONFIG_USE_LWIP ? '\0' : 'o'));
    // clang-format on

    // Allocate the endpoints for sending or receiving.

    if (gOptFlags & kOptFlagUseUDPIP)
    {
        lStatus = gUDP.NewEndPoint(&sUDPIPEndPoint);
        INET_FAIL_ERROR(lStatus, "UDP NewEndPoint failed");

        if (gInterfaceId.IsPresent())
        {
            lStatus = sUDPIPEndPoint->BindInterface(lIPAddressType, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::BindInterface failed");
        }
    }

    if (Common::IsReceiver())
    {
        if (gOptFlags & kOptFlagUseUDPIP)
        {
            lStatus = sUDPIPEndPoint->Bind(lIPAddressType, IPAddress::Any, kUDPPort);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::Bind failed");

            lStatus = sUDPIPEndPoint->Listen(HandleUDPMessageReceived, HandleUDPReceiveError);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::Listen failed");
        }
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        else if (gOptFlags & kOptFlagUseTCPIP)
        {
            const uint16_t lConnectionBacklogMax = 1;
            const bool lReuseAddress             = true;

            lStatus = gTCP.NewEndPoint(&sTCPIPListenEndPoint);
            INET_FAIL_ERROR(lStatus, "TCP NewEndPoint failed");

            sTCPIPListenEndPoint->OnConnectionReceived = HandleTCPConnectionReceived;
            sTCPIPListenEndPoint->OnAcceptError        = HandleTCPAcceptError;

            lStatus = sTCPIPListenEndPoint->Bind(lIPAddressType, IPAddress::Any, kTCPPort, lReuseAddress);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Bind failed");

            lStatus = sTCPIPListenEndPoint->Listen(lConnectionBacklogMax);
            INET_FAIL_ERROR(lStatus, "TCPEndPoint::Listen failed");
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    }

    if (Common::IsReceiver())
        printf("Listening...\n");
    else
        DriveSend();
}

static void CleanupTest()
{
    gSendIntervalExpired = false;
    gSystemLayer.CancelTimer(Common::HandleSendTimerComplete, nullptr);

    // Release the resources associated with the allocated end points.

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (sTCPIPEndPoint != nullptr)
    {
        sTCPIPEndPoint->Close();
        sTCPIPEndPoint->Free();
    }

    if (sTCPIPListenEndPoint != nullptr)
    {
        sTCPIPListenEndPoint->Shutdown();
        sTCPIPListenEndPoint->Free();
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    if (sUDPIPEndPoint != nullptr)
    {
        sUDPIPEndPoint->Free();
    }
}
