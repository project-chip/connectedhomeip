/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines and implements a number of miscellaneous
 *      templates for finding object minima and maxima and interface
 *      macros for assertion checking.
 *
 */

#pragma once

#ifdef __cplusplus

#include <core/CHIPError.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

/**
 *  @name chip-specific nlassert.h Overrides
 *
 *  @{
 *
 */

/**
 *  @def CHIP_ASSERT_ABORT()
 *
 *  @brief
 *    This implements a chip-specific override for #CHIP_ASSERT_ABORT *
 *    from nlassert.h.
 *
 */
#if !defined(CHIP_ASSERT_ABORT)
#define CHIP_ASSERT_ABORT() chipDie()
#endif

/**
 *  @def CHIP_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)
 *
 *  @brief
 *    This implements a chip-specific override for \c CHIP_ASSERT_LOG
 *    from nlassert.h.
 *
 *  @param[in]  aPrefix     A pointer to a NULL-terminated C string printed
 *                          at the beginning of the logged assertion
 *                          message. Typically this is and should be
 *                          \c CHIP_ASSERT_PREFIX_STRING.
 *  @param[in]  aName       A pointer to a NULL-terminated C string printed
 *                          following @a aPrefix that indicates what
 *                          module, program, application or subsystem
 *                          the assertion occurred in Typically this
 *                          is and should be
 *                          \c CHIP_ASSERT_COMPONENT_STRING.
 *  @param[in]  aCondition  A pointer to a NULL-terminated C string indicating
 *                          the expression that evaluated to false in
 *                          the assertion. Typically this is a
 *                          stringified version of the actual
 *                          assertion expression.
 *  @param[in]  aLabel      An optional pointer to a NULL-terminated C string
 *                          indicating, for exception-style
 *                          assertions, the label that will be
 *                          branched to when the assertion expression
 *                          evaluates to false.
 *  @param[in]  aFile       A pointer to a NULL-terminated C string indicating
 *                          the file in which the exception
 *                          occurred. Typically this is and should be
 *                          \_\_FILE\_\_ from the C preprocessor.
 *  @param[in]  aLine       The line number in @a aFile on which the assertion
 *                          expression evaluated to false. Typically
 *                          this is and should be \_\_LINE\_\_ from the C
 *                          preprocessor.
 *  @param[in]  aMessage    An optional pointer to a NULL-terminated C string
 *                          containing a caller-specified message
 *                          further describing the assertion failure.
 *
 */
// clang-format off
#if !defined(CHIP_ASSERT_LOG)
#define CHIP_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)         \
    do                                                                                    \
    {                                                                                     \
        ChipLogError(NotSpecified,                                                       \
                      CHIP_ASSERT_LOG_FORMAT_DEFAULT,                                       \
                      aPrefix,                                                            \
                      (((aName) == 0) || (*(aName) == '\0')) ? "" : aName,                \
                      (((aName) == 0) || (*(aName) == '\0')) ? "" : ": ",                 \
                      aCondition,                                                         \
                      (((aMessage) == 0) ? "" : aMessage),                                \
                      (((aMessage) == 0) ? "" : ", "),                                    \
                      aFile,                                                              \
                      aLine);                                                             \
    } while (0)
#endif
// clang-format on

/**
 *  @} chip-specific nlassert.h Overrides
 *
 */

#include <nlassert.h>

namespace chip {

// Generic min() and max() functions
//
template <typename _T>
constexpr inline const _T & min(const _T & a, const _T & b)
{
    if (b < a)
        return b;

    return a;
}

template <typename _T>
constexpr inline const _T & max(const _T & a, const _T & b)
{
    if (a < b)
        return b;

    return a;
}

} // namespace chip

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

/**
 *  @def ReturnErrorOnFailure(expr)
 *
 *  @brief
 *    Returns the error code if the expression returns something different
 *    than CHIP_NO_ERROR.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define ReturnErrorOnFailure(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnLogErrorOnFailure(expr)
 *
 *  @brief
 *    Returns the error code if the expression returns something different
 *    than CHIP_NO_ERROR.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnLogErrorOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define ReturnLogErrorOnFailure(expr)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnOnFailure(expr)
 *
 *  @brief
 *    Returns if the expression returns something different than CHIP_NO_ERROR
 *
 *  Example usage:
 *
 *  @code
 *    ReturnOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define ReturnOnFailure(expr)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            return;                                                                                                                \
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
 * @code
 *    VerifyOrReturn(param != nullptr, LogError("param is nullptr"));
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
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
 *  @def VerifyOrReturnError(expr, code)
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
 */
