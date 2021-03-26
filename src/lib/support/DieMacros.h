/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

#include <support/DieMacros.h>
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
