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

#include <app/AppConfig.h>
#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

class TestStatusIB : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        StatusIB::RegisterErrorFormatter();
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Macro so failures will blame the right line.
#define VERIFY_ROUNDTRIP(err, status)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        StatusIB newStatus(err);                                                                                                   \
        EXPECT_EQ(newStatus.mStatus, status.mStatus);                                                                              \
        EXPECT_EQ(newStatus.mClusterStatus, status.mClusterStatus);                                                                \
    } while (0);

TEST_F(TestStatusIB, TestStatusIBToFromChipError)
{
    StatusIB status;

    status.mStatus = Status::Success;
    CHIP_ERROR err = status.ToChipError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::Failure;
    err            = status.ToChipError();
    EXPECT_NE(err, CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::InvalidAction;
    err            = status.ToChipError();
    EXPECT_NE(err, CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mClusterStatus = MakeOptional(static_cast<ClusterStatus>(5));

    status.mStatus = Status::Success;
    err            = status.ToChipError();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    status.mStatus = Status::Failure;
    err            = status.ToChipError();
    EXPECT_NE(err, CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::InvalidAction;
    err            = status.ToChipError();
    EXPECT_NE(err, CHIP_NO_ERROR);
    {
        StatusIB newStatus(err);
        EXPECT_EQ(newStatus.mStatus, Status::Failure);
        EXPECT_EQ(newStatus.mClusterStatus, status.mClusterStatus);
    }

    err = CHIP_ERROR_NO_MEMORY;
    {
        StatusIB newStatus(err);
        EXPECT_EQ(newStatus.mStatus, Status::Failure);
        EXPECT_FALSE(newStatus.mClusterStatus.HasValue());
    }
}

#if !CHIP_CONFIG_SHORT_ERROR_STR
TEST_F(TestStatusIB, TestStatusIBErrorToString)
{
    StatusIB status;
    status.mStatus   = Status::InvalidAction;
    CHIP_ERROR err   = status.ToChipError();
    const char * str = ErrorStr(err);

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    EXPECT_STREQ(str, "IM Error 0x00000580: General error: 0x80 (INVALID_ACTION)");
#else  // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    EXPECT_STREQ(str, "IM Error 0x00000580: General error: 0x80");
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

    status.mStatus        = Status::Failure;
    status.mClusterStatus = MakeOptional(static_cast<ClusterStatus>(5));
    err                   = status.ToChipError();
    str                   = ErrorStr(err);
    EXPECT_STREQ(str, "IM Error 0x00000605: Cluster-specific error: 0x05");
}
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

TEST_F(TestStatusIB, TestStatusIBEqualityOperator)
{
    // Equality against self is true.
    StatusIB one;
    EXPECT_EQ(one, one);

    // Default constructors are equal.
    EXPECT_EQ(one, StatusIB());

    // Different imStatus is not equal.
    StatusIB with_imstatus(Status::Failure);
    EXPECT_NE(one, with_imstatus);

    // Same imStatus are equal.
    EXPECT_EQ(with_imstatus, StatusIB(Status::Failure));

    // Same imStatus but different clusterStatus are not equal.
    StatusIB with_cluster_status(Status::Failure, /*clusterStatus=*/2);
    EXPECT_NE(with_imstatus, with_cluster_status);

    // Different imStatus but same clusterStatus are not equal.
    EXPECT_NE(with_cluster_status, StatusIB(Status::Success, /*clusterStatus=*/2));

    // Same imStatus and clusterStatus are equal.
    EXPECT_EQ(with_cluster_status, StatusIB(Status::Failure, /*clusterStatus=*/2));

    // From same CHIP_ERROR are equal.
    StatusIB invalid_argument(CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(invalid_argument, StatusIB(CHIP_ERROR_INVALID_ARGUMENT));

    // Different CHIP_ERROR are equal if they are not from kIMClusterStatus or
    // kIMGlobalStatus.
    EXPECT_EQ(invalid_argument, StatusIB(CHIP_ERROR_INCORRECT_STATE));

    // Error never equals NO_ERROR
    EXPECT_NE(invalid_argument, StatusIB(CHIP_NO_ERROR));
}

TEST_F(TestStatusIB, ConversionsFromClusterStatusCodeWork)
{
    StatusIB successWithCode{ ClusterStatusCode::ClusterSpecificSuccess(123u) };
    EXPECT_EQ(successWithCode.mStatus, Status::Success);
    EXPECT_TRUE(successWithCode.IsSuccess());
    ASSERT_TRUE(successWithCode.mClusterStatus.HasValue());
    EXPECT_EQ(successWithCode.mClusterStatus.Value(), 123u);

    StatusIB failureWithCode{ ClusterStatusCode::ClusterSpecificFailure(42u) };
    EXPECT_EQ(failureWithCode.mStatus, Status::Failure);
    EXPECT_FALSE(failureWithCode.IsSuccess());
    ASSERT_TRUE(failureWithCode.mClusterStatus.HasValue());
    EXPECT_EQ(failureWithCode.mClusterStatus.Value(), 42u);

    StatusIB imStatusInClusterStatusCode{ ClusterStatusCode{ Status::ConstraintError } };
    EXPECT_EQ(imStatusInClusterStatusCode.mStatus, Status::ConstraintError);
    EXPECT_FALSE(imStatusInClusterStatusCode.IsSuccess());
    EXPECT_FALSE(imStatusInClusterStatusCode.mClusterStatus.HasValue());
}

} // namespace
