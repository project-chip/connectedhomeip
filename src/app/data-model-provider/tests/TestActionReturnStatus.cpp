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
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/StringBuilderAdapters.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

#include <pw_unit_test/framework.h>

using chip::app::DataModel::ActionReturnStatus;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

TEST(TestActionReturnStatus, TestEquality)
{
    // equality should happen between equivalent statuses and chip_errors
    ASSERT_EQ(ActionReturnStatus(Status::UnsupportedRead), Status::UnsupportedRead);
    ASSERT_EQ(ActionReturnStatus(Status::UnsupportedWrite), CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));

    ASSERT_EQ(ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(Busy)), Status::Busy);
    ASSERT_EQ(ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(Busy)), CHIP_IM_GLOBAL_STATUS(Busy));

    ASSERT_EQ(ActionReturnStatus(CHIP_IM_CLUSTER_STATUS(123)), CHIP_IM_CLUSTER_STATUS(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)), CHIP_IM_CLUSTER_STATUS(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)), ClusterStatusCode::ClusterSpecificFailure(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)), ClusterStatusCode::ClusterSpecificSuccess(123));

    // Successes (without cluster-specific codes) are equivalent
    ASSERT_EQ(ActionReturnStatus(Status::Success), Status::Success);
    ASSERT_EQ(ActionReturnStatus(CHIP_NO_ERROR), Status::Success);
    ASSERT_EQ(ActionReturnStatus(Status::Success), CHIP_NO_ERROR);

    // status specific success is has more data, so there is no equality (i.e. an action return
    // with specific success codes has more data than a simple success)
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)), CHIP_NO_ERROR);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)), Status::Success);
    ASSERT_NE(ActionReturnStatus(CHIP_NO_ERROR), ClusterStatusCode::ClusterSpecificSuccess(123));
    ASSERT_NE(ActionReturnStatus(Status::Success), ClusterStatusCode::ClusterSpecificSuccess(123));

    // things that are just not equal
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(11)), ClusterStatusCode::ClusterSpecificSuccess(22));
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(11)), ClusterStatusCode::ClusterSpecificSuccess(11));
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(11)), ClusterStatusCode::ClusterSpecificFailure(22));
    ASSERT_NE(ActionReturnStatus(CHIP_NO_ERROR), CHIP_ERROR_NOT_FOUND);
    ASSERT_NE(ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT), CHIP_ERROR_NOT_FOUND);
    ASSERT_NE(ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT), CHIP_NO_ERROR);
    ASSERT_NE(ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT), Status::Success);
    ASSERT_NE(ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT), Status::UnsupportedRead);
    ASSERT_NE(ActionReturnStatus(Status::Success), Status::UnsupportedRead);
    ASSERT_NE(ActionReturnStatus(Status::Success), CHIP_ERROR_INVALID_ARGUMENT);
    ASSERT_NE(ActionReturnStatus(CHIP_ERROR_NOT_FOUND), Status::Failure);
    ASSERT_NE(ActionReturnStatus(Status::Failure), CHIP_NO_ERROR);
    ASSERT_NE(ActionReturnStatus(Status::Failure), CHIP_ERROR_INVALID_ARGUMENT);
    ASSERT_NE(ActionReturnStatus(Status::Failure), ClusterStatusCode::ClusterSpecificSuccess(1));
    ASSERT_NE(ActionReturnStatus(Status::Failure), ClusterStatusCode::ClusterSpecificFailure(2));
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(1)), CHIP_NO_ERROR);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(2)), CHIP_NO_ERROR);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(3)), Status::Failure);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(4)), Status::Failure);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(3)), Status::Success);
    ASSERT_NE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(4)), Status::Success);
}

TEST(TestActionReturnStatus, TestIsError)
{
    ASSERT_TRUE(ActionReturnStatus(CHIP_IM_CLUSTER_STATUS(123)).IsError());
    ASSERT_TRUE(ActionReturnStatus(CHIP_ERROR_INTERNAL).IsError());
    ASSERT_TRUE(ActionReturnStatus(CHIP_ERROR_NO_MEMORY).IsError());
    ASSERT_TRUE(ActionReturnStatus(Status::UnsupportedRead).IsError());
    ASSERT_TRUE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)).IsError());

    ASSERT_FALSE(ActionReturnStatus(Status::Success).IsError());
    ASSERT_FALSE(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)).IsError());
    ASSERT_FALSE(ActionReturnStatus(CHIP_NO_ERROR).IsError());
}

TEST(TestActionReturnStatus, TestUnderlyingError)
{
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)).GetUnderlyingError(), CHIP_IM_CLUSTER_STATUS(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)).GetUnderlyingError(), CHIP_NO_ERROR);
    ASSERT_EQ(ActionReturnStatus(Status::Busy).GetUnderlyingError(), CHIP_IM_GLOBAL_STATUS(Busy));
    ASSERT_EQ(ActionReturnStatus(CHIP_ERROR_INTERNAL).GetUnderlyingError(), CHIP_ERROR_INTERNAL);
}

TEST(TestActionReturnStatus, TestStatusCode)
{
    ASSERT_EQ(ActionReturnStatus(CHIP_ERROR_INTERNAL).GetStatusCode(), ClusterStatusCode(Status::Failure));
    ASSERT_EQ(ActionReturnStatus(Status::Busy).GetStatusCode(), ClusterStatusCode(Status::Busy));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)).GetStatusCode(),
              ClusterStatusCode::ClusterSpecificSuccess(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)).GetStatusCode(),
              ClusterStatusCode::ClusterSpecificFailure(123));
    ASSERT_EQ(ActionReturnStatus(CHIP_IM_CLUSTER_STATUS(0x12)).GetStatusCode(), ClusterStatusCode::ClusterSpecificFailure(0x12));
    ASSERT_EQ(ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(Timeout)).GetStatusCode(), ClusterStatusCode(Status::Timeout));
}

TEST(TestActionReturnStatus, TestCString)
{
    /// only tests the strings that we build and NOT the CHIP_ERROR ones which
    /// are tested separately. for chip_error we just say it should not be empty.
    ActionReturnStatus::StringStorage buffer;
    ActionReturnStatus status(Status::Success);

    // chip-error returns something non-empty
    status = CHIP_ERROR_NOT_FOUND;
    ASSERT_STRNE(status.c_str(buffer), "");

    status = CHIP_NO_ERROR;
    ASSERT_STRNE(status.c_str(buffer), "");

    // the items below we control
#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    status = Status::Success;
    ASSERT_STREQ(status.c_str(buffer), "SUCCESS(0)");

    status = Status::UnsupportedCommand;
    ASSERT_STREQ(status.c_str(buffer), "UNSUPPORTED_COMMAND(129)");

    status = ClusterStatusCode::ClusterSpecificSuccess(31);
    ASSERT_STREQ(status.c_str(buffer), "SUCCESS(0), Code 31");

    status = ClusterStatusCode::ClusterSpecificFailure(32);
    ASSERT_STREQ(status.c_str(buffer), "FAILURE(1), Code 32");
#else
    status = Status::Success;
    ASSERT_STREQ(status.c_str(buffer), "Success");

    status = Status::UnsupportedCommand;
    ASSERT_STREQ(status.c_str(buffer), "Status<129>");

    status = ClusterStatusCode::ClusterSpecificSuccess(31);
    ASSERT_STREQ(status.c_str(buffer), "Success, Code 31");

    status = ClusterStatusCode::ClusterSpecificFailure(32);
    ASSERT_STREQ(status.c_str(buffer), "Status<1>, Code 32");
#endif
}
