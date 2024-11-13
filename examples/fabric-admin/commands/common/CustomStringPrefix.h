/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <string.h>

#include <lib/support/CodeUtils.h>

static constexpr char kJsonStringPrefix[]    = "json:";
inline constexpr size_t kJsonStringPrefixLen = ArraySize(kJsonStringPrefix) - 1; // Don't count the null

static constexpr char kBase64StringPrefix[]    = "base64:";
inline constexpr size_t kBase64StringPrefixLen = ArraySize(kBase64StringPrefix) - 1; // Don't count the null

static constexpr char kHexStringPrefix[]    = "hex:";
inline constexpr size_t kHexStringPrefixLen = ArraySize(kHexStringPrefix) - 1; // Don't count the null

static constexpr char kStrStringPrefix[]    = "str:";
inline constexpr size_t kStrStringPrefixLen = ArraySize(kStrStringPrefix) - 1; // Don't count the null

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
