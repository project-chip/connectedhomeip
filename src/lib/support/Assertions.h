/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      Define and implement basic support for handling assertions and aborting code execution.
 *      Macros and functions in this file do not perform any logging therefore can be used in
 *      contexts where I/O or memory allocation are not allowed.
 *
 *      The abort function is customizable by defining `CHIP_CONFIG_ABORT`.
 */

#pragma once

#include <cstdlib>
#include <lib/core/CHIPConfig.h>

/**
 *  @brief
 *    Base-level abnormal termination.
 *
 *    Terminate the program immediately, without invoking destructors, atexit callbacks, etc.
 *    Used to implement the default `chipAbort()` function.
 *
 *  @note
 *    This should never be invoked directly by code outside this file.
 */
#if !defined(CHIP_CONFIG_ABORT)
#define CHIP_CONFIG_ABORT() abort()
#endif

/**
 *  @brief
 *    This is invoked when an assertion expression evaluates to false.
 *
 *    Developers may override and customize this by defining #chipAbort
 *    before this file is included by the preprocessor.
 */
#ifndef chipAbort
extern "C" void chipAbort(void) __attribute((noreturn));

inline void chipAbort(void)
{
    while (true)
    {
        CHIP_CONFIG_ABORT();
    }
}
#endif // chipAbort

/**
 *  @def VerifyOrDieWithoutLogging(expr)
 *
 *  @brief
 *    This checks for the specified @a cond, which is expected to
 *    commonly be true. Aborts if the condition is false. This macro
 *    does not log.
 *
 *  @param[in]  cond  A Boolean expression to be evaluated.
 */
#define VerifyOrDieWithoutLogging(cond)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(cond))                                                                                                               \
        {                                                                                                                          \
            chipAbort();                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def AbortWithAction(anAction...)
 *
 *  @brief
 *     Do something and abort code execution by calling chipAbort()
 *
 *  @param[in]  ...         Statements to execute.
 */
#define AbortWithAction(...)                                                                                                       \
    do                                                                                                                             \
    {                                                                                                                              \
        __VA_ARGS__;                                                                                                               \
        chipAbort();                                                                                                               \
    } while (false)

/**
 *  @def VerifyOrExit(aCondition, anAction)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true, and both executes @a anAction and branches to
 *    the local label 'exit' if the condition is false.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR MakeBuffer(const uint8_t *& buf)
 *  {
 *      CHIP_ERROR err = CHIP_NO_ERROR;
 *
 *      buf = (uint8_t *)malloc(1024);
 *      VerifyOrExit(buf != NULL, err = CHIP_ERROR_NO_MEMORY);
 *
 *      memset(buf, 0, 1024);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition  A Boolean expression to be evaluated.
 *  @param[in]  anAction    An expression or block to execute when the
 *                          assertion fails.
 *
 */
#define VerifyOrExit(aCondition, anAction) VerifyOrDo(aCondition, anAction; goto exit)

/**
 *  @def VerifyOrReturnError(expr, code, ...)
 *
 *  @brief
 *    Returns a specified error code if expression evaluates to false
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnError(param != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturnError(expr, code, ...) VerifyOrReturnValue(expr, code, ##__VA_ARGS__)

/**
 *  @def VerifyOrReturnValue(expr, value, ...)
 *
 *  @brief
 *    Returns a specified value if expression evaluates to false
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnError(param != nullptr, Foo());
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  value       A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturnValue(expr, value, ...)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return (value);                                                                                                        \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrReturn(expr, ...)
 *
 *  @brief
 *    Returns from the void function if expression evaluates to false
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturn(param != nullptr, LogError("param is nullptr"));
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define VerifyOrReturn(expr, ...)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrDo(expr, ...)
 *
 *  @brief
 *    do something if expression evaluates to false
 *
 *  Example usage:
 *
 * @code
 *    VerifyOrDo(param != nullptr, LogError("param is nullptr"));
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute.
 */
#define VerifyOrDo(expr, ...)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ExitNow(...)
 *
 *  @brief
 *    This unconditionally executes @a ... and branches to the local
 *    label 'exit'.
 *
 *  @note The use of this interface implies neither success nor
 *        failure for the overall exit status of the enclosing function
 *        body.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR ReadAll(Reader& reader)
 *  {
 *      CHIP_ERROR err;
 *
 *      while (true)
 *      {
 *          err = reader.ReadNext();
 *          if (err == CHIP_ERROR_AT_END)
 *              ExitNow(err = CHIP_NO_ERROR);
 *          SuccessOrExit(err);
 *          DoSomething();
 *      }
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  ...         Statements to execute. Optional.
 */
#define ExitNow(...)                                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        __VA_ARGS__;                                                                                                               \
        goto exit;                                                                                                                 \
    } while (0)

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
