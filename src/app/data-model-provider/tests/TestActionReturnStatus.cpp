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

#include "lib/core/CHIPError.h"
#include "protocols/interaction_model/StatusCode.h"
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>

#include <pw_unit_test/framework.h>

using chip::app::DataModel::ActionReturnStatus;
using chip::Protocols::InteractionModel::Status;
using chip::Protocols::InteractionModel::ClusterStatusCode;

TEST(TestActionReturnStatus, TestEquality)
{
    // equality should happen between equivalent statuses and chip_errors
    ASSERT_EQ(ActionReturnStatus(Status::UnsupportedRead), Status::UnsupportedRead);
    ASSERT_EQ(ActionReturnStatus(Status::UnsupportedWrite), CHIP_IM_GLOBAL_STATUS(UnsupportedRead));

    ASSERT_EQ(ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(Busy)), Status::Busy);
    ASSERT_EQ(ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(Busy)), CHIP_IM_GLOBAL_STATUS(Busy));

    ASSERT_EQ(ActionReturnStatus(CHIP_IM_CLUSTER_STATUS(123)), CHIP_IM_CLUSTER_STATUS(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)), CHIP_IM_CLUSTER_STATUS(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificFailure(123)), ClusterStatusCode::ClusterSpecificFailure(123));
    ASSERT_EQ(ActionReturnStatus(ClusterStatusCode::ClusterSpecificSuccess(123)), ClusterStatusCode::ClusterSpecificSuccess(123));
}

TEST(TestActionReturnStatus, TestIsError)
{
    AS
}
