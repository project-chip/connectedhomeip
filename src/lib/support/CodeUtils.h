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
#include <lib/support/Assertions.h>
#include <lib/support/ObjectDump.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <memory>

/**
 *  @def ReturnErrorOnFailure(expr, ...)
 *
 *  @brief
 *    Returns the error code if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorOnFailure(channel->SendMsg(msg), mState = Uninitialized);
 *  @endcode
 *
 *  @param[in]  expr        An expression to be tested.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define ReturnErrorOnFailure(expr, ...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
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
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define ReturnErrorVariantOnFailure(variantType, expr, ...)                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
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
 *  @def SuccessOrLog(expr, MOD, MSG, ...)
 *
 *  @brief
 *    If expr returns something other than CHIP_NO_ERROR, log a message for the specified module
 *    in the 'Error' category.
 *
 *  Example usage:
 *
 *  @code
 *    SuccessOrLog(channel->SendMsg(msg), Module, "Failure message: %s", param);
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 *  @param[in]  MOD         The log module to use.
 *  @param[in]  MSG         The log message format string.
 *  @param[in]  ...         Optional arguments for the log message.
 */
#define SuccessOrLog(expr, MOD, MSG, ...)                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __lerr = (expr);                                                                                                \
        if (!::chip::ChipError::IsSuccess(__lerr))                                                                                 \
        {                                                                                                                          \
            ChipLogFailure(__lerr, MOD, MSG, ##__VA_ARGS__);                                                                       \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnAndLogOnFailure(expr, MOD, MSG, ...)
 *
 *  @brief
 *    If expr returns something than CHIP_NO_ERROR, log a chip message for the specified module
 *    in the 'Error' category and return.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnAndLogOnFailure(channel->SendMsg(msg), Module, "Failure message: %s", param);
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 *  @param[in]  MOD         The log module to use.
 *  @param[in]  MSG         The log message format string.
 *  @param[in]  ...         Optional arguments for the log message.
 */
#define ReturnAndLogOnFailure(expr, MOD, MSG, ...)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            ChipLogFailure(__err, MOD, MSG, ##__VA_ARGS__);                                                                        \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnErrorAndLogOnFailure(expr, MOD, MSG, ...)
 *
 *  @brief
 *    If expr returns something than CHIP_NO_ERROR, log a chip message for the specified module
 *    in the 'Error' category and return the error.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnErrorAndLogOnFailure(channel->SendMsg(msg), Module, "Failure message: %s", param);
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 *  @param[in]  MOD         The log module to use.
 *  @param[in]  MSG         The log message format string.
 *  @param[in]  ...         Optional arguments for the log message.
 */
#define ReturnErrorAndLogOnFailure(expr, MOD, MSG, ...)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            ChipLogFailure(__err, MOD, MSG, ##__VA_ARGS__);                                                                        \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnValueAndLogOnFailure(expr, value, MOD, MSG, ...)
 *
 *  @brief
 *    If expr returns something other than CHIP_NO_ERROR, log a message for the specified module
 *    in the 'Error' category and return the error.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnValueAndLogOnFailure(channel->SendMsg(msg), false, Module, "Failure message: %s", param);
 *  @endcode
 *
 *  @param[in]  expr        A scalar expression to be evaluated against CHIP_NO_ERROR.
 *  @param[in]  value       A value to return if @a expr is an error.
 *  @param[in]  MOD         The log module to use.
 *  @param[in]  MSG         The log message format string.
 *  @param[in]  ...         Optional arguments for the log message.
 */
#define ReturnValueAndLogOnFailure(expr, value, MOD, MSG, ...)                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            ChipLogFailure(__err, MOD, MSG, ##__VA_ARGS__);                                                                        \
            return value;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnOnFailure(expr, ...)
 *
 *  @brief
 *    Returns if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnOnFailure(channel->SendMsg(msg), mState = Uninitialized);
 *  @endcode
 *
 *  @param[in]  expr        An expression to be tested.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define ReturnOnFailure(expr, ...)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/**
 *  @def ReturnValueOnFailure(expr, value, ...)
 *
 *  @brief
 *    Returns value if the expression returns an error. For a CHIP_ERROR expression, this means any value other
 *    than CHIP_NO_ERROR. For an integer expression, this means non-zero.
 *
 *  Example usage:
 *
 *  @code
 *    ReturnValueOnFailure(channel->SendMsg(msg), Status::Failure, mState = Uninitialized);
 *  @endcode
 *
 *  @param[in]  expr        An expression to be tested.
 *  @param[in]  value       A value to return if @a expr is an error.
 *  @param[in]  ...         Statements to execute before returning. Optional.
 */
#define ReturnValueOnFailure(expr, value, ...)                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return value;                                                                                                          \
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
            auto __code = (code);                                                                                                  \
            ChipLogError(NotSpecified, "%s at %s:%d", ErrorStr(__code), __FILE__, __LINE__);                                       \
            return __code;                                                                                                         \
        }                                                                                                                          \
    } while (false)
