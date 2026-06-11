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

// Test ambient contexts
// Define the list of semantic tags for the endpoint
const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testAmbientContexts = {
    { .namespaceID = static_cast<uint8_t>(0x49), .tag = static_cast<uint8_t>(0x0B),  
      .label = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Object.Package"_span))},
    { .namespaceID = static_cast<uint8_t>(0x4B), .tag = static_cast<uint8_t>(0x08),  
      .label = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Delivery"_span))},
    { .namespaceID = static_cast<uint8_t>(0x4B), .tag = static_cast<uint8_t>(0x09),  
      .label = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Retrieval"_span))}
};

const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testErrorAmbientContext = {
    { .namespaceID = static_cast<uint8_t>(0x4A), .tag = static_cast<uint8_t>(0x02),  
      .label = MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Sound.Snoring"_span))}
};

// Minimal mock delegate for testing
class MockAvAnalysisDelegate : public AvAnalysisDelegate
{
public:
    void ShutdownApp() {}

    Protocols::InteractionModel::Status EnableContextTriggers() 
    {
        return Status::Success;
    }

    Protocols::InteractionModel::Status DisableContextTriggers()
    {
        return Status::Success;
    }

    Protocols::InteractionModel::Status EstablishAnalysisStream() 
    {
        return Status::Success;
    }

    Protocols::InteractionModel::Status ActivateAnalysisStream()
    {
        return Status::Success;
    }

    Protocols::InteractionModel::Status DeactivateAnalysisStream()
    {
        return Status::Success;
    }

    Protocols::InteractionModel::Status RemoveAnalysisStream()
    {
        return Status::Success;
    }
    
    CHIP_ERROR VerifyZoneIDsAreValid(DataModel::DecodableList<uint16_t>)
    {
        return CHIP_NO_ERROR;
    }

    bool CanAddContextTriggers() {return true; }
    
    void ActiveAmbientContextTriggersUpdated() {}

};

struct TestLocalAvAnalysisCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestLocalAvAnalysisCluster() :
        mServer(kTestEndpointId,
                chip::BitFlags<Feature>(Feature::kLocalContextDetection, 
                                        Feature::kPerZoneContextDetection),
                testAmbientContexts),
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
    bool are_equal = std::ranges::equal(testAmbientContexts, readContexts, [](const auto& p1, const auto& p2) {
        return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag;
    });
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

TEST_F(TestLocalAvAnalysisCluster, ExecuteEnableContextTriggersCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // Sub-tests
    // 1. Null, verify the active set is the supported set, with ZoneIDs set to Null.
    // 2. Provide a context that doesn't exist, ensure error
    // 3. Existing context, missing ZoneIDs, ensure error
    // 4. Existing context, null ZoneIDs, ensure success for all zones
    // 5. Existing context, non-null ZoneIDs, ensure success
    
    // 1. Null context triggers, active set is the same as the supported set.
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
    bool are_equal = std::ranges::equal(readSupportedContexts, readActiveContexts, [](const auto& p1, const auto& p2) {
        return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag;
    });
    ASSERT_TRUE(are_equal);
    
    // 2. Provide a context that doesn't exist, ensure error
    uint8_t tlvBuffer[512];
    Structs::ContextTriggerStruct::Type cts;
    cts.context = testErrorAmbientContext.front();
    cts.zoneIDs = chip::MakeOptional(DataModel::NullNullable);
    
    // Encode into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

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

    commandData.contextTriggers = DataModel::MakeNullable(decodedList);    
    response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

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
       

TEST_F(TestLocalAvAnalysisCluster, ExecuteDisableContextTriggersCommandTest)
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

} // namespace
