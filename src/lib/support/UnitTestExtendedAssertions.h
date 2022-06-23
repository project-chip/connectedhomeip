/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <nlunit-test.h>

/**
 *  @def NL_TEST_ASSERT_SUCCESS(inSuite, expression)
 *
 *  @brief
 *    This is used to assert that an expression is equal to CHIP_NO_ERROR
 *    throughout a test in a test suite.
 *
 *  @param[in]    inSuite       A pointer to the test suite the assertion
 *                              should be accounted against.
 *  @param[in]    inExpression  Expression to be checked for equality to CHIP_NO_ERROR.
 *                              If the expression is different than CHIP_NO_ERROR, the
 *                              assertion fails.
 *
 */
#define NL_TEST_ASSERT_SUCCESS(inSuite, inExpression)                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR _inner_err = (inExpression);                                                                                    \
        (inSuite)->performedAssertions += 1;                                                                                       \
                                                                                                                                   \
        if (_inner_err != CHIP_NO_ERROR)                                                                                           \
        {                                                                                                                          \
            printf("%s:%u: assertion failed due to error: \"%s\": %" CHIP_ERROR_FORMAT "\n", __FILE__, __LINE__, #inExpression,    \
                   _inner_err.Format());                                                                                           \
            (inSuite)->failedAssertions += 1;                                                                                      \
            (inSuite)->flagError = true;                                                                                           \
        }                                                                                                                          \
    } while (0)

/**
 *  @def NL_TEST_ASSERT_SUCCESS(inSuite, expression)
 *
 *  @brief
 *    This is used to assert that an expression is equal to CHIP_NO_ERROR
 *    throughout a test in a test suite.
 *
 *  @param[in]    inSuite       A pointer to the test suite the assertion
 *                              should be accounted against.
 *  @param[in]    inExpression  Expression to be checked for equality to CHIP_NO_ERROR.
 *                              If the expression is different than CHIP_NO_ERROR, the
 *                              assertion fails.
 *
 */
#define NL_TEST_ASSERT_EQUALS(inSuite, inExpr1, inExpr2)                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        (inSuite)->performedAssertions += 1;                                                                                       \
                                                                                                                                   \
        if ((inExpr1) != (inExpr2))                                                                                                \
        {                                                                                                                          \
            printf("%s:%u: assertion failed: %s == %s\n", __FILE__, __LINE__, #inExpr1, #inExpr2);                                 \
            (inSuite)->failedAssertions += 1;                                                                                      \
            (inSuite)->flagError = true;                                                                                           \
        }                                                                                                                          \
    } while (0)
