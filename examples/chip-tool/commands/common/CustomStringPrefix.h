/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <string.h>

#include <lib/support/CodeUtils.h>

static constexpr char kJsonStringPrefix[] = "json:";
constexpr size_t kJsonStringPrefixLen     = ArraySize(kJsonStringPrefix) - 1; // Don't count the null

static constexpr char kBase64StringPrefix[] = "base64:";
constexpr size_t kBase64StringPrefixLen     = ArraySize(kBase64StringPrefix) - 1; // Don't count the null

static constexpr char kHexStringPrefix[] = "hex:";
constexpr size_t kHexStringPrefixLen     = ArraySize(kHexStringPrefix) - 1; // Don't count the null

static constexpr char kStrStringPrefix[] = "str:";
constexpr size_t kStrStringPrefixLen     = ArraySize(kStrStringPrefix) - 1; // Don't count the null

inline bool IsJsonString(const char * str)
{
    return strncmp(str, kJsonStringPrefix, kJsonStringPrefixLen) == 0;
}

inline bool IsBase64String(const char * str)
{
    return strncmp(str, kBase64StringPrefix, kBase64StringPrefixLen) == 0;
}

inline bool IsHexString(const char * str)
{
    return strncmp(str, kHexStringPrefix, kHexStringPrefixLen) == 0;
}

inline bool IsStrString(const char * str)
{
    return strncmp(str, kStrStringPrefix, kStrStringPrefixLen) == 0;
}
