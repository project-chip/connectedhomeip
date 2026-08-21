/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/CASEClientPool.h>
#include <app/OperationalSessionSetup.h>
#include <app/tests/AppTestContext.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

namespace chip {
namespace {

constexpr NodeId kTestNodeId              = 0x123456789abcdefULL;
constexpr FabricIndex kFabricIndex        = 1;
constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

class MockCASEClientPool : public CASEClientPoolDelegate
{
public:
    CASEClient * Allocate() override { return nullptr; }
    void Release(CASEClient * client) override {}
};

class MockOperationalSessionReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup * sessionSetup) override {}
};

} // namespace

class TestOperationalSessionSetupBusy : public chip::Testing::AppContext
{
public:
    void BusyResponseAfterAttemptExhaustionGrantsOneRetry();
    void BusyResponseKeepsExistingResolveRetryBudget();
    void BusyResponseWithRemainingBudgetDoesNotGrantExtra();
    void BusyResponseGrantsExtraRetryOnlyOnce();

    void SetUp() override
    {
        AppContext::SetUp();
        Credentials::SetGroupDataProvider(&mGroupDataProvider);
    }

    void TearDown() override
    {
        Credentials::SetGroupDataProvider(nullptr);
        AppContext::TearDown();
    }

protected:
    CASEClientInitParams CreateInitParams()
    {
        CASEClientInitParams params;
        params.sessionManager    = &GetSecureSessionManager();
        params.exchangeMgr       = &GetExchangeManager();
        params.fabricTable       = &GetFabricTable();
        params.groupDataProvider = &mGroupDataProvider;
        return params;
    }

    OperationalSessionSetup CreateSessionSetup()
    {
        return OperationalSessionSetup(CreateInitParams(), &mClientPool, ScopedNodeId(kTestNodeId, kFabricIndex),
                                       &mReleaseDelegate);
    }

    Credentials::GroupDataProviderImpl mGroupDataProvider{ kMaxGroupsPerFabric, kMaxGroupKeysPerFabric };
    MockCASEClientPool mClientPool;
    MockOperationalSessionReleaseDelegate mReleaseDelegate;
};

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

TEST_F_FROM_FIXTURE(TestOperationalSessionSetupBusy, BusyResponseAfterAttemptExhaustionGrantsOneRetry)
{
    OperationalSessionSetup sessionSetup = CreateSessionSetup();

    sessionSetup.mRemainingAttempts                      = 0;
    sessionSetup.mResolveAttemptsAllowed                 = 0;
    sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion = false;

    sessionSetup.OnResponderBusy(System::Clock::Milliseconds16(5000));

    EXPECT_EQ(sessionSetup.mRemainingAttempts, 1u);
    EXPECT_EQ(sessionSetup.mResolveAttemptsAllowed, 1u);
    EXPECT_TRUE(sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion);
}

TEST_F_FROM_FIXTURE(TestOperationalSessionSetupBusy, BusyResponseKeepsExistingResolveRetryBudget)
{
    OperationalSessionSetup sessionSetup = CreateSessionSetup();

    sessionSetup.mRemainingAttempts                      = 0;
    sessionSetup.mResolveAttemptsAllowed                 = 3;
    sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion = false;

    sessionSetup.OnResponderBusy(System::Clock::Milliseconds16(5000));

    EXPECT_EQ(sessionSetup.mRemainingAttempts, 1u);
    EXPECT_EQ(sessionSetup.mResolveAttemptsAllowed, 3u);
    EXPECT_TRUE(sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion);
}

TEST_F_FROM_FIXTURE(TestOperationalSessionSetupBusy, BusyResponseWithRemainingBudgetDoesNotGrantExtra)
{
    OperationalSessionSetup sessionSetup = CreateSessionSetup();

    sessionSetup.mRemainingAttempts                      = 2;
    sessionSetup.mResolveAttemptsAllowed                 = 0;
    sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion = false;

    sessionSetup.OnResponderBusy(System::Clock::Milliseconds16(5000));

    EXPECT_EQ(sessionSetup.mRemainingAttempts, 2u);
    EXPECT_EQ(sessionSetup.mResolveAttemptsAllowed, 0u);
    EXPECT_FALSE(sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion);
}

TEST_F_FROM_FIXTURE(TestOperationalSessionSetupBusy, BusyResponseGrantsExtraRetryOnlyOnce)
{
    OperationalSessionSetup sessionSetup = CreateSessionSetup();

    sessionSetup.mRemainingAttempts                      = 0;
    sessionSetup.mResolveAttemptsAllowed                 = 0;
    sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion = false;

    sessionSetup.OnResponderBusy(System::Clock::Milliseconds16(5000));

    sessionSetup.mRemainingAttempts      = 0;
    sessionSetup.mResolveAttemptsAllowed = 0;

    sessionSetup.OnResponderBusy(System::Clock::Milliseconds16(5000));

    EXPECT_EQ(sessionSetup.mRemainingAttempts, 0u);
    EXPECT_EQ(sessionSetup.mResolveAttemptsAllowed, 0u);
    EXPECT_TRUE(sessionSetup.mGrantedBusyRetryAfterAttemptExhaustion);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

} // namespace chip
