/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines default compile-time configuration constants
 *      for the Nest Weave ASN1 subsystem.
 *
 */

#ifndef WEAVE_SUPPORT_ASN1_CONFIG_H_
#define WEAVE_SUPPORT_ASN1_CONFIG_H_

#include <core/CHIPConfig.h>

// clang-format off

/**
 *  @def ASN1_CONFIG_ERROR_TYPE
 *
 *  @brief
 *    This defines the data type used to represent errors for the ASN1
 *    subsystem.
 *
 */
#ifndef ASN1_CONFIG_ERROR_TYPE
#include <stdint.h>

#define ASN1_CONFIG_ERROR_TYPE                              int32_t
#endif // ASN1_CONFIG_ERROR_TYPE

/**
 *  @def ASN1_CONFIG_NO_ERROR
 *
 *  @brief
 *    This defines the ASN1 error code for no error or success.
 *
 */
#ifndef ASN1_CONFIG_NO_ERROR
#define ASN1_CONFIG_NO_ERROR                                0
#endif // ASN1_CONFIG_NO_ERROR

/**
 *  @def ASN1_CONFIG_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum ASN1 error number range.
 *
 */
#ifndef ASN1_CONFIG_ERROR_MIN
#define ASN1_CONFIG_ERROR_MIN                               5000
#endif // ASN1_CONFIG_ERROR_MIN

/**
 *  @def ASN1_CONFIG_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum ASN1 error number range.
 *
 */
#ifndef ASN1_CONFIG_ERROR_MAX
#define ASN1_CONFIG_ERROR_MAX                               5999
#endif // ASN1_CONFIG_ERROR_MAX

/**
 *  @def _ASN1_CONFIG_ERROR
 *
 *  @brief
 *    This defines a mapping function for ASN1 errors that allows
 *    mapping such errors into a platform- or system-specific range.
 *
 */
#ifndef _ASN1_CONFIG_ERROR
#define _ASN1_CONFIG_ERROR(e)                               (ASN1_ERROR_MIN + (e))
#endif // _ASN1_CONFIG_ERROR

// clang-format on

#endif /* WEAVE_SUPPORT_ASN1_CONFIG_H_ */
