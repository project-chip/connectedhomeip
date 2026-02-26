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

#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/groupcast/GroupcastCluster.h>
#include <app/clusters/groupcast/GroupcastLogic.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/tests/AppTestContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <clusters/Groupcast/Enums.h>
#include <clusters/Groupcast/Metadata.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>
#include <system/RAIIMockClock.h>

#include <array>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <set>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Testing;
using namespace chip::Credentials;
using namespace chip::app::Clusters::Groupcast;
using namespace chip::System;
using namespace chip::System::Clock::Literals;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
static constexpr size_t kMaxMembershipEndpoints = app::Clusters::GroupcastLogic::kMaxMembershipEndpoints;

template <typename DecodableListType>
CHIP_ERROR CountListElements(DecodableListType & list, size_t & count)
{
    count   = 0;
    auto it = list.begin();
    while (it.Next())
    {
        ++count;
    }
    // Return the iterator status to the caller for assertion
    return it.GetStatus();
}

chip::FabricIndex kTestFabricIndex = Testing::kTestFabricIndex;
class CustomDataModel : public EmptyProvider
{
public:
    // Override of the EmptyProvider to mock a large Endpoint list in the data model that will be used in the following tests.
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & builder) override
    {
        static constexpr size_t kEndpointCount                                       = 300;
        static const std::array<DataModel::EndpointEntry, kEndpointCount> kEndpoints = []() {
            std::array<DataModel::EndpointEntry, kEndpointCount> endpoints;

            for (size_t i = 0; i < kEndpointCount; i++)
            {
                endpoints[i] = DataModel::EndpointEntry{
                    .id                 = static_cast<EndpointId>(i + 1),
                    .parentId           = kInvalidEndpointId,
                    .compositionPattern = DataModel::EndpointCompositionPattern::kTree,
                };
            }

            return endpoints;
        }();

        return builder.ReferenceExisting(Span<const DataModel::EndpointEntry>(kEndpoints.data(), kEndpoints.size()));
    }
};

// initialize memory as ReadOnlyBufferBuilder may allocate
class TestGroupcastCluster : public AppContext
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        AppContext::SetUpTestSuite();
    }

    static void TearDownTestSuite()
    {
        AppContext::TearDownTestSuite();
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        mProvider.SetStorageDelegate(&mTestContext.StorageDelegate());
        mProvider.SetSessionKeystore(&mKeystore);
        ASSERT_EQ(mProvider.Init(), CHIP_NO_ERROR);

        // Replace the DataModel Provider in the ServerClusterContext provided to the cluster implementation
        // with our Mock DataModel Provider so we can test endpoints validations on JoinGroup command.
        ServerClusterContext context = mTestContext.Get();
        clusterContext               = std::make_unique<ServerClusterContext>(ServerClusterContext{
                          .provider           = customDataModel,
                          .storage            = context.storage,
                          .attributeStorage   = context.attributeStorage,
                          .interactionContext = context.interactionContext,
        });

        ASSERT_EQ(mSender.Startup(*clusterContext), CHIP_NO_ERROR);
        ASSERT_EQ(mListener.Startup(*clusterContext), CHIP_NO_ERROR);

        CHIP_ERROR err = mFabricHelper.SetUpTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&mProvider);
        DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
        AppContext::SetUp();
    }

    void TearDown() override
    {
        mSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        mListener.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        clusterContext.reset();
        Credentials::SetGroupDataProvider(nullptr);
        CHIP_ERROR err = mFabricHelper.TearDownTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        mProvider.Finish();
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        AppContext::TearDown();
    }

    void AssertStatus(std::optional<app::DataModel::ActionReturnStatus> & status,
                      const Protocols::InteractionModel::Status expected)
    {
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  expected);
    }

    TestServerClusterContext mTestContext;
    Credentials::GroupDataProviderImpl mProvider;
    Crypto::DefaultSessionKeystore mKeystore;
    CustomDataModel customDataModel;
    std::unique_ptr<ServerClusterContext> clusterContext;
    FabricTestFixture mFabricHelper{ &mTestContext.StorageDelegate() };
    app::Clusters::GroupcastCluster mSender{ { mFabricHelper.GetFabricTable(), mProvider }, BitFlags<Feature>{ Feature::kSender } };
    app::Clusters::GroupcastCluster mListener{ { mFabricHelper.GetFabricTable(), mProvider },
                                               BitFlags<Feature>{ Feature::kListener } };
};

