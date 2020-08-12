/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      The defines constants for the CHIP Common Profile, present in
 *      every CHIP device.
 *
 */

#ifndef _COMMON_PROFILE_H
#define _COMMON_PROFILE_H

/**
 *   @namespace chip::Profiles::Common
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Common profile.
 *
 *     The interfaces define message types and status codes.
 */

namespace chip {
namespace Profiles {
namespace Common {

/**
 * Common Profile Message Types
 */
enum
{
    kMsgType_StatusReport = 1,
    kMsgType_Null         = 2,

    // Reliable Messaging Protocol Message Types
    kMsgType_WRMP_Delayed_Delivery = 3,
    kMsgType_WRMP_Throttle_Flow    = 4
};

/**
 * Common Profile Status Codes
 */
enum
{
    kStatus_Success                = 0,      /**< The operation completed without error. */
    kStatus_Canceled               = 1,      /**< The operation was canceled. */
    kStatus_BadRequest             = 0x0010, /**< The request was unrecognized or malformed. */
    kStatus_UnsupportedMessage     = 0x0011, /**< An unrecognized or unsupported message was received. */
    kStatus_UnexpectedMessage      = 0x0012, /**< A message was received at an unexpected time or in an unexpected context. */
    kStatus_AuthenticationRequired = 0x0013, /**< The request can only be made by an authenticated party. */
    kStatus_AccessDenied           = 0x0014, /**< The request can only be made by a party with sufficient access. */
    kStatus_OutOfMemory =
        0x0017, /**< The sender is low on memory resources and cannot perform the requested operation at the current time. */
    kStatus_NotAvailable       = 0x0018, /**< The requested operation cannot be performed given the current state of the sender. */
    kStatus_LocalSetupRequired = 0x0019, /**< The requested operation could not be performed because one or more necessary local
                                            configuration steps have not been completed. */
    kStatus_InternalServerProblem = 0x0020, /**< Request could not be completeted because of problems on the server. */
    kStatus_Relocated             = 0x0030, /**< Request was made to the wrong endpoint. Client should query its
                                                 directory server for an updated endpoint list and try again. */
    kStatus_Busy          = 0x0040, /**< The sender is busy and cannot perform the requested operation at the current time. */
    kStatus_Timeout       = 0x0041, /**< The operation or protocol interaction failed to complete in the allotted time. */
    kStatus_InternalError = 0x0050, /**< An internal failure prevented an operation from completing. */
    kStatus_Continue      = 0x0090, /**< Context-specific signal to proceed. */
};

/**
 * Common Profile MIME file type
 */
enum
{
    kMIMEType_AppOctetStream = 0x0000, /**< The most general unrestricted set of bytes, same as "unspecified." */
    kMIMEType_TextPlain      = 0x0001, /**< Plain text, uncompressed. */
    kMIMEType_AppGzip        = 0x0002, /**< gzip-compressed data. */
};

/**
 * @var   kTag_SystemErrorCode
 * @brief [uint] System-specific error.
 *
 * Tag Category: Profile-specific.  Constraints: none
 *
 * @var   kTag_SchemaVersion
 * @brief [uint] Schema revision.
 *
 * Tag Category: Profile-specific.  Constraints: 0-255
 *
 * @var   kTag_MIMEType
 * @brief [uint] MIME type.
 *
 * Tag Category: Profile-agnostic.  Constraints: none
 */

/**
 * Common Profile Data Tags
 */
enum
{
    //                                      Value       Tag Category      Element Type      Constraints  Description
    //                                      ------------------------------------------------------------------------
    kTag_SystemErrorCode = 0x0001, // Profile-specific  Unsigned Integer  -            System-specific error
    kTag_SchemaVersion   = 0x0002, // Profile-specific  Unsigned Integer  0-255        Schema revision
    kTag_MIMEType        = 0x0004, // Profile-agnostic  Unsigned Integer  -            MIME type
};

} // namespace Common
} // namespace Profiles
} // namespace chip

#endif // _COMMON_PROFILE_H
