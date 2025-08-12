/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/support/RpcErrorMapping.h>

namespace {

struct StatusMappingTestCase
{
    pw_Status code;
    CHIP_ERROR expected;
};

class MapRpcStatusToChipErrorTest : public ::testing::TestWithParam<StatusMappingTestCase>
{
};

TEST_P(MapRpcStatusToChipErrorTest, MapsCorrectly)
{
    const auto & param = GetParam();
    pw::Status status(param.code);
    EXPECT_EQ(chip::rpc::MapRpcStatusToChipError(status), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    AllMappings, MapRpcStatusToChipErrorTest,
    ::testing::Values(StatusMappingTestCase{ pw_Status::PW_STATUS_OK, CHIP_NO_ERROR },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_CANCELLED, CHIP_ERROR_CANCELLED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_UNKNOWN, CHIP_ERROR_INTERNAL },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_INVALID_ARGUMENT, CHIP_ERROR_INVALID_ARGUMENT },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_DEADLINE_EXCEEDED, CHIP_ERROR_TIMEOUT },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_NOT_FOUND, CHIP_ERROR_NOT_FOUND },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_ALREADY_EXISTS, CHIP_ERROR_ALREADY_INITIALIZED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_PERMISSION_DENIED, CHIP_ERROR_ACCESS_DENIED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_RESOURCE_EXHAUSTED, CHIP_ERROR_NO_MEMORY },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_FAILED_PRECONDITION, CHIP_ERROR_INCORRECT_STATE },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_ABORTED, CHIP_ERROR_TRANSACTION_CANCELED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_OUT_OF_RANGE, CHIP_ERROR_INVALID_ARGUMENT },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_UNIMPLEMENTED, CHIP_ERROR_NOT_IMPLEMENTED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_INTERNAL, CHIP_ERROR_INTERNAL },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_UNAVAILABLE, CHIP_ERROR_NOT_CONNECTED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_DATA_LOSS, CHIP_ERROR_DECODE_FAILED },
                      StatusMappingTestCase{ pw_Status::PW_STATUS_UNAUTHENTICATED, CHIP_ERROR_INTEGRITY_CHECK_FAILED }));

TEST(MapRpcStatusToChipErrorTest, UnknownCodeMapsToInternal)
{
    // non-sense pw_Status enum
    pw::Status status(static_cast<pw_Status>(1337));
    EXPECT_EQ(chip::rpc::MapRpcStatusToChipError(status), CHIP_ERROR_INTERNAL);
}

} // namespace
