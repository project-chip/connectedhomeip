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

#include <pw_unit_test/framework.h>

#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace ::chip;
using namespace ::chip::Protocols::InteractionModel;

namespace {

TEST(TestStatusCode, TestStatusBasicValues)
{
    EXPECT_EQ(static_cast<int>(Status::Success), 0);
    EXPECT_EQ(static_cast<int>(Status::Failure), 1);
    EXPECT_EQ(static_cast<int>(Status::UnsupportedEndpoint), 0x7f);
    EXPECT_EQ(static_cast<int>(Status::InvalidInState), 0xcb);
}

TEST(TestStatusCode, TestClusterStatusCode)
{
    // Basic usage as a Status.
    {
        ClusterStatusCode status_code_success{ Status::Success };
        EXPECT_EQ(status_code_success.GetStatus(), Status::Success);
        EXPECT_FALSE(status_code_success.HasClusterSpecificCode());
        EXPECT_EQ(status_code_success.GetClusterSpecificCode(), chip::NullOptional);
        EXPECT_TRUE(status_code_success.IsSuccess());

        ClusterStatusCode status_code_failure{ Status::Failure };
        EXPECT_EQ(status_code_failure.GetStatus(), Status::Failure);
        EXPECT_FALSE(status_code_failure.HasClusterSpecificCode());
        EXPECT_FALSE(status_code_failure.IsSuccess());

        ClusterStatusCode status_code_unsupported_ep{ Status::UnsupportedEndpoint };
        EXPECT_EQ(status_code_unsupported_ep.GetStatus(), Status::UnsupportedEndpoint);
        EXPECT_FALSE(status_code_unsupported_ep.HasClusterSpecificCode());
        EXPECT_FALSE(status_code_unsupported_ep.IsSuccess());

        ClusterStatusCode status_code_invalid_in_state{ Status::InvalidInState };
        EXPECT_EQ(status_code_invalid_in_state.GetStatus(), Status::InvalidInState);
        EXPECT_FALSE(status_code_invalid_in_state.HasClusterSpecificCode());
        EXPECT_FALSE(status_code_invalid_in_state.IsSuccess());
    }

    enum RobotoClusterStatus : uint8_t
    {
        kSandwichError = 7,
        kSauceSuccess  = 81,
    };

    // Cluster-specific usage.
    {
        ClusterStatusCode status_code_success = ClusterStatusCode::ClusterSpecificSuccess(RobotoClusterStatus::kSauceSuccess);
        EXPECT_EQ(status_code_success.GetStatus(), Status::Success);
        EXPECT_TRUE(status_code_success.HasClusterSpecificCode());
        EXPECT_EQ(status_code_success.GetClusterSpecificCode(), static_cast<uint8_t>(RobotoClusterStatus::kSauceSuccess));
        EXPECT_TRUE(status_code_success.IsSuccess());

        ClusterStatusCode status_code_failure = ClusterStatusCode::ClusterSpecificFailure(RobotoClusterStatus::kSandwichError);
        EXPECT_EQ(status_code_failure.GetStatus(), Status::Failure);
        EXPECT_TRUE(status_code_failure.HasClusterSpecificCode());
        EXPECT_EQ(status_code_failure.GetClusterSpecificCode(), static_cast<uint8_t>(RobotoClusterStatus::kSandwichError));
        EXPECT_FALSE(status_code_failure.IsSuccess());
    }

    // Copy/Assignment
    {
        ClusterStatusCode status_code_failure1 = ClusterStatusCode::ClusterSpecificFailure(RobotoClusterStatus::kSandwichError);
        ClusterStatusCode status_code_failure2(status_code_failure1);

        EXPECT_EQ(status_code_failure1, status_code_failure2);
        EXPECT_TRUE(status_code_failure1.HasClusterSpecificCode());
        EXPECT_TRUE(status_code_failure2.HasClusterSpecificCode());

        EXPECT_EQ(status_code_failure1.GetClusterSpecificCode(), static_cast<uint8_t>(RobotoClusterStatus::kSandwichError));
        EXPECT_EQ(status_code_failure2.GetClusterSpecificCode(), static_cast<uint8_t>(RobotoClusterStatus::kSandwichError));

        ClusterStatusCode status_code_failure3{ Status::InvalidCommand };
        EXPECT_NE(status_code_failure2, status_code_failure3);

        status_code_failure3 = status_code_failure2;
        EXPECT_EQ(status_code_failure2, status_code_failure3);
    }
}

} // namespace
