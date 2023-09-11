/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines default compile-time configuration constants
 *      for the CHIP ASN1 subsystem.
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

// clang-format off

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
 *  @def ASN1_CONFIG_ERROR
 *
 *  @brief
 *    This defines a mapping function for ASN1 errors that allows
 *    mapping such errors into a platform- or system-specific range.
 *
 */
#ifndef ASN1_CONFIG_ERROR
#define ASN1_CONFIG_ERROR(e)                               (ASN1_ERROR_MIN + (e))
#endif // ASN1_CONFIG_ERROR

// clang-format on
