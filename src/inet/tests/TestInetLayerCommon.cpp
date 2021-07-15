/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC
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
 *      This file defines common preprocessor defintions, constants,
 *      functions, and globals for unit and functional tests for the
 *      Inet layer.
 *
 */

#include "TestInetLayerCommon.hpp"

#include <stdlib.h>
#include <string.h>
#include <utility>

#include <nlbyteorder.hpp>

#include <inet/InetLayer.h>

#include <support/CodeUtils.h>

#include "TestInetCommon.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

// Type Definitions

struct ICMPEchoHeader
{
    uint8_t mType;
    uint8_t mCode;
    uint16_t mChecksum;
    uint16_t mID;
    uint16_t mSequenceNumber;
} __attribute__((packed));

typedef struct ICMPEchoHeader ICMPv4EchoHeader;
typedef struct ICMPEchoHeader ICMPv6EchoHeader;

// Global Variables

static const uint8_t kICMPv4_EchoRequest = 8;
static const uint8_t kICMPv4_EchoReply   = 0;

static const uint8_t kICMPv6_EchoRequest = 128;
static const uint8_t kICMPv6_EchoReply   = 129;

// clang-format off
const uint8_t            gICMPv4Types[kICMPv4_FilterTypes] =
{
    kICMPv4_EchoRequest,
    kICMPv4_EchoReply
};

const uint8_t            gICMPv6Types[kICMPv6_FilterTypes] =
{
    kICMPv6_EchoRequest,
    kICMPv6_EchoReply
};
// clang-format on

bool gSendIntervalExpired = true;

uint32_t gSendIntervalMs = 1000;

const char * gInterfaceName = nullptr;

InterfaceId gInterfaceId = INET_NULL_INTERFACEID;

uint16_t gSendSize = 59;

uint32_t gOptFlags = 0;

namespace Common {

bool IsReceiver()
{
    return ((gOptFlags & kOptFlagListen) == kOptFlagListen);
}

bool IsSender()
{
    return (!IsReceiver());
}

bool IsTesting(const TestStatus & aTestStatus)
{
    bool lStatus;

    lStatus = (!aTestStatus.mFailed && !aTestStatus.mSucceeded);

    return (lStatus);
}

bool WasSuccessful(const TestStatus & aTestStatus)
{
    bool lStatus = false;

    if (aTestStatus.mFailed)
        lStatus = false;
    else if (aTestStatus.mSucceeded)
        lStatus = true;

    return (lStatus);
}

static void FillDataBufferPattern(uint8_t * aBuffer, uint16_t aLength, uint16_t aPatternStartOffset, uint8_t aFirstValue)
{
    for (uint16_t i = aPatternStartOffset; i < aLength; i++)
    {
        const uint8_t lValue = static_cast<uint8_t>(aFirstValue & 0xFF);

        aBuffer[i] = lValue;

        aFirstValue++;
    }
}

static bool CheckDataBufferPattern(const uint8_t * aBuffer, uint16_t aLength, uint16_t aPatternStartOffset, uint8_t aFirstValue)
{
    for (uint16_t i = aPatternStartOffset; i < aLength; i++)
    {
        const uint8_t lValue = aBuffer[i];

        if (lValue != static_cast<uint8_t>(aFirstValue))
        {
            printf("Bad data value at offset %u (0x%04x): "
                   "expected 0x%02x, found 0x%02x\n",
                   i, i, aFirstValue, lValue);
            DumpMemory(aBuffer + aPatternStartOffset, aLength - aPatternStartOffset, "0x", 16);
            return false;
        }

        aFirstValue++;
    }

    return true;
}

static PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength, uint16_t aPatternStartOffset, uint8_t aFirstValue)
{
    VerifyOrReturnError(aPatternStartOffset <= aDesiredLength, PacketBufferHandle());

    PacketBufferHandle lBuffer = PacketBufferHandle::New(aDesiredLength);
    VerifyOrReturnError(!lBuffer.IsNull(), lBuffer);

    aDesiredLength = min(lBuffer->MaxDataLength(), aDesiredLength);

    FillDataBufferPattern(lBuffer->Start(), aDesiredLength, aPatternStartOffset, aFirstValue);

    lBuffer->SetDataLength(aDesiredLength);

    return lBuffer;
}

static PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength, uint16_t aPatternStartOffset)
{
    constexpr uint8_t lFirstValue = 0;
    return MakeDataBuffer(aDesiredLength, aPatternStartOffset, lFirstValue);
}

