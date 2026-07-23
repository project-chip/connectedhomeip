/*
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

#include <app/clusters/mode-select-server/ModeSelectCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/GeneralDiagnostics/Enums.h>
#include <clusters/ModeSelect/Commands.h>
#include <clusters/ModeSelect/Metadata.h>
#include <platform/DiagnosticDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::app::Clusters::ModeSelect::Attributes;
using namespace chip::Testing;
using Status = Protocols::InteractionModel::Status;

using ModeOptionStructType  = ModeSelect::Structs::ModeOptionStruct::Type;
using SemanticTagStructType = ModeSelect::Structs::SemanticTagStruct::Type;

class TestDiagnosticDataProvider : public DeviceLayer::DiagnosticDataProvider
{
public:
    GeneralDiagnostics::BootReasonEnum mBootReason = GeneralDiagnostics::BootReasonEnum::kUnspecified;

    CHIP_ERROR GetBootReason(GeneralDiagnostics::BootReasonEnum & bootReason) override
    {
        bootReason = mBootReason;
        return CHIP_NO_ERROR;
    }
};

class MockDelegate : public ModeSelectCluster::Delegate
{
public:
    static constexpr ModeOptionStructType kModes[] = {
        { .label = "Normal"_span, .mode = 0, .semanticTags = Span<const SemanticTagStructType>() },
        { .label = "Eco"_span, .mode = 1, .semanticTags = Span<const SemanticTagStructType>() },
    };

    Span<const ModeOptionStructType> GetSupportedModes() const override { return Span<const ModeOptionStructType>(kModes); }
};

constexpr ModeOptionStructType MockDelegate::kModes[];

struct TestModeSelectCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        optionalAttributeSet               = {};
        diagnosticDataProvider.mBootReason = GeneralDiagnostics::BootReasonEnum::kUnspecified;
        testContext.StorageDelegate().ClearStorage();
    }

    ModeSelectCluster::Config MakeConfig(BitMask<Feature> featureMap = {}, bool onOffValueForStartUp = false,
                                         chip::CharSpan description = "TestMode"_span)
    {
        return ModeSelectCluster::Config{
            .featureMap             = featureMap,
            .optionalAttributeSet   = optionalAttributeSet,
            .description            = description,
            .standardNamespace      = DataModel::NullNullable,
            .onOffValueForStartUp   = onOffValueForStartUp,
            .diagnosticDataProvider = diagnosticDataProvider,
        };
    }

    TestServerClusterContext testContext;
    MockDelegate mockDelegate;
    TestDiagnosticDataProvider diagnosticDataProvider;
    ModeSelectCluster::OptionalAttributeSet optionalAttributeSet;
};

// ---- Step 1: Compilation smoke test ----

TEST_F(TestModeSelectCluster, StartupSucceeds)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestModeSelectCluster, StartupInitializesCurrentModeToFirstSupportedMode)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 0u);
}

// ---- Step 2: Attributes() ----

TEST_F(TestModeSelectCluster, AttributeListMandatoryOnly)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Description::kMetadataEntry,
                                            StandardNamespace::kMetadataEntry,
                                            SupportedModes::kMetadataEntry,
                                            CurrentMode::kMetadataEntry,
                                        }));
}

TEST_F(TestModeSelectCluster, AttributeListWithStartUpMode)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Description::kMetadataEntry,
                                            StandardNamespace::kMetadataEntry,
                                            SupportedModes::kMetadataEntry,
                                            CurrentMode::kMetadataEntry,
                                            StartUpMode::kMetadataEntry,
                                        }));
}

TEST_F(TestModeSelectCluster, AttributeListWithOnOffFeature)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Description::kMetadataEntry,
                                            StandardNamespace::kMetadataEntry,
                                            SupportedModes::kMetadataEntry,
                                            CurrentMode::kMetadataEntry,
                                            OnMode::kMetadataEntry,
                                        }));
}

// ---- Step 3: AcceptedCommands() ----

TEST_F(TestModeSelectCluster, AcceptedCommandsContainsChangeToMode)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { Commands::ChangeToMode::kMetadataEntry }));
}

// ---- Step 4: ReadAttribute ----

TEST_F(TestModeSelectCluster, ReadClusterRevision)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t revision = 0;
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, kRevision);
}

TEST_F(TestModeSelectCluster, ReadFeatureMap)
{
    BitMask<Feature> featureMap(Feature::kOnOff);
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(featureMap));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t readFeatureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, readFeatureMap), CHIP_NO_ERROR);
    EXPECT_EQ(readFeatureMap, featureMap.Raw());
}

TEST_F(TestModeSelectCluster, ReadDescription)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig({}, false, "Coffee"_span));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::CharSpan description;
    ASSERT_EQ(tester.ReadAttribute(Description::Id, description), CHIP_NO_ERROR);
    EXPECT_TRUE(description.data_equal("Coffee"_span));
}

TEST_F(TestModeSelectCluster, ReadStandardNamespaceDefaultNull)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> standardNamespace;
    ASSERT_EQ(tester.ReadAttribute(StandardNamespace::Id, standardNamespace), CHIP_NO_ERROR);
    EXPECT_TRUE(standardNamespace.IsNull());
}

TEST_F(TestModeSelectCluster, ReadCurrentMode)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 0u);
}

TEST_F(TestModeSelectCluster, ReadStartUpModeDefaultNull)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> startUpMode;
    ASSERT_EQ(tester.ReadAttribute(StartUpMode::Id, startUpMode), CHIP_NO_ERROR);
    EXPECT_TRUE(startUpMode.IsNull());
}

TEST_F(TestModeSelectCluster, ReadOnModeDefaultNull)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> onMode;
    ASSERT_EQ(tester.ReadAttribute(OnMode::Id, onMode), CHIP_NO_ERROR);
    EXPECT_TRUE(onMode.IsNull());
}

TEST_F(TestModeSelectCluster, ReadUnsupportedAttributeReturnsError)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t unused = 0;
    EXPECT_FALSE(tester.ReadAttribute(0xFFFF, unused).IsSuccess());
}

// ---- Step 5: WriteAttribute ----

TEST_F(TestModeSelectCluster, WriteStartUpModeValidValue)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> newStartUpMode(1);
    ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, newStartUpMode).IsSuccess());

    DataModel::Nullable<uint8_t> readBack;
    ASSERT_EQ(tester.ReadAttribute(StartUpMode::Id, readBack), CHIP_NO_ERROR);
    EXPECT_EQ(readBack.Value(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(StartUpMode::Id));
}

TEST_F(TestModeSelectCluster, WriteStartUpModeInvalidValueReturnsConstraintError)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> invalidMode(99);
    EXPECT_EQ(tester.WriteAttribute(StartUpMode::Id, invalidMode), Status::ConstraintError);
}

TEST_F(TestModeSelectCluster, WriteOnModeValidValue)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> newOnMode(1);
    ASSERT_TRUE(tester.WriteAttribute(OnMode::Id, newOnMode).IsSuccess());

    DataModel::Nullable<uint8_t> readBack;
    ASSERT_EQ(tester.ReadAttribute(OnMode::Id, readBack), CHIP_NO_ERROR);
    EXPECT_EQ(readBack.Value(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(OnMode::Id));
}

TEST_F(TestModeSelectCluster, WriteOnModeNullIsValid)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> nullMode;
    ASSERT_TRUE(tester.WriteAttribute(OnMode::Id, nullMode).IsSuccess());
}

// ---- Step 6: ChangeToMode command ----

TEST_F(TestModeSelectCluster, ChangeToModeValidMode)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ChangeToMode::Type request;
    request.newMode = 1;
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 1u);
}

TEST_F(TestModeSelectCluster, ChangeToModeInvalidModeReturnsInvalidCommand)
{
    ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ChangeToMode::Type request;
    request.newMode = 99;
    EXPECT_FALSE(tester.Invoke(request).IsSuccess());
}

// ---- Step 7: Startup logic ----

TEST_F(TestModeSelectCluster, StartupAppliesStartUpMode)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    {
        ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
        ClusterTester tester(cluster);
        // Use fixture's testContext so persistence survives across cluster instances.
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, DataModel::Nullable<uint8_t>(1)).IsSuccess());
    }

    // Re-create cluster to simulate power cycle — same testContext storage.
    ModeSelectCluster cluster2(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester2(cluster2);
    ASSERT_EQ(cluster2.Startup(testContext.Get()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester2.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 1u);
}

TEST_F(TestModeSelectCluster, StartupSkipsStartUpModeOnOtaReboot)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    {
        ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig());
        ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, DataModel::Nullable<uint8_t>(1)).IsSuccess());
    }

    diagnosticDataProvider.mBootReason = GeneralDiagnostics::BootReasonEnum::kSoftwareUpdateCompleted;

    ModeSelectCluster cluster2(kRootEndpointId, mockDelegate, MakeConfig());
    ClusterTester tester2(cluster2);
    ASSERT_EQ(cluster2.Startup(testContext.Get()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester2.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    // StartUpMode (1) is skipped on OTA reboot; CurrentMode stays at default (0).
    EXPECT_EQ(currentMode, 0u);
}

TEST_F(TestModeSelectCluster, StartupAppliesOnModeOverStartUpMode)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    {
        ModeSelectCluster cluster(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff), true));
        ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, DataModel::Nullable<uint8_t>(0)).IsSuccess());
        ASSERT_TRUE(tester.WriteAttribute(OnMode::Id, DataModel::Nullable<uint8_t>(1)).IsSuccess());
    }

    // Re-create cluster to simulate power cycle — same testContext storage.
    ModeSelectCluster cluster2(kRootEndpointId, mockDelegate, MakeConfig(BitMask<Feature>(Feature::kOnOff), true));
    ClusterTester tester2(cluster2);
    ASSERT_EQ(cluster2.Startup(testContext.Get()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester2.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    // OnMode (1) wins over StartUpMode (0).
    EXPECT_EQ(currentMode, 1u);
}

} // namespace
