/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines a set of macros for declaring C and C++
 *      symbol visibility for dynamic shared objects (DSOs) across
 *      different compilers and operating systems.
 *
 *      See https://gcc.gnu.org/wiki/Visibility for additional information
 *      regarding GCC.
 *
 */

#pragma once

#if defined _WIN32 || defined __CYGWIN__
#define DLL_EXPORT __declspec(dllexport)
#else
#if __GNUC__ >= 4
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif
#endif
