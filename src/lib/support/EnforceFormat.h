/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * gcc and clang provide a way to warn for a custom formatter when formats don't
 * match arguments.  Use that so we catch mistakes.  The "format"
 * attribute takes the type of format, which arg is the format string, and which
 * arg is the first variadic arg, with both arg numbers 1-based.
 *
 * The second arg should be set to 0 if the function takes a va_list instead of
 * varargs.
 */

#if defined(__GNUC__) || defined(__clang__)
#define ENFORCE_FORMAT(n, m) __attribute__((format(printf, n, m)))
#else                        // __GNUC__
#define ENFORCE_FORMAT(n, m) /* How to do with MSVC? */
#endif                       // __GNUC__
