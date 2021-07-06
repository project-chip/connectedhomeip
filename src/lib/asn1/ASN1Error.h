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
 *      This file defines constants for the CHIP ASN1 subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate ASN1_CONFIG_* or _ASN1_CONFIG_*
 *      macros.
 *
 */

#pragma once

#include "ASN1Config.h"

#include <core/CHIPError.h>

namespace chip {
namespace ASN1 {

// clang-format off

/**
 *  @def ASN1_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum ASN1 error number range.
 *    This value may be configured via #ASN1_CONFIG_ERROR_MIN.
 *
 */
#define ASN1_ERROR_MIN                  ASN1_CONFIG_ERROR_MIN

/**
 *  @def ASN1_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum ASN1 error number range.
 *    This value may be configured via #ASN1_CONFIG_ERROR_MAX.
 *
 */
#define ASN1_ERROR_MAX                  ASN1_CONFIG_ERROR_MAX

/**
 *  @def CHIP_ASN1_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for ASN1 errors that allows
 *    mapping such errors into a platform- or system-specific
 *    range. This function may be configured via
 *    #ASN1_CONFIG_ERROR(e).
 *
 *  @param[in]  e  The ASN1 error to map.
 *
 *  @return The mapped ASN1 error.
 *
 */
#define CHIP_ASN1_ERROR(e)              ASN1_CONFIG_ERROR(e)


/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def ASN1_END
 *
 *  @brief
 *    An end of ASN1 container or stream condition occurred.
 *
 */
#define ASN1_END                        CHIP_ASN1_ERROR(0)

/**
 *  @def ASN1_ERROR_UNDERRUN
 *
 *  @brief
 *    The ASN.1 encoding ended prematurely.
 *
 */
#define ASN1_ERROR_UNDERRUN             CHIP_ASN1_ERROR(1)

/**
 *  @def ASN1_ERROR_OVERFLOW
 *
 *  @brief
 *    The encoding exceeds the available space required to write it.
 *
 */
#define ASN1_ERROR_OVERFLOW             CHIP_ASN1_ERROR(2)

/**
 *  @def ASN1_ERROR_INVALID_STATE
 *
 *  @brief
 *    An unexpected or invalid state was encountered.
 *
 */
#define ASN1_ERROR_INVALID_STATE        CHIP_ASN1_ERROR(3)

/**
 *  @def ASN1_ERROR_MAX_DEPTH_EXCEEDED
 *
 *  @brief
 *    The maximum number of container reading contexts was exceeded.
 *
 */
#define ASN1_ERROR_MAX_DEPTH_EXCEEDED   CHIP_ASN1_ERROR(4)

/**
 *  @def ASN1_ERROR_INVALID_ENCODING
 *
 *  @brief
 *    The ASN.1 encoding is invalid.
 *
 */
#define ASN1_ERROR_INVALID_ENCODING     CHIP_ASN1_ERROR(5)

/**
 *  @def ASN1_ERROR_UNSUPPORTED_ENCODING
 *
 *  @brief
 *    An unsupported encoding was requested or encountered.
 *
 */
#define ASN1_ERROR_UNSUPPORTED_ENCODING CHIP_ASN1_ERROR(6)

/**
 *  @def ASN1_ERROR_TAG_OVERFLOW
 *
 *  @brief
 *    An encoded tag exceeds the available or allowed space required
 *    for it.
 *
 */
#define ASN1_ERROR_TAG_OVERFLOW         CHIP_ASN1_ERROR(7)

/**
 *  @def ASN1_ERROR_LENGTH_OVERFLOW
 *
 *  @brief
 *    An encoded length exceeds the available or allowed space
 *    required for it.
 *
 */
#define ASN1_ERROR_LENGTH_OVERFLOW      CHIP_ASN1_ERROR(8)

/**
 *  @def ASN1_ERROR_VALUE_OVERFLOW
 *
 *  @brief
 *    An encoded value exceeds the available or allowed space
 *    required for it.
 *
 */
#define ASN1_ERROR_VALUE_OVERFLOW       CHIP_ASN1_ERROR(9)

/**
 *  @def ASN1_ERROR_UNKNOWN_OBJECT_ID
 *
 *  @brief
 *    A requested object identifier does not match the list of
 *    supported object identifiers.
 *
 */
#define ASN1_ERROR_UNKNOWN_OBJECT_ID    CHIP_ASN1_ERROR(10)

//                        !!!!! IMPORTANT !!!!!
//
// If you add new ASN1 errors, please update the translation of error
// codes to strings in ASN1Error.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

/**
 *  @}
 */

// !!!!! IMPORTANT !!!!!
// These definitions are present temporarily in order to reduce breakage for PRs in flight.
// TODO: remove compatibility definitions
#define ASN1_ERROR                              CHIP_ERROR
#define ASN1_NO_ERROR                           CHIP_NO_ERROR

// clang-format on

bool FormatASN1Error(char * buf, uint16_t bufSize, int32_t err);

} // namespace ASN1
} // namespace chip
