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
 *      The defines constants for the CHIP Common Protocol, present in
 *      every CHIP device.
 *
 */

#pragma once

/**
 *   @namespace chip::Protocols::Common
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Common profile.
 *
 *     The interfaces define message types and status codes.
 */

namespace chip {
namespace Protocols {
namespace Common {

/**
 * Common Profile Message Types
 */
enum class MsgType
{
    // Status Report contians operation results that a responder sends as a reply for requests sent from an initiator.
    StatusReport = 1
};

/**
 * Common Protocol Status Codes
 */
enum class StatusCode
{
    Success           = 0,  /**< Operation completed successfully. */
    Failure           = 1,  /**< Generic failure, additional details may be included in the protocol specific status. */
    BadPrecondition   = 2,  /**< Operation was rejected by the system because the system is in an invalid state. */
    OutOfRange        = 3,  /**< A value was out of a required range. */
    BadRequest        = 4,  /**< A request was unrecognized or malformed. */
    Unsupported       = 5,  /**< An unrecognized or unsupported request was received. */
    Unexpected        = 6,  /**< A request was not expected at this time. */
    ResourceExhausted = 7,  /**< Insufficient resources to process the given request. */
    Busy              = 8,  /**< Device is busy and cannot handle this request at this time. */
    Timeout           = 9,  /**< A timeout occurred. */
    Continue          = 10, /**< Context-specific signal to proceed. */
    Aborted           = 11, /**< Failure, often due to a concurrency error. */
    InvalidArgument   = 12, /**< An invalid/unsupported argument was provided. */
    NotFound          = 13, /**< Some requested entity was not found. */
    AlreadyExists     = 14, /**< The caller attempted to create something that already exists. */
    PermissionDenied  = 15, /**< Caller does not have sufficient permissions to execute the requested operations. */
    DataLoss          = 16, /**< Unrecoverable data loss or corruption has occurred. */
};

} // namespace Common
} // namespace Protocols
} // namespace chip
