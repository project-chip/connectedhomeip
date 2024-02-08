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

#include <array>
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

inline constexpr char kProtocolName[] = "SecureChannel";

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
    PASE_Pake1         = 0x22,
    PASE_Pake2         = 0x23,
    PASE_Pake3         = 0x24,

    // Certificate-based session establishment Message Types
    CASE_Sigma1       = 0x30,
    CASE_Sigma2       = 0x31,
    CASE_Sigma3       = 0x32,
    CASE_Sigma2Resume = 0x33,

    StatusReport = 0x40,

    ICD_CheckIn = 0x50,
};

// Placeholder value for the ProtocolCode field when the GeneralCode is Success or Continue.
inline constexpr uint16_t kProtocolCodeSuccess         = 0x0000;
inline constexpr uint16_t kProtocolCodeNoSharedRoot    = 0x0001;
inline constexpr uint16_t kProtocolCodeInvalidParam    = 0x0002;
inline constexpr uint16_t kProtocolCodeCloseSession    = 0x0003;
inline constexpr uint16_t kProtocolCodeBusy            = 0x0004;
inline constexpr uint16_t kProtocolCodeSessionNotFound = 0x0005;

// Placeholder value for the ProtocolCode field when there is no additional protocol-specific code to provide more information.
inline constexpr uint16_t kProtocolCodeGeneralFailure = 0xFFFF;

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

    static auto GetTypeToNameTable()
    {
        static const std::array<MessageTypeNameLookup, 14> typeToNameTable = {
            {
                { SecureChannel::MsgType::MsgCounterSyncReq, "MsgCounterSyncReq" },
                { SecureChannel::MsgType::MsgCounterSyncRsp, "MsgCounterSyncRsp" },
                { SecureChannel::MsgType::StandaloneAck, "StandaloneAck" },
                { SecureChannel::MsgType::PBKDFParamRequest, "PBKDFParamRequest" },
                { SecureChannel::MsgType::PBKDFParamResponse, "PBKDFParamResponse" },
                { SecureChannel::MsgType::PASE_Pake1, "PASE_Pake1" },
                { SecureChannel::MsgType::PASE_Pake2, "PASE_Pake2" },
                { SecureChannel::MsgType::PASE_Pake3, "PASE_Pake3" },
                { SecureChannel::MsgType::CASE_Sigma1, "CASE_Sigma1" },
                { SecureChannel::MsgType::CASE_Sigma2, "CASE_Sigma2" },
                { SecureChannel::MsgType::CASE_Sigma3, "CASE_Sigma3" },
                { SecureChannel::MsgType::CASE_Sigma2Resume, "CASE_Sigma2Resume" },
                { SecureChannel::MsgType::StatusReport, "StatusReport" },
                { SecureChannel::MsgType::ICD_CheckIn, "ICD_CheckInMessage" },
            },
        };

        return &typeToNameTable;
    }
};

} // namespace Protocols
} // namespace chip
