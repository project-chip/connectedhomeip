/*
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

#include <app/clusters/alarm-base-server/AlarmBaseCluster.h>
#include <app/clusters/alarm-base-server/Delegate.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DishwasherAlarm/Enums.h>
#include <clusters/DishwasherAlarm/Metadata.h>
#include <clusters/RefrigeratorAlarm/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AlarmBase;
using namespace chip::app::Clusters::AlarmBase::Attributes;
using namespace chip::Testing;
using Status            = Protocols::InteractionModel::Status;
using ClusterStatusCode = Protocols::InteractionModel::ClusterStatusCode;

constexpr ClusterId kRefrigeratorClusterId = RefrigeratorAlarm::Id;

class TestAlarmDelegate : public AlarmBase::Delegate
{
public:
    bool ModifyEnabledAlarms(AlarmMap mask) override
    {
        mLastModifyMask = mask;
        return mAllowModify;
    }

    bool ResetAlarms(AlarmMap alarms) override
    {
        mLastResetAlarms = alarms;
        return mAllowReset;
    }

    AlarmMap mLastModifyMask{};
    AlarmMap mLastResetAlarms{};
    bool mAllowModify = true;
    bool mAllowReset  = true;
};

class TestDishwasherAlarmCluster : public AlarmBaseCluster
{
public:
    TestDishwasherAlarmCluster(EndpointId endpointId, const AlarmBaseCluster::Config & config) :
        AlarmBaseCluster(endpointId, { DishwasherAlarm::Id, DishwasherAlarm::kRevision }, config)
    {}

protected:
    void SendNotifyEvent(AlarmMap, AlarmMap, AlarmMap, AlarmMap) override {}
};

class TestRefrigeratorAlarmCluster : public AlarmBaseCluster
{
public:
    TestRefrigeratorAlarmCluster(EndpointId endpointId, const AlarmBaseCluster::Config & config) :
        AlarmBaseCluster(endpointId, { RefrigeratorAlarm::Id, RefrigeratorAlarm::kRevision }, config)
    {}

protected:
    void SendNotifyEvent(AlarmMap, AlarmMap, AlarmMap, AlarmMap) override {}
};

struct TestAlarmBaseCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    AlarmBaseCluster::Config MakeDishwasherConfig(bool withReset = true, bool withModifyCommand = true)
    {
        BitMask<Feature> feature;
        if (withReset)
        {
            feature.Set(Feature::kReset);
        }

        return {
            .delegate                    = delegate,
            .feature                     = feature,
            .supported                   = AlarmMap(0x3F),
            .latch                       = AlarmMap(0),
            .supportsModifyEnabledAlarms = withModifyCommand,
        };
    }

    AlarmBaseCluster::Config MakeRefrigeratorConfig()
    {
        return {
            .delegate  = defaultDelegate,
            .supported = AlarmMap(0x1),
        };
    }

    TestServerClusterContext testContext;
    TestAlarmDelegate delegate;
    AlarmBase::Delegate defaultDelegate;
};

TEST_F(TestAlarmBaseCluster, DishwasherAttributeListWithResetFeature)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Mask::kMetadataEntry,
                                            State::kMetadataEntry,
                                            Supported::kMetadataEntry,
                                            Latch::kMetadataEntry,
                                        }));
}

TEST_F(TestAlarmBaseCluster, RefrigeratorAttributeListWithoutResetFeature)
{
    TestRefrigeratorAlarmCluster cluster(kRootEndpointId, MakeRefrigeratorConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Mask::kMetadataEntry,
                                            State::kMetadataEntry,
                                            Supported::kMetadataEntry,
                                        }));
}

TEST_F(TestAlarmBaseCluster, ReadInitialAttributes)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t revision = 0;
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, DishwasherAlarm::kRevision);

    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, to_underlying(Feature::kReset));

    AlarmMap supported{};
    ASSERT_EQ(tester.ReadAttribute(Supported::Id, supported), CHIP_NO_ERROR);
    EXPECT_EQ(supported.Raw(), 0x3Fu);

    AlarmMap mask{};
    ASSERT_EQ(tester.ReadAttribute(Mask::Id, mask), CHIP_NO_ERROR);
    EXPECT_EQ(mask.Raw(), 0u);
}

TEST_F(TestAlarmBaseCluster, SetStateValueHonorsSupportedAndMask)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMask(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.SetState(AlarmMap(0x1)), Status::Success);

    EXPECT_EQ(cluster.GetState().Raw(), 0x1u);

    EXPECT_EQ(cluster.SetState(AlarmMap(0x4)), Status::Failure);

    EXPECT_EQ(cluster.SetMask(AlarmMap(0x1)), Status::Success);
    EXPECT_EQ(cluster.SetState(AlarmMap(0x3)), Status::Failure);
}

TEST_F(TestAlarmBaseCluster, SetStateValueHonorsLatchUnlessIgnored)
{
    AlarmBaseCluster::Config config = MakeDishwasherConfig();
    config.latch                    = AlarmMap(0x1);
    TestDishwasherAlarmCluster cluster(kRootEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMask(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.SetState(AlarmMap(0x1)), Status::Success);

    EXPECT_EQ(cluster.SetState(AlarmMap(0)), Status::Success);
    EXPECT_EQ(cluster.GetState().Raw(), 0x1u);

    EXPECT_EQ(cluster.SetState(AlarmMap(0x2)), Status::Success);
    EXPECT_EQ(cluster.GetState().Raw(), 0x3u);

    EXPECT_EQ(cluster.ResetLatchedAlarms(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.GetState().Raw(), 0u);
}

TEST_F(TestAlarmBaseCluster, ResetLatchedAlarmsClearsStateBits)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMask(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.SetState(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.ResetLatchedAlarms(AlarmMap(0x2)), Status::Success);

    EXPECT_EQ(cluster.GetState().Raw(), 0x1u);
}

TEST_F(TestAlarmBaseCluster, AcceptedCommandsWithResetAndModify)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::Reset::kMetadataEntry,
                                                  Commands::ModifyEnabledAlarms::kMetadataEntry,
                                              }));
}

TEST_F(TestAlarmBaseCluster, RefrigeratorHasNoCommands)
{
    TestRefrigeratorAlarmCluster cluster(kRootEndpointId, MakeRefrigeratorConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, kRefrigeratorClusterId), commands), CHIP_NO_ERROR);
    EXPECT_EQ(commands.Size(), 0u);
}

TEST_F(TestAlarmBaseCluster, ModifyEnabledAlarmsCommandUpdatesMask)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ModifyEnabledAlarms::Type command{ .mask = BitMask<DishwasherAlarm::AlarmBitmap>(0x3) };
    EXPECT_TRUE(tester.Invoke(command).IsSuccess());
    EXPECT_EQ(delegate.mLastModifyMask.Raw(), 0x3u);

    EXPECT_EQ(cluster.GetMask().Raw(), 0x3u);
}

TEST_F(TestAlarmBaseCluster, ResetCommandClearsRequestedAlarms)
{
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMask(AlarmMap(0x3)), Status::Success);
    EXPECT_EQ(cluster.SetState(AlarmMap(0x3)), Status::Success);

    Commands::Reset::Type command{ .alarms = BitMask<DishwasherAlarm::AlarmBitmap>(0x2) };
    EXPECT_TRUE(tester.Invoke(command).IsSuccess());
    EXPECT_EQ(delegate.mLastResetAlarms.Raw(), 0x2u);

    EXPECT_EQ(cluster.GetState().Raw(), 0x1u);
}

TEST_F(TestAlarmBaseCluster, DelegateCanRejectModifyEnabledAlarms)
{
    delegate.mAllowModify = false;
    TestDishwasherAlarmCluster cluster(kRootEndpointId, MakeDishwasherConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ModifyEnabledAlarms::Type command{ .mask = BitMask<DishwasherAlarm::AlarmBitmap>(0x1) };
    auto result = tester.Invoke(command);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Failure));
}

} // namespace
