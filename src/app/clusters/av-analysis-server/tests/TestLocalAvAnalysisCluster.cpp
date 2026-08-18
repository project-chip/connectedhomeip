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

// Test ambient contexts
// Define the list of semantic tags for the endpoint
const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testAmbientContexts = {
    { .namespaceID = static_cast<uint8_t>(0x49),
      .tag         = static_cast<uint8_t>(0x0B),
      .label       = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Object.Package"_span)) },
    { .namespaceID = static_cast<uint8_t>(0x4B),
      .tag         = static_cast<uint8_t>(0x08),
      .label       = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Delivery"_span)) },
    { .namespaceID = static_cast<uint8_t>(0x4B),
      .tag         = static_cast<uint8_t>(0x09),
      .label       = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Retrieval"_span)) }
};

const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testErrorAmbientContext = {
    { .namespaceID = static_cast<uint8_t>(0x4A),
      .tag         = static_cast<uint8_t>(0x02),
      .label       = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Sound.Snoring"_span)) }
};

// Define the test tracked contexts for the event framework
const std::vector<app::Clusters::AvAnalysis::Structs::TrackedContext::Type> testTrackedContext = {
    { .identifiedContextID = 0,
      .identifiedContext   = { .namespaceID = static_cast<uint8_t>(0x49), .tag = static_cast<uint8_t>(0x0B)},
      .startTime           = 0,
      .endTime             = DataModel::NullNullable }
};

const std::vector<app::Clusters::AvAnalysis::Structs::TrackedContext::Type> testAdditionalTrackedContext = {
    { .identifiedContextID = 0,
      .identifiedContext   = { .namespaceID = static_cast<uint8_t>(0x4B), .tag = static_cast<uint8_t>(0x09)},
      .startTime           = 0,
      .endTime             = DataModel::NullNullable }
};
    
const std::vector<uint16_t> testZoneIDList = { static_cast<uint16_t>(0x01), static_cast<uint16_t>(0x02),
                                               static_cast<uint16_t>(0x03), static_cast<uint16_t>(0x04) };

const std::vector<uint16_t> testZoneIDDisableList = { static_cast<uint16_t>(0x03), static_cast<uint16_t>(0x04) };

const std::vector<uint16_t> testZoneIDRemainingList = { static_cast<uint16_t>(0x01), static_cast<uint16_t>(0x02) };

// Minimal mock delegate for testing
class MockAvAnalysisDelegate : public AvAnalysisDelegate
{
public:
    void ShutdownApp() {}

    Protocols::InteractionModel::Status EnableContextTriggers() { return Status::Success; }

    Protocols::InteractionModel::Status DisableContextTriggers() { return Status::Success; }

    Protocols::InteractionModel::Status EstablishAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status ActivateAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status DeactivateAnalysisStream() { return Status::Success; }

    Protocols::InteractionModel::Status RemoveAnalysisStream() { return Status::Success; }

    CHIP_ERROR VerifyZoneIDsAreValid(std::vector<uint16_t> aZoneIDs) { return CHIP_NO_ERROR; }

    bool CanAddContextTriggers() { return true; }

    void ActiveAmbientContextTriggersUpdated() {}

    CHIP_ERROR PersistentAttributesLoadedCallback() { return CHIP_NO_ERROR; }
};

struct TestLocalAvAnalysisCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    // We always have to have Zones supplied, they could be Null
    DataModel::Nullable<DataModel::DecodableList<Structs::ContextTriggerStruct::DecodableType>>
    CreateCommandData(Globals::Structs::SemanticTagStruct::Type context, DataModel::Nullable<std::vector<uint16_t>> zones,
                      uint8_t * tlvBuffer, size_t tlvBufferSize, bool noZones = false)
    {
        Structs::ContextTriggerStruct::Type cts;
        cts.context = context;

        if (!noZones)
        {
            // Convert the zones if we have them to a List (Span)
            if (zones.IsNull())
            {
                cts.zoneIDs = MakeOptional(DataModel::NullNullable);
            }
            else
            {
                cts.zoneIDs = MakeOptional(
                    DataModel::MakeNullable(DataModel::List<const uint16_t>(zones.Value().data(), zones.Value().size())));
            }
        }
        else
        {
            cts.zoneIDs.ClearValue();
        }

        // Encode into a TLV buffer
        TLV::TLVWriter writer;
        writer.Init(tlvBuffer, static_cast<uint32_t>(tlvBufferSize));

        TLV::TLVWriter containerWriter;
        CHIP_ERROR err;

        err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), cts);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.CloseContainer(containerWriter);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        size_t encodedLen = writer.GetLengthWritten();

        // Decode the TLV into a DecodableList
        TLV::TLVReader ctsReader;
        ctsReader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
        err = ctsReader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        DataModel::DecodableList<Structs::ContextTriggerStruct::DecodableType> decodedList;
        err = decodedList.Decode(ctsReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        return DataModel::MakeNullable(decodedList);
    }

    bool EnableAllTestContexts()
    {
        Testing::MockCommandHandler commandHandler;
        commandHandler.SetFabricIndex(1);
        ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
        Commands::EnableContextTriggers::DecodableType commandData;

        // Null context triggers, active set is the same as the supported set.
        commandData.contextTriggers.SetNull();

        auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

        if (response.has_value())
        {
            return response.value().IsSuccess();
        }

        return false;
    }

    bool EnableSpecificTestContexts(Globals::Structs::SemanticTagStruct::Type context,
                                    DataModel::Nullable<std::vector<uint16_t>> zones)
    {
        Testing::MockCommandHandler commandHandler;
        commandHandler.SetFabricIndex(1);
        ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
        Commands::EnableContextTriggers::DecodableType commandData;
        uint8_t tlvBuffer[512];

        commandData.contextTriggers = CreateCommandData(context, zones, tlvBuffer, sizeof(tlvBuffer));
        auto response               = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

        if (response.has_value())
        {
            return response.value().IsSuccess();
        }

        return false;
    }

    TestLocalAvAnalysisCluster() :
        mServer(kTestEndpointId, chip::BitFlags<Feature>(Feature::kLocalContextDetection, Feature::kPerZoneContextDetection),
                testAmbientContexts, DataModel::MakeNullable(kTestMaxZones)),
        mClusterTester(mServer)
    {}

    void SetUp() override
    {
        mServer.SetDelegate(&mMockDelegate);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
    }

    void TearDown() override { mServer.Shutdown(ClusterShutdownType::kClusterShutdown); }

    MockAvAnalysisDelegate mMockDelegate;
    AvAnalysisCluster mServer;
    ClusterTester mClusterTester;
};

TEST_F(TestLocalAvAnalysisCluster, TestAttributes)
{
    ASSERT_TRUE(IsAttributesListEqualTo(mServer,
                                        {
                                            Attributes::SupportedAmbientContexts::kMetadataEntry,
                                            Attributes::ActiveAmbientContextTriggers::kMetadataEntry,
                                            Attributes::TrackingEnabled::kMetadataEntry,
                                        }));
}

TEST_F(TestLocalAvAnalysisCluster, TestCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mServer,
                                              {
                                                  Commands::EnableContextTriggers::kMetadataEntry,
                                                  Commands::DisableContextTriggers::kMetadataEntry,
                                              }));
}