#else // CHIP_CONFIG_ERROR_SOURCE
#define VerifyOrReturnLogError(expr, code)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            auto __code = (code);                                                                                                  \
            ChipLogError(NotSpecified, "%s:%d false: %" CHIP_ERROR_FORMAT, #expr, __LINE__, __code.Format());                      \
            return __code;                                                                                                         \
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
#define SuccessOrExit(error) VerifyOrExit(::chip::ChipError::IsSuccess((error)), {})

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
#define SuccessOrExitAction(error, action) VerifyOrExit(::chip::ChipError::IsSuccess((error)), action)

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
    VerifyOrDo(aCondition, ChipLogError(Support, "VerifyOrDie failure at %s:%d", __FILE__, __LINE__); chipAbort())
#elif CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDie(aCondition)                                                                                                    \
    VerifyOrDo(aCondition, ChipLogError(Support, "VerifyOrDie failure at %s:%d: %s", __FILE__, __LINE__, #aCondition); chipAbort())
#else // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define VerifyOrDie(aCondition) VerifyOrDieWithoutLogging(aCondition)
#endif // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE

/**
 *  @def SuccessOrDie(error)
 *
 *  @brief
 *    This checks for the specified error, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), forces an immediate abort if the status
 *    is unsuccessful.
 *
 *
 *  Example Usage:
 *
 *  @code
 *  uint8_t* AllocateBuffer()
 *  {
 *      uint8_t* buffer;
 *      SuccessOrDie(ChipAllocateBuffer(buffer));
 *      return buffer;
 *  }
 *  @endcode
 *
 *  @param[in]  error  A ChipError object to be evaluated against success (CHIP_NO_ERROR).
 *
 */
#if CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE && CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE_NO_COND
#define SuccessOrDie(error)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (error);                                                                                                      \
        VerifyOrDo(::chip::ChipError::IsSuccess(__err),                                                                            \
                   ChipLogError(Support, "SuccessOrDie failure %s at %s:%d", ErrorStr(__err), __FILE__, __LINE__);                 \
                   chipAbort());                                                                                                   \
    } while (false)
#elif CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define SuccessOrDie(error)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (error);                                                                                                      \
        VerifyOrDo(::chip::ChipError::IsSuccess(__err),                                                                            \
                   ChipLogError(Support, "SuccessOrDie failure %s at %s:%d: %s", ErrorStr(__err), __FILE__, __LINE__, #error);     \
                   chipAbort());                                                                                                   \
    } while (false)
#else // CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define SuccessOrDie(error) VerifyOrDieWithoutLogging(::chip::ChipError::IsSuccess((error)))
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
    VerifyOrDo(aCondition, ::chip::DumpObjectToLog(aObject);                                                                       \
               ChipLogError(Support, "VerifyOrDie failure at %s:%d: %s", __FILE__, __LINE__, #aCondition); chipAbort())
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
    VerifyOrDo(aCondition, ChipLogError(aModule, aMessage, ##__VA_ARGS__); chipAbort())

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

/**
 *  @def ScopeExit(fn)
 *
 *  @brief
 *    RAII to automatically release resources on scope exit (instead of depending on goto exit)
 *    See https://en.cppreference.com/w/cpp/experimental/scope_exit.html
 *    Use with ReturnOnFailure, ReturnLogErrorOnFailure, ReturnAndLogOnFailure and other such methods
 *    to return an error code result from a method call without needing to store in a local var
 *
 *  Example usage:
 *
 *  @code
 *  Resource * resource = GetResource();
 *  auto resourceHolder = ScopeExit([&] { resource->Release() });
 *  // If the call below fails, logs, returns the error code, and calls resourceHolder
 *  ReturnAndLogOnFailure(ProcessAndSaveResource(resource), Module, "Failure message: %s", param);
 *  resourceHolder->release(); // Cancel clean-up at end of successful method
 *  @endcode
 */
template <class F>
__attribute__((always_inline)) inline auto ScopeExit(F && fn)
{
    auto deleter = [f = std::forward<F>(fn)](void *) mutable { f(); };
    return std::unique_ptr<void, decltype(deleter)>(reinterpret_cast<void *>(1), std::move(deleter));
}

} // namespace chip
