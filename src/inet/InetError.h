/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines constants for the CHIP InetLayer subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate INET_CONFIG_* or _INET_CONFIG_*
 *      macros.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#include <inet/InetConfig.h>

#include <system/SystemError.h>

// clang-format off

/**
 *  @def CHIP_INET_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for InetLayer errors that allows
 *    mapping such errors into a platform- or system-specific
 *    range. This function may be configured via
 *    #INET_CONFIG_ERROR(e).
 *
 *  @param[in]  e  The InetLayer error to map.
 *
 *  @return The mapped InetLayer error.
 *
 */
#define CHIP_INET_ERROR(e)                                  INET_CONFIG_ERROR(e)

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def INET_ERROR_WRONG_ADDRESS_TYPE
 *
 *  @brief
 *    The Internet Protocol (IP) address type or scope does not match
 *    the expected type or scope.
 *
 */
#define INET_ERROR_WRONG_ADDRESS_TYPE                       CHIP_INET_ERROR(0)

// unused                                                   CHIP_INET_ERROR(1)

/**
 *  @def INET_ERROR_PEER_DISCONNECTED
 *
 *  @brief
 *    A remote connection peer disconnected.
 *
 */
#define INET_ERROR_PEER_DISCONNECTED                        CHIP_INET_ERROR(2)

// unused                                                   CHIP_INET_ERROR(3)
// unused                                                   CHIP_INET_ERROR(4)
// unused                                                   CHIP_INET_ERROR(5)
// unused                                                   CHIP_INET_ERROR(6)
// unused                                                   CHIP_INET_ERROR(7)
// unused                                                   CHIP_INET_ERROR(8)

/**
 *  @def INET_ERROR_HOST_NOT_FOUND
 *
 *  @brief
 *    A requested host name could not be resolved to an address.
 *
 */
#define INET_ERROR_HOST_NOT_FOUND                           CHIP_INET_ERROR(9)

/**
 *  @def INET_ERROR_DNS_TRY_AGAIN
 *
 *  @brief
 *    A name server returned a temporary failure indication; try
 *    again later.
 *
 */
#define INET_ERROR_DNS_TRY_AGAIN                            CHIP_INET_ERROR(10)

/**
 *  @def INET_ERROR_DNS_NO_RECOVERY
 *
 *  @brief
 *    A name server returned an unrecoverable error.
 *
 */
#define INET_ERROR_DNS_NO_RECOVERY                          CHIP_INET_ERROR(11)

// unused                                                   CHIP_INET_ERROR(12)

/**
 *  @def INET_ERROR_WRONG_PROTOCOL_TYPE
 *
 *  @brief
 *    An incorrect or unexpected protocol type was encountered.
 *
 */
#define INET_ERROR_WRONG_PROTOCOL_TYPE                      CHIP_INET_ERROR(13)

/**
 *  @def INET_ERROR_UNKNOWN_INTERFACE
 *
 *  @brief
 *    An unknown interface identifier was encountered.
 *
 */
#define INET_ERROR_UNKNOWN_INTERFACE                        CHIP_INET_ERROR(14)

// unused                                                   CHIP_INET_ERROR(15)

/**
 *  @def INET_ERROR_ADDRESS_NOT_FOUND
 *
 *  @brief
 *    A requested address type, class, or scope cannot be found.
 *
 */
#define INET_ERROR_ADDRESS_NOT_FOUND                        CHIP_INET_ERROR(16)

/**
 *  @def INET_ERROR_HOST_NAME_TOO_LONG
 *
 *  @brief
 *    A requested host name is too long.
 *
 */
#define INET_ERROR_HOST_NAME_TOO_LONG                       CHIP_INET_ERROR(17)

/**
 *  @def INET_ERROR_INVALID_HOST_NAME
 *
 *  @brief
 *    A requested host name and port is invalid.
 *
 */
#define INET_ERROR_INVALID_HOST_NAME                        CHIP_INET_ERROR(18)

// unused                                                   CHIP_INET_ERROR(19)
// unused                                                   CHIP_INET_ERROR(20)

/**
 *  @def INET_ERROR_IDLE_TIMEOUT
 *
 *  @brief
 *    A TCP connection timed out due to inactivity.
 *
 */
#define INET_ERROR_IDLE_TIMEOUT                             CHIP_INET_ERROR(21)

// unused                                                   CHIP_INET_ERROR(22)

/**
 *  @def INET_ERROR_INVALID_IPV6_PKT
 *
 *  @brief
 *    An IPv6 packet is invalid.
 *
 */
#define INET_ERROR_INVALID_IPV6_PKT                         CHIP_INET_ERROR(23)

/**
 *  @def INET_ERROR_INTERFACE_INIT_FAILURE
 *
 *  @brief
 *    Failure to initialize an interface.
 *
 */
#define INET_ERROR_INTERFACE_INIT_FAILURE                   CHIP_INET_ERROR(24)

/**
 *  @def INET_ERROR_TCP_USER_TIMEOUT
 *
 *  @brief
 *    TCP Connection timed out waiting for
 *    acknowledgment for transmitted packet.
 *
 */
#define INET_ERROR_TCP_USER_TIMEOUT                         CHIP_INET_ERROR(25)

/**
 *  @def INET_ERROR_TCP_CONNECT_TIMEOUT
 *
 *  @brief
 *    TCP Connection timed out waiting for
 *    a successful connection or a report
 *    of an error.
 *
 */
#define INET_ERROR_TCP_CONNECT_TIMEOUT                      CHIP_INET_ERROR(26)

/**
 *  @def INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE
 *
 *  @brief
 *    The supplied text-form IP address was not compatible with the requested
 *    IP address type.
 *
 */
#define INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE             CHIP_INET_ERROR(27)

//                        !!!!! IMPORTANT !!!!!
//
// If you add new Inet errors, please update the translation of error
// codes to strings in InetError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

/**
 *  @}
 */

// !!!!! IMPORTANT !!!!!
// These definitions are present temporarily in order to reduce breakage for PRs in flight.
// TODO: remove compatibility definitions
#define INET_ERROR                              CHIP_ERROR
#define INET_NO_ERROR                           CHIP_NO_ERROR
#define INET_ERROR_BAD_ARGS                     CHIP_ERROR_INVALID_ARGUMENT
#define INET_ERROR_INBOUND_MESSAGE_TOO_BIG      CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG
#define INET_ERROR_INCORRECT_STATE              CHIP_ERROR_INCORRECT_STATE
#define INET_ERROR_MESSAGE_TOO_LONG             CHIP_ERROR_MESSAGE_TOO_LONG
#define INET_ERROR_NO_CONNECTION_HANDLER        CHIP_ERROR_NO_CONNECTION_HANDLER
#define INET_ERROR_NO_ENDPOINTS                 CHIP_ERROR_ENDPOINT_POOL_FULL
#define INET_ERROR_NOT_IMPLEMENTED              CHIP_ERROR_NOT_IMPLEMENTED
#define INET_ERROR_NOT_SUPPORTED                CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
#define INET_ERROR_NO_MEMORY                    CHIP_ERROR_NO_MEMORY
#define INET_ERROR_CONNECTION_ABORTED           CHIP_ERROR_CONNECTION_ABORTED

// clang-format on

namespace chip {
namespace Inet {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace Inet
} // namespace chip
