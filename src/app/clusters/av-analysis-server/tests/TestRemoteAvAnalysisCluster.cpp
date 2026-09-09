/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <ranges>

#include <app/CommandHandler.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AvAnalysis/Attributes.h>
#include <clusters/AvAnalysis/Commands.h>
#include <clusters/AvAnalysis/Enums.h>
#include <clusters/AvAnalysis/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AvAnalysis;

using namespace Protocols::InteractionModel;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

static constexpr chip::EndpointId kTestEndpointId = 1;
static constexpr uint8_t kTestMaxZones            = 5;
static constexpr uint8_t kTestMaxAnalysisStreams  = 8;

// Test ambient contexts
// Define the list of semantic tags for the endpoint
const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testAmbientContexts = {
    { std::nullopt, static_cast<uint8_t>(0x49), static_cast<uint8_t>(0x0B),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Object.Package"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x08),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Delivery"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x09),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Retrieval"_span)) }
};

// Minimal mock delegate for testing
class MockAvAnalysisDelegate : public AvAnalysisDelegate
{
public:
    void ShutdownApp() {}

    Protocols::InteractionModel::Status EstablishAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status ActivateAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status DeactivateAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status RemoveAnalysisStream() { return Status::Success; }

    CHIP_ERROR VerifyZoneIDsAreValid(DataModel::DecodableList<uint16_t> aZoneIDs) { return CHIP_NO_ERROR; }

    bool CanAddContextTriggers() { return true; }

    void ActiveAmbientContextTriggersUpdated() {}

    CHIP_ERROR PersistentAttributesLoadedCallback() { return CHIP_NO_ERROR; }
};

struct TestRemoteAvAnalysisCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestRemoteAvAnalysisCluster() :
        mServer(kTestEndpointId, chip::BitFlags<Feature>(Feature::kRemoteContextDetection, Feature::kPerZoneContextDetection),
                testAmbientContexts, DataModel::MakeNullable(kTestMaxZones)),
        mClusterTester(mServer)
    {}

    void SetUp() override
    {
        mServer.SetDelegate(&mMockDelegate);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.SetMaxAnalysisStreamCount(kTestMaxAnalysisStreams), CHIP_NO_ERROR);
    }

    void TearDown() override { mServer.Shutdown(ClusterShutdownType::kClusterShutdown); }

    MockAvAnalysisDelegate mMockDelegate;
    AvAnalysisCluster mServer;
    ClusterTester mClusterTester;
};

TEST_F(TestRemoteAvAnalysisCluster, TestAttributes)
{
    ASSERT_TRUE(IsAttributesListEqualTo(mServer,
                                        {
                                            Attributes::SupportedAmbientContexts::kMetadataEntry,
                                            Attributes::ActiveAmbientContextTriggers::kMetadataEntry,
                                            Attributes::MaxAnalysisStreamCount::kMetadataEntry,
                                            Attributes::CurrentAnalysisStreamCount::kMetadataEntry,
                                            Attributes::AnalysisStreams::kMetadataEntry,
                                            Attributes::TrackingEnabled::kMetadataEntry,
                                        }));
}

TEST_F(TestRemoteAvAnalysisCluster, TestCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mServer,
                                              {
                                                  Commands::EnableContextTriggers::kMetadataEntry,
                                                  Commands::DisableContextTriggers::kMetadataEntry,
                                                  Commands::EstablishAnalysisStream::kMetadataEntry,
                                                  Commands::ActivateAnalysisStream::kMetadataEntry,
                                                  Commands::DeactivateAnalysisStream::kMetadataEntry,
                                                  Commands::RemoveAnalysisStream::kMetadataEntry,
                                              }));
}

TEST_F(TestRemoteAvAnalysisCluster, ReadAllAttributesWithClusterTesterTest)
{
    Attributes::SupportedAmbientContexts::TypeInfo::DecodableType aSupportedAmbientContexts;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::SupportedAmbientContexts::Id, aSupportedAmbientContexts), CHIP_NO_ERROR);

    // Verify that the entries in the DecodableList match the entries used in construction of the instance by
    // creating a vactor of the values then comparing the two vectors
    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> readContexts;
    auto aContextIterator = aSupportedAmbientContexts.begin();
    while (aContextIterator.Next())
    {
        readContexts.push_back(aContextIterator.GetValue());
    }

    // No == exists for the Struct, and creating one fails due to the Struct structure, check value by value
    bool are_equal =
        std::equal(testAmbientContexts.begin(), testAmbientContexts.end(), readContexts.begin(), readContexts.end(),
                   [](const auto & p1, const auto & p2) { return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag; });
    ASSERT_TRUE(are_equal);

    // On startup there should be no active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    size_t triggersSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED aActiveContextTriggers.ComputeSize(&triggersSize);
    ASSERT_EQ(triggersSize, static_cast<size_t>(0));

    uint8_t aMaxAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MaxAnalysisStreamCount::Id, aMaxAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(kTestMaxAnalysisStreams, aMaxAnalysisStreamCount);

    uint8_t aCurrentAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, aCurrentAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(0, aCurrentAnalysisStreamCount);

    // On startup there should be no analysis streams
    Attributes::AnalysisStreams::TypeInfo::DecodableType aAnalysisStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, aAnalysisStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED aAnalysisStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteEnableContextTriggersCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Initially with no streams established, EnableContextTriggers must return InvalidInState
    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));

    // Establish an analysis stream
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // With a stream established, EnableContextTriggers should succeed
    response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(response.value().IsSuccess());
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDisableContextTriggersCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteEstablishAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteActivateAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;

<<<<<<< HEAD
    auto response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
=======
    commandData.analysisStreamID = 77;
    auto response                = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::NotFound));

    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    commandData.analysisStreamID = 0;

    // Activating a PendingInitiation stream succeeds
    response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(response.value().IsSuccess());

    // Stream state is now WebRTCActive
    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 0);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kWebRTCActive);
    ASSERT_FALSE(iter.Next());

    // Activating an already active stream returns InvalidInState
    response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));
>>>>>>> 05ad181 ([Camera] AV Analysis cluster test scripts (TC_AVANALY_2_4 to 2_14) and updates to cluster server and camera-app logic (#73851))
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDeactivateAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;

<<<<<<< HEAD
    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteRemoveAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
=======
    // Spec 11.9.8.6.2: an unknown AnalysisStreamID is NOT_FOUND before any state gating
    commandData.analysisStreamID = 77;
    auto response                = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::NotFound));

    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    commandData.analysisStreamID = 0;

    // Stream is in PendingInitiation: deactivating returns InvalidInState
    response = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));

    // Activate the stream first
    ConcreteCommandPath kActivatePath{ 1, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType activateData;
    activateData.analysisStreamID = 0;
    auto activateResponse         = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kActivatePath, activateData);
    ASSERT_TRUE(activateResponse.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(activateResponse.value().IsSuccess());

    // Now deactivating the active stream succeeds and transitions back to PendingInitiation
    response = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(response.value().IsSuccess());

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 0);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);
    ASSERT_FALSE(iter.Next());

    // Deactivating again returns InvalidInState
    response = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    ASSERT_TRUE(response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(response->GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));
>>>>>>> 05ad181 ([Camera] AV Analysis cluster test scripts (TC_AVANALY_2_4 to 2_14) and updates to cluster server and camera-app logic (#73851))
}
} // namespace
