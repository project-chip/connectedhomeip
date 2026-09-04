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

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/core/TLV.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <oob-accessors/clusters/AmbientContextOOBAccessor.h>
#include <oob-accessors/clusters/BasicInformationOOBAccessor.h>
#include <oob-accessors/clusters/BooleanStateOOBAccessor.h>
#include <oob-accessors/clusters/ElectricalEnergyMeasurementOOBAccessor.h>
#include <oob-accessors/clusters/OccupancyOOBAccessor.h>
#include <oob-accessors/clusters/OnOffOOBAccessor.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>


using namespace chip;
using namespace chip::app;

namespace {

class TestOOBAccessors : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
};

TEST_F(TestOOBAccessors, RegistryLifecycle)
{
    InMemoryOOBAccessorRegistry registry;
    EXPECT_EQ(registry.Size(), 0U);

    EXPECT_EQ(registry.Register(nullptr), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("NonExistentAction"_span, ByteSpan(buffer, writer.GetLengthWritten())),
              CHIP_ERROR_NOT_FOUND);

    registry.Clear();
    EXPECT_EQ(registry.Size(), 0U);
}

TEST_F(TestOOBAccessors, OnOffOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    Clusters::OnOffCluster cluster(1, { .timerDelegate = mTimerDelegate });
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<OnOffOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);
    EXPECT_EQ(registry.Size(), 1U);

    // Initial state is false
    EXPECT_FALSE(cluster.GetOnOff());

    // Encode TLV payload: Tag 1 = Endpoint 1, Tag 2 = true
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), true), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetOnOff"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetOnOff());

    // Send SetOnOff for Endpoint 2 (should return CHIP_ERROR_NOT_FOUND as not handled)
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(2)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), false), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetOnOff"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_ERROR_NOT_FOUND);
    EXPECT_TRUE(cluster.GetOnOff());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestOOBAccessors, OccupancyOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    Clusters::OccupancySensingCluster::Config config(1);
    Clusters::OccupancySensingCluster cluster(config);
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<OccupancyOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    EXPECT_FALSE(cluster.IsOccupied());

    // SetOccupancy = true
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), true), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetOccupancy"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.IsOccupied());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestOOBAccessors, BooleanStateOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    Clusters::BooleanStateCluster cluster(1);
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<BooleanStateOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    EXPECT_FALSE(cluster.GetStateValue());

    // SetBooleanState = true
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), true), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetBooleanState"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetStateValue());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

class FakeAmbientContextDelegate : public Clusters::AmbientContextSensing::AmbientContextSensingDelegate
{
public:
    Clusters::AmbientContextSensing::SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) override
    {
        return mTypeSupportedBuf;
    }
    CHIP_ERROR SetPredictedActivity(const Span<Clusters::AmbientContextSensing::PredictedActivityType> & list) override
    {
        return CHIP_NO_ERROR;
    }
    Clusters::AmbientContextSensing::PredictActivity * GetPredictedActivityBuf() override { return mPredictedActivityBuf; }
    Clusters::AmbientContextSensing::SemanticTagType * GetSensorFusionSupportedBuf(size_t size) override
    {
        return mSensorFusionBuf;
    }
    Clusters::AmbientContextSensing::AmbientContextSensed * AllocDetection() override { return &mSensed; }
    CHIP_ERROR DelDetection(Clusters::AmbientContextSensing::AmbientContextSensed * pitem) override { return CHIP_NO_ERROR; }
    uint64_t GetEpochNow() override { return 1000; }

private:
    Clusters::AmbientContextSensing::SemanticTagType mTypeSupportedBuf[10];
    Clusters::AmbientContextSensing::PredictActivity mPredictedActivityBuf[10];
    Clusters::AmbientContextSensing::SemanticTagType mSensorFusionBuf[10];
    Clusters::AmbientContextSensing::AmbientContextSensed mSensed;
};

TEST_F(TestOOBAccessors, AmbientContextOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    FakeAmbientContextDelegate delegate;
    Clusters::AmbientContextSensingCluster::Config config(mTimerDelegate);
    config.WithFeatures(Clusters::AmbientContextSensing::Feature::kSensorFusion);
    Clusters::AmbientContextSensingCluster cluster(1, config);
    cluster.SetDelegate(&delegate);
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);


    auto accessor = std::make_unique<AmbientContextOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    // SetSensorFusionSupported with empty tag list
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    TLV::TLVType arrayOuter;
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetSensorFusionSupported"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}



} // namespace

