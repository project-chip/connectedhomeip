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

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DishwasherMode/Metadata.h>
#include <clusters/GeneralDiagnostics/Enums.h>
#include <platform/DiagnosticDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::ModeBase::Attributes;
using namespace chip::Testing;
using Status = Protocols::InteractionModel::Status;

using ModeTagStructType    = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using ModeOptionStructType = chip::app::Clusters::detail::Structs::ModeOptionStruct::DecodableType;

constexpr ClusterId kTestClusterId = DishwasherMode::Id;

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

class TestAppDelegate : public ModeBase::AppDelegate
{
public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override
    {
        static constexpr CharSpan kLabels[] = { "Normal"_span, "Eco"_span };
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return CopyCharSpanToMutableCharSpan(kLabels[modeIndex], label);
    }

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override
    {
        static constexpr uint8_t kValues[] = { 0, 1 };
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        value = kValues[modeIndex];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & modeTags) override
    {
        static constexpr uint16_t kTagValues[] = { to_underlying(ModeTag::kAuto), to_underlying(ModeTag::kLowEnergy) };
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

        modeTags[0].value = kTagValues[modeIndex];
        modeTags.reduce_size(1);
        return CHIP_NO_ERROR;
    }

    void HandleChangeToMode(uint8_t newMode, Commands::ChangeToModeResponse::Type & response) override
    {
        if (mRejectNextChange)
        {
            response.status     = to_underlying(StatusCode::kGenericFailure);
            mRejectNextChange   = false;
            mLastHandledNewMode = newMode;
            return;
        }
        response.status     = to_underlying(StatusCode::kSuccess);
        mLastHandledNewMode = newMode;
    }

    bool mRejectNextChange      = false;
    uint8_t mLastHandledNewMode = 0xFF;
};

struct TestModeBaseCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        optionalAttributeSet               = {};
        appDelegate.mRejectNextChange      = false;
        appDelegate.mLastHandledNewMode    = 0xFF;
        diagnosticDataProvider.mBootReason = GeneralDiagnostics::BootReasonEnum::kUnspecified;
        testContext.StorageDelegate().ClearStorage();
        ASSERT_EQ(persistenceProvider.Init(&testContext.StorageDelegate()), CHIP_NO_ERROR);
    }

    ModeBaseCluster::Config MakeConfig(BitMask<Feature> feature = {}, bool onOffValueForStartUp = false)
    {
        return ModeBaseCluster::Config{
            .feature                          = feature,
            .optionalAttributeSet             = optionalAttributeSet,
            .appDelegate                      = appDelegate,
            .onOffValueForStartUp             = onOffValueForStartUp,
            .safeAttributePersistenceProvider = persistenceProvider,
            .diagnosticDataProvider           = diagnosticDataProvider,
            .clusterRevision                  = DishwasherMode::kRevision,
        };
    }

    TestServerClusterContext testContext;
    DefaultSafeAttributePersistenceProvider persistenceProvider;
    TestAppDelegate appDelegate;
    TestDiagnosticDataProvider diagnosticDataProvider;
    ModeBaseCluster::OptionalAttributeSet optionalAttributeSet;
};

TEST_F(TestModeBaseCluster, StartupInitializesCurrentModeToFirstSupportedMode)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 0u);
}

TEST_F(TestModeBaseCluster, AttributeListMandatoryOnly)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            SupportedModes::kMetadataEntry,
                                            CurrentMode::kMetadataEntry,
                                        }));
}

TEST_F(TestModeBaseCluster, AttributeListWithOptionalAttributes)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            SupportedModes::kMetadataEntry,
                                            CurrentMode::kMetadataEntry,
                                            StartUpMode::kMetadataEntry,
                                            OnMode::kMetadataEntry,
                                        }));
}

TEST_F(TestModeBaseCluster, AcceptedAndGeneratedCommands)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { Commands::ChangeToMode::kMetadataEntry }));
    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::ChangeToModeResponse::Id }));
}

TEST_F(TestModeBaseCluster, ReadMandatoryAttributes)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t revision = 0;
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, DishwasherMode::kRevision);

    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 0u);

    Attributes::SupportedModes::TypeInfo::DecodableType supportedModes;
    ASSERT_EQ(tester.ReadAttribute(SupportedModes::Id, supportedModes), CHIP_NO_ERROR);
    auto it = supportedModes.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_TRUE(it.GetValue().label.data_equal("Normal"_span));
    EXPECT_EQ(it.GetValue().mode, 0u);
    ASSERT_TRUE(it.Next());
    EXPECT_TRUE(it.GetValue().label.data_equal("Eco"_span));
    EXPECT_EQ(it.GetValue().mode, 1u);
    EXPECT_FALSE(it.Next());

    uint8_t unsupported = 0;
    EXPECT_FALSE(tester.ReadAttribute(0xFFFF, unsupported).IsSuccess());
}

TEST_F(TestModeBaseCluster, ReadOptionalAttributes)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, to_underlying(Feature::kOnOff));

    DataModel::Nullable<uint8_t> startUpMode;
    ASSERT_EQ(tester.ReadAttribute(StartUpMode::Id, startUpMode), CHIP_NO_ERROR);
    EXPECT_TRUE(startUpMode.IsNull());

    DataModel::Nullable<uint8_t> onMode;
    ASSERT_EQ(tester.ReadAttribute(OnMode::Id, onMode), CHIP_NO_ERROR);
    EXPECT_TRUE(onMode.IsNull());
}

