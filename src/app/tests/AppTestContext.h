/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <messaging/tests/MessagingContext.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class AppContext : public LoopbackMessagingContext
{
public:
    // Performs shared setup for all tests in the test suite
    virtual CHIP_ERROR SetUpTestSuite();
    // Performs shared teardown for all tests in the test suite
    virtual void TearDownTestSuite();
    // Performs setup for each individual test in the test suite
    virtual CHIP_ERROR SetUp();
    // Performs teardown for each individual test in the test suite
    virtual void TearDown();

    // Helpers that can be used directly by the nlTestSuite

    static int nlTestSetUpTestSuite(void * context)
    {
        auto err = static_cast<AppContext *>(context)->SetUpTestSuite();
        return err == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int nlTestTearDownTestSuite(void * context)
    {
        static_cast<AppContext *>(context)->TearDownTestSuite();
        return SUCCESS;
    }

    static int nlTestSetUp(void * context)
    {
        auto err = static_cast<AppContext *>(context)->SetUp();
        return err == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int nlTestTearDown(void * context)
    {
        static_cast<AppContext *>(context)->TearDown();
        return SUCCESS;
    }
};

} // namespace Test
} // namespace chip
