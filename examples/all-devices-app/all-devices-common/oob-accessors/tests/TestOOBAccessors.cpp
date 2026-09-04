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
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class TestOOBAccessors : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EXPECT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    void TearDown() override
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }

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
    Clusters::OccupancySensing::Structs::HoldTimeLimitsStruct::Type limits{ .holdTimeMin = 1, .holdTimeMax = 100, .holdTimeDefault = 30 };
    config.WithHoldTime(30, limits, mTimerDelegate);
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

    // SetHoldTime = 30
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint16_t>(30)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetHoldTime"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

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
    config.WithFeatures(BitFlags<Clusters::AmbientContextSensing::Feature>(
        Clusters::AmbientContextSensing::Feature::kHumanActivity,
        Clusters::AmbientContextSensing::Feature::kObjectIdentification,
        Clusters::AmbientContextSensing::Feature::kSoundIdentification,
        Clusters::AmbientContextSensing::Feature::kObjectCounting,
        Clusters::AmbientContextSensing::Feature::kPredictedActivity,
        Clusters::AmbientContextSensing::Feature::kSensorFusion
    ));
    Clusters::AmbientContextSensingCluster cluster(1, config);
    cluster.SetDelegate(&delegate);
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<AmbientContextOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    uint8_t buffer[256];
    TLV::TLVWriter writer;
    TLV::TLVType outer;
    TLV::TLVType arrayOuter;
    TLV::TLVType structOuter;

    // 1. SetSensorFusionSupported with empty tag list
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetSensorFusionSupported"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    // 2. SetAmbientContextSupport: Namespace 0x4B (HumanActivity), Tag 1 (Fall)
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint8_t>(0x4B)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetAmbientContextSupport"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    // 3. AddAmbientContextDetect: Namespace 0x4B, Tag 1, Confidence 80
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint8_t>(0x4B)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(3), static_cast<uint8_t>(80)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("AddAmbientContextDetect"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    // 4. SetPredictedActivity: Namespace 0x4B, Tag 1
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint32_t>(1000)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint32_t>(2000)), CHIP_NO_ERROR);
    TLV::TLVType tagArrayOuter;
    EXPECT_EQ(writer.StartContainer(TLV::ContextTag(3), TLV::kTLVType_Array, tagArrayOuter), CHIP_NO_ERROR);
    TLV::TLVType tagStructOuter;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, tagStructOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint8_t>(0x4B)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(tagStructOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(tagArrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(4), false), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(5), static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(6), static_cast<uint8_t>(90)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(structOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayOuter), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetPredictedActivity"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    // 5. SetObjCount
    writer.Init(buffer);
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(2), static_cast<uint16_t>(5)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("SetObjCount"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

class FakeElectricalEnergyDelegate : public Clusters::ElectricalEnergyMeasurement::Delegate
{
public:
    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override { return DataModel::MakeNullable(static_cast<int64_t>(100)); }
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override { return DataModel::Nullable<int64_t>(); }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override { return DataModel::Nullable<int64_t>(); }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override { return DataModel::Nullable<int64_t>(); }
};

TEST_F(TestOOBAccessors, ElectricalEnergyMeasurementOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    FakeElectricalEnergyDelegate energyDelegate;
    Clusters::ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type accuracy;
    Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster::Config config{
        .endpointId         = 1,
        .featureFlags       = BitFlags<Clusters::ElectricalEnergyMeasurement::Feature>(Clusters::ElectricalEnergyMeasurement::Feature::kCumulativeEnergy),
        .optionalAttributes = {},
        .accuracyStruct     = accuracy,
        .delegate           = energyDelegate,
        .timerDelegate      = mTimerDelegate,
    };
    Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster cluster(config);
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<ElectricalEnergyMeasurementOOBAccessor>(cluster, 1);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("GenerateElectricalEnergyMeasurementSnapshots"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

class FakeDeviceInstanceInfoProvider : public DeviceLayer::DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override { vendorId = 0xFFF1; return CHIP_NO_ERROR; }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetProductId(uint16_t & productId) override { productId = 0x8000; return CHIP_NO_ERROR; }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override { hardwareVersion = 1; return CHIP_NO_ERROR; }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_NO_ERROR; }
};

TEST_F(TestOOBAccessors, BasicInformationOOBAccessor)
{
    InMemoryOOBAccessorRegistry registry;
    FakeDeviceInstanceInfoProvider fakeDeviceInfo;
    EXPECT_EQ(DeviceLayer::ConfigurationMgr().StoreConfigurationVersion(1), CHIP_NO_ERROR);
    Clusters::BasicInformationCluster cluster(
        Clusters::BasicInformationOptionalAttributesSet{},
        fakeDeviceInfo,
        DeviceLayer::ConfigurationMgr(),
        DeviceLayer::PlatformMgr(),
        static_cast<uint16_t>(10)
    );
    EXPECT_EQ(cluster.Startup(mClusterContext.Get()), CHIP_NO_ERROR);

    auto accessor = std::make_unique<BasicInformationOOBAccessor>(cluster, 0);
    EXPECT_EQ(registry.Register(std::move(accessor)), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outer;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(0)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    EXPECT_EQ(registry.HandleAction("IncreaseConfigurationVersion"_span, ByteSpan(buffer, writer.GetLengthWritten())), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace

