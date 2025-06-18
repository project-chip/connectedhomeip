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

#include <gtest/gtest.h>

#include <controller/CommissioningWindowOpener.h>
#include <controller/CommissioningWindowParams.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace {

class TestCommissioningWindowVerifierParams : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST(TestCommissioningWindowPasscodeParams, CommissioningWindowCommonParams_SuccessSetGet)
{
    Controller::CommissioningWindowPasscodeParams params = Controller::CommissioningWindowPasscodeParams();

    //NodeId
    EXPECT_FALSE(params.HasNodeId());
    params.SetNodeId(kMinGroupNodeId);
    EXPECT_EQ(params.GetNodeId(), kMinGroupNodeId);
    EXPECT_TRUE(params.HasNodeId());

    //EndpointId
    EXPECT_EQ(params.GetEndpointId(), kRootEndpointId);
    params.SetEndpointId(kRootEndpointId);
    EXPECT_EQ(params.GetEndpointId(), kRootEndpointId);

    //Timeout
    EXPECT_EQ(params.GetTimeout(), chip::System::Clock::Seconds16(300));
    params.SetTimeout(400);
    EXPECT_EQ(params.GetTimeout(), chip::System::Clock::Seconds16(400));
    params.SetTimeout(chip::System::Clock::Seconds16(5));
    EXPECT_EQ(params.GetTimeout(), chip::System::Clock::Seconds16(5));

    //Iteration
    EXPECT_EQ(params.GetIteration(), uint32_t(1000));
    params.SetIteration(200);
    EXPECT_EQ(params.GetIteration(), uint32_t(200));
    params.SetIteration(400);
    EXPECT_EQ(params.GetIteration(), uint32_t(400));

    //Descriminator
    EXPECT_FALSE(params.HasDiscriminator());
    params.SetDiscriminator(100);
    EXPECT_EQ(params.GetDiscriminator(), 100);
    EXPECT_TRUE(params.HasDiscriminator());

}
} // namespace