template <typename tType>
static PacketBufferHandle MakeICMPDataBuffer(uint16_t aDesiredUserLength, uint16_t aHeaderLength, uint16_t aPatternStartOffset,
                                             uint8_t aType)
{
    static uint16_t lSequenceNumber = 0;
    PacketBufferHandle lBuffer;

    // To ensure there is enough room for the user data and the ICMP
    // header, include both the user data size and the ICMP header length.

    lBuffer = MakeDataBuffer(static_cast<uint16_t>(aDesiredUserLength + aHeaderLength), aPatternStartOffset);

    if (!lBuffer.IsNull())
    {
        tType * lHeader = reinterpret_cast<tType *>(lBuffer->Start());

        lHeader->mType           = aType;
        lHeader->mCode           = 0;
        lHeader->mChecksum       = 0;
        lHeader->mID             = static_cast<uint16_t>(rand() & UINT16_MAX);
        lHeader->mSequenceNumber = nlByteOrderSwap16HostToBig(lSequenceNumber++);
    }

    return (lBuffer);
}

PacketBufferHandle MakeICMPv4DataBuffer(uint16_t aDesiredUserLength)
{
    constexpr uint16_t lICMPHeaderLength   = sizeof(ICMPv4EchoHeader);
    constexpr uint16_t lPatternStartOffset = lICMPHeaderLength;
    const uint8_t lType                    = gICMPv4Types[kICMP_EchoRequestIndex];

    return MakeICMPDataBuffer<ICMPv4EchoHeader>(aDesiredUserLength, lICMPHeaderLength, lPatternStartOffset, lType);
}

PacketBufferHandle MakeICMPv6DataBuffer(uint16_t aDesiredUserLength)
{
    constexpr uint16_t lICMPHeaderLength   = sizeof(ICMPv6EchoHeader);
    constexpr uint16_t lPatternStartOffset = lICMPHeaderLength;
    const uint8_t lType                    = gICMPv6Types[kICMP_EchoRequestIndex];

    return MakeICMPDataBuffer<ICMPv6EchoHeader>(aDesiredUserLength, lICMPHeaderLength, lPatternStartOffset, lType);
}

PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength, uint8_t aFirstValue)
{
    constexpr uint16_t lPatternStartOffset = 0;
    return MakeDataBuffer(aDesiredLength, lPatternStartOffset, aFirstValue);
}

PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength)
{
    constexpr uint16_t lPatternStartOffset = 0;
    return MakeDataBuffer(aDesiredLength, lPatternStartOffset);
}

static bool HandleDataReceived(const PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer,
                               uint16_t aPatternStartOffset, uint8_t aFirstValue)
{
    uint16_t lTotalDataLength = 0;

    // Walk through each buffer in the packet chain, checking the
    // buffer for the expected pattern, if requested.

    for (PacketBufferHandle lBuffer = aBuffer.Retain(); !lBuffer.IsNull(); lBuffer.Advance())
    {
        const uint16_t lDataLength = lBuffer->DataLength();
        const uint8_t * const p    = lBuffer->Start();

        if (aCheckBuffer && !CheckDataBufferPattern(p, lDataLength, aPatternStartOffset, aFirstValue))
        {
            return false;
        }

        lTotalDataLength = static_cast<uint16_t>(lTotalDataLength + lDataLength);
        aFirstValue      = static_cast<uint8_t>(aFirstValue + lDataLength);
    }

    // If we are accumulating stats by packet rather than by size,
    // then increment by one (1) rather than the total buffer length.

    aStats.mReceive.mActual += ((aStatsByPacket) ? 1 : lTotalDataLength);

    return true;
}

static bool HandleICMPDataReceived(PacketBufferHandle aBuffer, uint16_t aHeaderLength, TransferStats & aStats, bool aStatsByPacket,
                                   bool aCheckBuffer)
{
    const uint16_t lPatternStartOffset = 0;
    bool lStatus;

    aBuffer->ConsumeHead(aHeaderLength);

    lStatus = HandleDataReceived(aBuffer, aStats, aStatsByPacket, aCheckBuffer, lPatternStartOffset);

    return (lStatus);
}

bool HandleICMPv4DataReceived(PacketBufferHandle && aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer)
{
    const uint16_t lICMPHeaderLength = sizeof(ICMPv4EchoHeader);
    bool lStatus;

    lStatus = HandleICMPDataReceived(std::move(aBuffer), lICMPHeaderLength, aStats, aStatsByPacket, aCheckBuffer);

    return (lStatus);
}

