/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CHIP_TEST_INETLAYER_COMMON_HPP
#define CHIP_TEST_INETLAYER_COMMON_HPP

#include <stddef.h>
#include <stdint.h>

#include <inet/InetError.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

// Preprocessor Macros

#define SetStatusFailed(aTestStatus)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        (aTestStatus).mFailed = true;                                                                                              \
        fprintf(stderr, "Test failed at %s:%u!\n", __func__, __LINE__);                                                            \
    } while (0)

#define kToolOptBase 1000

#define kToolOptInterface 'I'
#define kToolOptIPv4Only '4'
#define kToolOptIPv6Only '6'
#define kToolOptInterval 'i'
#define kToolOptListen 'l'
#define kToolOptSendSize 's'
#define kToolOptUDPIP 'u'

// Type Definitions

enum OptFlagsCommon
{
    kOptFlagUseIPv4 = 0x00000001,
    kOptFlagUseIPv6 = 0x00000002,

    kOptFlagUseUDPIP = 0x00000008,

    kOptFlagListen = 0x00000010,
};

enum kICMPTypeIndex
{
    kICMP_EchoRequestIndex = 0,
    kICMP_EchoReplyIndex   = 1
};

struct Stats
{
    uint32_t mExpected;
    uint32_t mActual;
};

struct TransferStats
{
    struct Stats mReceive;
    struct Stats mTransmit;
};

struct TestStatus
{
    bool mFailed;
    bool mSucceeded;
};

// Global Variables

static const uint16_t kUDPPort = 4242;

static const size_t kICMPv4_FilterTypes = 2;

static const size_t kICMPv6_FilterTypes = 2;

extern const uint8_t gICMPv4Types[kICMPv4_FilterTypes];

extern const uint8_t gICMPv6Types[kICMPv6_FilterTypes];

extern bool gSendIntervalExpired;

extern uint32_t gSendIntervalMs;

extern const char * gInterfaceName;

extern chip::Inet::InterfaceId gInterfaceId;

extern uint16_t gSendSize;

extern uint32_t gOptFlags;

// Function Prototypes

namespace Common {

extern bool IsReceiver();
extern bool IsSender();

extern bool IsTesting(const TestStatus & aTestStatus);
extern bool WasSuccessful(const TestStatus & aTestStatus);

extern chip::System::PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength, uint8_t aFirstValue);
extern chip::System::PacketBufferHandle MakeDataBuffer(uint16_t aDesiredLength);
extern chip::System::PacketBufferHandle MakeICMPv4DataBuffer(uint16_t aDesiredUserLength);
extern chip::System::PacketBufferHandle MakeICMPv6DataBuffer(uint16_t aDesiredUserLength);

extern bool HandleDataReceived(const chip::System::PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket,
                               bool aCheckBuffer, uint8_t aFirstValue);
extern bool HandleDataReceived(const chip::System::PacketBufferHandle & aBuffer, TransferStats & aStats, bool aStatsByPacket,
                               bool aCheckBuffer);
extern bool HandleICMPv4DataReceived(chip::System::PacketBufferHandle && aBuffer, TransferStats & aStats, bool aStatsByPacket,
                                     bool aCheckBuffer);
extern bool HandleICMPv6DataReceived(chip::System::PacketBufferHandle && aBuffer, TransferStats & aStats, bool aStatsByPacket,
                                     bool aCheckBuffer);

// Timer Callback Handler

extern void HandleSendTimerComplete(chip::System::Layer * aSystemLayer, void * aAppState);

// UDP Endpoint Callback Handlers

extern void HandleUDPMessageReceived(const chip::Inet::UDPEndPoint * aEndPoint, const chip::System::PacketBufferHandle & aBuffer,
                                     const chip::Inet::IPPacketInfo * aPacketInfo);
extern void HandleUDPReceiveError(const chip::Inet::UDPEndPoint * aEndPoint, const CHIP_ERROR & aError,
                                  const chip::Inet::IPPacketInfo * aPacketInfo);

} // namespace Common

// Period send function to be implemented by individual tests but
// referenced by common code.

extern void DriveSend();

#endif // CHIP_TEST_INETLAYER_COMMON_HPP
