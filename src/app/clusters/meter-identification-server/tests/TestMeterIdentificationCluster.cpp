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

#include <app/clusters/meter-identification-server/MeterIdentificationCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/MeterIdentification/Attributes.h>
#include <clusters/MeterIdentification/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;
using namespace chip::Testing;

struct TestMeterIdentificationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

} // namespace

TEST_F(TestMeterIdentificationCluster, AttributeListTest)
{
    // No optional attributes, no features
    {
        MeterIdentificationCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, MeterIdentification::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With ProtocolVersion optional attribute
    {
        const DataModel::AttributeEntry optionalAttributes[] = { ProtocolVersion::kMetadataEntry,
                                                                 PowerThreshold::kMetadataEntry };
        MeterIdentificationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<ProtocolVersion::Id>();

        MeterIdentificationCluster cluster(kRootEndpointId,
                                           MeterIdentificationCluster::Config{}.WithProtocolVersion());
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, MeterIdentification::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With PowerThreshold feature
    {
        const DataModel::AttributeEntry optionalAttributes[] = { ProtocolVersion::kMetadataEntry,
                                                                 PowerThreshold::kMetadataEntry };
        MeterIdentificationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<PowerThreshold::Id>();

        MeterIdentificationCluster cluster(kRootEndpointId,
                                           MeterIdentificationCluster::Config{}.WithFeature(Feature::kPowerThreshold));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, MeterIdentification::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With both ProtocolVersion and PowerThreshold
    {
        const DataModel::AttributeEntry optionalAttributes[] = { ProtocolVersion::kMetadataEntry,
                                                                 PowerThreshold::kMetadataEntry };
        MeterIdentificationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<ProtocolVersion::Id>();
        optionalAttributeSet.Set<PowerThreshold::Id>();

        MeterIdentificationCluster cluster(
            kRootEndpointId,
            MeterIdentificationCluster::Config{}.WithProtocolVersion().WithFeature(Feature::kPowerThreshold));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, MeterIdentification::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestMeterIdentificationCluster, ReadMandatoryAttributesDefaultNull)
{
    MeterIdentificationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, MeterIdentification::kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    DataModel::Nullable<MeterTypeEnum> meterType;
    ASSERT_EQ(tester.ReadAttribute(MeterType::Id, meterType), CHIP_NO_ERROR);
    EXPECT_TRUE(meterType.IsNull());

    DataModel::Nullable<CharSpan> pointOfDelivery;
    ASSERT_EQ(tester.ReadAttribute(PointOfDelivery::Id, pointOfDelivery), CHIP_NO_ERROR);
    EXPECT_TRUE(pointOfDelivery.IsNull());

    DataModel::Nullable<CharSpan> meterSerialNumber;
    ASSERT_EQ(tester.ReadAttribute(MeterSerialNumber::Id, meterSerialNumber), CHIP_NO_ERROR);
    EXPECT_TRUE(meterSerialNumber.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMeterIdentificationCluster, SetMeterType)
{
    MeterIdentificationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    testContext.ChangeListener().DirtyList().clear();

    // Valid values
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kUtility)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    DataModel::Nullable<MeterTypeEnum> readVal;
    ASSERT_EQ(tester.ReadAttribute(MeterType::Id, readVal), CHIP_NO_ERROR);
    ASSERT_FALSE(readVal.IsNull());
    EXPECT_EQ(readVal.Value(), MeterTypeEnum::kUtility);

    // Setting same value -> no dirty notification
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kUtility)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 0u);

    // Private
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kPrivate)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    // Generic
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kGeneric)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    // Set back to null
    EXPECT_EQ(cluster.SetMeterType(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    // Invalid enum values -> ConstraintError
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(MeterTypeEnum::kUnknownEnumValue)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetMeterType(DataModel::MakeNullable(static_cast<MeterTypeEnum>(10))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMeterIdentificationCluster, SetPointOfDelivery)
{
    MeterIdentificationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    testContext.ChangeListener().DirtyList().clear();

    // Normal string
    EXPECT_EQ(cluster.SetPointOfDelivery(DataModel::MakeNullable("POD-12345"_span)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    DataModel::Nullable<CharSpan> readVal;
    ASSERT_EQ(tester.ReadAttribute(PointOfDelivery::Id, readVal), CHIP_NO_ERROR);
    ASSERT_FALSE(readVal.IsNull());
    EXPECT_TRUE(readVal.Value().data_equal("POD-12345"_span));

    // Same string -> no dirty notification
    EXPECT_EQ(cluster.SetPointOfDelivery(DataModel::MakeNullable("POD-12345"_span)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 0u);

    // Exactly 64 bytes string -> valid, and every byte must be preserved (no truncation from the
    // internal null-terminator handling).
    char buffer64[64];
    memset(buffer64, 'A', sizeof(buffer64));
    EXPECT_EQ(cluster.SetPointOfDelivery(DataModel::MakeNullable(CharSpan(buffer64, sizeof(buffer64)))), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    ASSERT_EQ(tester.ReadAttribute(PointOfDelivery::Id, readVal), CHIP_NO_ERROR);
    ASSERT_FALSE(readVal.IsNull());
    EXPECT_TRUE(readVal.Value().data_equal(CharSpan(buffer64, sizeof(buffer64))));

    // 65 bytes string -> ConstraintError
    char buffer65[65];
    memset(buffer65, 'B', sizeof(buffer65));
    EXPECT_EQ(cluster.SetPointOfDelivery(DataModel::MakeNullable(CharSpan(buffer65, sizeof(buffer65)))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Null string -> valid
    EXPECT_EQ(cluster.SetPointOfDelivery(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMeterIdentificationCluster, SetMeterSerialNumber)
{
    MeterIdentificationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    testContext.ChangeListener().DirtyList().clear();

    EXPECT_EQ(cluster.SetMeterSerialNumber(DataModel::MakeNullable("SN-987654321"_span)), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    DataModel::Nullable<CharSpan> readVal;
    ASSERT_EQ(tester.ReadAttribute(MeterSerialNumber::Id, readVal), CHIP_NO_ERROR);
    ASSERT_FALSE(readVal.IsNull());
    EXPECT_TRUE(readVal.Value().data_equal("SN-987654321"_span));

    // Exactly 64 bytes string -> valid, and every byte must be preserved (no truncation from the
    // internal null-terminator handling).
    char buffer64[64];
    memset(buffer64, 'S', sizeof(buffer64));
    EXPECT_EQ(cluster.SetMeterSerialNumber(DataModel::MakeNullable(CharSpan(buffer64, sizeof(buffer64)))), CHIP_NO_ERROR);
    EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
    testContext.ChangeListener().DirtyList().clear();

    ASSERT_EQ(tester.ReadAttribute(MeterSerialNumber::Id, readVal), CHIP_NO_ERROR);
    ASSERT_FALSE(readVal.IsNull());
    EXPECT_TRUE(readVal.Value().data_equal(CharSpan(buffer64, sizeof(buffer64))));

    // Over 64 chars -> ConstraintError
    char buffer65[65];
    memset(buffer65, 'S', sizeof(buffer65));
    EXPECT_EQ(cluster.SetMeterSerialNumber(DataModel::MakeNullable(CharSpan(buffer65, sizeof(buffer65)))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestMeterIdentificationCluster, SetProtocolVersion)
{
    // Without ProtocolVersion enabled
    {
        MeterIdentificationCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.SetProtocolVersion(DataModel::MakeNullable("1.0"_span)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With ProtocolVersion enabled
    {
        MeterIdentificationCluster cluster(kRootEndpointId,
                                           MeterIdentificationCluster::Config{}.WithProtocolVersion());
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        testContext.ChangeListener().DirtyList().clear();

        EXPECT_EQ(cluster.SetProtocolVersion(DataModel::MakeNullable("v1.2.3"_span)), CHIP_NO_ERROR);
        EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
        testContext.ChangeListener().DirtyList().clear();

        DataModel::Nullable<CharSpan> readVal;
        ASSERT_EQ(tester.ReadAttribute(ProtocolVersion::Id, readVal), CHIP_NO_ERROR);
        ASSERT_FALSE(readVal.IsNull());
        EXPECT_TRUE(readVal.Value().data_equal("v1.2.3"_span));

        // Exactly 64 bytes string -> valid, and every byte must be preserved (no truncation from
        // the internal null-terminator handling).
        char buffer64[64];
        memset(buffer64, 'V', sizeof(buffer64));
        EXPECT_EQ(cluster.SetProtocolVersion(DataModel::MakeNullable(CharSpan(buffer64, sizeof(buffer64)))), CHIP_NO_ERROR);
        EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
        testContext.ChangeListener().DirtyList().clear();

        ASSERT_EQ(tester.ReadAttribute(ProtocolVersion::Id, readVal), CHIP_NO_ERROR);
        ASSERT_FALSE(readVal.IsNull());
        EXPECT_TRUE(readVal.Value().data_equal(CharSpan(buffer64, sizeof(buffer64))));

        // Over 64 characters -> ConstraintError
        char buffer65[65];
        memset(buffer65, 'V', sizeof(buffer65));
        EXPECT_EQ(cluster.SetProtocolVersion(DataModel::MakeNullable(CharSpan(buffer65, sizeof(buffer65)))),
                  CHIP_IM_GLOBAL_STATUS(ConstraintError));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestMeterIdentificationCluster, SetPowerThreshold)
{
    // Without PWRTHLD feature
    {
        MeterIdentificationCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        Globals::Structs::PowerThresholdStruct::Type threshold;
        EXPECT_EQ(cluster.SetPowerThreshold(DataModel::MakeNullable(threshold)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With PWRTHLD feature
    {
        MeterIdentificationCluster cluster(kRootEndpointId,
                                           MeterIdentificationCluster::Config{}.WithFeature(Feature::kPowerThreshold));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        testContext.ChangeListener().DirtyList().clear();

        Globals::Structs::PowerThresholdStruct::Type threshold;
        threshold.powerThreshold.SetValue(1000);
        threshold.apparentPowerThreshold.SetValue(1200);
        threshold.powerThresholdSource.SetNonNull(Globals::PowerThresholdSourceEnum::kContract);

        EXPECT_EQ(cluster.SetPowerThreshold(DataModel::MakeNullable(threshold)), CHIP_NO_ERROR);
        EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
        testContext.ChangeListener().DirtyList().clear();

        DataModel::Nullable<Globals::Structs::PowerThresholdStruct::DecodableType> readVal;
        ASSERT_EQ(tester.ReadAttribute(PowerThreshold::Id, readVal), CHIP_NO_ERROR);
        ASSERT_FALSE(readVal.IsNull());
        EXPECT_TRUE(readVal.Value().powerThreshold.HasValue());
        EXPECT_EQ(readVal.Value().powerThreshold.Value(), 1000);
        EXPECT_TRUE(readVal.Value().apparentPowerThreshold.HasValue());
        EXPECT_EQ(readVal.Value().apparentPowerThreshold.Value(), 1200);
        ASSERT_FALSE(readVal.Value().powerThresholdSource.IsNull());
        EXPECT_EQ(readVal.Value().powerThresholdSource.Value(), Globals::PowerThresholdSourceEnum::kContract);

        // Setting same struct -> no dirty notification
        EXPECT_EQ(cluster.SetPowerThreshold(DataModel::MakeNullable(threshold)), CHIP_NO_ERROR);
        EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 0u);

        // Invalid powerThresholdSource -> ConstraintError
        Globals::Structs::PowerThresholdStruct::Type invalidThreshold;
        invalidThreshold.powerThresholdSource.SetNonNull(Globals::PowerThresholdSourceEnum::kUnknownEnumValue);
        EXPECT_EQ(cluster.SetPowerThreshold(DataModel::MakeNullable(invalidThreshold)),
                  CHIP_IM_GLOBAL_STATUS(ConstraintError));

        // Set to null
        EXPECT_EQ(cluster.SetPowerThreshold(DataModel::NullNullable), CHIP_NO_ERROR);
        EXPECT_EQ(testContext.ChangeListener().DirtyList().size(), 1u);
        testContext.ChangeListener().DirtyList().clear();

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestMeterIdentificationCluster, ConfigInitialization)
{
    MeterIdentificationCluster::Config config;
    config.WithFeature(Feature::kPowerThreshold);
    config.WithProtocolVersion();
    config.WithMeterType(DataModel::MakeNullable(MeterTypeEnum::kPrivate));
    config.WithPointOfDelivery(DataModel::MakeNullable("POD-INITIAL"_span));
    config.WithMeterSerialNumber(DataModel::MakeNullable("SN-INITIAL"_span));
    config.WithProtocolVersion(DataModel::MakeNullable("v0.1"_span));

    Globals::Structs::PowerThresholdStruct::Type threshold;
    threshold.powerThreshold.SetValue(5000);
    config.mPowerThreshold = DataModel::MakeNullable(threshold);

    MeterIdentificationCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_TRUE(cluster.HasFeature(Feature::kPowerThreshold));

    DataModel::Nullable<MeterTypeEnum> readMeterType;
    ASSERT_EQ(tester.ReadAttribute(MeterType::Id, readMeterType), CHIP_NO_ERROR);
    ASSERT_FALSE(readMeterType.IsNull());
    EXPECT_EQ(readMeterType.Value(), MeterTypeEnum::kPrivate);

    DataModel::Nullable<CharSpan> readPod;
    ASSERT_EQ(tester.ReadAttribute(PointOfDelivery::Id, readPod), CHIP_NO_ERROR);
    ASSERT_FALSE(readPod.IsNull());
    EXPECT_TRUE(readPod.Value().data_equal("POD-INITIAL"_span));

    DataModel::Nullable<CharSpan> readSn;
    ASSERT_EQ(tester.ReadAttribute(MeterSerialNumber::Id, readSn), CHIP_NO_ERROR);
    ASSERT_FALSE(readSn.IsNull());
    EXPECT_TRUE(readSn.Value().data_equal("SN-INITIAL"_span));

    DataModel::Nullable<CharSpan> readPv;
    ASSERT_EQ(tester.ReadAttribute(ProtocolVersion::Id, readPv), CHIP_NO_ERROR);
    ASSERT_FALSE(readPv.IsNull());
    EXPECT_TRUE(readPv.Value().data_equal("v0.1"_span));

    DataModel::Nullable<Globals::Structs::PowerThresholdStruct::DecodableType> readPt;
    ASSERT_EQ(tester.ReadAttribute(PowerThreshold::Id, readPt), CHIP_NO_ERROR);
    ASSERT_FALSE(readPt.IsNull());
    EXPECT_TRUE(readPt.Value().powerThreshold.HasValue());
    EXPECT_EQ(readPt.Value().powerThreshold.Value(), 5000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