TEST_F(TestLocalAvAnalysisCluster, ReadAllAttributesWithClusterTesterTest)
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

    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);
}

TEST_F(TestLocalAvAnalysisCluster, TestReadWriteTrackingEnabled)
{
    bool trackingEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(trackingEnabled); // Default should be false

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::TrackingEnabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(trackingEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::TrackingEnabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(trackingEnabled);
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteActiveAmbientContextsPersistenceTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Provide a context that does exist with test zone ids
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::MakeNullable(testZoneIDList), tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Read our set of active triggers, make sure valid
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    // There is only one test entry, verify that is correct
    auto aActiveContextIterator = aActiveContextTriggers.begin();
    aActiveContextIterator.Next();

    auto contextTrigger = aActiveContextIterator.GetValue();

    // Are the provided and read active contexts equal
    //
    app::Clusters::Descriptor::Structs::SemanticTagStruct::Type writtenContext = testAmbientContexts.front();

    ASSERT_EQ(writtenContext.namespaceID, contextTrigger.context.namespaceID);
    ASSERT_EQ(writtenContext.tag, contextTrigger.context.tag);

    // Extract our read ZoneID List
    std::vector<uint16_t> zoneIDs;

    ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
    ASSERT_TRUE(!contextTrigger.zoneIDs.Value().IsNull());

    auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

    while (zone_iter.Next())
    {
        zoneIDs.push_back(zone_iter.GetValue());
    }
    ASSERT_EQ(testZoneIDList, zoneIDs);

    // Shutdown the Server
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    // Start the Server back up
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Re-read our set of active triggers, make sure still valid
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    // There is only one test entry, verify that is correct
    aActiveContextIterator = aActiveContextTriggers.begin();
    aActiveContextIterator.Next();

    contextTrigger = aActiveContextIterator.GetValue();

    // Are the provided and read active contexts equal
    //
    ASSERT_EQ(writtenContext.namespaceID, contextTrigger.context.namespaceID);
    ASSERT_EQ(writtenContext.tag, contextTrigger.context.tag);

    // Extract our read ZoneID List
    ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
    ASSERT_TRUE(!contextTrigger.zoneIDs.Value().IsNull());

    zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

    // Reset our local copy of the Zoneids
    zoneIDs.clear();
    while (zone_iter.Next())
    {
        zoneIDs.push_back(zone_iter.GetValue());
    }
    ASSERT_EQ(testZoneIDList, zoneIDs);
}

// EnableContextTriggers Sub-tests
// 1. Null, verify the active set is the supported set, with ZoneIDs set to Null.
// 2. Provide a context that doesn't exist, ensure error
// 3. Existing context, missing ZoneIDs, ensure error
// 4. Existing context, null ZoneIDs, ensure success for all zones
// 5. Existing context, non-null ZoneIDs, ensure success
TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTestContextTriggersIsNull)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Null context triggers, active set is the same as the supported set.
    commandData.contextTriggers.SetNull();
    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Read the supported triggers
    Attributes::SupportedAmbientContexts::TypeInfo::DecodableType aSupportedAmbientContexts;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::SupportedAmbientContexts::Id, aSupportedAmbientContexts), CHIP_NO_ERROR);

    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> readSupportedContexts;
    auto aSupportedContextIterator = aSupportedAmbientContexts.begin();
    while (aSupportedContextIterator.Next())
    {
        readSupportedContexts.push_back(aSupportedContextIterator.GetValue());
    }

    // Read the active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> readActiveContexts;
    auto aActiveContextIterator = aActiveContextTriggers.begin();
    while (aActiveContextIterator.Next())
    {
        auto contextTrigger = aActiveContextIterator.GetValue();
        readActiveContexts.push_back(contextTrigger.context);

        // For each context make sure the ZoneID has a Value and that it is null
        ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
        ASSERT_TRUE(contextTrigger.zoneIDs.Value().IsNull());
    }

    // Are the supported and active contexts equal
    //
    bool are_equal =
        std::equal(readSupportedContexts.begin(), readSupportedContexts.end(), readActiveContexts.begin(), readActiveContexts.end(),
                   [](const auto & p1, const auto & p2) { return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag; });
    ASSERT_TRUE(are_equal);
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTestContextTriggerDoesNotExist)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Provide a context that doesn't exist, ensure error
    uint8_t tlvBuffer[512];

    commandData.contextTriggers =
        CreateCommandData(testErrorAmbientContext.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::ConstraintError);
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTestContextTriggerHasNoZones)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Provide a context that does exist but no ZoneIDs, ensure error
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer), true);

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::InvalidCommand);
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTestContextTriggerHasNullZones)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Provide a context that does exist, set the zones to null, ensure success and the result also has null zones
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Read our set of active triggers, make sure valid
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    auto aActiveContextIterator = aActiveContextTriggers.begin();
    while (aActiveContextIterator.Next())
    {
        auto contextTrigger = aActiveContextIterator.GetValue();

        // For each context make sure the ZoneID has a Value and that it is null
        ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
        ASSERT_TRUE(contextTrigger.zoneIDs.Value().IsNull());
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTestContextTriggerHasDefinedZones)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Provide a context that does exist with test zone ids
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::MakeNullable(testZoneIDList), tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Read our set of active triggers, make sure valid
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    // There is only one test entry, verify that is correct
    auto aActiveContextIterator = aActiveContextTriggers.begin();
    aActiveContextIterator.Next();

    auto contextTrigger = aActiveContextIterator.GetValue();

    // Are the provided and read active contexts equal
    //
    app::Clusters::Descriptor::Structs::SemanticTagStruct::Type writtenContext = testAmbientContexts.front();

    ASSERT_EQ(writtenContext.namespaceID, contextTrigger.context.namespaceID);
    ASSERT_EQ(writtenContext.tag, contextTrigger.context.tag);

    // Extract our read ZoneID List
    std::vector<uint16_t> zoneIDs;

    ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
    ASSERT_TRUE(!contextTrigger.zoneIDs.Value().IsNull());

    auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

    while (zone_iter.Next())
    {
        zoneIDs.push_back(zone_iter.GetValue());
    }
    ASSERT_EQ(testZoneIDList, zoneIDs);
}

