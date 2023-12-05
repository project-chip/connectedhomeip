/*
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 *    @file
 *      This file defines and implements macros for verifying values that do not
 *      involve logging.
 */

#pragma once

#include <nlassert.h>

#define VerifyOrDieWithoutLogging(aCondition) nlABORT(aCondition)

/**
 *  @def IgnoreUnusedVariable(aVariable)
 *
 *  @brief
 *    This casts the specified @a aVariable to void to quell any
 *    compiler-issued unused variable warnings or errors.
 *
 *  @code
 *  void foo (int err)
 *  {
 *      IgnoreUnusedVariable(err)
 *  }
 *  @endcode
 *
 */
#define IgnoreUnusedVariable(aVariable) ((void) (aVariable))
