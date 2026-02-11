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
#include <clusters/Groupcast/Enums.h>
#include <clusters/Groupcast/Metadata.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::Testing;
using namespace chip::Credentials;
using namespace chip::app::Clusters::Groupcast;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

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

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestGroupcastCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext mTestContext;
    Credentials::GroupDataProviderImpl mProvider;
    Crypto::DefaultSessionKeystore mKeystore;
    FabricTestFixture mFabricHelper{ &mTestContext.StorageDelegate() };
    app::Clusters::GroupcastCluster mSender{ { mFabricHelper.GetFabricTable(), mProvider }, BitFlags<Feature>{ Feature::kSender } };
    app::Clusters::GroupcastCluster mListener{ { mFabricHelper.GetFabricTable(), mProvider },
                                               BitFlags<Feature>{ Feature::kListener } };

    void SetUp() override
    {
        mProvider.SetStorageDelegate(&mTestContext.StorageDelegate());
        mProvider.SetSessionKeystore(&mKeystore);
        ASSERT_EQ(mProvider.Init(), CHIP_NO_ERROR);

        ASSERT_EQ(mSender.Startup(mTestContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(mListener.Startup(mTestContext.Get()), CHIP_NO_ERROR);

        CHIP_ERROR err = mFabricHelper.SetUpTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&mProvider);
    }

    void TearDown() override
    {
        mSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        mListener.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        Credentials::SetGroupDataProvider(nullptr);
        CHIP_ERROR err = mFabricHelper.TearDownTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        mProvider.Finish();
    }

    void AssertStatus(std::optional<app::DataModel::ActionReturnStatus> & status,
                      const Protocols::InteractionModel::Status expected)
    {
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  expected);
    }
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

TEST_F(TestGroupcastCluster, TestAcceptedCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mListener,
                                              {
                                                  Commands::JoinGroup::kMetadataEntry,
                                                  Commands::LeaveGroup::kMetadataEntry,
                                                  Commands::UpdateGroupKey::kMetadataEntry,
                                                  Commands::ConfigureAuxiliaryACL::kMetadataEntry,
                                              }));
}

TEST_F(TestGroupcastCluster, TestReadMembership)
{
    static constexpr uint16_t kMaxEndpoints = app::Clusters::GroupcastLogic::kMaxCommandEndpoints;
    static constexpr uint16_t kIntervals    = 15;
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
    GroupId kGroup1 = 0xab01;
    GroupId kGroup2 = 0xcd02;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join groups
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = 0xabcd;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = chip::app::DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
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
            data.endpoints = chip::app::DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
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

        size_t memershipCount = 0;
        ASSERT_EQ(CountListElements(memberships, memershipCount), CHIP_NO_ERROR);
        ASSERT_EQ(memershipCount, 3u); // Group1 [1..255], Group1 [256..300], Group2 [1..40]

        GroupId expected_groups[]          = { kGroup1, kGroup1, kGroup2 };
        GroupId expected_endpoint_counts[] = { 255, 45, 40 };
        GroupId prev_group                 = kGroup1;
        size_t i = 0, j = 0;
        auto iter = memberships.begin();
        while (iter.Next())
        {
            auto item             = iter.GetValue();
            size_t endpoint_count = 0;
            // Check group
            ASSERT_EQ(item.groupID, expected_groups[i]);
            ASSERT_TRUE(item.hasAuxiliaryACL.HasValue());
            ASSERT_EQ(item.hasAuxiliaryACL.Value(), item.groupID == kGroup1);
            ChipLogProgress(Crypto, "~~~ g:#%04x, a:%u, p:%u", (unsigned) item.groupID, (unsigned) item.hasAuxiliaryACL.Value(),
                            (unsigned) item.mcastAddrPolicy);
            ASSERT_EQ(item.mcastAddrPolicy,
                      item.groupID == kGroup1 ? app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr
                                              : app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
            // Check endpoint count
            ASSERT_TRUE(item.endpoints.HasValue());
            ASSERT_EQ(item.endpoints.Value().ComputeSize(&endpoint_count), CHIP_NO_ERROR);
            ASSERT_EQ(endpoint_count, expected_endpoint_counts[i]);
            // Check individual endpoints
            if (item.groupID != prev_group)
            {
                // Reset endpoint idex for the new group
                j = 0;
            }
            auto iter2 = item.endpoints.Value().begin();
            while (iter2.Next())
            {
                EndpointId endpoint_id = iter2.GetValue();
                EndpointId expected_id = kEndpoints[j / kMaxEndpoints][j % kMaxEndpoints];
                ASSERT_EQ(endpoint_id, expected_id);
                j++;
            }
            prev_group = item.groupID;
            i++;
        }
    }
}

