/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdint.h>

#include <lib/core/Optional.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/interaction_model/StatusCode.h>

#include <nlunit-test.h>

using namespace ::chip;
using namespace ::chip::Protocols::InteractionModel;

namespace {

void TestStatusBasicValues(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT_EQUALS(inSuite, static_cast<int>(Status::Success), 0);
    NL_TEST_ASSERT_EQUALS(inSuite, static_cast<int>(Status::Failure), 1);
    NL_TEST_ASSERT_EQUALS(inSuite, static_cast<int>(Status::UnsupportedEndpoint), 0x7f);
    NL_TEST_ASSERT_EQUALS(inSuite, static_cast<int>(Status::InvalidInState), 0xcb);
}

void TestStatusCode(nlTestSuite * inSuite, void * inContext)
{
    // Basic usage as a Status.
    {
        StatusCode status_code_success{ Status::Success };
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success, Status::Success);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success.GetStatus(), Status::Success);
        NL_TEST_ASSERT(inSuite, !status_code_success.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success, 0);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success.GetClusterSpecificCode(), chip::NullOptional);
        NL_TEST_ASSERT(inSuite, status_code_success.IsSuccess());

        StatusCode status_code_failure{ Status::Failure };
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure, Status::Failure);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure.GetStatus(), Status::Failure);
        NL_TEST_ASSERT(inSuite, !status_code_failure.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure, 1u);
        NL_TEST_ASSERT(inSuite, !status_code_failure.IsSuccess());

        StatusCode status_code_unsupported_ep{ Status::UnsupportedEndpoint };
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_unsupported_ep, Status::UnsupportedEndpoint);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_unsupported_ep.GetStatus(), Status::UnsupportedEndpoint);
        NL_TEST_ASSERT(inSuite, !status_code_unsupported_ep.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_unsupported_ep, static_cast<uint8_t>(0x7f));
        NL_TEST_ASSERT(inSuite, !status_code_unsupported_ep.IsSuccess());

        StatusCode status_code_invalid_in_state{ Status::InvalidInState };
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_invalid_in_state, Status::InvalidInState);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_invalid_in_state.GetStatus(), Status::InvalidInState);
        NL_TEST_ASSERT(inSuite, !status_code_invalid_in_state.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_invalid_in_state, static_cast<uint8_t>(0xcb));
        NL_TEST_ASSERT(inSuite, !status_code_invalid_in_state.IsSuccess());
    }

    enum RobotoStatusCode : uint8_t
    {
        kSandwichError = 7,
        kSauceSuccess  = 81,
    };

    // Cluster-specific usage.
    {
        StatusCode status_code_success = StatusCode::ClusterSpecificSuccess(RobotoStatusCode::kSauceSuccess);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success, Status::Success);
        NL_TEST_ASSERT(inSuite, status_code_success.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success, 0);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_success.GetClusterSpecificCode(),
                              static_cast<uint8_t>(RobotoStatusCode::kSauceSuccess));
        NL_TEST_ASSERT(inSuite, status_code_success.IsSuccess());

        StatusCode status_code_failure = StatusCode::ClusterSpecificFailure(RobotoStatusCode::kSandwichError);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure, Status::Failure);
        NL_TEST_ASSERT(inSuite, status_code_failure.HasClusterSpecificCode());
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure, 1);
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure.GetClusterSpecificCode(),
                              static_cast<uint8_t>(RobotoStatusCode::kSandwichError));
        NL_TEST_ASSERT(inSuite, !status_code_failure.IsSuccess());
    }

    // Copy/Assignment
    {
        StatusCode status_code_failure1 = StatusCode::ClusterSpecificFailure(RobotoStatusCode::kSandwichError);
        StatusCode status_code_failure2(status_code_failure1);

        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure1, status_code_failure2);
        NL_TEST_ASSERT(inSuite, status_code_failure1.HasClusterSpecificCode());
        NL_TEST_ASSERT(inSuite, status_code_failure2.HasClusterSpecificCode());

        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure1.GetClusterSpecificCode(),
                              static_cast<uint8_t>(RobotoStatusCode::kSandwichError));
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure2.GetClusterSpecificCode(),
                              static_cast<uint8_t>(RobotoStatusCode::kSandwichError));

        StatusCode status_code_failure3{ Status::InvalidCommand };
        NL_TEST_ASSERT(inSuite, status_code_failure2 != status_code_failure3);

        status_code_failure3 = status_code_failure2;
        NL_TEST_ASSERT_EQUALS(inSuite, status_code_failure2, status_code_failure3);
    }
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestStatusBasicValues", TestStatusBasicValues),
    NL_TEST_DEF("TestStatusCode", TestStatusCode),
    NL_TEST_SENTINEL()
};
// clang-format on

nlTestSuite sSuite = { "Test IM Status Code abstractions", &sTests[0], nullptr, nullptr };
} // namespace

int TestStatusCode()
{
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestStatusCode)
