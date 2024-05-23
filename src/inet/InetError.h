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
 */

#pragma once

#include <inet/InetConfig.h>

#include <lib/core/CHIPError.h>
#include <system/SystemError.h>

#define CHIP_INET_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kInet, (e))

// clang-format off

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
#define INET_ERROR_WRONG_ADDRESS_TYPE                       CHIP_INET_ERROR(0x01)

/**
 *  @def INET_ERROR_PEER_DISCONNECTED
 *
 *  @brief
 *    A remote connection peer disconnected.
 *
 */
#define INET_ERROR_PEER_DISCONNECTED                        CHIP_INET_ERROR(0x02)

// unused                                                   CHIP_INET_ERROR(0x03)
// unused                                                   CHIP_INET_ERROR(0x04)
// unused                                                   CHIP_INET_ERROR(0x05)
// unused                                                   CHIP_INET_ERROR(0x06)
// unused                                                   CHIP_INET_ERROR(0x07)
// unused                                                   CHIP_INET_ERROR(0x08)

/**
 *  @def INET_ERROR_HOST_NOT_FOUND
 *
 *  @brief
 *    A requested host name could not be resolved to an address.
 *
 */
#define INET_ERROR_HOST_NOT_FOUND                           CHIP_INET_ERROR(0x09)

/**
 *  @def INET_ERROR_DNS_TRY_AGAIN
 *
 *  @brief
 *    A name server returned a temporary failure indication; try
 *    again later.
 *
 */
#define INET_ERROR_DNS_TRY_AGAIN                            CHIP_INET_ERROR(0x0a)

/**
 *  @def INET_ERROR_DNS_NO_RECOVERY
 *
 *  @brief
 *    A name server returned an unrecoverable error.
 *
 */
#define INET_ERROR_DNS_NO_RECOVERY                          CHIP_INET_ERROR(0x0b)

// unused                                                   CHIP_INET_ERROR(0x0c)

/**
 *  @def INET_ERROR_WRONG_PROTOCOL_TYPE
 *
 *  @brief
 *    An incorrect or unexpected protocol type was encountered.
 *
 */
#define INET_ERROR_WRONG_PROTOCOL_TYPE                      CHIP_INET_ERROR(0x0d)

/**
 *  @def INET_ERROR_UNKNOWN_INTERFACE
 *
 *  @brief
 *    An unknown interface identifier was encountered.
 *
 */
#define INET_ERROR_UNKNOWN_INTERFACE                        CHIP_INET_ERROR(0x0e)

// unused                                                   CHIP_INET_ERROR(0x0f)

/**
 *  @def INET_ERROR_ADDRESS_NOT_FOUND
 *
 *  @brief
 *    A requested address type, class, or scope cannot be found.
 *
 */
#define INET_ERROR_ADDRESS_NOT_FOUND                        CHIP_INET_ERROR(0x10)

/**
 *  @def INET_ERROR_HOST_NAME_TOO_LONG
 *
 *  @brief
 *    A requested host name is too long.
 *
 */
#define INET_ERROR_HOST_NAME_TOO_LONG                       CHIP_INET_ERROR(0x11)

/**
 *  @def INET_ERROR_INVALID_HOST_NAME
 *
 *  @brief
 *    A requested host name and port is invalid.
 *
 */
#define INET_ERROR_INVALID_HOST_NAME                        CHIP_INET_ERROR(0x12)

// unused                                                   CHIP_INET_ERROR(0x13)
// unused                                                   CHIP_INET_ERROR(0x14)

/**
 *  @def INET_ERROR_IDLE_TIMEOUT
 *
 *  @brief
 *    A TCP connection timed out due to inactivity.
 *
 */
#define INET_ERROR_IDLE_TIMEOUT                             CHIP_INET_ERROR(0x15)

// unused                                                   CHIP_INET_ERROR(0x16)

/**
 *  @def INET_ERROR_INVALID_IPV6_PKT
 *
 *  @brief
 *    An IPv6 packet is invalid.
 *
 */
#define INET_ERROR_INVALID_IPV6_PKT                         CHIP_INET_ERROR(0x17)

/**
 *  @def INET_ERROR_INTERFACE_INIT_FAILURE
 *
 *  @brief
 *    Failure to initialize an interface.
 *
 */
#define INET_ERROR_INTERFACE_INIT_FAILURE                   CHIP_INET_ERROR(0x18)

/**
 *  @def INET_ERROR_TCP_USER_TIMEOUT
 *
 *  @brief
 *    TCP Connection timed out waiting for
 *    acknowledgment for transmitted packet.
 *
 */
#define INET_ERROR_TCP_USER_TIMEOUT                         CHIP_INET_ERROR(0x19)

/**
 *  @def INET_ERROR_TCP_CONNECT_TIMEOUT
 *
 *  @brief
 *    TCP Connection timed out waiting for
 *    a successful connection or a report
 *    of an error.
 *
 */
#define INET_ERROR_TCP_CONNECT_TIMEOUT                      CHIP_INET_ERROR(0x1a)

/**
 *  @def INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE
 *
 *  @brief
 *    The supplied text-form IP address was not compatible with the requested
 *    IP address type.
 *
 */
#define INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE             CHIP_INET_ERROR(0x1b)

//                        !!!!! IMPORTANT !!!!!
//
// If you add new Inet errors, please update the translation of error
// codes to strings in InetError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

/**
 *  @}
 */

// clang-format on

namespace chip {
namespace Inet {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace Inet
} // namespace chip