// DisableContextTriggers Sub-tests
// As part of each test, first enable the test set of triggers
// 1. Null, verify the active set is an empty list
// 2. Provide a context that doesn't exist, ensure error
// 3. Existing context, missing ZoneIDs, ensure error
// 4. Existing context, enabled all triggers and all zones, try to remove some zones, ensure error
// 5. Existing context, enabled all triggers and all zones, remove all zones, ensure success
// 6. Existing context, enabled all triggers and some zones, removal of a subset of zones, ensure success
TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestContextTriggersIsNull)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableAllTestContexts())
    {
        FAIL();
    }

    // Null context triggers, active set is an empty list
    commandData.contextTriggers.SetNull();
    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // Read the active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    // It should be an empty list
    size_t size;
    CHIP_ERROR err = aActiveContextTriggers.ComputeSize(&size);
    if (err != CHIP_NO_ERROR)
    {
        FAIL();
    }
    ASSERT_TRUE(size == 0);
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestContextTriggerDoesNotExist)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableAllTestContexts())
    {
        FAIL();
    }

    // Provide a context that doesn't exist, ensure error
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testErrorAmbientContext.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::DynamicConstraintError);
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestContextTriggerHasNoZones)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableAllTestContexts())
    {
        FAIL();
    }

    // Provide a context that does exist but no ZoneIDs, ensure error
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer), true);

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::InvalidCommand);
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestRemoveSomeZonesWhenAllSet)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableAllTestContexts())
    {
        FAIL();
    }

    // Remove context with only certain zones set
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::MakeNullable(testZoneIDList), tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_EQ(response.value().GetStatusCode().GetStatus(), Status::DynamicConstraintError);
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestRemoveAllZonesWhenAllSet)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableAllTestContexts())
    {
        FAIL();
    }

    // Remove context with zones set to Null (all zones)
    uint8_t tlvBuffer[512];
    commandData.contextTriggers =
        CreateCommandData(testAmbientContexts.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // The active context should be the second and third of our test contexts, as we have removed the first
    // context (effectively)
    // Read the active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> readActiveContexts;
    auto aActiveContextIterator = aActiveContextTriggers.begin();
    while (aActiveContextIterator.Next())
    {
        auto contextTrigger = aActiveContextIterator.GetValue();
        readActiveContexts.push_back(contextTrigger.context);

        // For each context make sure the ZoneID has a Value and that it is null
        ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
        ASSERT_TRUE(contextTrigger.zoneIDs.Value().IsNull());
    }

    // Are the active contexts our test contexts minus the first value
    //
    bool are_equal =
        std::equal(testAmbientContexts.begin() + 1, testAmbientContexts.end(), readActiveContexts.begin(), readActiveContexts.end(),
                   [](const auto & p1, const auto & p2) { return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag; });
    ASSERT_TRUE(are_equal);
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTestRemoveSomeZonesWhenAllExplicitlySet)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    if (!EnableSpecificTestContexts(testAmbientContexts.front(), DataModel::MakeNullable(testZoneIDList)))
    {
        FAIL();
    }

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType testActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, testActiveContextTriggers), CHIP_NO_ERROR);

    // Remove context with zones set to the final two zones of the test set
    uint8_t tlvBuffer[512];
    commandData.contextTriggers = CreateCommandData(testAmbientContexts.front(), DataModel::MakeNullable(testZoneIDDisableList),
                                                    tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }

    // The first active context should be present, with only Zone IDs 1 and 2
    // Read the active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);

    auto aActiveContextIterator = aActiveContextTriggers.begin();
    aActiveContextIterator.Next();

    auto contextTrigger = aActiveContextIterator.GetValue();

    // For the first context make sure the ZoneID has a Value and that it is the first two defined IDs only
    ASSERT_TRUE(contextTrigger.zoneIDs.HasValue());
    ASSERT_TRUE(!contextTrigger.zoneIDs.Value().IsNull());

    // Extract our read ZoneID List
    std::vector<uint16_t> zoneIDs;

    auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

    while (zone_iter.Next())
    {
        zoneIDs.push_back(zone_iter.GetValue());
    }

    ASSERT_EQ(testZoneIDRemainingList, zoneIDs);
}

