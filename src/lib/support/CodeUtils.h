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

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/ObjectDump.h>
#include <lib/support/VerificationMacrosNoLogging.h>
#include <lib/support/logging/TextOnlyLogging.h>

/**
 * Base-level abnormal termination.
 *
 * Terminate the program immediately, without invoking destructors, atexit callbacks, etc.
 * Used to implement the default `chipDie()`.
 *
 * @note
 *  This should never be invoked directly by code outside this file.
 */
#if !defined(CHIP_CONFIG_ABORT)
#define CHIP_CONFIG_ABORT() abort()
#endif

/**
 *  @name chip-specific nlassert.h Overrides
 *
 *  @{
 *
 */

/**
 *  @def NL_ASSERT_ABORT()
 *
 *  @brief
 *    This implements a chip-specific override for #NL_ASSERT_ABORT *
 *    from nlassert.h.
 *
 */
#if !defined(NL_ASSERT_ABORT)
#define NL_ASSERT_ABORT() chipAbort()
#endif

/**
 *  @def NL_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)
 *
 *  @brief
 *    This implements a chip-specific override for \c NL_ASSERT_LOG
 *    from nlassert.h.
 *
 *  @param[in]  aPrefix     A pointer to a NULL-terminated C string printed
 *                          at the beginning of the logged assertion
 *                          message. Typically this is and should be
 *                          \c NL_ASSERT_PREFIX_STRING.
 *  @param[in]  aName       A pointer to a NULL-terminated C string printed
 *                          following @a aPrefix that indicates what
 *                          module, program, application or subsystem
 *                          the assertion occurred in Typically this
 *                          is and should be
 *                          \c NL_ASSERT_COMPONENT_STRING.
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
#if !defined(NL_ASSERT_LOG)
#define NL_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)         \
    do                                                                                    \
    {                                                                                     \
        ChipLogError(NotSpecified,                                                       \
                      NL_ASSERT_LOG_FORMAT_DEFAULT,                                       \
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

/**
 *  @def ReturnErrorOnFailure(expr)
 *
 *  @brief
 *    Returns the error code if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        An expression to be tested.
 */
#define ReturnErrorOnFailure(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnErrorVariantOnFailure(expr)
 *
 *  @brief
 *    This is for use when the calling function returns a Variant type. It returns a CHIP_ERROR variant with the corresponding error
 *    code if the expression returns an error.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorVariantOnFailure(NextStep, ParseSigma1(tlvReader, parsedSigma1));
 *  @endcode
 *
 *  @param[in]  variantType   The Variant type that the calling function returns.
 *  @param[in]  expr          An expression to be tested.

 */
#define ReturnErrorVariantOnFailure(variantType, expr)                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            return variantType::Create<CHIP_ERROR>(__err);                                                                         \
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
 *    Returns if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        An expression to be tested.
 */
#define ReturnOnFailure(expr)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
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
#if CHIP_CONFIG_ERROR_SOURCE
#define VerifyOrReturnLogError(expr, code)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(code), __FILE__, __LINE__);                                         \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)
#else // CHIP_CONFIG_ERROR_SOURCE
#define VerifyOrReturnLogError(expr, code)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "%s:%d false: %" CHIP_ERROR_FORMAT, #expr, __LINE__, code.Format());                        \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)
#endif // CHIP_CONFIG_ERROR_SOURCE

/**
 *  @def SuccessOrExit(error)
 *
 *  @brief
 *    This checks for the specified error, which is expected to
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
 *  @param[in]  error  A ChipError object to be evaluated against success (CHIP_NO_ERROR).
 *
 */
#define SuccessOrExit(error) nlEXPECT(::chip::ChipError::IsSuccess((error)), exit)

/**
 *  @def SuccessOrExitAction(error, anAction)
 *
 *  @brief
 *    This checks for the specified error, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), and both executes
 *    @a anAction and branches to the local label 'exit' if the
 *    status is unsuccessful.
 *
 *  @param[in]  error  A ChipError object to be evaluated against success (CHIP_NO_ERROR).
 */
#define SuccessOrExitAction(error, action) nlEXPECT_ACTION(::chip::ChipError::IsSuccess((error)), exit, action)

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
 *  @param[in]  ...         Statements to execute. Optional.
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
#ifndef chipAbort
extern "C" void chipAbort(void) __attribute((noreturn));

inline void chipAbort(void)
{
    while (true)
    {
        // NL_ASSERT_ABORT is redefined to be chipAbort, so not useful here.
        CHIP_CONFIG_ABORT();
    }
}
#endif // chipAbort
#ifndef chipDie
extern "C" void chipDie(void) __attribute((noreturn));

inline void chipDie(void)
{
    ChipLogError(NotSpecified, "chipDie chipDie chipDie");
    chipAbort();
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
#if CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE && CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE_NO_COND
#define VerifyOrDie(aCondition)                                                                                                    \
    nlABORT_ACTION(aCondition, ChipLogError(Support, "VerifyOrDie failure at %s:%d", __FILE__, __LINE__))
#elif CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDie(aCondition)                                                                                                    \
    nlABORT_ACTION(aCondition, ChipLogError(Support, "VerifyOrDie failure at %s:%d: %s", __FILE__, __LINE__, #aCondition))
#else // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDie(aCondition) VerifyOrDieWithoutLogging(aCondition)
#endif // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE

/**
 * @def VerifyOrDieWithObject(aCondition, aObject)
 *
 * Like VerifyOrDie(), but calls DumpObjectToLog()
 * on the provided object on failure before aborting
 * if CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE is enabled.
 */
#if CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDieWithObject(aCondition, aObject)                                                                                 \
    nlABORT_ACTION(aCondition, ::chip::DumpObjectToLog(aObject);                                                                   \
                   ChipLogError(Support, "VerifyOrDie failure at %s:%d: %s", __FILE__, __LINE__, #aCondition))
#else // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDieWithObject(aCondition, aObject) VerifyOrDieWithoutLogging(aCondition)
#endif // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE

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
    nlABORT_ACTION(aCondition, ChipLogError(aModule, aMessage, ##__VA_ARGS__))

/**
 *  @def LogErrorOnFailure(expr)
 *
 *  @brief
 *    Logs a message if the expression returns something different than CHIP_NO_ERROR.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnLogErrorOnFailure(channel->SendMsg(msg));
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 */
#define LogErrorOnFailure(expr)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                                        \
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
#elif defined(__GNUC__)
#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH (void) 0
#endif

/**
 * @def MATTER_ARRAY_SIZE(aArray)
 *
 * @brief
 *   Returns the size of an array in number of elements.
 *
 * Example Usage:
 *
 * @code
 * int numbers[10];
 * SortNumbers(numbers, MATTER_ARRAY_SIZE(numbers));
 * @endcode
 *
 * @return      The size of an array in number of elements.
 *
 * @note Clever template-based solutions seem to fail when MATTER_ARRAY_SIZE is used
 *       with a variable-length array argument, so we just do the C-compatible
 *       thing in C++ as well.
 */
#ifndef MATTER_ARRAY_SIZE
#define MATTER_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

/**
 * @brief Ensures that if `str` is NULL, a non-null `default_str_value` is provided
 *
 * @param str - null-terminated string pointer or nullptr
 * @param default_str_value - replacement value if `str` is nullptr
 * @return `str` if not null, otherwise `default_str_value`
 */
inline const char * DefaultStringWhenNull(const char * str, const char * default_str_value)
{
    return (str != nullptr) ? str : default_str_value;
}

/**
 * @brief Ensure that a string for a %s specifier is shown as "(null)" if null
 *
 * @param str - null-terminated string pointer or nullptr
 * @return `str` if not null, otherwise literal "(null)"
 */
inline const char * StringOrNullMarker(const char * str)
{
    return DefaultStringWhenNull(str, "(null)");
}

namespace chip {

/**
 * Utility for checking, at compile time if the array is constexpr, whether an
 * array is sorted.  Can be used for static_asserts.
 */

template <typename T>
constexpr bool ArrayIsSorted(const T * aArray, size_t aLength)
{
    if (aLength == 0 || aLength == 1)
    {
        return true;
    }

    if (aArray[0] > aArray[1])
    {
        return false;
    }

    return ArrayIsSorted(aArray + 1, aLength - 1);
}

template <typename T, size_t N>
constexpr bool ArrayIsSorted(const T (&aArray)[N])
{
    return ArrayIsSorted(aArray, N);
}

} // namespace chip