TEST_F(TestGroupcastCluster, TestAttributes)
{
    // Attributes
    {
        ASSERT_TRUE(IsAttributesListEqualTo(mSender,
                                            {
                                                Attributes::Membership::kMetadataEntry,
                                                Attributes::MaxMembershipCount::kMetadataEntry,
                                                Attributes::MaxMcastAddrCount::kMetadataEntry,
                                                Attributes::UsedMcastAddrCount::kMetadataEntry,
                                                Attributes::FabricUnderTest::kMetadataEntry,
                                            }));
    }

    // Read attributes for expected values
    {
        chip::Testing::ClusterTester tester(mSender);
        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
        ASSERT_EQ(revision, app::Clusters::Groupcast::kRevision);

        // Validate Constructor sets features correctly and is readable from attribute
        uint32_t features{};
        ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
        ASSERT_EQ(features, to_underlying(Feature::kSender));
    }
}

void ValidateMembership(const Attributes::Membership::TypeInfo::DecodableType & memberships,
                        const Clusters::Groupcast::Structs::MembershipStruct::Type * expectedMemberships,
                        size_t expectedMembershipsCount)
{
    size_t membershipCount = 0;
    ASSERT_EQ(CountListElements(memberships, membershipCount), CHIP_NO_ERROR);
    ASSERT_EQ(membershipCount, expectedMembershipsCount);

    size_t index        = 0;
    auto iterMembership = memberships.begin();
    while (iterMembership.Next() && (index < expectedMembershipsCount))
    {
        auto membership = iterMembership.GetValue();
        ASSERT_EQ(membership.groupID, expectedMemberships[index].groupID);
        ASSERT_EQ(membership.keySetID, expectedMemberships[index].keySetID);
        ASSERT_EQ(membership.mcastAddrPolicy, expectedMemberships[index].mcastAddrPolicy);
        ASSERT_EQ(membership.hasAuxiliaryACL.HasValue(), expectedMemberships[index].hasAuxiliaryACL.HasValue());
        if (expectedMemberships[index].hasAuxiliaryACL.HasValue())
        {
            ASSERT_EQ(membership.hasAuxiliaryACL, expectedMemberships[index].hasAuxiliaryACL);
        }

        ASSERT_EQ(membership.endpoints.HasValue(), expectedMemberships[index].endpoints.HasValue());
        if (membership.endpoints.HasValue())
        {
            size_t endpoint_count = 0;
            ASSERT_EQ(membership.endpoints.Value().ComputeSize(&endpoint_count), CHIP_NO_ERROR);
            ASSERT_EQ(endpoint_count, expectedMemberships[index].endpoints.Value().size());
            // Build set from expected endpoints
            std::set<EndpointId> expectedEndpoints;
            for (size_t i = 0; i < expectedMemberships[index].endpoints.Value().size(); i++)
            {
                expectedEndpoints.insert(expectedMemberships[index].endpoints.Value()[i]);
            }
            // Check each actual endpoint is in the expected set
            auto iterEndpoints = membership.endpoints.Value().begin();
            while (iterEndpoints.Next())
            {
                auto endpoint = iterEndpoints.GetValue();
                ASSERT_NE(expectedEndpoints.find(endpoint), expectedEndpoints.end());
            }
        }
        index++;
    }
    ASSERT_EQ(index, membershipCount);
}

TEST_F(TestGroupcastCluster, TestAcceptedCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mListener,
                                              {
                                                  Commands::JoinGroup::kMetadataEntry,
                                                  Commands::LeaveGroup::kMetadataEntry,
                                                  Commands::UpdateGroupKey::kMetadataEntry,
                                                  Commands::ConfigureAuxiliaryACL::kMetadataEntry,
                                                  Commands::GroupcastTesting::kMetadataEntry,
                                              }));
}

