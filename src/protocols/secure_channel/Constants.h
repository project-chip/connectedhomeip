/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      The defines constants for the CHIP Secure Channel Protocol, present in
 *      every CHIP device.
 *
 */

#pragma once

#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>

/**
 *   @namespace chip::Protocols::SecureChannel
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP SecureChannel protocol.
 *
 *     The interfaces define message types and status codes.
 */

namespace chip {
namespace Protocols {
namespace SecureChannel {

/**
 * SecureChannel Protocol Message Types
 */
enum class MsgType : uint8_t
{
    // Message Counter Synchronization Protocol Message Types
    MsgCounterSyncReq = 0x00,
    MsgCounterSyncRsp = 0x01,

    // Reliable Messaging Protocol Message Types
    StandaloneAck = 0x10,

    // Password-based session establishment Message Types
    PBKDFParamRequest  = 0x20,
    PBKDFParamResponse = 0x21,
    PASE_Spake2p1      = 0x22,
    PASE_Spake2p2      = 0x23,
    PASE_Spake2p3      = 0x24,
    PASE_Spake2pError  = 0x2F,

    // Certificate-based session establishment Message Types
    CASE_SigmaR1  = 0x30,
    CASE_SigmaR2  = 0x31,
    CASE_SigmaR3  = 0x32,
    CASE_SigmaErr = 0x3F,

    StatusReport = 0x40,
};

// Placeholder value for the ProtocolCode field when the GeneralCode is Success or Continue.
constexpr uint16_t kProtocolCodeSuccess = 0x0000;

// Placeholder value for the ProtocolCode field when there is no additional protocol-specific code to provide more information.
constexpr uint16_t kProtocolCodeGeneralFailure = 0xFFFF;

/**
 * Status Report - General Status Codes used to convey protocol-agnostic status info.
 */
enum class GeneralStatusCode : uint16_t
{
    kSuccess           = 0,  /**< Operation completed successfully. */
    kFailure           = 1,  /**< Generic failure, additional details may be included in the protocol specific status. */
    kBadPrecondition   = 2,  /**< Operation was rejected by the system because the system is in an invalid state. */
    kOutOfRange        = 3,  /**< A value was out of a required range. */
    kBadRequest        = 4,  /**< A request was unrecognized or malformed. */
    kUnsupported       = 5,  /**< An unrecognized or unsupported request was received. */
    kUnexpected        = 6,  /**< A request was not expected at this time. */
    kResourceExhausted = 7,  /**< Insufficient resources to process the given request. */
    kBusy              = 8,  /**< Device is busy and cannot handle this request at this time. */
    kTimeout           = 9,  /**< A timeout occurred. */
    kContinue          = 10, /**< Context-specific signal to proceed. */
    kAborted           = 11, /**< Failure, often due to a concurrency error. */
    kInvalidArgument   = 12, /**< An invalid/unsupported argument was provided. */
    kNotFound          = 13, /**< Some requested entity was not found. */
    kAlreadyExists     = 14, /**< The caller attempted to create something that already exists. */
    kPermissionDenied  = 15, /**< Caller does not have sufficient permissions to execute the requested operations. */
    kDataLoss          = 16, /**< Unrecoverable data loss or corruption has occurred. */
};

/**
 * Status Report - Status Codes specific only to the SecureChannel Protocol
 */
enum class StatusCode
{
    AlreadyMemberOfFabric = 1, /**< The recipient is already a member of a fabric. */
    NotMemberOfFabric     = 2, /**< The recipient is not a member of a fabric. */
    InvalidFabricConfig   = 3  /**< The specified fabric configuration was invalid. */
};

} // namespace SecureChannel

template <>
struct MessageTypeTraits<SecureChannel::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return SecureChannel::Id; }
};

} // namespace Protocols
} // namespace chip
