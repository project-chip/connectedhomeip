/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

/**
 * @file ClosureUIStrings.h
 * @brief UI string constants

 * @note IMPORTANT FOR DEVELOPERS:
 *       If you modify existing strings or add new strings in this file, you MUST also
 *       check and update the corresponding buffer sizes.
 *       - sPositionText[] - must accommodate longest "Position: " + suffix combination
 *       - sLatchText[] - must accommodate longest "Latch: " + suffix combination
 *       - sSecureText[] - must accommodate longest "Secure: " + suffix combination
 *       - sSpeedText[] - must accommodate longest "Speed: " + suffix combination
 *       - sStateText[] - must accommodate longest "State: " + suffix combination
 */

namespace ClosureUIStrings {

    inline constexpr const char POSITION_PREFIX[]     = "Position: ";
    inline constexpr const char POSITION_SUFFIX_UNKNOWN[]    = "Unknown";
    inline constexpr const char POSITION_SUFFIX_CLOSED[]     = "Closed";
    inline constexpr const char POSITION_SUFFIX_OPEN[]       = "Open";
    inline constexpr const char POSITION_SUFFIX_PARTIAL[]    = "Partial";
    inline constexpr const char POSITION_SUFFIX_PEDESTRIAN[] = "Pedestrian";
    inline constexpr const char POSITION_SUFFIX_VENTILATION[] = "Ventilation";

    inline constexpr const char LATCH_PREFIX[]   = "Latch: ";
    inline constexpr const char LATCH_SUFFIX_UNKNOWN[]   = "Unknown";
    inline constexpr const char LATCH_SUFFIX_ENGAGED[]   = "Engaged";
    inline constexpr const char LATCH_SUFFIX_RELEASED[]  = "Released";

    inline constexpr const char SECURE_PREFIX[]   = "Secure: ";
    inline constexpr const char SECURE_SUFFIX_UNKNOWN[]  = "Unknown";
    inline constexpr const char SECURE_SUFFIX_YES[]      = "Yes";
    inline constexpr const char SECURE_SUFFIX_NO[]       = "No";

    inline constexpr const char SPEED_PREFIX[]    = "Speed: ";
    inline constexpr const char SPEED_SUFFIX_UNKNOWN[] = "Unknown";
    inline constexpr const char SPEED_SUFFIX_LOW[]     = "Low";
    inline constexpr const char SPEED_SUFFIX_MEDIUM[]  = "Medium";
    inline constexpr const char SPEED_SUFFIX_HIGH[]    = "High";
    inline constexpr const char SPEED_SUFFIX_AUTO[]    = "Auto";

    inline constexpr const char STATE_PREFIX[] = "State: ";
    inline constexpr const char STATE_SUFFIX_STOPPED[] = "Stopped";
    inline constexpr const char STATE_SUFFIX_MOVING[] = "Moving";
    inline constexpr const char STATE_SUFFIX_WAITING[] = "Waiting";
    inline constexpr const char STATE_SUFFIX_ERROR[] = "Error";
    inline constexpr const char STATE_SUFFIX_CALIBRATING[] = "Calibrating";
    inline constexpr const char STATE_SUFFIX_PROTECTED[] = "Protected";
    inline constexpr const char STATE_SUFFIX_DISENGAGED[] = "Disengaged";
    inline constexpr const char STATE_SUFFIX_SETUP_REQUIRED[] = "Setup Required";
    inline constexpr const char STATE_SUFFIX_UNKNOWN[] = "Unknown";

    inline constexpr const char FOOTER_TEXT[] = "Closure App";

    // Buffer sizes for UI text arrays (single source of truth)
    // These are calculated as: (prefix length + longest suffix length) + 1 (for null terminator)
    inline constexpr size_t BUFFER_SIZE_POSITION = 23;
    inline constexpr size_t BUFFER_SIZE_LATCH = 16;
    inline constexpr size_t BUFFER_SIZE_SECURE = 16;
    inline constexpr size_t BUFFER_SIZE_SPEED = 15;
    inline constexpr size_t BUFFER_SIZE_STATE = 23;
}
