/*
 *
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

#include <pw_unit_test/framework.h>

/**
 * Run Fixture's class function as a test.
 *
 * It is used to execute test cases that need to use private members of a particular class.
 * Unlike the pigweed macro `FRIEND_TEST`, this approach allows you to define the entire
 * test_fixture class as a friend, rather than having to define each testcase as a friend.
 *
 * @param test_fixture - the fixture class.
 *
 * @param test_name - the name of the test function.
 *
 * Example:
 * class Foo // class to be tested
 * {
 *     friend class TestCtx;
 * private:
 *     bool privateFunction();
 * };
 *
 * class TestCtx: public ::testing::Test
 * {
 * public:
 *    void testFunction();
 * };
 *
 * TEST_F_FROM_FIXTURE(TestCtx, testFunction)
 * {
 *   Foo foo;
 *   EXPECT_TRUE(foo.privateFunction());
 * }
 *
 */
#define TEST_F_FROM_FIXTURE(test_fixture, test_name)                                                                               \
    TEST_F(test_fixture, test_name)                                                                                                \
    {                                                                                                                              \
        test_name();                                                                                                               \
    }                                                                                                                              \
    void test_fixture::test_name()

/**
 * Run Fixture's class function as a test.
 *
 * This macro does not define the body of the test function. It can be used to
 * run a single test case which needs to be run against different fixtures.
 */
#define TEST_F_FROM_FIXTURE_NO_BODY(test_fixture, test_name)                                                                       \
    TEST_F(test_fixture, test_name)                                                                                                \
    {                                                                                                                              \
        test_name();                                                                                                               \
    }

/**
 * Shorthand for EXPECT_EQ(expr, CHIP_NO_ERROR)
 */
#define EXPECT_SUCCESS(expr) EXPECT_EQ((expr), CHIP_NO_ERROR)

/**
 * Shorthand for ASSERT_EQ(expr, CHIP_NO_ERROR)
 */
#define ASSERT_SUCCESS(expr) ASSERT_EQ((expr), CHIP_NO_ERROR)

// Override ASSERT_TRUE and ASSERT_FALSE from pw_unit_test in a way that
// is friendly to static analysis tools like clang-tidy.
//
// The pw_unit_test implementation wraps all expression evaluation in a lambda
// to be able to capture expression results for logging purposes, but this hides
// the relationship between the expression being asserted and the function's
// control flow. Since boolean expressions have only two possible values, we can
// avoid this problem and directly evaluate the expression in the if() condition.

// clang-format off
#define ADD_SIMPLE_EXPECTATION(expression, evaluated, success)               \
    ::pw::unit_test::internal::ReturnHelper() =                              \
        ::pw::unit_test::internal::Framework::Get().CurrentTestExpectSimple( \
            expression, evaluated, __FILE__, __LINE__, success)

/// @def ASSERT_TRUE
/// Verifies that @p expr evaluates to true, otherwise the current function will be aborted.
///
/// @param[in] expr The expression to evaluate.
#undef ASSERT_TRUE
#define ASSERT_TRUE(expr)                                                          \
    if (expr)   ADD_SIMPLE_EXPECTATION(#expr " is true", #expr " is true", true);  \
    else return ADD_SIMPLE_EXPECTATION(#expr " is true", #expr " is false", false)

/// @def ASSERT_FALSE
/// Verifies that @p expr evaluates to false, otherwise the current function will be aborted.
///
/// @param[in] expr The expression to evaluate.
#undef ASSERT_FALSE
#define ASSERT_FALSE(expr)                                                           \
    if (!(expr)) ADD_SIMPLE_EXPECTATION(#expr " is false", #expr " is false", true); \
    else return  ADD_SIMPLE_EXPECTATION(#expr " is false", #expr " is true", false)
// clang-format on
