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

#include <app-common/zap-generated/cluster-enums.h>
#include <pw_unit_test/framework.h>

#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/tests/ICDConfigurationDataTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TimeUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <system/SystemLayerImpl.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

namespace {} // namespace

namespace chip {
namespace app {

class TestICDConfigurationData : public Test::LoopbackMessagingContext
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        LoopbackMessagingContext::SetUpTestSuite();
        VerifyOrReturn(!HasFailure());

        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        DeviceLayer::SetSystemLayerForTesting(nullptr);

        DeviceLayer::PlatformMgr().Shutdown();

        LoopbackMessagingContext::TearDownTestSuite();
    }

    // Performs setup for each individual test in the test suite
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override { LoopbackMessagingContext::TearDown(); }
};

TEST_F(TestICDConfigurationData, TestICDModeSwitching)
{
    auto & configData = ICDConfigurationData::GetInstance();
    chip::Test::ICDConfigurationDataTestAccess privateConfigData(&configData);

    // Default mode should be SIT
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Switch to LIT and check
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::LIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Switch back to SIT and check
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::SIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::SIT);
}

TEST_F(TestICDConfigurationData, TestSetSITPollingInterval)
{
    ICDConfigurationDataTestAccess privateConfigData(&ICDConfigurationData::GetInstance());
    System::Clock::Milliseconds32 validSITPollInterval(10000);
    System::Clock::Milliseconds32 invalidSITPollInterval =
        privateConfigData.GetSitSlowPollMaximum() + System::Clock::Milliseconds32(1000); // Above SIT threshold

    // Should succeed for valid value
    EXPECT_EQ(privateConfigData.SetSITPollingInterval(validSITPollInterval), CHIP_NO_ERROR);

    // Should fail for invalid value
    EXPECT_EQ(privateConfigData.SetSITPollingInterval(invalidSITPollInterval), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestICDConfigurationData, TestGetAndSetSlowPollingInterval)
{
    auto & configData = ICDConfigurationData::GetInstance();
    ICDConfigurationDataTestAccess privateConfigData(&configData);

    // Set featuremap to include LIT support
    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport);
    privateConfigData.SetFeatureMap(featureMap);

    // Set a ICD SIT Slow Poll Interval
    System::Clock::Milliseconds32 SITPollInterval(10000);
    EXPECT_EQ(privateConfigData.SetSITPollingInterval(SITPollInterval), CHIP_NO_ERROR);

    // Set operation mode to LIT and confirm mode
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::LIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Set a slow polling interval of 60s, used in LIT mode and verify
    System::Clock::Milliseconds32 newInterval(60000);
    EXPECT_EQ(privateConfigData.SetSlowPollingInterval(newInterval), CHIP_NO_ERROR);

    // In LIT mode Standard SlowPollingInterval is always used
    EXPECT_EQ(configData.GetSlowPollingInterval(), newInterval);

    // Switch operation mode to SIT and confirm mode
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::SIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // In SIT mode The shortest interval between SlowPollingInterval and SITPollingInterval is used
    // In this case the the SITPollInterval interval is used
    EXPECT_EQ(configData.GetSlowPollingInterval(), SITPollInterval);

    // Reduce slow polling interval to 5s, shorter than SITPollingInterval
    System::Clock::Milliseconds32 shortSlowPollInterval(5000);
    privateConfigData.SetSlowPollingInterval(shortSlowPollInterval);
    EXPECT_EQ(privateConfigData.SetSlowPollingInterval(shortSlowPollInterval), CHIP_NO_ERROR);
    EXPECT_EQ(configData.GetSlowPollingInterval(), shortSlowPollInterval);

    // Switch operation mode to LIT and confirm mode
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::LIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::LIT);
    // slow polling interval remains in use
    EXPECT_EQ(configData.GetSlowPollingInterval(), shortSlowPollInterval);

    // increase slow polling interval to 20s, longer than SITPollingInterval
    System::Clock::Milliseconds32 longerSlowPollInterval(20000);
    EXPECT_EQ(privateConfigData.SetSlowPollingInterval(longerSlowPollInterval), CHIP_NO_ERROR);
    // longerSlowPollInterval is used
    EXPECT_EQ(configData.GetSlowPollingInterval(), longerSlowPollInterval);

    // Switch operation mode to SIT and confirm mode
    privateConfigData.SetICDMode(ICDConfigurationData::ICDMode::SIT);
    EXPECT_EQ(configData.GetICDMode(), ICDConfigurationData::ICDMode::SIT);
    //  SIT Polling Interval is used
    EXPECT_EQ(configData.GetSlowPollingInterval(), SITPollInterval);

    featureMap.Clear(Feature::kLongIdleTimeSupport);
    privateConfigData.SetFeatureMap(featureMap);
    // Without LIT support, the slow polling interval cannot bet set greater than the SIT polling threshold
    EXPECT_EQ(privateConfigData.SetSlowPollingInterval(longerSlowPollInterval), CHIP_ERROR_INVALID_ARGUMENT);
    // Set a Valid Slow Polling Interval greater than the SIT Polling Interval
    System::Clock::Milliseconds32 validSlowPollInterval(12000);
    EXPECT_EQ(privateConfigData.SetSlowPollingInterval(validSlowPollInterval), CHIP_NO_ERROR);
    //  Without LIT support, the slow polling interval is used and the SIT Polling Interval value is not taken into account
    EXPECT_EQ(configData.GetSlowPollingInterval(), validSlowPollInterval);
}

TEST_F(TestICDConfigurationData, TestSetModeDurations)
{
    auto & configData = ICDConfigurationData::GetInstance();
    ICDConfigurationDataTestAccess privateConfigData(&configData);
    using namespace System::Clock;

    // Save original values
    Seconds32 origIdle        = configData.GetIdleModeDuration();
    Milliseconds32 origActive = configData.GetActiveModeDuration();

    // Set valid durations
    Milliseconds32 newActive(2000);
    Seconds32 newIdle(10);
    EXPECT_EQ(privateConfigData.SetModeDurations(MakeOptional(newActive), MakeOptional(Milliseconds32(newIdle.count() * 1000))),
              CHIP_NO_ERROR);
    EXPECT_EQ(configData.GetActiveModeDuration(), newActive);
    EXPECT_EQ(configData.GetIdleModeDuration(), newIdle);

    // Set invalid: active > idle
    EXPECT_EQ(privateConfigData.SetModeDurations(MakeOptional(Milliseconds32(20000)), MakeOptional(Milliseconds32(1000))),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Restore original values
    EXPECT_EQ(privateConfigData.SetModeDurations(MakeOptional(origActive), MakeOptional(Milliseconds32(origIdle.count() * 1000))),
              CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