bool HandleICMPv6DataReceived(PacketBufferHandle && aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer)
{
    const uint16_t lICMPHeaderLength = sizeof(ICMPv6EchoHeader);
    bool lStatus;

    lStatus = HandleICMPDataReceived(std::move(aBuffer), lICMPHeaderLength, aStats, aStatsByPacket, aCheckBuffer);

    return (lStatus);
}

bool HandleDataReceived(const PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer,
                        uint8_t aFirstValue)
{
    const uint16_t lPatternStartOffset = 0;
    bool lStatus;

    lStatus = HandleDataReceived(aBuffer, aStats, aStatsByPacket, aCheckBuffer, lPatternStartOffset, aFirstValue);

    return (lStatus);
}

bool HandleDataReceived(const PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer)
{
    const uint8_t lFirstValue          = 0;
    const uint16_t lPatternStartOffset = 0;
    bool lStatus;

    lStatus = HandleDataReceived(aBuffer, aStats, aStatsByPacket, aCheckBuffer, lPatternStartOffset, lFirstValue);

    return (lStatus);
}

bool HandleUDPDataReceived(const PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer)
{
    bool lStatus;

    lStatus = HandleDataReceived(aBuffer, aStats, aStatsByPacket, aCheckBuffer);

    return (lStatus);
}

bool HandleTCPDataReceived(const PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket, bool aCheckBuffer)
{
    bool lStatus;

    lStatus = HandleDataReceived(aBuffer, aStats, aStatsByPacket, aCheckBuffer);

    return (lStatus);
}

// Timer Callback Handler

void HandleSendTimerComplete(System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    INET_FAIL_ERROR(aError, "Send timer completed with error");

    gSendIntervalExpired = true;

    DriveSend();
}

// Raw Endpoint Callback Handlers

void HandleRawMessageReceived(const IPEndPointBasis * aEndPoint, const PacketBufferHandle & aBuffer,
                              const IPPacketInfo * aPacketInfo)
{
    char lSourceAddressBuffer[INET6_ADDRSTRLEN];
    char lDestinationAddressBuffer[INET6_ADDRSTRLEN];

    aPacketInfo->SrcAddress.ToString(lSourceAddressBuffer);
    aPacketInfo->DestAddress.ToString(lDestinationAddressBuffer);

    printf("Raw message received from %s to %s (%zu bytes)\n", lSourceAddressBuffer, lDestinationAddressBuffer,
           static_cast<size_t>(aBuffer->DataLength()));
}

void HandleRawReceiveError(const IPEndPointBasis * aEndPoint, const CHIP_ERROR & aError, const IPPacketInfo * aPacketInfo)
{
    char lAddressBuffer[INET6_ADDRSTRLEN];

    if (aPacketInfo != nullptr)
    {
        aPacketInfo->SrcAddress.ToString(lAddressBuffer);
    }
    else
    {
        strcpy(lAddressBuffer, "(unknown)");
    }

    printf("IP receive error from %s %s\n", lAddressBuffer, ErrorStr(aError));
}

// UDP Endpoint Callback Handlers

void HandleUDPMessageReceived(const IPEndPointBasis * aEndPoint, const PacketBufferHandle & aBuffer,
                              const IPPacketInfo * aPacketInfo)
{
    char lSourceAddressBuffer[INET6_ADDRSTRLEN];
    char lDestinationAddressBuffer[INET6_ADDRSTRLEN];

    aPacketInfo->SrcAddress.ToString(lSourceAddressBuffer);
    aPacketInfo->DestAddress.ToString(lDestinationAddressBuffer);

    printf("UDP packet received from %s:%u to %s:%u (%zu bytes)\n", lSourceAddressBuffer, aPacketInfo->SrcPort,
           lDestinationAddressBuffer, aPacketInfo->DestPort, static_cast<size_t>(aBuffer->DataLength()));
}

void HandleUDPReceiveError(const IPEndPointBasis * aEndPoint, const CHIP_ERROR & aError, const IPPacketInfo * aPacketInfo)
{
    char lAddressBuffer[INET6_ADDRSTRLEN];
    uint16_t lSourcePort;

    if (aPacketInfo != nullptr)
    {
        aPacketInfo->SrcAddress.ToString(lAddressBuffer);
        lSourcePort = aPacketInfo->SrcPort;
    }
    else
    {
        strcpy(lAddressBuffer, "(unknown)");
        lSourcePort = 0;
    }

    printf("UDP receive error from %s:%u: %s\n", lAddressBuffer, lSourcePort, ErrorStr(aError));
}

} // namespace Common
