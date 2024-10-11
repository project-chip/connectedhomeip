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

/*
 * Run Fixture's class function as a test.
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
