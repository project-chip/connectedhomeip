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

#include <cstddef>

/**
 * @file ClosureUIStrings.h
 * @brief UI string constants

 * @note IMPORTANT FOR DEVELOPERS:
 *       If you modify existing strings or add new strings in this file, you MUST ensure
 *       that all prefix + suffix combinations fit within LCD_CH_LINE_LEN (16 characters),
 *       as the LCD display can only show 16 characters per line. All text buffers are
 *       sized to LCD_STRING_BUFFER_SIZE (17 bytes: 16 characters + null terminator).
 */

namespace ClosureUIStrings {

// Shared suffix for unknown state
inline constexpr const char SUFFIX_UNKNOWN[] = "Unknown";

inline constexpr const char POSITION_PREFIX[]             = "Pos: ";
inline constexpr const char POSITION_SUFFIX_CLOSED[]      = "Closed";
inline constexpr const char POSITION_SUFFIX_OPEN[]        = "Open";
inline constexpr const char POSITION_SUFFIX_PARTIAL[]     = "Partial";
inline constexpr const char POSITION_SUFFIX_PEDESTRIAN[]  = "Pedest";
inline constexpr const char POSITION_SUFFIX_VENTILATION[] = "Ventil";
inline constexpr const char POSITION_SUFFIX_SIGNATURE[]   = "Sign";

inline constexpr const char LATCH_PREFIX[]          = "Latch: ";
inline constexpr const char LATCH_SUFFIX_ENGAGED[]  = "Yes";
inline constexpr const char LATCH_SUFFIX_RELEASED[] = "No";

inline constexpr const char SECURE_PREFIX[]     = "Secure: ";
inline constexpr const char SECURE_SUFFIX_YES[] = "Yes";
inline constexpr const char SECURE_SUFFIX_NO[]  = "No";

inline constexpr const char SPEED_PREFIX[]        = "Speed: ";
inline constexpr const char SPEED_SUFFIX_LOW[]    = "Low";
inline constexpr const char SPEED_SUFFIX_MEDIUM[] = "Med";
inline constexpr const char SPEED_SUFFIX_HIGH[]   = "High";
inline constexpr const char SPEED_SUFFIX_AUTO[]   = "Auto";

inline constexpr const char STATE_PREFIX[]                = "State: ";
inline constexpr const char STATE_SUFFIX_STOPPED[]        = "Stopped";
inline constexpr const char STATE_SUFFIX_MOVING[]         = "Moving";
inline constexpr const char STATE_SUFFIX_WAITING[]        = "Waiting";
inline constexpr const char STATE_SUFFIX_ERROR[]          = "Error";
inline constexpr const char STATE_SUFFIX_CALIBRATING[]    = "Calib";
inline constexpr const char STATE_SUFFIX_PROTECTED[]      = "Protect";
inline constexpr const char STATE_SUFFIX_DISENGAGED[]     = "Diseng";
inline constexpr const char STATE_SUFFIX_SETUP_REQUIRED[] = "SetupReq";

inline constexpr const char FOOTER_TEXT[] = "Closure App";

// LCD display line maximum length
inline constexpr size_t LCD_CH_LINE_LEN = 16;

// LCD string buffer size (lcd line length + null terminator)
inline constexpr size_t LCD_STRING_BUFFER_SIZE = LCD_CH_LINE_LEN + 1;
} // namespace ClosureUIStrings
