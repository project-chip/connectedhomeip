/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "pw_assert_matter/handler.h"

// Use __PRETTY_FUNCTION__, a GNU extension, in place of the __func__ macro when
// supported. __PRETTY_FUNCTION__ expands to the full C++ function name.
#ifdef __GNUC__
#define _PW_ASSERT_MATTER_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define _PW_ASSERT_MATTER_FUNCTION_NAME __func__
#endif // __GNUC__

// Die with a message with many attributes included. This is the crash macro
// frontend that funnels everything into the C handler provided by the user,
// pw_assert_MATTER_HandleFailure().
#define PW_HANDLE_CRASH(...)                                                                                                       \
    pw_assert_matter_HandleFailure(__FILE__, __LINE__, _PW_ASSERT_MATTER_FUNCTION_NAME PW_COMMA_ARGS(__VA_ARGS__))

// Die with a message with many attributes included. This is the crash macro
// frontend that funnels everything into the C handler provided by the user,
// pw_assert_matter_HandleFailure().
#define PW_HANDLE_ASSERT_FAILURE(condition_string, message, ...)                                                                   \
    pw_assert_matter_HandleFailure(__FILE__, __LINE__, _PW_ASSERT_MATTER_FUNCTION_NAME,                                            \
                                   "Check failed: " condition_string ". " message PW_COMMA_ARGS(__VA_ARGS__))

// Sample assert failure message produced by the below implementation:
//
//   Check failed: current_sensor (=610) < new_sensor (=50). More details!
//
// Putting the value next to the operand makes the string easier to read.

// clang-format off
// This is too hairy for clang format to handle and retain readability.
#define PW_HANDLE_ASSERT_BINARY_COMPARE_FAILURE(arg_a_str,         \
                                                arg_a_val,         \
                                                comparison_op_str, \
                                                arg_b_str,         \
                                                arg_b_val,         \
                                                type_fmt,          \
                                                message, ...)      \
  pw_assert_matter_HandleFailure(                                   \
      __FILE__,                                                    \
      __LINE__,                                                    \
      _PW_ASSERT_MATTER_FUNCTION_NAME,                              \
      "Check failed: "                                             \
          arg_a_str " (=" type_fmt ") "                            \
          comparison_op_str " "                                    \
          arg_b_str " (=" type_fmt ")"                             \
          ". " message,                                            \
      arg_a_val, arg_b_val PW_COMMA_ARGS(__VA_ARGS__))
