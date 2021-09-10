/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018-2019 Google LLC
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
 *      the InetLayer Internet Protocol stack abstraction interfaces
 *      for handling IP (v4 or v6) multicast on either bare IP (i.e.,
 *      "raw") or UDP endpoints.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <nlbyteorder.hpp>

#include <CHIPVersion.h>

#include <inet/IPAddress.h>
#include <inet/InetArgParser.h>
#include <lib/support/CHIPArgParser.hpp>

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"
#include "TestInetLayerCommon.hpp"
#include "TestSetupFaultInjection.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Inet;
using namespace chip::System;

/* Preprocessor Macros */

#define kToolName "TestInetLayerMulticast"

#define kToolOptNoLoopback 'L'
#define kToolOptGroup 'g'

#define kToolOptExpectedGroupRxPackets (kToolOptBase + 0)
#define kToolOptExpectedGroupTxPackets (kToolOptBase + 1)

/* Type Definitions */

enum OptFlags
{
    kOptFlagNoLoopback = 0x00010000
};

struct GroupAddress
{
    uint32_t mGroup;
    TransferStats mStats;
    IPAddress mMulticastAddress;
};

template <size_t tCapacity>
struct GroupAddresses
{
    size_t mSize;
    const size_t mCapacity = tCapacity;
    GroupAddress mAddresses[tCapacity];
};

template <size_t tCapacity>
struct TestState
{
    GroupAddresses<tCapacity> & mGroupAddresses;
    TestStatus mStatus;
};

/* Function Declarations */

static void HandleSignal(int aSignal);
static bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);
static bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[]);
static bool ParseGroupOpt(const char * aProgram, const char * aValue, bool aIPv6, uint32_t & aOutLastGroupIndex);
static bool ParseAndUpdateExpectedGroupPackets(const char * aProgram, const char * aValue, uint32_t aGroup,
                                               const char * aDescription, uint32_t & aOutExpected);

static void StartTest();
static void CleanupTest();

/* Global Variables */

// clang-format off
static const uint32_t    kOptFlagsDefault       = (kOptFlagUseIPv6 | kOptFlagUseRawIP);

static RawEndPoint *     sRawIPEndPoint         = nullptr;
static UDPEndPoint *     sUDPIPEndPoint         = nullptr;

static GroupAddresses<4> sGroupAddresses;

static TestState<4>      sTestState             =
{
    sGroupAddresses,
    { false, false }
};

static uint32_t          sLastGroupIndex        = 0;

static OptionDef         sToolOptionDefs[] =
{
    { "interface",                 kArgumentRequired,  kToolOptInterface              },
    { "group",                     kArgumentRequired,  kToolOptGroup                  },
    { "group-expected-rx-packets", kArgumentRequired,  kToolOptExpectedGroupRxPackets },
    { "group-expected-tx-packets", kArgumentRequired,  kToolOptExpectedGroupTxPackets },
    { "interval",                  kArgumentRequired,  kToolOptInterval               },
#if INET_CONFIG_ENABLE_IPV4
    { "ipv4",                      kNoArgument,        kToolOptIPv4Only               },
#endif // INET_CONFIG_ENABLE_IPV4
    { "ipv6",                      kNoArgument,        kToolOptIPv6Only               },
    { "listen",                    kNoArgument,        kToolOptListen                 },
    { "no-loopback",               kNoArgument,        kToolOptNoLoopback             },
    { "raw",                       kNoArgument,        kToolOptRawIP                  },
    { "send-size",                 kArgumentRequired,  kToolOptSendSize               },
    { "udp",                       kNoArgument,        kToolOptUDPIP                  },
    { }
};

static const char *      sToolOptionHelp =
    "  -I, --interface <interface>\n"
    "       The network interface to bind to and from which to send and receive all packets.\n"
    "\n"
    "  -L, --no-loopback\n"
    "       Suppress the loopback of multicast packets.\n"
    "\n"
    "  -g, --group <group>\n"
    "       Multicast group number to join.\n"
    "\n"
    "  --group-expected-rx-packets <packets>\n"
    "       Expect to receive this number of packets for the previously-specified multicast group.\n"
    "\n"
    "  --group-expected-tx-packets <packets>\n"
    "       Expect to send this number of packets for the previously-specified multicast group.\n"
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
    "  -u, --udp\n"
    "       Use UDP over IP.\n"
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
    "Usage: " kToolName " [ <options> ] [ -g <group> [ ... ] -I <interface> ]\n",
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

