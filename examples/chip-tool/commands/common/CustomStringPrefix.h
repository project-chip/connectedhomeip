/*
 *   Copyright (c) 2022 Project CHIP Authors
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

static constexpr char kHexStringPrefix[] = "hex:";
constexpr size_t kHexStringPrefixLen     = ArraySize(kHexStringPrefix) - 1; // Don't count the null

static constexpr char kStrStringPrefix[] = "str:";
constexpr size_t kStrStringPrefixLen     = ArraySize(kStrStringPrefix) - 1; // Don't count the null

inline bool IsHexString(const char * str)
{
    return strncmp(str, kHexStringPrefix, kHexStringPrefixLen) == 0;
}

inline bool IsStrString(const char * str)
{
    return strncmp(str, kStrStringPrefix, kStrStringPrefixLen) == 0;
}