TEST_F(TestGroupcastCluster, TestReadMembership)
{
    static constexpr uint16_t kMaxEndpoints   = app::Clusters::GroupcastLogic::kMaxCommandEndpoints;
    static constexpr uint16_t kIntervals      = 15;
    static constexpr uint16_t kTotalEndpoints = kMaxEndpoints * kIntervals;
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[kIntervals][kMaxEndpoints] = {
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
        { 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 },
        { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60 },
        { 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80 },
        { 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100 },
        { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120 },
        { 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140 },
        { 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160 },
        { 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180 },
        { 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200 },
        { 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220 },
        { 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240 },
        { 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260 },
        { 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280 },
        { 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300 }
    };
    GroupId kGroup1   = 0xab01;
    GroupId kGroup2   = 0xcd02;
    GroupId kGroup3   = 0xef03;
    KeysetId kKeyset1 = 0xabcd;
    KeysetId kKeyset2 = 0xcafe;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join groups
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = kKeyset1;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            ASSERT_TRUE(result.status.has_value());
            EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                      Protocols::InteractionModel::Status::Success);
        }
        // Join group 2
        data.groupID         = kGroup2;
        data.useAuxiliaryACL = MakeOptional(false);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        for (int i = 0; i < 2; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            ASSERT_TRUE(result.status.has_value());
            EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                      Protocols::InteractionModel::Status::Success);
        }

        // Join group 3
        data.groupID         = kGroup3;
        data.keySetID        = kKeyset2;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(false);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[4], 8);
        result               = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Remove keyset used by Group 2
    EXPECT_EQ(CHIP_NO_ERROR, mProvider.RemoveKeySet(kTestFabricIndex, kKeyset2));

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        // Build expected endpoint arrays
        // Group1 chunk 1: first kMaxMembershipEndpoints endpoints (intervals 0-12, plus 15 from interval 13 = 255 endpoints)
        EndpointId group1_chunk1[kMaxMembershipEndpoints];
        for (size_t i = 0; i < kMaxMembershipEndpoints; i++)
        {
            group1_chunk1[i] = static_cast<EndpointId>(i + 1);
        }

        // Group1 chunk 2: remaining 45 endpoints (last 5 from interval 13, plus all 20 from interval 14)
        EndpointId group1_chunk2[kTotalEndpoints - kMaxMembershipEndpoints];
        for (size_t i = 0; i < MATTER_ARRAY_SIZE(group1_chunk2); i++)
        {
            group1_chunk2[i] = static_cast<EndpointId>(kMaxMembershipEndpoints + i + 1);
        }

        // Group2: 40 endpoints from intervals 1-2
        EndpointId group2_endpoints[2 * kMaxEndpoints];
        for (size_t i = 0; i < 2 * kMaxEndpoints; i++)
        {
            group2_endpoints[i] = static_cast<EndpointId>(i + 1);
        }

        // Group3: 8 endpoints from interval 4
        EndpointId group3_endpoints[8];
        for (size_t i = 0; i < 8; i++)
        {
            group3_endpoints[i] = static_cast<EndpointId>(4 * kMaxEndpoints + i + 1);
        }

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(group1_chunk1, kMaxMembershipEndpoints)),
                .keySetID        = kKeyset1,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(group1_chunk2, MATTER_ARRAY_SIZE(group1_chunk2))),
                .keySetID        = kKeyset1,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID   = kGroup2,
                .endpoints = MakeOptional(DataModel::List<const EndpointId>(group2_endpoints, MATTER_ARRAY_SIZE(group2_endpoints))),
                .keySetID  = kKeyset1,
                .hasAuxiliaryACL = MakeOptional(false),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup,
            },
            {
                .groupID   = kGroup3,
                .endpoints = MakeOptional(DataModel::List<const EndpointId>(group3_endpoints, MATTER_ARRAY_SIZE(group3_endpoints))),
                .keySetID  = kInvalidKeysetId,
                .hasAuxiliaryACL = MakeOptional(false),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }
}

