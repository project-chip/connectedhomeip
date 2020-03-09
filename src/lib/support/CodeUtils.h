/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file defines and implements a number of miscellaneous
 *      templates for finding object minima and maxima and interface
 *      macros for assertion checking.
 *
 */

#ifndef CODEUTILS_H_
#define CODEUTILS_H_

#include <Weave/Core/WeaveError.h>
#include <Weave/Support/ErrorStr.h>
#include <Weave/Support/logging/WeaveLogging.h>

/**
 *  @name Weave-specific nlassert.h Overrides
 *
 *  @{
 *
 */

/**
 *  @def NL_ASSERT_ABORT()
 *
 *  @brief
 *    This implements a Weave-specific override for #NL_ASSERT_ABORT *
 *    from nlassert.h.
 *
 */
#if !defined(NL_ASSERT_ABORT)
#define NL_ASSERT_ABORT()                                              \
    WeaveDie()
#endif

/**
 *  @def NL_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)
 *
 *  @brief
 *    This implements a Weave-specific override for \c NL_ASSERT_LOG
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
#if !defined(NL_ASSERT_LOG)
#define NL_ASSERT_LOG(aPrefix, aName, aCondition, aLabel, aFile, aLine, aMessage)         \
    do                                                                                    \
    {                                                                                     \
        WeaveLogError(NotSpecified,                                                       \
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

/**
 *  @} Weave-specific nlassert.h Overrides
 *
 */

#include <nlassert.h>

namespace nl {
namespace Weave {

// Generic min() and max() functions
//
template <typename _T>
inline const _T &
min(const _T &a, const _T &b)
{
    if (b < a)
        return b;

    return a;
}

template <typename _T>
inline const _T &
max(const _T &a, const _T &b)
{
    if (a < b)
        return b;

    return a;
}

} // namespace Weave
} // namespace nl

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
#define IgnoreUnusedVariable(aVariable)                                \
    ((void)(aVariable))

/**
 *  @def SuccessOrExit(aStatus)
 *
 *  @brief
 *    This checks for the specified status, which is expected to
 *    commonly be successful (WEAVE_NO_ERROR), and branches to
 *    the local label 'exit' if the status is unsuccessful.
 *
 *  Example Usage:
 *
 *  @code
 *  WEAVE_ERROR TryHard()
 *  {
 *      WEAVE_ERROR err;
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
#define SuccessOrExit(aStatus)                                         \
    nlEXPECT((aStatus) == WEAVE_NO_ERROR, exit)

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
 *  WEAVE_ERROR MakeBuffer(const uint8_t *& buf)
 *  {
 *      WEAVE_ERROR err = WEAVE_NO_ERROR;
 *
 *      buf = (uint8_t *)malloc(1024);
 *      VerifyOrExit(buf != NULL, err = WEAVE_ERROR_NO_MEMORY);
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
#define VerifyOrExit(aCondition, anAction)                             \
    nlEXPECT_ACTION(aCondition, exit, anAction)

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
 *  WEAVE_ERROR ReadAll(Reader& reader)
 *  {
 *      WEAVE_ERROR err;
 *
 *      while (true)
 *      {
 *          err = reader.ReadNext();
 *          if (err == WEAVE_ERROR_AT_END)
 *              ExitNow(err = WEAVE_NO_ERROR);
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
#define ExitNow(...)                                                   \
    do {                                                               \
        __VA_ARGS__;                                                   \
        goto exit;                                                     \
    } while (0)

/**
 *  @brief
 *    This is invoked when a #VerifyOrDie or #VerifyOrDieWithMsg
 *    assertion expression evaluates to false.
 *
 *    Developers may override and customize this by defining #WeaveDie
 *    before CodeUtils.h is included by the preprocessor.
 *
 *  Example Usage:
 *
 *  @code
 *  WeaveDie();
 *  @endcode
 *
 */
#ifndef WeaveDie
extern "C" void WeaveDie(void) __attribute((noreturn));

inline void WeaveDie(void)
{
    WeaveLogError(NotSpecified, "WeaveDie WeaveDie WeaveDie");

    while (true)
    {
        // NL_ASSERT_ABORT is redefined to be WeaveDie, so not useful here.
        abort();
    }
}
#endif // WeaveDie

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
 *  @sa #WeaveDie
 *
 */
#define VerifyOrDie(aCondition)                                        \
    nlABORT(aCondition)

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
 *  @param[in]  aModule     A Weave LogModule short-hand mnemonic identifing
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
 *  @sa #WeaveDie
 *
 */
#define VerifyOrDieWithMsg(aCondition, aModule, aMessage, ...)         \
    nlABORT_ACTION(aCondition, WeaveLogDetail(aModule, aMessage, ## __VA_ARGS__))

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
 */
#define ArraySize(a) (sizeof(a)/sizeof((a)[0]))

#if defined(__cplusplus) && (__cplusplus >= 201103L)

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

#endif // defined(__cplusplus) && (__cplusplus >= 201103L)

#endif /* CODEUTILS_H_ */
