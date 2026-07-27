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
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <platform/silabs/wifi/icd/WifiSleepManager.h>

using namespace chip::DeviceLayer::Silabs;

namespace {

class TestMock : public PowerSaveInterface, public WifiStateProvider
{
public:
    void Reset()
    {
        mConfigurePowerSaveCalled       = false;
        mConfigureBroadcastFilterCalled = false;
        mIsWifiProvisioned              = false;
        mBroadcastFilterEnabled         = false;
    }

    // Getters to check if methods were called
    bool WasConfigurePowerSaveCalled()
    {
        bool wasCalled            = mConfigurePowerSaveCalled;
        mConfigurePowerSaveCalled = false;
        return wasCalled;
    }

    bool WasConfigureBroadcastFilterCalled()
    {
        bool wasCalled                  = mConfigureBroadcastFilterCalled;
        mConfigureBroadcastFilterCalled = false;
        return wasCalled;
    }

    bool WasBroadcastFilterEnabled()
    {
        bool wasEnabled         = mBroadcastFilterEnabled;
        mBroadcastFilterEnabled = false;
        return wasEnabled;
    }

    PowerSaveConfiguration GetLastPowerSaveConfiguration() const { return mLastPowerSaveConfiguration; }

    // Setter for IsWifiProvisioned
    void SetIsWifiProvisioned(bool isProvisioned) { mIsWifiProvisioned = isProvisioned; }

    CHIP_ERROR ConfigurePowerSave(PowerSaveConfiguration configuration, uint32_t listenInterval) override
    {
        mConfigurePowerSaveCalled   = true;
        mLastPowerSaveConfiguration = configuration;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter) override
    {
        mConfigureBroadcastFilterCalled = true;
        mBroadcastFilterEnabled         = enableBroadcastFilter;
        return CHIP_NO_ERROR;
    }

    bool IsWifiProvisioned() override { return mIsWifiProvisioned; }

    bool IsStationConnected() override { return false; }
    bool IsStationModeEnabled() override { return false; }
    bool IsStationReady() override { return false; }
    bool HasAnIPv6Address() override { return false; }
    bool HasAnIPv4Address() override { return false; }

private:
    bool mConfigurePowerSaveCalled       = false;
    bool mConfigureBroadcastFilterCalled = false;
    bool mBroadcastFilterEnabled         = false;
    PowerSaveConfiguration mLastPowerSaveConfiguration;
    bool mIsWifiProvisioned = false;
};

} // namespace

class TestWifiSleepManager : public ::testing::Test
{
protected:
    void SetUp()
    {
        EXPECT_EQ(WifiSleepManager::GetInstance().Init(&mMock, &mMock), CHIP_NO_ERROR);
        EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
        EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kDeepSleep);
    }
    void TearDown() { mMock.Reset(); }

    static TestMock mMock;
};

TestMock TestWifiSleepManager::mMock;

TEST_F(TestWifiSleepManager, TestInitFailureNullPowerSaveInterface)
{
    TestMock mock;
    EXPECT_EQ(WifiSleepManager::GetInstance().Init(nullptr, &mock), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(mMock.WasConfigurePowerSaveCalled());
}

TEST_F(TestWifiSleepManager, TestInitFailureNullWifiStateProvider)
{
    TestMock mock;
    EXPECT_EQ(WifiSleepManager::GetInstance().Init(&mock, nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(mMock.WasConfigurePowerSaveCalled());
}

TEST_F(TestWifiSleepManager, TestRequestHighPerformanceMaxCounter)
{
    // Set the counter to its maximum value
    for (uint8_t i = 0; i < std::numeric_limits<uint8_t>::max(); i++)
    {
        EXPECT_EQ(WifiSleepManager::GetInstance().RequestHighPerformanceWithTransition(), CHIP_NO_ERROR);
        EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kHighPerformance);
        EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
    }

    // The next request should fail
    EXPECT_EQ(WifiSleepManager::GetInstance().RequestHighPerformanceWithTransition(), CHIP_ERROR_INTERNAL);
    EXPECT_FALSE(mMock.WasConfigurePowerSaveCalled());

    // Reset the counter & validate reset
    for (uint8_t i = 0; i < std::numeric_limits<uint8_t>::max(); i++)
    {
        EXPECT_EQ(WifiSleepManager::GetInstance().RemoveHighPerformanceRequest(), CHIP_NO_ERROR);
    }
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kDeepSleep);
}

TEST_F(TestWifiSleepManager, TestRequestRemoveHighPerformance)
{
    EXPECT_EQ(WifiSleepManager::GetInstance().RequestHighPerformanceWithTransition(), CHIP_NO_ERROR);
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kHighPerformance);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());

    EXPECT_EQ(WifiSleepManager::GetInstance().RemoveHighPerformanceRequest(), CHIP_NO_ERROR);
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kDeepSleep);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
}

TEST_F(TestWifiSleepManager, TestRemovePerformanceRequestSubMinimum)
{
    EXPECT_EQ(WifiSleepManager::GetInstance().RemoveHighPerformanceRequest(), CHIP_NO_ERROR);
    EXPECT_FALSE(mMock.WasConfigurePowerSaveCalled());
}

// This tests will need to be updated once the High performance req is removed from the WifiSleepManager
TEST_F(TestWifiSleepManager, TestCommissioningInProgressAndStopped)
{
    WifiSleepManager::GetInstance().HandleCommissioningSessionStarted();
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kHighPerformance);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());

    WifiSleepManager::GetInstance().HandleCommissioningSessionStopped();
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kDeepSleep);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
}

TEST_F(TestWifiSleepManager, TestCommissioningInProgressAndCompleted)
{
    WifiSleepManager::GetInstance().HandleCommissioningSessionStarted();
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kHighPerformance);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());

    WifiSleepManager::GetInstance().HandleCommissioningSessionStopped();
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kDeepSleep);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
}

TEST_F(TestWifiSleepManager, TestVerifyOrTransitionStandardOperation)
{
    mMock.SetIsWifiProvisioned(true);

    // Test the standard operation
    EXPECT_EQ(WifiSleepManager::GetInstance().VerifyAndTransitionToLowPowerMode(WifiSleepManager::PowerEvent::kGenericEvent),
              CHIP_NO_ERROR);

    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), PowerSaveInterface::PowerSaveConfiguration::kConnectedSleep);
    EXPECT_TRUE(mMock.WasConfigurePowerSaveCalled());
    EXPECT_TRUE(mMock.WasConfigureBroadcastFilterCalled());
    EXPECT_FALSE(mMock.WasBroadcastFilterEnabled());
}

TEST_F(TestWifiSleepManager, TestRequestHighPerformanceWithoutProvisioning)
{
    EXPECT_EQ(WifiSleepManager::GetInstance().VerifyAndTransitionToLowPowerMode(WifiSleepManager::PowerEvent::kGenericEvent),
              CHIP_NO_ERROR);

    PowerSaveInterface::PowerSaveConfiguration config = mMock.GetLastPowerSaveConfiguration();
    EXPECT_EQ(PowerSaveInterface::PowerSaveConfiguration::kDeepSleep, config);

    // The configuration should not change
    EXPECT_EQ(WifiSleepManager::GetInstance().RequestHighPerformanceWithoutTransition(), CHIP_NO_ERROR);
    EXPECT_EQ(mMock.GetLastPowerSaveConfiguration(), config);
}
