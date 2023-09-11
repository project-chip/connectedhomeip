/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file provides the Mbedtls platform aternative implementation.
 *      It contains custom setup() and teardown() functions.
 */

#ifndef MBEDTLS_PLATFORM_ALT
#define MBEDTLS_PLATFORM_ALT

/**
 * \brief The dummy platform context structure.
 */
typedef struct mbedtls_platform_context
{
    char dummy;
} mbedtls_platform_context;

#endif /* MBEDTLS_PLATFORM_ALT */