TEST_F(TestGroupcastCluster, TestJoinGroupCommand)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };

    Commands::JoinGroup::Type data;
    data.groupID         = 1;
    data.keySetID        = 0xabcd;
    data.key             = MakeOptional(ByteSpan(key));
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

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
    }

    // Sender
    {
        chip::Testing::ClusterTester tester(mSender);
        tester.SetFabricIndex(kTestFabricIndex);
        data.endpoints = chip::app::DataModel::List<const EndpointId>();

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
        data.endpoints = chip::app::DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
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

    GroupId kGroup1 = 0xab01;
    GroupId kGroup2 = kGroup1 + 1;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join grojups
    {
        // Group 1
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = 0xabcd;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints[0], kMaxEndpoints);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = chip::app::DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
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
            data.endpoints = chip::app::DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
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

        size_t memershipCount = 0;
        ASSERT_EQ(CountListElements(memberships, memershipCount), CHIP_NO_ERROR);
        ASSERT_EQ(memershipCount, 2u); // Group1 [1..255], Group1 [256..300], Group2 [1..40]

        GroupId group_id = kGroup1;
        auto iter        = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, group_id);
            // Check the endpoints are still in the group
            size_t found          = 0;
            size_t endpoint_count = 0;
            // Check endpoint count
            ASSERT_TRUE(item.endpoints.HasValue());
            ASSERT_EQ(item.endpoints.Value().ComputeSize(&endpoint_count), CHIP_NO_ERROR);
            ASSERT_EQ(endpoint_count, kTotalEndpoints);
            // Check endpoints are stil in the group
            auto iter2 = item.endpoints.Value().begin();
            while (iter2.Next())
            {
                if (kRemoveSet1.find(iter2.GetValue()) != kRemoveSet1.end())
                {
                    found++;
                }
            }
            ASSERT_EQ(found, kRemoveSet1.size());
            group_id++;
        }
    }

    // LeaveGroup
    {
        Commands::LeaveGroup::Type data;

        // Update existing key (invalid)
        data.groupID = kGroup1;
        data.endpoints =
            MakeOptional(chip::app::DataModel::List<const EndpointId>(kLeaveEndpoints1, MATTER_ARRAY_SIZE(kLeaveEndpoints1)));
        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        size_t memershipCount = 0;
        ASSERT_EQ(CountListElements(memberships, memershipCount), CHIP_NO_ERROR);
        ASSERT_EQ(memershipCount, 2u); // Group1 [1..255], Group1 [256..300], Group2 [1..40]

        GroupId group_id = kGroup1;
        auto iter        = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, group_id);
            // Check the endpoints are still in the group
            size_t found          = 0;
            size_t endpoint_count = 0;
            // Check endpoint count

            size_t expected_count = (kGroup1 == group_id) ? kTotalEndpoints - kRemoveSet1.size() : kTotalEndpoints;
            ASSERT_TRUE(item.endpoints.HasValue());
            ASSERT_EQ(item.endpoints.Value().ComputeSize(&endpoint_count), CHIP_NO_ERROR);
            ASSERT_EQ(endpoint_count, expected_count);
            // Check endpoints are stil in the group
            auto iter2 = item.endpoints.Value().begin();
            while (iter2.Next())
            {
                if (kRemoveSet1.find(iter2.GetValue()) != kRemoveSet1.end())
                {
                    found++;
                }
            }
            // Endpoints removed from group 1, but not from group 2
            ASSERT_EQ(found, kGroup1 == group_id ? 0 : kRemoveSet1.size());
            group_id++;
        }
    }

    // LeaveGroup (all)
    {
        Commands::LeaveGroup::Type data;

        // Update existing key (invalid)
        data.groupID = 0;
        data.endpoints =
            MakeOptional(chip::app::DataModel::List<const EndpointId>(kLeaveEndpoints2, MATTER_ARRAY_SIZE(kLeaveEndpoints2)));
        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

        size_t memershipCount = 0;
        ASSERT_EQ(CountListElements(memberships, memershipCount), CHIP_NO_ERROR);
        ASSERT_EQ(memershipCount, 2u); // Group1 [1..255], Group1 [256..300], Group2 [1..40]

        GroupId group_id = kGroup1;
        auto iter        = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, group_id);
            // Check the endpoints are still in the group
            size_t found          = 0;
            size_t endpoint_count = 0;
            // Check endpoint count
            size_t expected_count = (kGroup1 == group_id) ? kTotalEndpoints - kRemoveSet1.size() - kRemoveSet2.size()
                                                          : kTotalEndpoints - kRemoveSet2.size();

            ASSERT_TRUE(item.endpoints.HasValue());
            ASSERT_EQ(item.endpoints.Value().ComputeSize(&endpoint_count), CHIP_NO_ERROR);
            ASSERT_EQ(endpoint_count, expected_count);
            // Check endpoints are stil in the group
            auto iter2 = item.endpoints.Value().begin();
            while (iter2.Next())
            {
                if (kRemoveSet2.find(iter2.GetValue()) != kRemoveSet2.end())
                {
                    found++;
                }
            }
            // Endpoints removed from both all groups
            ASSERT_EQ(found, static_cast<size_t>(0));
            group_id++;
        }
    }
}