#define VerifyOrReturnError(expr, code)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def VerifyOrReturnLogError(expr, code)
 *
 *  @brief
 *    Returns and print a specified error code if expression evaluates to false
 *
 *  Example usage:
 *
 *  @code
 *    VerifyOrReturnLogError(param != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 */
#define VerifyOrReturnLogError(expr, code)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(code), __FILE__, __LINE__);                                         \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnErrorCodeIf(expr, code)
 *
 *  @brief
 *    Returns a specified error code if expression evaluates to true
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorCodeIf(state == kInitialized, CHIP_NO_ERROR);
 *    ReturnErrorCodeIf(state == kInitialized, CHIP_ERROR_INVALID_STATE);
 *  @endcode
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  code        A value to return if @a expr is false.
 */
#define ReturnErrorCodeIf(expr, code)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        if (expr)                                                                                                                  \
        {                                                                                                                          \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/**
 *  @def SuccessOrExit(aStatus)
 *
 *  @brief
 *    This checks for the specified status, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), and branches to
 *    the local label 'exit' if the status is unsuccessful.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR TryHard()
 *  {
 *      CHIP_ERROR err;
 *
 *      err = TrySomething();
 *      SuccessOrExit(err);
 *
 *      err = TrySomethingElse();
 *      SuccessOrExit(err);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  aStatus     A scalar status to be evaluated against zero (0).
 *
 */
#define SuccessOrExit(aStatus) nlEXPECT((aStatus) == CHIP_NO_ERROR, exit)

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
#define VerifyOrExit(aCondition, anAction) nlEXPECT_ACTION(aCondition, exit, anAction)

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
 *  @param[in]  ...         An optional expression or block to execute
 *                          when the assertion fails.
 *
 */
// clang-format off
#define ExitNow(...)                                                   \
    do {                                                               \
        __VA_ARGS__;                                                   \
        goto exit;                                                     \
    } while (0)
// clang-format on

/**
 *  @brief
 *    This is invoked when a #VerifyOrDie or #VerifyOrDieWithMsg
 *    assertion expression evaluates to false.
 *
 *    Developers may override and customize this by defining #chipDie
 *    before CodeUtils.h is included by the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  chipDie();
 *  @endcode
 *
 */
#ifndef chipDie
extern "C" void chipDie(void) __attribute((noreturn));

inline void chipDie(void)
{
    ChipLogError(NotSpecified, "chipDie chipDie chipDie");

    while (true)
    {
        // CHIP_ASSERT_ABORT is redefined to be chipDie, so not useful here.
        abort();
    }
}
#endif // chipDie

/**
 *  @def VerifyOrDie(aCondition)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true and forces an immediate abort if the condition
 *    is false.
 *
 *  Example Usage:
 *
 *  @code
 *  void FreeBuffer(const uint8_t *buf)
 *  {
 *      VerifyOrDie(buf != NULL);
 *      free(buf);
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition  A Boolean expression to be evaluated.
 *
 *  @sa #VerifyOrDieWithMsg
 *  @sa #chipDie
 *
 */
#define VerifyOrDie(aCondition) nlABORT(aCondition)

/**
 *  @def VerifyOrDieWithMsg(aCondition, aModule, aMessage, ...)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true and both prints @a aMessage and forces an
 *    immediate abort if the condition is false.
 *
 *  Example Usage:
 *
 *  @code
 *  void FreeBuffer(const uint8_t *buf)
 *  {
 *      VerifyOrDieWithMsg(buf != NULL, MemoryManagement, "Invalid pointer passed to FreeBuffer");
 *      free(buf);
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition  A Boolean expression to be evaluated.
 *  @param[in]  aModule     A chip LogModule short-hand mnemonic identifing
 *                          the logical section of code that is a
 *                          source the logged message.
 *  @param[in]  aMessage    A pointer to a NULL-terminated C string with
 *                          C Standard Library-style format specifiers
 *                          containing the log message to be formatted
 *                          and logged.
 *  @param[in]  ...         A variadic argument list whose elements should
 *                          correspond to the format specifiers in @a
 *                          aMessage.
 *
 *  @sa #VerifyOrDie
 *  @sa #chipDie
 *
 */
#define VerifyOrDieWithMsg(aCondition, aModule, aMessage, ...)                                                                     \
    nlABORT_ACTION(aCondition, ChipLogDetail(aModule, aMessage, ##__VA_ARGS__))

#if (__cplusplus >= 201103L)

#ifndef __FINAL
#define __FINAL final
#endif

#ifndef __OVERRIDE
#define __OVERRIDE override
#endif

#ifndef __CONSTEXPR
#define __CONSTEXPR constexpr
#endif

#else

#ifndef __FINAL
#define __FINAL
#endif

#ifndef __OVERRIDE
#define __OVERRIDE
#endif

#ifndef __CONSTEXPR
#define __CONSTEXPR constexpr
#endif

#endif // (__cplusplus >= 201103L)

#if ((__cplusplus >= 201703L) || (defined(__GNUC__) && (__GNUC__ >= 7)) || (defined(__clang__)) && (__clang_major__ >= 4))
#define CHECK_RETURN_VALUE [[nodiscard]]
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RETURN_VALUE __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RETURN_VALUE _Check_return_
#else
#define CHECK_RETURN_VALUE
#endif

#if defined(__clang__)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH (void) 0
#endif

#endif // __cplusplus

/**
 * @def ArraySize(aArray)
 *
 * @brief
 *   Returns the size of an array in number of elements.
 *
 * Example Usage:
 *
 * @code
 * int numbers[10];
 * SortNumbers(numbers, ArraySize(numbers));
 * @endcode
 *
 * @return      The size of an array in number of elements.
 *
 * @note Clever template-based solutions seem to fail when ArraySize is used
 *       with a variable-length array argument, so we just do the C-compatible
 *       thing in C++ as well.
 */
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))
