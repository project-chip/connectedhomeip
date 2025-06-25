/*
 *
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
#include <vector>

#include "lib/support/CHIPMem.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/actions-server/actions-server.h>
#include <app/tests/test-ember-api.h>
#include <app/util/attribute-storage.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {

using namespace Clusters::Actions;
using namespace chip::Protocols::InteractionModel;

class TestActionsDelegateImpl : public Clusters::Actions::Delegate
{
public:
    static constexpr uint8_t kMaxActionNameLength       = 128u;
    static constexpr uint8_t kMaxEndpointListNameLength = 128u;
    static constexpr uint16_t kEndpointListMaxSize      = 256u;
    static constexpr uint8_t kMaxActions                = 2;
    static constexpr uint8_t kMaxEndpointLists          = 2;

    ActionStructStorage mActions[kMaxActions];
    uint16_t mNumActions = 0;

    EndpointListStorage mEndpointLists[kMaxEndpointLists];
    uint16_t mNumEndpointLists = 0;

    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override
    {
        if (index >= mNumActions)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        action = mActions[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override
    {
        if (index >= mNumEndpointLists)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        epList = mEndpointLists[index];
        return CHIP_NO_ERROR;
    }

    bool HaveActionWithId(uint16_t actionId, uint16_t & aActionIndex) override
    {
        for (uint16_t i = 0; i < mNumActions; i++)
        {
            if (mActions[i].actionID == actionId)
            {
                aActionIndex = i;
                return true;
            }
        }
        return false;
    }

    // Test helper methods
    CHIP_ERROR AddTestAction(const ActionStructStorage & action)
    {
        if (mNumActions >= kMaxActions)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        mActions[mNumActions++] = action;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddTestEndpointList(const EndpointListStorage & epList)
    {
        if (mNumEndpointLists >= kMaxEndpointLists)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        mEndpointLists[mNumEndpointLists++] = epList;
        return CHIP_NO_ERROR;
    }

    Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                          Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                       Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
    Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                        Optional<uint32_t> invokeId) override
    {
        return Status::NotFound;
    }
};

static TestActionsDelegateImpl * sActionsDelegateImpl = nullptr;
static ActionsServer * sActionsServer                 = nullptr;

class TestActionsCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        sActionsDelegateImpl = new TestActionsDelegateImpl;
        sActionsServer       = new ActionsServer(1, *sActionsDelegateImpl);
        sActionsServer->Init();
    }
    static void TearDownTestSuite()
    {
        sActionsServer->Shutdown();
        delete sActionsDelegateImpl;
        delete sActionsServer;
        chip::Platform::MemoryShutdown();
    }
};

TEST_F(TestActionsCluster, TestActionListConstraints)
{
    // Test 1: Action name length constraint
    TestActionsDelegateImpl delegate = *sActionsDelegateImpl;
    delegate.mNumActions             = 0;
    char longName[kActionNameMaxSize + 10];
    memset(longName, 'A', sizeof(longName));
    longName[sizeof(longName) - 1] = '\0';

    ActionStructStorage actionWithLongName(1, CharSpan::fromCharString(longName), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                                           ActionStateEnum::kInactive);

    // Add action and verify it was added successfully
    EXPECT_EQ(delegate.AddTestAction(actionWithLongName), CHIP_NO_ERROR);

    // Verify the name was truncated by reading it back
    ActionStructStorage readAction;
    EXPECT_EQ(delegate.ReadActionAtIndex(0, readAction), CHIP_NO_ERROR);
    EXPECT_EQ(readAction.name.size(), kActionNameMaxSize);

    // Test 2: Maximum action list size
    delegate.mNumActions = 0;
    for (uint16_t i = 0; i < delegate.kMaxActions; i++)
    {
        ActionStructStorage action(i, CharSpan::fromCharString("Action"), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                                   ActionStateEnum::kInactive);
        EXPECT_EQ(delegate.AddTestAction(action), CHIP_NO_ERROR);
    }

    // Try to add one more action beyond the limit
    ActionStructStorage extraAction(99, CharSpan::fromCharString("Extra"), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                                    ActionStateEnum::kInactive);
    EXPECT_EQ(delegate.AddTestAction(extraAction), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestActionsCluster, TestEndpointListConstraints)
{
    // Test 1: Endpoint list name length constraint
    TestActionsDelegateImpl delegate = *sActionsDelegateImpl;
    delegate.mNumEndpointLists       = 0;
    char longName[kEndpointListNameMaxSize + 10];
    memset(longName, 'B', sizeof(longName));
    longName[sizeof(longName) - 1] = '\0';

    const EndpointId endpoints[] = { 1, 2, 3 };
    EndpointListStorage epListWithLongName(1, CharSpan::fromCharString(longName), EndpointListTypeEnum::kOther,
                                           DataModel::List<const EndpointId>(endpoints));

    // Add endpoint list and verify it was added successfully
    EXPECT_EQ(delegate.AddTestEndpointList(epListWithLongName), CHIP_NO_ERROR);

    // Verify the name was truncated by reading it back
    EndpointListStorage readEpList;
    EXPECT_EQ(delegate.ReadEndpointListAtIndex(0, readEpList), CHIP_NO_ERROR);
    EXPECT_EQ(readEpList.name.size(), kEndpointListNameMaxSize);

    // Test 2: Maximum endpoint list size
    delegate.mNumEndpointLists = 0;
    for (uint16_t i = 0; i < delegate.kMaxEndpointLists; i++)
    {
        EndpointListStorage epList(i, CharSpan::fromCharString("List"), EndpointListTypeEnum::kOther,
                                   DataModel::List<const EndpointId>(endpoints));
        EXPECT_EQ(delegate.AddTestEndpointList(epList), CHIP_NO_ERROR);
    }

    // Try to add one more endpoint list beyond the limit
    EndpointListStorage extraEpList(99, CharSpan::fromCharString("Extra"), EndpointListTypeEnum::kOther,
                                    DataModel::List<const EndpointId>(endpoints));
    EXPECT_EQ(delegate.AddTestEndpointList(extraEpList), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test 3: Maximum endpoints per list
    delegate.mNumEndpointLists = 0;
    EndpointId tooManyEndpoints[kEndpointListMaxSize + 5];
    for (uint16_t i = 0; i < kEndpointListMaxSize + 5; i++)
    {
        tooManyEndpoints[i] = i;
    }

    EndpointListStorage epListWithTooManyEndpoints(1, CharSpan::fromCharString("List"), EndpointListTypeEnum::kOther,
                                                   DataModel::List<const EndpointId>(tooManyEndpoints));

    // The list should be added but truncated to kEndpointListMaxSize
    EXPECT_EQ(delegate.AddTestEndpointList(epListWithTooManyEndpoints), CHIP_NO_ERROR);

    // Verify the endpoint list was truncated
    EXPECT_EQ(delegate.ReadEndpointListAtIndex(0, readEpList), CHIP_NO_ERROR);
    EXPECT_EQ(readEpList.endpoints.size(), kEndpointListMaxSize);
}

TEST_F(TestActionsCluster, TestActionListAttributeAccess)
{
    TestActionsDelegateImpl * delegate = sActionsDelegateImpl;
    delegate->mNumActions              = 0;

    // Add test actions
    ActionStructStorage action1(1, CharSpan::fromCharString("FirstAction"), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                                ActionStateEnum::kInactive);
    ActionStructStorage action2(2, CharSpan::fromCharString("SecondAction"), ActionTypeEnum::kScene, 1, BitMask<CommandBits>(),
                                ActionStateEnum::kActive);

    EXPECT_EQ(delegate->AddTestAction(action1), CHIP_NO_ERROR);
    EXPECT_EQ(delegate->AddTestAction(action2), CHIP_NO_ERROR);

    // Test reading actions through attribute reader
    uint8_t buf[1024];

    // Create the TLV writer
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    builder.Init(&tlvWriter);

    ConcreteAttributePath path(1, Clusters::Actions::Id, Clusters::Actions::Attributes::ActionList::Id);
    ConcreteReadAttributePath readPath(path);
    chip::DataVersion dataVersion(0);
    Access::SubjectDescriptor subjectDescriptor;
    AttributeValueEncoder encoder(builder, subjectDescriptor, path, dataVersion);

    // Read the action list using the Actions cluster's Read function
    EXPECT_EQ(sActionsServer->Read(readPath, encoder), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buf);

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    reader.Next();
    reader.OpenContainer(attrReportsReader);

    attrReportsReader.Next();
    attrReportsReader.OpenContainer(attrReportReader);

    attrReportReader.Next();
    attrReportReader.OpenContainer(attrDataReader);

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    attrDataReader.Next();
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        attrDataReader.Next();
    }
    EXPECT_TRUE(IsContextTag(attrDataReader.GetTag()));
    EXPECT_EQ(TagNumFromTag(attrDataReader.GetTag()), 2u);

    Clusters::Actions::Attributes::ActionList::TypeInfo::DecodableType list;
    CHIP_ERROR err = list.Decode(attrDataReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = list.begin();
    EXPECT_TRUE(iter.Next());
    Clusters::Actions::Structs::ActionStruct::Type action = iter.GetValue();
    EXPECT_EQ(action.actionID, 1);
    EXPECT_TRUE(strncmp(action.name.data(), "FirstAction", action.name.size()) == 0);
    EXPECT_EQ(action.type, ActionTypeEnum::kScene);
    EXPECT_EQ(action.endpointListID, 0);
    EXPECT_EQ(action.supportedCommands.Raw(), 0);
    EXPECT_EQ(action.state, ActionStateEnum::kInactive);

    EXPECT_TRUE(iter.Next());
    action = iter.GetValue();
    EXPECT_EQ(action.actionID, 2);
    EXPECT_TRUE(strncmp(action.name.data(), "SecondAction", action.name.size()) == 0);
    EXPECT_EQ(action.type, ActionTypeEnum::kScene);
    EXPECT_EQ(action.endpointListID, 1);
    EXPECT_EQ(action.supportedCommands.Raw(), 0);
    EXPECT_EQ(action.state, ActionStateEnum::kActive);
}

TEST_F(TestActionsCluster, TestEndpointListAttributeAccess)
{
    TestActionsDelegateImpl * delegate = sActionsDelegateImpl;
    delegate->mNumEndpointLists        = 0;

    // Add test endpoint lists
    const EndpointId endpoints1[] = { 1, 2 };
    const EndpointId endpoints2[] = { 3, 4, 5 };

    EndpointListStorage epList1(1, CharSpan::fromCharString("FirstList"), EndpointListTypeEnum::kOther,
                                DataModel::List<const EndpointId>(endpoints1, 2));
    EndpointListStorage epList2(2, CharSpan::fromCharString("SecondList"), EndpointListTypeEnum::kOther,
                                DataModel::List<const EndpointId>(endpoints2, 3));

    EXPECT_EQ(delegate->AddTestEndpointList(epList1), CHIP_NO_ERROR);
    EXPECT_EQ(delegate->AddTestEndpointList(epList2), CHIP_NO_ERROR);

    // Test reading endpoint lists through attribute reader
    TLV::TLVWriter writer;
    uint8_t buf[1024];
    writer.Init(buf);

    // Create the builders
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    builder.Init(&tlvWriter);

    ConcreteAttributePath path(1, Clusters::Actions::Id, Clusters::Actions::Attributes::EndpointLists::Id);
    ConcreteReadAttributePath readPath(path);
    chip::DataVersion dataVersion(0);
    Access::SubjectDescriptor subjectDescriptor;
    AttributeValueEncoder encoder(builder, subjectDescriptor, path, dataVersion);

    // Read the endpoint lists using the Actions cluster's Read function
    EXPECT_EQ(sActionsServer->Read(path, encoder), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buf);

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    reader.Next();
    reader.OpenContainer(attrReportsReader);

    attrReportsReader.Next();
    attrReportsReader.OpenContainer(attrReportReader);

    attrReportReader.Next();
    attrReportReader.OpenContainer(attrDataReader);

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    attrDataReader.Next();
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        attrDataReader.Next();
    }
    EXPECT_TRUE(IsContextTag(attrDataReader.GetTag()));
    EXPECT_EQ(TagNumFromTag(attrDataReader.GetTag()), 2u);

    Clusters::Actions::Attributes::EndpointLists::TypeInfo::DecodableType list;
    CHIP_ERROR err = list.Decode(attrDataReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = list.begin();
    EXPECT_TRUE(iter.Next());
    Clusters::Actions::Structs::EndpointListStruct::DecodableType endpointList = iter.GetValue();
    EXPECT_EQ(endpointList.endpointListID, 1);
    EXPECT_TRUE(strncmp(endpointList.name.data(), "FirstList", endpointList.name.size()) == 0);
    EXPECT_EQ(endpointList.type, EndpointListTypeEnum::kOther);
    auto it   = endpointList.endpoints.begin();
    uint8_t i = 0;
    while (it.Next())
    {
        if (i < 2)
        {
            EXPECT_EQ(endpoints1[i++], it.GetValue());
        }
    }

    EXPECT_TRUE(iter.Next());
    endpointList = iter.GetValue();
    EXPECT_EQ(endpointList.endpointListID, 2);
    EXPECT_TRUE(strncmp(endpointList.name.data(), "SecondList", endpointList.name.size()) == 0);
    EXPECT_EQ(endpointList.type, EndpointListTypeEnum::kOther);
    it = endpointList.endpoints.begin();
    i  = 0;
    while (it.Next())
    {
        if (i < 3)
        {
            EXPECT_EQ(endpoints2[i++], it.GetValue());
        }
    }
}

} // namespace app
} // namespace chip