TEST_F(TestGroupcastCluster, TestUpdateGroupKey)
{
    const uint8_t key1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const uint8_t key2[] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
    const EndpointId kEndpoints[] = { 1 };
    const KeysetId kKeyset1       = 0xabcd;
    const KeysetId kKeyset2       = 0xcafe;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);

    // Join groups
    {
        Commands::JoinGroup::Type data;
        data.groupID         = 1;
        data.keySetID        = kKeyset1;
        data.key             = MakeOptional(ByteSpan(key1));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        data.groupID  = 2;
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
        GroupId group_id = 1;
        auto iter        = memberships.begin();
        while (iter.Next())
        {
            auto item            = iter.GetValue();
            KeysetId expected_id = (1 == item.groupID) ? kKeyset1 : kKeyset2;
            ASSERT_EQ(item.groupID, group_id);
            ASSERT_EQ(item.keySetID, expected_id);
            group_id++;
        }
    }

    // Update
    {
        Commands::UpdateGroupKey::Type data;

        // Update existing key (invalid)
        data.groupID  = 2;
        data.keySetID = kKeyset1;
        data.key      = MakeOptional(ByteSpan(key1));

        auto result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::AlreadyExists);

        // Update existing keyset (valid)
        data.keySetID = kKeyset1;
        data.key.ClearValue();
        result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
        GroupId group_id = 1;
        auto iter        = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, group_id);
            ASSERT_EQ(item.keySetID, kKeyset1);
            group_id++;
        }
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
        data.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.status.has_value());
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
    }

    // Read Membership
    {
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
        auto iter = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, kGroupId);
            ASSERT_EQ(item.keySetID, kKeyset);
            ASSERT_TRUE(item.hasAuxiliaryACL.HasValue());
            ASSERT_FALSE(item.hasAuxiliaryACL.Value());
        }
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
        auto iter = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, kGroupId);
            ASSERT_EQ(item.keySetID, kKeyset);
            ASSERT_TRUE(item.hasAuxiliaryACL.HasValue());
            ASSERT_TRUE(item.hasAuxiliaryACL.Value());
        }
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
        auto iter = memberships.begin();
        while (iter.Next())
        {
            auto item = iter.GetValue();
            ASSERT_EQ(item.groupID, kGroupId);
            ASSERT_EQ(item.keySetID, kKeyset);
            ASSERT_TRUE(item.hasAuxiliaryACL.HasValue());
            ASSERT_FALSE(item.hasAuxiliaryACL.Value());
        }
    }
}

} // namespace