TEST_F(TestModeBaseCluster, WriteStartUpModeAndOnMode)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig(BitMask<Feature>(Feature::kOnOff)));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> startUpMode(1);
    ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, startUpMode).IsSuccess());
    ASSERT_EQ(tester.ReadAttribute(StartUpMode::Id, startUpMode), CHIP_NO_ERROR);
    EXPECT_EQ(startUpMode.Value(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(StartUpMode::Id));

    DataModel::Nullable<uint8_t> onMode(1);
    ASSERT_TRUE(tester.WriteAttribute(OnMode::Id, onMode).IsSuccess());
    ASSERT_EQ(tester.ReadAttribute(OnMode::Id, onMode), CHIP_NO_ERROR);
    EXPECT_EQ(onMode.Value(), 1u);

    DataModel::Nullable<uint8_t> invalidMode(99);
    EXPECT_EQ(tester.WriteAttribute(StartUpMode::Id, invalidMode), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(OnMode::Id, invalidMode), Status::ConstraintError);

    uint8_t currentMode = 0;
    EXPECT_EQ(tester.WriteAttribute(CurrentMode::Id, currentMode), Status::UnsupportedAttribute);
}

TEST_F(TestModeBaseCluster, UpdateCurrentModeRejectsUnsupportedMode)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.UpdateCurrentMode(99), Status::ConstraintError);
    EXPECT_EQ(cluster.GetCurrentMode(), 0u);
}

TEST_F(TestModeBaseCluster, ChangeToModeUnsupportedMode)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ChangeToMode::Type request;
    request.newMode = 99;
    auto result     = tester.Invoke(request);
    ASSERT_TRUE(result.status.has_value() && result.status->IsSuccess());
    ASSERT_TRUE(result.response.has_value() && result.response->status == to_underlying(StatusCode::kUnsupportedMode));
    EXPECT_EQ(cluster.GetCurrentMode(), 0u);
}

TEST_F(TestModeBaseCluster, ChangeToModeSameModeReturnsSuccessWithoutUpdating)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ChangeToMode::Type request;
    request.newMode = 0;
    auto result     = tester.Invoke(request);
    ASSERT_TRUE(result.status.has_value() && result.status->IsSuccess());
    ASSERT_TRUE(result.response.has_value() && result.response->status == to_underlying(StatusCode::kSuccess));
    EXPECT_EQ(cluster.GetCurrentMode(), 0u);
    EXPECT_EQ(appDelegate.mLastHandledNewMode, 0xFFu);
}

TEST_F(TestModeBaseCluster, ChangeToModeSuccessUpdatesCurrentMode)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::ChangeToMode::Type request;
    request.newMode = 1;
    auto result     = tester.Invoke(request);
    ASSERT_TRUE(result.status.has_value() && result.status->IsSuccess());
    ASSERT_TRUE(result.response.has_value() && result.response->status == to_underlying(StatusCode::kSuccess));
    EXPECT_EQ(cluster.GetCurrentMode(), 1u);
    EXPECT_EQ(appDelegate.mLastHandledNewMode, 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(CurrentMode::Id));
}

TEST_F(TestModeBaseCluster, ChangeToModeDelegateFailureLeavesCurrentModeUnchanged)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    appDelegate.mRejectNextChange = true;

    Commands::ChangeToMode::Type request;
    request.newMode = 1;
    auto result     = tester.Invoke(request);
    ASSERT_TRUE(result.status.has_value() && result.status->IsSuccess());
    ASSERT_TRUE(result.response.has_value() && result.response->status == to_underlying(StatusCode::kGenericFailure));
    EXPECT_EQ(cluster.GetCurrentMode(), 0u);
}

TEST_F(TestModeBaseCluster, StartupAppliesStartUpModeOnBoot)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    diagnosticDataProvider.mBootReason = GeneralDiagnostics::BootReasonEnum::kPowerOnReboot;

    {
        ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
        ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        DataModel::Nullable<uint8_t> startUpMode(1);
        ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, startUpMode).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 1u);
}

TEST_F(TestModeBaseCluster, StartupIgnoresStartUpModeAfterOtaReboot)
{
    optionalAttributeSet.Set<StartUpMode::Id>();
    diagnosticDataProvider.mBootReason = GeneralDiagnostics::BootReasonEnum::kSoftwareUpdateCompleted;

    {
        ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
        ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        DataModel::Nullable<uint8_t> startUpMode(1);
        ASSERT_TRUE(tester.WriteAttribute(StartUpMode::Id, startUpMode).IsSuccess());
        ASSERT_TRUE(cluster.UpdateCurrentMode(0) == Status::Success);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 0u);
}

TEST_F(TestModeBaseCluster, StartupAppliesOnModeWhenOnOffFeatureEnabled)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig(BitMask<Feature>(Feature::kOnOff), true));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> onMode(1);
    ASSERT_TRUE(tester.WriteAttribute(OnMode::Id, onMode).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentMode = 0xFF;
    ASSERT_EQ(tester.ReadAttribute(CurrentMode::Id, currentMode), CHIP_NO_ERROR);
    EXPECT_EQ(currentMode, 1u);
}

TEST_F(TestModeBaseCluster, GetModeValueByModeTag)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t value = 0xFF;
    EXPECT_EQ(cluster.GetModeValueByModeTag(to_underlying(ModeTag::kLowEnergy), value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 1u);

    EXPECT_EQ(cluster.GetModeValueByModeTag(0xFFFF, value), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
}

TEST_F(TestModeBaseCluster, IsSupportedMode)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.IsSupportedMode(0));
    EXPECT_TRUE(cluster.IsSupportedMode(1));
    EXPECT_FALSE(cluster.IsSupportedMode(99));
}

TEST_F(TestModeBaseCluster, ReportSupportedModesChangeNotifiesAttribute)
{
    ModeBaseCluster cluster(kRootEndpointId, kTestClusterId, MakeConfig());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();
    cluster.ReportSupportedModesChange();
    EXPECT_TRUE(tester.IsAttributeDirty(SupportedModes::Id));
}

} // namespace