TEST_F(TestLocalAvAnalysisCluster, ExecuteEventGenerationSequence)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    if (!EnableAllTestContexts())
    {
        FAIL();
    }
    
    // 1. Session Start Event
    uint16_t mSessionId;

    ASSERT_EQ(mServer.GetLogic().AnalysisSessionStart(mSessionId, DataModel::NullNullable, &mClusterTester.GetServerClusterContext()),
              CHIP_NO_ERROR);
    auto analysisStartEvent = mClusterTester.GetNextGeneratedEvent();
    if (!analysisStartEvent.has_value())
    {
        FAIL() << "Expected analysisStartEvent to have a value";
        return;
    }
    Events::AnalysisSessionStart::DecodableType startData;
    ASSERT_EQ(analysisStartEvent->GetEventData(startData), CHIP_NO_ERROR);
    ASSERT_EQ(startData.sessionID, 0);
    if (!startData.triggeredZones.IsNull())
    {
        FAIL() << "Expected Zones to be Null";
        return;
    }
    
    // 2a. Initial Perceived Context Event - Invalid Session ID
    uint16_t invalidSessionID = 99;
    ASSERT_EQ(mServer.GetLogic().InitialTriggeringContextDetected(invalidSessionID, testTrackedContext, &mClusterTester.GetServerClusterContext()),
              CHIP_ERROR_NOT_FOUND);
              
    // 2b. Initial Perceived Context Event
     ASSERT_EQ(mServer.GetLogic().InitialTriggeringContextDetected(mSessionId, testTrackedContext, &mClusterTester.GetServerClusterContext()),
              CHIP_NO_ERROR);   
    auto perceivedContextEvent = mClusterTester.GetNextGeneratedEvent();
    if (!perceivedContextEvent.has_value())
    {
        FAIL() << "Expected perceivedContextEvent to have a value";
        return;
    }
    
    Events::PerceivedContext::DecodableType perceivedContextData;
    ASSERT_EQ(perceivedContextEvent->GetEventData(perceivedContextData), CHIP_NO_ERROR);
    ASSERT_EQ(perceivedContextData.sessionID, 0);
    
    // Verify that the event contains only a new identified context, and that it has one value.
    ASSERT_TRUE(perceivedContextData.newIdentifiedContexts.HasValue());
    ASSERT_FALSE(perceivedContextData.currentIdentifiedContexts.HasValue());
    ASSERT_FALSE(perceivedContextData.expiredContexts.HasValue());

    // Starting with element count, should be one
    size_t count;                                                                                                                                                                       
    CHIP_ERROR err = perceivedContextData.newIdentifiedContexts.Value().ComputeSize(&count);                                                                                                                                 
    if (err != CHIP_NO_ERROR) {       
        FAIL() << "Error in computing the size of the elements in a PerceivedContext Event";                                                                                                                                                                                                                                            
    }  
    ASSERT_EQ(count, static_cast<size_t>(1));

    // Ensure the identifiedContext matches the test context
    // Verify that the entries in the DecodableList match the entries used in construction of the instance by
    // creating a vector of the values then comparing the two vectors
    std::vector<Structs::TrackedContext::Type> initialEventContexts;
    auto aInitialContextIterator = perceivedContextData.newIdentifiedContexts.Value().begin();
    while (aInitialContextIterator.Next())
    {
        initialEventContexts.push_back(aInitialContextIterator.GetValue());
    }

    // No == exists for the Struct, and creating one fails due to the Struct structure, check value by value
    bool are_equal =
        std::equal(testTrackedContext.begin(), testTrackedContext.end(), initialEventContexts.begin(), initialEventContexts.end(),
                   [](const auto & tc1, const auto & tc2) { return tc1.identifiedContext.namespaceID == tc2.identifiedContext.namespaceID && 
                                                            tc1.identifiedContext.tag == tc2.identifiedContext.tag; });
    ASSERT_TRUE(are_equal);
    
    // 3. Updated context 
    ASSERT_EQ(mServer.GetLogic().NewContextDetected(mSessionId, testAdditionalTrackedContext, &mClusterTester.GetServerClusterContext()),
              CHIP_NO_ERROR);   
    auto secondPerceivedContextEvent = mClusterTester.GetNextGeneratedEvent();
    if (!secondPerceivedContextEvent.has_value())
    {
        FAIL() << "Expected perceivedContextEvent to have a value";
        return;
    }
    
    Events::PerceivedContext::DecodableType secondPerceivedContextData;
    ASSERT_EQ(secondPerceivedContextEvent->GetEventData(secondPerceivedContextData), CHIP_NO_ERROR);
    ASSERT_EQ(secondPerceivedContextData.sessionID, 0);    
    
    // Verify that the event contains a new and current identified context only
    ASSERT_TRUE(secondPerceivedContextData.newIdentifiedContexts.HasValue());
    ASSERT_TRUE(secondPerceivedContextData.currentIdentifiedContexts.HasValue());
    ASSERT_FALSE(secondPerceivedContextData.expiredContexts.HasValue());
    
    // Ensure the new and current identifiedContexts matches the test contexts
    // Create vectors of the values for the new values and the current values then compare
    std::vector<Structs::TrackedContext::Type> currentEventContexts;
    auto aCurrentContextIterator = secondPerceivedContextData.currentIdentifiedContexts.Value().begin();
    while (aCurrentContextIterator.Next())
    {
        currentEventContexts.push_back(aCurrentContextIterator.GetValue());
    }
    
    std::vector<Structs::TrackedContext::Type> newEventContexts;
    auto aNewContextIterator = secondPerceivedContextData.newIdentifiedContexts.Value().begin();
    while (aNewContextIterator.Next())
    {
        newEventContexts.push_back(aNewContextIterator.GetValue());
    }
    
    // No == exists for the Struct, and creating one fails due to the Struct structure, check value by value for the two
    // event fields
    are_equal =
        std::equal(testTrackedContext.begin(), testTrackedContext.end(), currentEventContexts.begin(), currentEventContexts.end(),
                   [](const auto & tc1, const auto & tc2) { return tc1.identifiedContext.namespaceID == tc2.identifiedContext.namespaceID && 
                                                            tc1.identifiedContext.tag == tc2.identifiedContext.tag; });
    ASSERT_TRUE(are_equal);
    
    are_equal =
        std::equal(testAdditionalTrackedContext.begin(), testAdditionalTrackedContext.end(), newEventContexts.begin(), newEventContexts.end(),
                   [](const auto & tc1, const auto & tc2) { return tc1.identifiedContext.namespaceID == tc2.identifiedContext.namespaceID && 
                                                            tc1.identifiedContext.tag == tc2.identifiedContext.tag; });
    ASSERT_TRUE(are_equal);
        
    // 4. Deleted Context
    ASSERT_EQ(mServer.GetLogic().ContextNoLongerDetected(mSessionId, testTrackedContext, &mClusterTester.GetServerClusterContext()),
              CHIP_NO_ERROR);   
              
    auto thirdPerceivedContextEvent = mClusterTester.GetNextGeneratedEvent();
    if (!thirdPerceivedContextEvent.has_value())
    {
        FAIL() << "Expected perceivedContextEvent to have a value";
        return;
    }
    
    Events::PerceivedContext::DecodableType thirdPerceivedContextData;
    ASSERT_EQ(thirdPerceivedContextEvent->GetEventData(thirdPerceivedContextData), CHIP_NO_ERROR);
    ASSERT_EQ(thirdPerceivedContextData.sessionID, 0);    
    
    // Verify that the event contains a current and expired identified context only
    ASSERT_FALSE(thirdPerceivedContextData.newIdentifiedContexts.HasValue());
    ASSERT_TRUE(thirdPerceivedContextData.currentIdentifiedContexts.HasValue());
    ASSERT_TRUE(thirdPerceivedContextData.expiredContexts.HasValue());
    
    // Ensure the expired and current identifiedContexts matches the test contexts
    // Create vectors of the values for the new values and the current values then compare
    currentEventContexts.clear();
    aCurrentContextIterator = thirdPerceivedContextData.currentIdentifiedContexts.Value().begin();
    while (aCurrentContextIterator.Next())
    {
        currentEventContexts.push_back(aCurrentContextIterator.GetValue());
    }
    
    std::vector<Structs::TrackedContext::Type> expiredEventContexts;
    auto aExpiredContextIterator = thirdPerceivedContextData.expiredContexts.Value().begin();
    while (aExpiredContextIterator.Next())
    {
        expiredEventContexts.push_back(aExpiredContextIterator.GetValue());
    }    

    // No == exists for the Struct, and creating one fails due to the Struct structure, check value by value for the two
    // event fields
    are_equal =
        std::equal(testTrackedContext.begin(), testTrackedContext.end(), expiredEventContexts.begin(), expiredEventContexts.end(),
                   [](const auto & tc1, const auto & tc2) { return tc1.identifiedContext.namespaceID == tc2.identifiedContext.namespaceID && 
                                                            tc1.identifiedContext.tag == tc2.identifiedContext.tag; });
    ASSERT_TRUE(are_equal);
    
    are_equal =
        std::equal(testAdditionalTrackedContext.begin(), testAdditionalTrackedContext.end(), currentEventContexts.begin(), currentEventContexts.end(),
                   [](const auto & tc1, const auto & tc2) { return tc1.identifiedContext.namespaceID == tc2.identifiedContext.namespaceID && 
                                                            tc1.identifiedContext.tag == tc2.identifiedContext.tag; });
    ASSERT_TRUE(are_equal);
    
    // 5. End Session
    ASSERT_EQ(mServer.GetLogic().AnalysisSessionEnd(mSessionId, &mClusterTester.GetServerClusterContext()),
              CHIP_NO_ERROR);   
              
    auto endSessionEvent = mClusterTester.GetNextGeneratedEvent();
    if (!endSessionEvent.has_value())
    {
        FAIL() << "Expected endSessionEvent to have a value";
        return;
    }
    
    Events::AnalysisSessionEnd::DecodableType endSessionData;
    ASSERT_EQ(endSessionEvent->GetEventData(endSessionData), CHIP_NO_ERROR);
    ASSERT_EQ(endSessionData.sessionID, 0);        
}

} // namespace