TEST_F(TestGroupcastCluster, TestReadUsedMcastAddrCount)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    GroupId kGroup1               = 0xab01;
    GroupId kGroup2               = 0xcd02;
    GroupId kGroup3               = 0xef03;
    GroupId kGroup4               = 0xff04;
    KeysetId kKeyset              = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::TypeInfo::DecodableType multicastAddrCount;
    app::ConcreteAttributePath membershipAttributePath(kRootEndpointId, app::Clusters::Groupcast::Id,
                                                       app::Clusters::Groupcast::Attributes::Membership::Id);
    app::ConcreteAttributePath usedMcastAddrCountAttributePath(kRootEndpointId, app::Clusters::Groupcast::Id,
                                                               app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id);
    ASSERT_FALSE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
    ASSERT_FALSE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
    // Read UsedMcastAddrCount
    ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
              CHIP_NO_ERROR);
    ASSERT_EQ(multicastAddrCount, 0u);

    // Join groups
    {
        // Group 1 (IanaAddr)
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = kKeyset;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 1u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 2 (PerGroup)
        data.groupID = kGroup2;
        data.key.ClearValue();
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        result               = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 2u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 3 (PerGroup)
        data.groupID = kGroup3;
        result       = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 3u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 4 (IanaAddr)
        data.groupID         = kGroup4;
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        result               = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_FALSE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 3u);
        mTestContext.ChangeListener().DirtyList().clear();
    }

    // Leave groups
    {
        // Group 2 (PerGroup)
        Commands::LeaveGroup::Type data;
        data.groupID = kGroup2;
        auto result  = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 2u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 1 (IanaAddr)
        data.groupID = kGroup1;
        result       = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_FALSE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 2u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 3 (PerGroup)
        data.groupID = kGroup3;
        result       = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 1u);
        mTestContext.ChangeListener().DirtyList().clear();

        // Group 4 (IanaAddr)
        data.groupID = kGroup4;
        result       = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 0u);
        mTestContext.ChangeListener().DirtyList().clear();
    }
}

TEST_F(TestGroupcastCluster, TestJoinGroupCommand)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };
    const KeysetId kKeyset        = 0xabcd;

    Commands::JoinGroup::Type data;
    data.groupID         = 1;
    data.keySetID        = kKeyset;
    data.key             = MakeOptional(ByteSpan(key));
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

    // Neither Listener, nor Sender
    {
        app::Clusters::GroupcastCluster cluster({ mFabricHelper.GetFabricTable(), mProvider });
        chip::Testing::ClusterTester tester(cluster);
        tester.SetFabricIndex(kTestFabricIndex);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);
    }

    // Listener
    {
        chip::Testing::ClusterTester tester(mListener);
        tester.SetFabricIndex(kTestFabricIndex);

        // Join group: New keyset and key
        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Join group: Existing keyset and key (invalid)
        data.groupID = 2;
        result       = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::AlreadyExists);

        // Join group: Existing keyset but no key
        data.groupID = 2;
        data.key.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Join group: Existing keyset but no key
        data.groupID = 2;
        data.key.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Join group with root endpoint: Invalid Endpoint
        const EndpointId kRootEndpoint[] = { kRootEndpointId };
        data.groupID                     = 3;
        data.endpoints                   = DataModel::List<const EndpointId>(kRootEndpoint, MATTER_ARRAY_SIZE(kRootEndpoint));
        result                           = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::UnsupportedEndpoint);

        // Join group with an invalid endpoint in the data model
        const EndpointId kInvalidEndpoint[] = { 301 };
        data.groupID                        = 3;
        data.endpoints = DataModel::List<const EndpointId>(kInvalidEndpoint, MATTER_ARRAY_SIZE(kInvalidEndpoint));
        result         = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::UnsupportedEndpoint);
    }

    // Sender
    {
        chip::Testing::ClusterTester tester(mSender);
        tester.SetFabricIndex(kTestFabricIndex);
        data.endpoints = DataModel::List<const EndpointId>();

        // Join group: UseAuxiliaryACL can't be set
        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);

        // Join group: UseAuxiliaryACL unset
        data.useAuxiliaryACL.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Join group: Non-empty endpoints
        data.groupID   = 3;
        data.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
        result         = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);
    }
}

