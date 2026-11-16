/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "../SilabsTimeSyncDelegate.h"
#include <app/AppConfig.h>
#include <app/clusters/time-synchronization-server/TimeSynchronizationCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <system/RAIIMockClock.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::DeviceLayer::Silabs::SilabsTimeSyncDelegate;

namespace TimeSync = chip::app::Clusters::TimeSynchronization;

class TestSilabsTimeSyncDelegate : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void CheckDirectConstruction(BitFlags<TimeSync::Feature> features)
    {
        TimeSynchronizationCluster cluster(kRootEndpointId, features, {},
                                           TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetDelegate(), &delegate);
        EXPECT_EQ(cluster.GetGranularity(), TimeSync::GranularityEnum::kNoTimeGranularity);

        Testing::ClusterTester tester(cluster);
        uint32_t featureMap = 0;
        EXPECT_EQ(tester.ReadAttribute(TimeSync::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, features.Raw());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    Testing::TestServerClusterContext testContext;
    SilabsTimeSyncDelegate delegate;
    const TimeSynchronizationCluster::Context context = { .fabricTable            = testContext.GetFabricTable(),
                                                          .caseSessionManager     = testContext.GetCASESessionManager(),
                                                          .platformManager        = testContext.GetPlatformManager(),
                                                          .interactionModelEngine = testContext.GetInteractionModelEngine() };
};

TEST_F(TestSilabsTimeSyncDelegate, SoftwareClockIsNotAnUpstreamSource)
{
    System::Clock::Internal::RAIIMockClock clock;
    constexpr System::Clock::Microseconds64 kUnixTime(1793200000000000);
    ASSERT_EQ(clock.SetClock_RealTime(kUnixTime), CHIP_NO_ERROR);

    bool completionCalled = false;
    Callback::Callback<TimeSync::OnTimeSyncCompletion> callback(
        [](void * callbackContext, TimeSync::TimeSourceEnum, TimeSync::GranularityEnum) {
            *static_cast<bool *>(callbackContext) = true;
        },
        &completionCalled);

    EXPECT_EQ(delegate.UpdateTimeFromPlatformSource(&callback), CHIP_ERROR_NOT_IMPLEMENTED);
    EXPECT_FALSE(completionCalled);
}

TEST_F(TestSilabsTimeSyncDelegate, DirectBasicCluster)
{
    CheckDirectConstruction(BitFlags<TimeSync::Feature>{});
}

TEST_F(TestSilabsTimeSyncDelegate, DirectTimeZoneCluster)
{
    CheckDirectConstruction(BitFlags<TimeSync::Feature>{ TimeSync::Feature::kTimeZone });
}

#if TIME_SYNC_ENABLE_TSC_FEATURE
TEST_F(TestSilabsTimeSyncDelegate, DirectTrustedTimeSourceAndTimeZoneCluster)
{
    CheckDirectConstruction(BitFlags<TimeSync::Feature>{ TimeSync::Feature::kTimeSyncClient, TimeSync::Feature::kTimeZone });
}
#endif

} // namespace