static void CheckSucceededOrFailed(const GroupAddress & aAddress, bool & aOutSucceeded, bool & aOutFailed)
{
    const TransferStats & lStats = aAddress.mStats;

#if DEBUG
    printf("Group %u: %u/%u sent, %u/%u received\n", aAddress.mGroup, lStats.mTransmit.mActual, lStats.mTransmit.mExpected,
           lStats.mReceive.mActual, lStats.mReceive.mExpected);
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
}

template <size_t tCapacity>
static void CheckSucceededOrFailed(TestState<tCapacity> & aTestState, bool & aOutSucceeded, bool & aOutFailed)
{
    for (size_t i = 0; i < aTestState.mGroupAddresses.mSize; i++)
    {
        const GroupAddress & lGroup = aTestState.mGroupAddresses.mAddresses[i];

        CheckSucceededOrFailed(lGroup, aOutSucceeded, aOutFailed);
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

namespace TestInetLayerMulticast {
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
        lStatus = InterfaceNameToId(gInterfaceName, gInterfaceId);
        if (lStatus != CHIP_NO_ERROR)
        {
            PrintArgError("%s: unknown network interface %s\n", kToolName, gInterfaceName);
            lSuccessful = false;
            goto shutdown;
        }
    }

    // If any multicast groups have been specified, ensure that a
    // network interface identifier has been specified and is valid.

    if ((sGroupAddresses.mSize > 0) && !IsInterfaceIdPresent(gInterfaceId))
    {
        PrintArgError("%s: a network interface is required when specifying one or more multicast groups\n", kToolName);
        lSuccessful = false;
        goto shutdown;
    }

    StartTest();

    while (Common::IsTesting(sTestState.mStatus))
    {
        bool lSucceeded = true;
        bool lFailed    = false;

        constexpr uint32_t kSleepTimeMilliseconds = 10;
        ServiceNetwork(kSleepTimeMilliseconds);

        CheckSucceededOrFailed(sTestState, lSucceeded, lFailed);

#if DEBUG
        printf("%s %s number of expected packets\n", ((lSucceeded) ? "successfully" : ((lFailed) ? "failed to" : "has not yet")),
               ((lSucceeded)
                    ? (Common::IsReceiver() ? "received" : "sent")
                    : ((lFailed) ? (Common::IsReceiver() ? "receive" : "send") : Common::IsReceiver() ? "received" : "sent")));
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
} // namespace TestInetLayerMulticast

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

    case kToolOptNoLoopback:
        gOptFlags |= kOptFlagNoLoopback;
        break;

    case kToolOptGroup:
        if (!ParseGroupOpt(aProgram, aValue, gOptFlags & kOptFlagUseIPv6, sLastGroupIndex))
        {
            retval = false;
        }
        break;

    case kToolOptExpectedGroupRxPackets: {
        GroupAddress & lGroupAddress = sGroupAddresses.mAddresses[sLastGroupIndex];

        if (!ParseAndUpdateExpectedGroupPackets(aProgram, aValue, lGroupAddress.mGroup, "received",
                                                lGroupAddress.mStats.mReceive.mExpected))
        {
            retval = false;
        }
    }
    break;

    case kToolOptExpectedGroupTxPackets: {
        GroupAddress & lGroupAddress = sGroupAddresses.mAddresses[sLastGroupIndex];

        if (!ParseAndUpdateExpectedGroupPackets(aProgram, aValue, lGroupAddress.mGroup, "sent",
                                                lGroupAddress.mStats.mTransmit.mExpected))
        {
            retval = false;
        }
    }
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
        gOptFlags |= kOptFlagUseRawIP;
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
    if ((gOptFlags & (kOptFlagListen | kOptFlagNoLoopback)) == (kOptFlagListen | kOptFlagNoLoopback))
    {
        PrintArgError("%s: the listen option is exclusive with the loopback suppression option. Please select one or the other.\n",
                      aProgram);
        return false;
    }

    // If there were any additional, non-parsed arguments, it's an error.

    if (argc > 0)
    {
        PrintArgError("%s: unexpected argument: %s\n", aProgram, argv[0]);
        return false;
    }

    // If no IP version or transport flags were specified, use the defaults.

    if (!(gOptFlags & (kOptFlagUseIPv4 | kOptFlagUseIPv6 | kOptFlagUseRawIP | kOptFlagUseUDPIP)))
    {
        gOptFlags |= kOptFlagsDefault;
    }

    return true;
}

// Create an IPv4 administratively-scoped multicast address

static IPAddress MakeIPv4Multicast(uint32_t aGroupIdentifier)
{
    IPAddress lAddress;

    lAddress.Addr[0] = 0;
    lAddress.Addr[1] = 0;
    lAddress.Addr[2] = nlByteOrderSwap32HostToBig(0xFFFF);
    lAddress.Addr[3] = nlByteOrderSwap32HostToBig((239 << 24) | (aGroupIdentifier & 0xFFFFFF));

    return (lAddress);
}

// Create an IPv6 site-scoped multicast address

static IPAddress MakeIPv6Multicast(uint32_t aGroupIdentifier)
{
    const uint8_t lFlags = kIPv6MulticastFlag_Transient;

    return (IPAddress::MakeIPv6Multicast(lFlags, kIPv6MulticastScope_Site, aGroupIdentifier));
}

static void SetGroup(GroupAddress & aGroupAddress, uint32_t aGroupIdentifier, uint32_t aExpectedRx, uint32_t aExpectedTx)
{
    aGroupAddress.mGroup                     = aGroupIdentifier;
    aGroupAddress.mStats.mReceive.mExpected  = aExpectedRx;
    aGroupAddress.mStats.mReceive.mActual    = 0;
    aGroupAddress.mStats.mTransmit.mExpected = aExpectedTx;
    aGroupAddress.mStats.mTransmit.mActual   = 0;
}

static bool ParseGroupOpt(const char * aProgram, const char * aValue, bool aIPv6, uint32_t & aOutLastGroupIndex)
{
    uint32_t lGroupIdentifier;

    if (sGroupAddresses.mSize == sGroupAddresses.mCapacity)
    {
        PrintArgError("%s: the maximum number of allowed groups (%zu) have been specified\n", aProgram, sGroupAddresses.mCapacity);
        return false;
    }

    if (!ParseInt(aValue, lGroupIdentifier))
    {
        PrintArgError("%s: unrecognized group %s\n", aProgram, aValue);
        return false;
    }

    aOutLastGroupIndex = sGroupAddresses.mSize++;

    SetGroup(sGroupAddresses.mAddresses[aOutLastGroupIndex], lGroupIdentifier, lGroupIdentifier, lGroupIdentifier);

    return true;
}

static bool ParseAndUpdateExpectedGroupPackets(const char * aProgram, const char * aValue, uint32_t aGroup,
                                               const char * aDescription, uint32_t & aOutExpected)
{
    uint32_t lExpectedGroupPackets;

    if (!ParseInt(aValue, lExpectedGroupPackets))
    {
        PrintArgError("%s: invalid value specified for expected group %u %s packets: %s\n", aProgram, aGroup, aDescription, aValue);
        return false;
    }

    aOutExpected = lExpectedGroupPackets;

    return true;
}

static GroupAddress * FindGroupAddress(const IPAddress & aSourceAddress)
{
    GroupAddress * lResult = nullptr;

    for (size_t i = 0; i < sGroupAddresses.mSize; i++)
    {
        GroupAddress & lGroupAddress = sGroupAddresses.mAddresses[i];

        if (lGroupAddress.mMulticastAddress == aSourceAddress)
        {
            lResult = &lGroupAddress;
            break;
        }
    }

    return (lResult);
}

static void PrintReceivedStats(const GroupAddress & aGroupAddress)
{
    printf("%u/%u received for multicast group %u\n", aGroupAddress.mStats.mReceive.mActual,
           aGroupAddress.mStats.mReceive.mExpected, aGroupAddress.mGroup);
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, GroupAddress & aGroupAddress, bool aCheckBuffer)
{
    constexpr bool lStatsByPacket = true;
    if (!Common::HandleDataReceived(aBuffer, aGroupAddress.mStats, lStatsByPacket, aCheckBuffer))
    {
        return false;
    }

    PrintReceivedStats(aGroupAddress);

    return true;
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, const IPPacketInfo & aPacketInfo, bool aCheckBuffer)
{
    GroupAddress * const lGroupAddress = FindGroupAddress(aPacketInfo.DestAddress);
    if (lGroupAddress != nullptr)
    {
        return HandleDataReceived(aBuffer, *lGroupAddress, aCheckBuffer);
    }
    return true;
}

// Raw Endpoint Callbacks

static void HandleRawMessageReceived(IPEndPointBasis * aEndPoint, PacketBufferHandle && aBuffer, const IPPacketInfo * aPacketInfo)
{
    const bool lCheckBuffer   = true;
    const bool lStatsByPacket = true;
    IPAddressType lAddressType;
    bool lStatus = true;
    GroupAddress * lGroupAddress;

    VerifyOrExit(aEndPoint != nullptr, lStatus = false);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = false);
    VerifyOrExit(aPacketInfo != nullptr, lStatus = false);

    Common::HandleRawMessageReceived(aEndPoint, aBuffer, aPacketInfo);

    lGroupAddress = FindGroupAddress(aPacketInfo->DestAddress);

    if (lGroupAddress != nullptr)
    {
        lAddressType = aPacketInfo->DestAddress.Type();

        if (lAddressType == kIPAddressType_IPv4)
        {
            const uint16_t kIPv4HeaderSize = 20;

            aBuffer->ConsumeHead(kIPv4HeaderSize);

            lStatus = Common::HandleICMPv4DataReceived(std::move(aBuffer), lGroupAddress->mStats, lStatsByPacket, lCheckBuffer);
        }
        else if (lAddressType == kIPAddressType_IPv6)
        {
            lStatus = Common::HandleICMPv6DataReceived(std::move(aBuffer), lGroupAddress->mStats, lStatsByPacket, lCheckBuffer);
        }
        else
        {
            lStatus = false;
        }

        if (lStatus)
        {
            PrintReceivedStats(*lGroupAddress);
        }
    }

exit:
    if (!lStatus)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleRawReceiveError(IPEndPointBasis * aEndPoint, CHIP_ERROR aError, const IPPacketInfo * aPacketInfo)
{
    Common::HandleRawReceiveError(aEndPoint, aError, aPacketInfo);

    SetStatusFailed(sTestState.mStatus);
}

// UDP Endpoint Callbacks

static void HandleUDPMessageReceived(IPEndPointBasis * aEndPoint, PacketBufferHandle && aBuffer, const IPPacketInfo * aPacketInfo)
{
    const bool lCheckBuffer = true;
    bool lStatus;

    VerifyOrExit(aEndPoint != nullptr, lStatus = false);
    VerifyOrExit(!aBuffer.IsNull(), lStatus = false);
    VerifyOrExit(aPacketInfo != nullptr, lStatus = false);

    Common::HandleUDPMessageReceived(aEndPoint, aBuffer, aPacketInfo);

    lStatus = HandleDataReceived(std::move(aBuffer), *aPacketInfo, lCheckBuffer);

exit:
    if (!lStatus)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void HandleUDPReceiveError(IPEndPointBasis * aEndPoint, CHIP_ERROR aError, const IPPacketInfo * aPacketInfo)
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

    return (lStatus);
}

static CHIP_ERROR PrepareTransportForSend()
{
    CHIP_ERROR lStatus = CHIP_NO_ERROR;

    return (lStatus);
}

static CHIP_ERROR DriveSendForDestination(const IPAddress & aAddress, uint16_t aSize)
{
    PacketBufferHandle lBuffer;

    if ((gOptFlags & (kOptFlagUseRawIP)) == (kOptFlagUseRawIP))
    {
        // For ICMP (v4 or v6), we'll send n aSize or smaller
        // datagrams (with overhead for the ICMP header), each
        // patterned from zero to aSize - 1, following the ICMP
        // header.

        if ((gOptFlags & kOptFlagUseIPv6) == (kOptFlagUseIPv6))
        {
            lBuffer = Common::MakeICMPv6DataBuffer(aSize);
            VerifyOrReturnError(!lBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if ((gOptFlags & kOptFlagUseIPv4) == (kOptFlagUseIPv4))
        {
            lBuffer = Common::MakeICMPv4DataBuffer(aSize);
            VerifyOrReturnError(!lBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);
        }
#endif // INET_CONFIG_ENABLE_IPV4

        return sRawIPEndPoint->SendTo(aAddress, std::move(lBuffer));
    }

    if ((gOptFlags & kOptFlagUseUDPIP) == kOptFlagUseUDPIP)
    {
        const uint8_t lFirstValue = 0;

        // For UDP, we'll send n aSize or smaller datagrams, each
        // patterned from zero to aSize - 1.

        lBuffer = Common::MakeDataBuffer(aSize, lFirstValue);
        VerifyOrReturnError(!lBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        return sUDPIPEndPoint->SendTo(aAddress, kUDPPort, std::move(lBuffer));
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR DriveSendForGroup(GroupAddress & aGroupAddress)
{
    if (aGroupAddress.mStats.mTransmit.mActual < aGroupAddress.mStats.mTransmit.mExpected)
    {
        ReturnErrorOnFailure(DriveSendForDestination(aGroupAddress.mMulticastAddress, gSendSize));

        aGroupAddress.mStats.mTransmit.mActual++;

        printf("%u/%u transmitted for multicast group %u\n", aGroupAddress.mStats.mTransmit.mActual,
               aGroupAddress.mStats.mTransmit.mExpected, aGroupAddress.mGroup);
    }

    return CHIP_NO_ERROR;
}

template <size_t tCapacity>
static CHIP_ERROR DriveSendForGroups(GroupAddresses<tCapacity> & aGroupAddresses)
{
    // Iterate over each multicast group for which this node is a
    // member and send a packet.
    for (size_t i = 0; i < aGroupAddresses.mSize; i++)
    {
        ReturnErrorOnFailure(DriveSendForGroup(aGroupAddresses.mAddresses[i]));
    }

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
        gSystemLayer.StartTimer(gSendIntervalMs, Common::HandleSendTimerComplete, nullptr);

        lStatus = DriveSendForGroups(sGroupAddresses);
        SuccessOrExit(lStatus);
    }

exit:
    if (lStatus != CHIP_NO_ERROR)
    {
        SetStatusFailed(sTestState.mStatus);
    }
}

static void StartTest()
{
    IPAddressType lIPAddressType = kIPAddressType_IPv6;
    IPProtocol lIPProtocol       = kIPProtocol_ICMPv6;
    IPVersion lIPVersion         = kIPVersion_6;
    IPAddress lAddress           = IPAddress::Any;
    IPEndPointBasis * lEndPoint  = nullptr;
    const bool lUseLoopback      = ((gOptFlags & kOptFlagNoLoopback) == 0);
    CHIP_ERROR lStatus;

#if INET_CONFIG_ENABLE_IPV4
    if (gOptFlags & kOptFlagUseIPv4)
    {
        lIPAddressType = kIPAddressType_IPv4;
        lIPProtocol    = kIPProtocol_ICMPv4;
        lIPVersion     = kIPVersion_4;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    // clang-format off
    printf("Using %sIP%s, device interface: %s (w/%c LwIP)\n",
           ((gOptFlags & kOptFlagUseRawIP) ? "" : "UDP/"),
           ((gOptFlags & kOptFlagUseIPv4) ? "v4" : "v6"),
           ((gInterfaceName) ? gInterfaceName : "<none>"),
           (CHIP_SYSTEM_CONFIG_USE_LWIP ? '\0' : 'o'));
	// clang-format ob

    // Allocate the endpoints for sending or receiving.

    if (gOptFlags & kOptFlagUseRawIP)
    {
        lStatus = gInet.NewRawEndPoint(lIPVersion, lIPProtocol, &sRawIPEndPoint);
        INET_FAIL_ERROR(lStatus, "InetLayer::NewRawEndPoint failed");

        lStatus = sRawIPEndPoint->Bind(lIPAddressType, lAddress);
        INET_FAIL_ERROR(lStatus, "RawEndPoint::Bind failed");

        if (gOptFlags & kOptFlagUseIPv6)
        {
            lStatus = sRawIPEndPoint->SetICMPFilter(kICMPv6_FilterTypes, gICMPv6Types);
            INET_FAIL_ERROR(lStatus, "RawEndPoint::SetICMPFilter (IPv6) failed");
        }

        if (IsInterfaceIdPresent(gInterfaceId))
        {
            lStatus = sRawIPEndPoint->BindInterface(lIPAddressType, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "RawEndPoint::BindInterface failed");
        }

        lStatus = sRawIPEndPoint->Listen(HandleRawMessageReceived, HandleRawReceiveError);
        INET_FAIL_ERROR(lStatus, "RawEndPoint::Listen failed");

        lEndPoint = sRawIPEndPoint;
    }
    else if (gOptFlags & kOptFlagUseUDPIP)
    {
        lStatus = gInet.NewUDPEndPoint(&sUDPIPEndPoint);
        INET_FAIL_ERROR(lStatus, "InetLayer::NewUDPEndPoint failed");

        lStatus = sUDPIPEndPoint->Bind(lIPAddressType, lAddress, kUDPPort);
        INET_FAIL_ERROR(lStatus, "UDPEndPoint::Bind failed");

        if (IsInterfaceIdPresent(gInterfaceId))
        {
            lStatus = sUDPIPEndPoint->BindInterface(lIPAddressType, gInterfaceId);
            INET_FAIL_ERROR(lStatus, "UDPEndPoint::BindInterface failed");
        }

        lStatus = sUDPIPEndPoint->Listen(HandleUDPMessageReceived, HandleUDPReceiveError);
        INET_FAIL_ERROR(lStatus, "UDPEndPoint::Listen failed");

        lEndPoint = sUDPIPEndPoint;
    }

    // If loopback suppression has been requested, attempt to disable
    // it; otherwise, attempt to enable it.

    lStatus = lEndPoint->SetMulticastLoopback(lIPVersion, lUseLoopback);
    INET_FAIL_ERROR(lStatus, "SetMulticastLoopback failed");

    // Configure and join the multicast groups

    for (size_t i = 0; i < sGroupAddresses.mSize; i++)
    {
        char lAddressBuffer[INET6_ADDRSTRLEN];
        GroupAddress & lGroupAddress  = sGroupAddresses.mAddresses[i];
        IPAddress & lMulticastAddress = lGroupAddress.mMulticastAddress;

        if ((lEndPoint != nullptr) && IsInterfaceIdPresent(gInterfaceId))
        {
            if (gOptFlags & kOptFlagUseIPv4)
            {
                lMulticastAddress = MakeIPv4Multicast(lGroupAddress.mGroup);
            }
            else
            {
                lMulticastAddress = MakeIPv6Multicast(lGroupAddress.mGroup);
            }

            lMulticastAddress.ToString(lAddressBuffer, sizeof(lAddressBuffer));

            printf("Will join multicast group %s\n", lAddressBuffer);

            lStatus = lEndPoint->JoinMulticastGroup(gInterfaceId, lMulticastAddress);
            INET_FAIL_ERROR(lStatus, "Could not join multicast group");
        }
    }

    if (Common::IsReceiver())
        printf("Listening...\n");
    else
        DriveSend();
}

static void CleanupTest()
{
    IPEndPointBasis * lEndPoint = nullptr;
    CHIP_ERROR lStatus;

    gSendIntervalExpired = false;
    gSystemLayer.CancelTimer(Common::HandleSendTimerComplete, nullptr);

    //  Leave the multicast groups

    if (gOptFlags & kOptFlagUseRawIP)
    {
        lEndPoint = sRawIPEndPoint;
    }
    else if (gOptFlags & kOptFlagUseUDPIP)
    {
        lEndPoint = sUDPIPEndPoint;
    }

    for (size_t i = 0; i < sGroupAddresses.mSize; i++)
    {
        char lAddressBuffer[INET6_ADDRSTRLEN];
        GroupAddress & lGroupAddress  = sGroupAddresses.mAddresses[i];
        IPAddress & lMulticastAddress = lGroupAddress.mMulticastAddress;

        if ((lEndPoint != nullptr) && IsInterfaceIdPresent(gInterfaceId))
        {
            lMulticastAddress.ToString(lAddressBuffer, sizeof(lAddressBuffer));

            printf("Will leave multicast group %s\n", lAddressBuffer);

            lStatus = lEndPoint->LeaveMulticastGroup(gInterfaceId, lMulticastAddress);
            INET_FAIL_ERROR(lStatus, "Could not leave multicast group");
        }
    }

    // Release the resources associated with the allocated end points.

    if (sRawIPEndPoint != nullptr)
    {
        sRawIPEndPoint->Free();
    }

    if (sUDPIPEndPoint != nullptr)
    {
        sUDPIPEndPoint->Free();
    }
}