TEST_F(TestGroupcastCluster, TestLeaveGroup)
{
    static constexpr uint16_t kMaxEndpoints   = app::Clusters::GroupcastLogic::kMaxCommandEndpoints;
    static constexpr uint16_t kIntervals      = 5;
    static constexpr uint16_t kTotalEndpoints = kMaxEndpoints * kIntervals;
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[kIntervals][kMaxEndpoints] = {
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
        { 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 },
        { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60 },
        { 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80 },
        { 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100 }
    };
    const EndpointId kLeaveEndpoints1[] = { 1, 23, 45, 56, 67, 78, 89, 100 };
    const EndpointId kLeaveEndpoints2[] = { 3, 6, 29, 42, 48, 66, 76, 91 };

    static const std::set<EndpointId> kRemoveSet1(std::begin(kLeaveEndpoints1), std::end(kLeaveEndpoints1));
    static const std::set<EndpointId> kRemoveSet2(std::begin(kLeaveEndpoints2), std::end(kLeaveEndpoints2));

    GroupId kGroup1  = 0xab01;
    GroupId kGroup2  = 0xcd02;
    GroupId kGroup3  = 0xef03;
    KeysetId kKeyset = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    EndpointId all_endpoints[kTotalEndpoints];
    for (size_t i = 0; i < kTotalEndpoints; i++)
    {
        all_endpoints[i] = static_cast<EndpointId>(i + 1);
    }

    // Join groups
    {
        // Group 1
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = kKeyset;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            ASSERT_TRUE(result.status.has_value());
            EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                      Protocols::InteractionModel::Status::Success);
        }
        // Group 2
        data.groupID         = kGroup2;
        data.useAuxiliaryACL = MakeOptional(false);
        for (int i = 0; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            ASSERT_TRUE(result.status.has_value());
            EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                      Protocols::InteractionModel::Status::Success);
        }
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        // Build expected endpoint arrays - both groups have all 100 endpoints
        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(all_endpoints, kTotalEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup2,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(all_endpoints, kTotalEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(false),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // LeaveGroup
    {
        Commands::LeaveGroup::Type data;

        // Update existing key (invalid)
        data.groupID   = kGroup1;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(kLeaveEndpoints1, MATTER_ARRAY_SIZE(kLeaveEndpoints1)));
        auto result    = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        // Build expected endpoint arrays
        // Group1: all endpoints except kLeaveEndpoints1 (100 - 8 = 92 endpoints)
        EndpointId group1_endpoints[kTotalEndpoints - MATTER_ARRAY_SIZE(kLeaveEndpoints1)];
        for (size_t i = 0, j = 0; (i < kTotalEndpoints) && (j < MATTER_ARRAY_SIZE(group1_endpoints)); i++)
        {
            EndpointId ep = static_cast<EndpointId>(i + 1);
            if (kRemoveSet1.find(ep) == kRemoveSet1.end())
            {
                group1_endpoints[j++] = ep;
            }
        }

        // Group2: all endpoints (100 endpoints)

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID   = kGroup1,
                .endpoints = MakeOptional(DataModel::List<const EndpointId>(group1_endpoints, MATTER_ARRAY_SIZE(group1_endpoints))),
                .keySetID  = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup2,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(all_endpoints, kTotalEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(false),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // LeaveGroup a List of endpoints from all groups
    {
        Commands::LeaveGroup::Type data;

        data.groupID   = 0;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(kLeaveEndpoints2, MATTER_ARRAY_SIZE(kLeaveEndpoints2)));
        auto result    = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        // Build expected endpoint arrays
        // Group1: all endpoints except kLeaveEndpoints1 and kLeaveEndpoints2 (100 - 8 - 8 = 84 endpoints)
        EndpointId group1_endpoints[kTotalEndpoints - MATTER_ARRAY_SIZE(kLeaveEndpoints1) - MATTER_ARRAY_SIZE(kLeaveEndpoints2)];
        for (size_t i = 0, j = 0; (i < kTotalEndpoints) && (j < MATTER_ARRAY_SIZE(group1_endpoints)); i++)
        {
            EndpointId ep = static_cast<EndpointId>(i + 1);
            if (kRemoveSet1.find(ep) == kRemoveSet1.end() && kRemoveSet2.find(ep) == kRemoveSet2.end())
            {
                group1_endpoints[j++] = ep;
            }
        }

        // Group2: all endpoints except kLeaveEndpoints2 (100 - 8 = 92 endpoints)
        EndpointId group2_endpoints[kTotalEndpoints - MATTER_ARRAY_SIZE(kLeaveEndpoints2)];
        for (size_t i = 0, j = 0; (i < kTotalEndpoints) && (j < MATTER_ARRAY_SIZE(group2_endpoints)); i++)
        {
            EndpointId ep = static_cast<EndpointId>(i + 1);
            if (kRemoveSet2.find(ep) == kRemoveSet2.end())
            {
                group2_endpoints[j++] = ep;
            }
        }

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID   = kGroup1,
                .endpoints = MakeOptional(DataModel::List<const EndpointId>(group1_endpoints, MATTER_ARRAY_SIZE(group1_endpoints))),
                .keySetID  = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID   = kGroup2,
                .endpoints = MakeOptional(DataModel::List<const EndpointId>(group2_endpoints, MATTER_ARRAY_SIZE(group2_endpoints))),
                .keySetID  = kKeyset,
                .hasAuxiliaryACL = MakeOptional(false),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // LeaveGroup all groups completely.
    {
        Commands::LeaveGroup::Type data;
        data.groupID = 0;
        auto result  = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        // After leaving all groups, membership should be empty
        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[0] = {};
        ValidateMembership(memberships, expectedMembership, 0);
    }

    // JoinGroup for GroupID 1 and then GroupID 2 with the same endpoint list.
    {
        // JoinGroup for GroupID 1
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints);
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(true);
        auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        // JoinGroup for GroupID 2
        data.groupID = kGroup2;
        result       = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Read Membership
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup2,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // LeaveGroup for GroupID 2 without providing any endpoints
    {
        Commands::LeaveGroup::Type data;
        data.groupID = kGroup2;
        data.endpoints.ClearValue();
        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Read Membership
        Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = { {
            .groupID         = kGroup1,
            .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints)),
            .keySetID        = kKeyset,
            .hasAuxiliaryACL = MakeOptional(true),
            .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
        } };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // Create a Listener and Sender capable group with 1 endpoint.
    // Remove the endpoint from the group. Verify that the group still exists for Sender.
    app::Clusters::GroupcastCluster ListenerAndSender{ { mFabricHelper.GetFabricTable(), mProvider },
                                                       BitFlags<Feature>{ Feature::kListener, Feature::kSender } };
    ASSERT_EQ(ListenerAndSender.Startup(*clusterContext), CHIP_NO_ERROR);
    chip::Testing::ClusterTester listenerAndSendertester(ListenerAndSender);
    listenerAndSendertester.SetFabricIndex(kTestFabricIndex);
    {
        // JoinGroup for GroupID 3
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup3;
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[0], 1);
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(true);
        auto result          = listenerAndSendertester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Read Membership
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(listenerAndSendertester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup3,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], 1)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    {
        // LeaveGroup for GroupID 3
        Commands::LeaveGroup::Type data;
        data.groupID   = kGroup3;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], 1));
        auto result    = listenerAndSendertester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(listenerAndSendertester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints)),
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID   = kGroup3,
                .endpoints = MakeOptional(
                    DataModel::List<const chip::EndpointId>()), // Listener is supported, so an empty endpoints list is expected.
                .keySetID        = kKeyset,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }
    ListenerAndSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestGroupcastCluster, TestUpdateGroupKey)
{
    const uint8_t key1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const uint8_t key2[] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
    const EndpointId kEndpoints[] = { 1 };
    GroupId kGroup1               = 0xab01;
    GroupId kGroup2               = 0xcd02;
    const KeysetId kKeyset1       = 0xabcd;
    const KeysetId kKeyset2       = 0xcafe;
    const KeysetId kKeyset3       = 0xface;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join groups
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = kKeyset1;
        data.key             = MakeOptional(ByteSpan(key1));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        data.groupID  = kGroup2;
        data.keySetID = kKeyset2;
        data.key      = MakeOptional(ByteSpan(key2));
        result        = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
                .keySetID        = kKeyset1,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup2,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
                .keySetID        = kKeyset2,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // Update
    {
        Commands::UpdateGroupKey::Type data;

        // Update existing key (invalid)
        data.groupID  = kGroup2;
        data.keySetID = kKeyset1;
        data.key      = MakeOptional(ByteSpan(key1));
        auto result   = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::AlreadyExists);

        // Update to non-existing keyset (invalid)
        data.groupID  = kGroup2;
        data.keySetID = kKeyset3;
        data.key.ClearValue();
        result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::NotFound);

        // Update without key (always valid)
        data.groupID  = kGroup2;
        data.keySetID = kKeyset1;
        data.key.ClearValue();
        result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        // Create a new key (valid, if i <= mProvider.GetMaxGroupKeysPerFabric())
        // 2 keysets already in use
        data.key = MakeOptional(ByteSpan(key1));
        for (uint16_t i = 3; i <= mProvider.GetMaxGroupKeysPerFabric() + 1; ++i)
        {
            data.keySetID = kKeyset2 + i;
            result        = tester.Invoke(Commands::UpdateGroupKey::Id, data);
            ASSERT_TRUE(result.status.has_value());
            EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                      i > mProvider.GetMaxGroupKeysPerFabric() ? Protocols::InteractionModel::Status::ResourceExhausted
                                                               : Protocols::InteractionModel::Status::Success);
        }
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = {
            {
                .groupID         = kGroup1,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
                .keySetID        = kKeyset1,
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            },
            {
                .groupID         = kGroup2,
                .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
                .keySetID        = static_cast<KeysetId>(kKeyset2 + mProvider.GetMaxGroupKeysPerFabric()),
                .hasAuxiliaryACL = MakeOptional(true),
                .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
            }
        };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }
}

TEST_F(TestGroupcastCluster, TestConfigureAuxiliaryACL)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };
    const GroupId kGroupId        = 0xcafe;
    const KeysetId kKeyset        = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join group
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupId;
        data.keySetID        = kKeyset;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(false);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = { {
            .groupID         = kGroupId,
            .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
            .keySetID        = kKeyset,
            .hasAuxiliaryACL = MakeOptional(false),
            .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
        } };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // Update Sender (false to true), invalid
    {
        chip::Testing::ClusterTester sender_tester(mSender);
        sender_tester.SetFabricIndex(kTestFabricIndex);

        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = true;

        auto result = sender_tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);
    }

    // Update (false to true)
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = true;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = { {
            .groupID         = kGroupId,
            .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
            .keySetID        = kKeyset,
            .hasAuxiliaryACL = MakeOptional(true),
            .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
        } };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }

    // Update (true to false)
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = false;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership[] = { {
            .groupID         = kGroupId,
            .endpoints       = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints))),
            .keySetID        = kKeyset,
            .hasAuxiliaryACL = MakeOptional(false),
            .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
        } };

        ValidateMembership(memberships, expectedMembership, MATTER_ARRAY_SIZE(expectedMembership));
    }
}

TEST_F(TestGroupcastCluster, TestGroupcastTestingCommand)
{
    ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Default should be "no fabric under test"
    FabricIndex fabricUnderTest = kUndefinedFabricIndex;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
    EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);

    // Test invalid duration
    {
        Commands::GroupcastTesting::Type data;
        data.testOperation = GroupcastTestingEnum::kEnableListenerTesting;
        // Too small
        data.durationSeconds = MakeOptional(static_cast<uint16_t>(9));

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);

        // Command failed; should not have changed.
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);

        // Too large
        data.durationSeconds = MakeOptional(static_cast<uint16_t>(1201));
        result               = tester.Invoke(Commands::GroupcastTesting::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::ConstraintError);

        // Command failed; should not have changed from enabled state.
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable testing (no duration)
    {
        Commands::GroupcastTesting::Type data;
        data.testOperation = GroupcastTestingEnum::kEnableListenerTesting;

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);
    }

    // Disable testing should clear the fabric under test
    {
        Commands::GroupcastTesting::Type data;
        data.testOperation = GroupcastTestingEnum::kDisableTesting;

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable Listener Testing with duration
    {
        chip::System::Clock::Internal::RAIIMockClock mockClock;
        const uint16_t durationSeconds = 10;

        Commands::GroupcastTesting::Type data;
        data.testOperation   = GroupcastTestingEnum::kEnableListenerTesting;
        data.durationSeconds = MakeOptional(durationSeconds);

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);

        //  Testing should still be active
        mockClock.AdvanceMonotonic(Clock::Seconds16(durationSeconds - 1));
        GetIOContext().DriveIO();
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);

        //  Testing should end after the duration
        mockClock.AdvanceMonotonic(Clock::Seconds16(durationSeconds + 1));
        GetIOContext().DriveIO();
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable Sender Testing with duration
}

} // namespace
