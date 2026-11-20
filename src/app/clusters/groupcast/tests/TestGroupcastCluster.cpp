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
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <access/AccessControl.h>
#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/groupcast/GroupcastCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <clusters/Groupcast/Enums.h>
#include <clusters/Groupcast/Metadata.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/NetworkCommissioning.h>

#include <array>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <cstring>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <optional>
#include <set>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Access;
using namespace chip::Testing;
using namespace chip::Credentials;
using namespace chip::app::Clusters::Groupcast;
using namespace chip::System::Clock::Literals;
using namespace chip::Protocols::InteractionModel;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
static constexpr size_t kMaxMembershipEndpoints = app::Clusters::GroupcastCluster::kMaxMembershipEndpoints;

static constexpr FabricIndex kTestFabricIndex = Testing::kTestFabricIndex;
static constexpr NodeId kNodeIdForAdmin       = 1ULL;
static constexpr NodeId kNodeIdForManage      = 2ULL;

static constexpr SubjectDescriptor kAdminSubjectDescriptor{ .fabricIndex = kTestFabricIndex,
                                                            .authMode    = AuthMode::kCase,
                                                            .subject     = kNodeIdForAdmin };

static constexpr SubjectDescriptor kManageSubjectDescriptor{ .fabricIndex = kTestFabricIndex,
                                                             .authMode    = AuthMode::kCase,
                                                             .subject     = kNodeIdForManage };

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
struct TestGroupcastCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    class TestDeviceTypeResolver : public Access::AccessControl::DeviceTypeResolver
    {
    public:
        bool IsDeviceTypeOnEndpoint(chip::DeviceTypeId deviceType, chip::EndpointId endpoint) override
        {
            (void) deviceType;
            (void) endpoint;
            return false;
        }
    };

    class GroupcastTestingAccessControlDelegate : public Access::AccessControl::Delegate
    {
        CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                         Privilege requestPrivilege) override
        {
            if (subjectDescriptor.authMode != AuthMode::kCase)
            {
                return CHIP_ERROR_ACCESS_DENIED;
            }

            switch (subjectDescriptor.subject)
            {
            case kNodeIdForAdmin:
                // Admin always has access
                return CHIP_NO_ERROR;
            case kNodeIdForManage: {
                // Manage can access everything but Administer-level things
                CHIP_ERROR err = (requestPrivilege == Access::Privilege::kAdminister) ? CHIP_ERROR_ACCESS_DENIED : CHIP_NO_ERROR;
                return err;
            }
            default:
                return CHIP_ERROR_ACCESS_DENIED;
            }
        }
    };

    void SetUp() override
    {
        mProvider.SetStorageDelegate(&mTestContext.StorageDelegate());
        mProvider.SetSessionKeystore(&mKeystore);
        mProvider.SetGroupcastEnabled(true);
        ASSERT_EQ(mProvider.Init(), CHIP_NO_ERROR);
        ASSERT_EQ(mAccessDelegate.Init(), CHIP_NO_ERROR);
        ASSERT_EQ(mAccessControl.Init(&mAccessDelegate, mDeviceTypeResolver), CHIP_NO_ERROR);

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

        CHIP_ERROR err = mFabricHelper.SetUpTestFabric(mFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        ASSERT_EQ(mFabricIndex, kTestFabricIndex);
        Credentials::SetGroupDataProvider(&mProvider);
    }

    void TearDown() override
    {
        mTestContext.StorageDelegate().ClearPoisonKeys();
        mAccessDelegate.Finish();
        mAccessControl.Finish();
        mSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        mListener.Shutdown(app::ClusterShutdownType::kClusterShutdown);
        clusterContext.reset();
        Credentials::SetGroupDataProvider(nullptr);
        CHIP_ERROR err = mFabricHelper.TearDownTestFabric(mFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        mProvider.Finish();
    }

    void AssertStatus(std::optional<app::DataModel::ActionReturnStatus> & status,
                      const Protocols::InteractionModel::Status expected)
    {
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(status->GetStatusCode().GetStatus(), expected);
    }

    // Makes every write and delete of the storage record fail, while reads still succeed.
    void FailWritesOf(const StorageKeyName & recordKey)
    {
        mTestContext.StorageDelegate().AddPoisonKey(recordKey.KeyName(), /* readPattern = */ -1, /* writePattern = */ 0);
    }

    FabricIndex mFabricIndex = kTestFabricIndex;
    TestServerClusterContext mTestContext;
    TestPersistentStorageDelegate mStorageDelegate;
    GroupcastTestingAccessControlDelegate mAccessDelegate;
    TestDeviceTypeResolver mDeviceTypeResolver;
    Access::AccessControl mAccessControl;
    Credentials::GroupDataProviderImpl mProvider;
    TimerDelegateMock mMockTimerDelegate;
    Crypto::DefaultSessionKeystore mKeystore;
    CustomDataModel customDataModel;
    std::unique_ptr<ServerClusterContext> clusterContext;
    FabricTestFixture mFabricHelper{ &mTestContext.StorageDelegate() };
    ScopedAttributeChangeListenerRegistration mScopedListenerRegistration; // RAII registration

    chip::Groupcast::Testing mTesting;
    app::Clusters::GroupcastCluster mSender{ { mFabricHelper.GetFabricTable(), mProvider, mMockTimerDelegate, mAccessControl,
                                               mTesting },
                                             BitFlags<Feature>{ Feature::kSender } };
    app::Clusters::GroupcastCluster mListener{ { mFabricHelper.GetFabricTable(), mProvider, mMockTimerDelegate, mAccessControl,
                                                 mTesting },
                                               BitFlags<Feature>{ Feature::kListener, Feature::kPerGroup } };

    TestGroupcastCluster() : mScopedListenerRegistration(customDataModel, mTestContext) {}
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

TEST_F(TestGroupcastCluster, TestJoinGroupFailsOnBadAargs)
{
    GroupId kGroup1     = 0xab01;
    KeysetId kKeyset1   = 0xabcd;
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[1] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Admin privilege, and valid arguments (including key) : JoinGroup should work.
    {
        Commands::JoinGroup::Type joinGroupCmd;
        joinGroupCmd.groupID         = kGroup1;
        joinGroupCmd.keySetID        = kKeyset1;
        joinGroupCmd.key             = MakeOptional(ByteSpan(key));
        joinGroupCmd.useAuxiliaryACL = NullOptional;
        joinGroupCmd.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        joinGroupCmd.endpoints       = DataModel::List<const EndpointId>(kEndpoints, 1);

        auto result = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::Success);
    }

    // Now group1 is OK and keys OK

    // Constraint on groupId >= 1 must be respected.
    {
        Commands::JoinGroup::Type joinGroupCmd;
        joinGroupCmd.groupID         = 0; // Disallowed
        joinGroupCmd.keySetID        = kKeyset1;
        joinGroupCmd.key             = MakeOptional(ByteSpan(key));
        joinGroupCmd.useAuxiliaryACL = NullOptional;
        joinGroupCmd.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        joinGroupCmd.endpoints       = DataModel::List<const EndpointId>(kEndpoints, 1);

        auto result = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);
    }

    // Constraint on keySetId >= 1 must be respected.
    {
        Commands::JoinGroup::Type joinGroupCmd;
        joinGroupCmd.groupID         = kGroup1;
        joinGroupCmd.keySetID        = 0; // Disallowed
        joinGroupCmd.key             = MakeOptional(ByteSpan(key));
        joinGroupCmd.useAuxiliaryACL = NullOptional;
        joinGroupCmd.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        joinGroupCmd.endpoints       = DataModel::List<const EndpointId>(kEndpoints, 1);

        auto result = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);
    }

    // Manage access: can't touch keys or useAuxiliary ACL
    tester.SetSubjectDescriptor(kManageSubjectDescriptor);

    // Enabling useAuxiliaryACL should fail unless subject has administer privileges.
    {
        Commands::JoinGroup::Type joinGroupCmd;
        joinGroupCmd.groupID         = kGroup1;
        joinGroupCmd.keySetID        = kKeyset1;
        joinGroupCmd.key             = NullOptional; // <-- Not touching key
        joinGroupCmd.useAuxiliaryACL = NullOptional; // <-- Not touching useAuxACL
        joinGroupCmd.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        joinGroupCmd.endpoints       = DataModel::List<const EndpointId>(kEndpoints, 1);

        // Manage should work here
        auto result = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::Success);

        // Touching useAuxiliaryACL, even if false should fail if present --> cannot update the field.
        joinGroupCmd.useAuxiliaryACL = MakeOptional(false);
        result                       = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::UnsupportedAccess);

        joinGroupCmd.useAuxiliaryACL = MakeOptional(true);
        result                       = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::UnsupportedAccess);

        // Starts working after no longer touching AuxACL.
        joinGroupCmd.key             = NullOptional; // <-- Not touching key
        joinGroupCmd.useAuxiliaryACL = NullOptional; // <-- Not touching useAuxACL
        result                       = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::Success);

        // If touching key, but not useAuxiliaryACL, still not legal.

        joinGroupCmd.key             = MakeOptional(ByteSpan(key)); // <-- Touching key
        joinGroupCmd.useAuxiliaryACL = NullOptional;                // <-- Not touching useAuxACL
        result                       = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
        ASSERT_TRUE(result.GetStatusCode().has_value());
        EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::UnsupportedAccess);
    }
}

TEST_F(TestGroupcastCluster, TestUpdateGroupKeyFailsOnBadAargs)
{
    GroupId kGroup1      = 0xab01;
    KeysetId kKeyset1    = 0xabcd;
    KeysetId kKeyset2    = 0xa123;
    const uint8_t key1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const uint8_t key2[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[1] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinGroupCmd;
    joinGroupCmd.groupID         = kGroup1;
    joinGroupCmd.keySetID        = kKeyset1;
    joinGroupCmd.key             = MakeOptional(ByteSpan(key1));
    joinGroupCmd.useAuxiliaryACL = MakeOptional(false);
    joinGroupCmd.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
    joinGroupCmd.endpoints       = DataModel::List<const EndpointId>(kEndpoints, 1);

    // Join must work
    auto result = tester.Invoke(Commands::JoinGroup::Id, joinGroupCmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    Commands::UpdateGroupKey::Type updateGroupKeyCmd;
    updateGroupKeyCmd.groupID  = kGroup1;
    updateGroupKeyCmd.keySetID = kKeyset1;

    // UpdateGroupKey from Admin access should succeed (no key update).
    result = tester.Invoke(Commands::UpdateGroupKey::Id, updateGroupKeyCmd);
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::Success);

    // If changing a key, should fail with Manage, succeed with Admin
    tester.SetSubjectDescriptor(kManageSubjectDescriptor);

    updateGroupKeyCmd.groupID  = kGroup1;
    updateGroupKeyCmd.keySetID = kKeyset2;
    updateGroupKeyCmd.key      = MakeOptional(ByteSpan(key2));

    result = tester.Invoke(Commands::UpdateGroupKey::Id, updateGroupKeyCmd);
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::UnsupportedAccess);

    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);
    result = tester.Invoke(Commands::UpdateGroupKey::Id, updateGroupKeyCmd);
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::Success);

    // Constraint on groupId >= 1 must be respected.
    updateGroupKeyCmd.groupID  = 0;
    updateGroupKeyCmd.keySetID = kKeyset1;
    updateGroupKeyCmd.key      = NullOptional;

    result = tester.Invoke(Commands::UpdateGroupKey::Id, updateGroupKeyCmd);
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);

    // Constraint on keySetId >= 1 must be respected.
    updateGroupKeyCmd.groupID  = kGroup1;
    updateGroupKeyCmd.keySetID = 0;
    updateGroupKeyCmd.key      = NullOptional;

    result = tester.Invoke(Commands::UpdateGroupKey::Id, updateGroupKeyCmd);
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);
}

TEST_F(TestGroupcastCluster, TestReadMembership)
{
    static constexpr uint16_t kMaxEndpoints   = app::Clusters::GroupcastCluster::kMaxCommandEndpoints;
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
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        }
        // Join group 2
        data.groupID         = kGroup2;
        data.useAuxiliaryACL = MakeOptional(false);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        for (int i = 0; i < 2; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        }

        // Join group 3
        data.groupID         = kGroup3;
        data.keySetID        = kKeyset2;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(false);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[4], 8);
        result               = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EndpointId group2_endpoints[2 * static_cast<size_t>(kMaxEndpoints)];
        for (size_t i = 0; i < 2 * static_cast<size_t>(kMaxEndpoints); i++)
        {
            group2_endpoints[i] = static_cast<EndpointId>(i + 1);
        }

        // Group3: 8 endpoints from interval 4
        EndpointId group3_endpoints[8];
        for (size_t i = 0; i < 8; i++)
        {
            group3_endpoints[i] = static_cast<EndpointId>(4 * static_cast<size_t>(kMaxEndpoints) + i + 1);
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

    constexpr System::Clock::Milliseconds32 kChangeTemporisation = System::Clock::Milliseconds32(251);
    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(kChangeTemporisation));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(membershipAttributePath));
        ASSERT_TRUE(mTestContext.ChangeListener().IsDirty(usedMcastAddrCountAttributePath));
        // Read UsedMcastAddrCount
        ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::UsedMcastAddrCount::Id, multicastAddrCount),
                  CHIP_NO_ERROR);
        ASSERT_EQ(multicastAddrCount, 0u);
        mTestContext.ChangeListener().DirtyList().clear();
    }
}

TEST_F(TestGroupcastCluster, TestMaxMcastAddrCount)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints1[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    const EndpointId kEndpoints2[] = { 101, 102 };
    KeysetId kKeyset               = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Read MaxMcastAddrCount
    app::Clusters::Groupcast::Attributes::MaxMcastAddrCount::TypeInfo::DecodableType maxMcastAddrCount{};
    ASSERT_EQ(tester.ReadAttribute(app::Clusters::Groupcast::Attributes::MaxMcastAddrCount::Id, maxMcastAddrCount), CHIP_NO_ERROR);
    ASSERT_GT(maxMcastAddrCount, 0u);

    // Group
    Commands::JoinGroup::Type data;
    data.groupID         = 1;
    data.keySetID        = kKeyset;
    data.key             = MakeOptional(ByteSpan(key));
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = DataModel::List<const EndpointId>(kEndpoints1, MATTER_ARRAY_SIZE(kEndpoints1));

    // Join MaxMcastAddrCount IANA address groups
    {
        for (GroupId i = 0; i < maxMcastAddrCount; i++)
        {
            data.groupID = i + 1;
            auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
            data.key.ClearValue();
        }
    }
    // Join MaxMcastAddrCount+1
    {
        data.groupID = maxMcastAddrCount + 1;
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }
    // Leave all groups
    {
        data.groupID = 0;
        auto result  = tester.Invoke(Commands::LeaveGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }
    // Join MaxMcastAddrCount PerGroup address groups
    {
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);

        for (GroupId i = 0; i < maxMcastAddrCount; i++)
        {
            data.groupID = i + 1;
            auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        }
    }
    // Join MaxMcastAddrCount+1
    {
        data.groupID = maxMcastAddrCount + 1;
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));
    }
    // Leave all groups
    {
        data.groupID = 0;
        auto result  = tester.Invoke(Commands::LeaveGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }
    // Join 1 IANA address plus (MaxMcastAddrCount - 1) PerGroup groups
    {
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);

        for (GroupId i = 0; i < maxMcastAddrCount; i++)
        {
            data.groupID         = i + 1;
            data.mcastAddrPolicy = MakeOptional((0 == i) ? app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr
                                                         : app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
            auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        }
    }
    // Join MaxMcastAddrCount+1
    {
        data.groupID         = maxMcastAddrCount + 1;
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }
    // Join MaxMcastAddrCount+2
    {
        data.groupID         = maxMcastAddrCount + 2;
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));
    }
    // Join MaxMcastAddrCount+1 (existing group)
    {
        data.groupID         = 2;
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints2, MATTER_ARRAY_SIZE(kEndpoints2));
        auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        app::Clusters::GroupcastCluster cluster(
            { mFabricHelper.GetFabricTable(), mProvider, mMockTimerDelegate, mAccessControl, mTesting });
        chip::Testing::ClusterTester tester(cluster);
        tester.SetFabricIndex(kTestFabricIndex);
        tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
    }

    // Listener basic flow
    {
        chip::Testing::ClusterTester tester(mListener);
        tester.SetFabricIndex(kTestFabricIndex);
        tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

        // Join group: New keyset and key
        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Per spec 11.27.7.1.4 (JoinGroup Command/Key Field), a keyset created via JoinGroup with a Key has exactly one epoch
        // key (num_keys_used == 1) with EpochStartTime0 = 1.
        {
            Credentials::GroupDataProvider::KeySet storedKeyset;
            EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kKeyset, storedKeyset), CHIP_NO_ERROR);
            EXPECT_EQ(storedKeyset.keyset_id, kKeyset);
            EXPECT_EQ(storedKeyset.policy, Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst);
            EXPECT_EQ(storedKeyset.num_keys_used, 1u);
            EXPECT_EQ(storedKeyset.epoch_keys[0].start_time, 1u);
        }

        // Join group: Existing keyset and key (invalid)
        data.groupID = 2;
        result       = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::AlreadyExists));

        // Join group: Existing keyset but no key
        data.groupID = 2;
        data.key.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group: Existing keyset but no key
        data.groupID = 2;
        data.key.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group with root endpoint: Invalid Endpoint
        const EndpointId kRootEndpoint[] = { kRootEndpointId };
        data.groupID                     = 3;
        data.endpoints                   = DataModel::List<const EndpointId>(kRootEndpoint, MATTER_ARRAY_SIZE(kRootEndpoint));
        result                           = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedEndpoint));

        // Join group with an invalid endpoint in the data model
        const EndpointId kInvalidEndpoint[] = { 301 };
        data.groupID                        = 3;
        data.endpoints = DataModel::List<const EndpointId>(kInvalidEndpoint, MATTER_ARRAY_SIZE(kInvalidEndpoint));
        result         = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedEndpoint));
    }

    // Sender
    {
        chip::Testing::ClusterTester tester(mSender);
        tester.SetFabricIndex(kTestFabricIndex);
        tester.SetSubjectDescriptor(kAdminSubjectDescriptor);
        data.endpoints = DataModel::List<const EndpointId>();

        // Join group: UseAuxiliaryACL can't be set
        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        // Join group: UseAuxiliaryACL unset
        data.useAuxiliaryACL.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group: ReplaceEndpoints
        data.replaceEndpoints = MakeOptional(true);
        result                = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        data.replaceEndpoints = MakeOptional(false);
        result                = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        data.replaceEndpoints.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group: McastAddrPolicy kPerGroup without kPerGroup feature set
        data.groupID         = 2;
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        result               = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        // Join group: McastAddrPolicy kIanaAddr without kPerGroup feature
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        data.key.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group: McastAddrPolicy absent without kPerGroup feature
        data.groupID = 3;
        data.mcastAddrPolicy.ClearValue();
        result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Join group: Non-empty endpoints
        data.groupID   = 3;
        data.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
        result         = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
    }
}

TEST_F(TestGroupcastCluster, TestReplaceEndpointsSucceedsProperly)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    GroupId kGroup1     = 0xab01;
    KeysetId kKeyset1   = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    EndpointId initialEndpoints[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Commands::JoinGroup::Type data;
    data.groupID         = kGroup1;
    data.keySetID        = kKeyset1;
    data.key             = MakeOptional(ByteSpan(key));
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = DataModel::List<const EndpointId>(initialEndpoints, MATTER_ARRAY_SIZE(initialEndpoints));

    auto result = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    // Read Membership
    app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
    ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

    Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership0[] = { {
        .groupID         = kGroup1,
        .endpoints       = MakeOptional(DataModel::List<const EndpointId>(initialEndpoints, MATTER_ARRAY_SIZE(initialEndpoints))),
        .keySetID        = kKeyset1,
        .hasAuxiliaryACL = MakeOptional(true),
        .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
    } };
    ValidateMembership(memberships, expectedMembership0, MATTER_ARRAY_SIZE(expectedMembership0));

    // Replace with 8 members
    EndpointId replacementEndpoints[8] = { 11, 12, 13, 14, 15, 16, 17, 18 };
    data.key.ClearValue();
    data.replaceEndpoints = MakeOptional(true);
    data.endpoints        = DataModel::List<const EndpointId>(replacementEndpoints, MATTER_ARRAY_SIZE(replacementEndpoints));

    result = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

    Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership1[] = { {
        .groupID   = kGroup1,
        .endpoints = MakeOptional(DataModel::List<const EndpointId>(replacementEndpoints, MATTER_ARRAY_SIZE(replacementEndpoints))),
        .keySetID  = kKeyset1,
        .hasAuxiliaryACL = MakeOptional(true),
        .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
    } };
    ValidateMembership(memberships, expectedMembership1, MATTER_ARRAY_SIZE(expectedMembership1));

    // Add 40 endpoints (requires 2 JoinGroup commands)
    EndpointId fortyEndpoints1[20];
    EndpointId fortyEndpoints2[20];
    for (uint16_t i = 0; i < 20; i++)
    {
        fortyEndpoints1[i] = static_cast<EndpointId>(100 + i);
        fortyEndpoints2[i] = static_cast<EndpointId>(120 + i);
    }

    data.replaceEndpoints = MakeOptional(true);
    data.endpoints        = DataModel::List<const EndpointId>(fortyEndpoints1, MATTER_ARRAY_SIZE(fortyEndpoints1));
    result                = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    data.replaceEndpoints = MakeOptional(false);
    data.endpoints        = DataModel::List<const EndpointId>(fortyEndpoints2, MATTER_ARRAY_SIZE(fortyEndpoints2));
    result                = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    EndpointId expectedForty[40];
    for (uint16_t i = 0; i < 40; i++)
    {
        expectedForty[i] = static_cast<EndpointId>(100 + i);
    }

    ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
    Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership2[] = { {
        .groupID         = kGroup1,
        .endpoints       = MakeOptional(DataModel::List<const EndpointId>(expectedForty, MATTER_ARRAY_SIZE(expectedForty))),
        .keySetID        = kKeyset1,
        .hasAuxiliaryACL = MakeOptional(true),
        .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
    } };
    ValidateMembership(memberships, expectedMembership2, MATTER_ARRAY_SIZE(expectedMembership2));

    // JoinGroup with replace endpoints that adds only 1 endpoint
    EndpointId oneEndpoint[1] = { 200 };
    data.replaceEndpoints     = MakeOptional(true);
    data.endpoints            = DataModel::List<const EndpointId>(oneEndpoint, MATTER_ARRAY_SIZE(oneEndpoint));
    result                    = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
    Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership3[] = { {
        .groupID         = kGroup1,
        .endpoints       = MakeOptional(DataModel::List<const EndpointId>(oneEndpoint, MATTER_ARRAY_SIZE(oneEndpoint))),
        .keySetID        = kKeyset1,
        .hasAuxiliaryACL = MakeOptional(true),
        .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
    } };
    ValidateMembership(memberships, expectedMembership3, MATTER_ARRAY_SIZE(expectedMembership3));

    // Add 40 more endpoints
    data.replaceEndpoints = MakeOptional(false);
    data.endpoints        = DataModel::List<const EndpointId>(fortyEndpoints1, MATTER_ARRAY_SIZE(fortyEndpoints1));
    result                = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    data.endpoints = DataModel::List<const EndpointId>(fortyEndpoints2, MATTER_ARRAY_SIZE(fortyEndpoints2));
    result         = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

    EndpointId expectedFortyOne[41];
    expectedFortyOne[0] = 200;
    for (uint16_t i = 0; i < 40; i++)
    {
        expectedFortyOne[i + 1] = static_cast<EndpointId>(100 + i);
    }

    ASSERT_EQ(tester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);
    Clusters::Groupcast::Structs::MembershipStruct::Type expectedMembership4[] = { {
        .groupID         = kGroup1,
        .endpoints       = MakeOptional(DataModel::List<const EndpointId>(expectedFortyOne, MATTER_ARRAY_SIZE(expectedFortyOne))),
        .keySetID        = kKeyset1,
        .hasAuxiliaryACL = MakeOptional(true),
        .mcastAddrPolicy = app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr,
    } };
    ValidateMembership(memberships, expectedMembership4, MATTER_ARRAY_SIZE(expectedMembership4));
}

TEST_F(TestGroupcastCluster, TestLeaveGroup)
{
    static constexpr uint16_t kMaxEndpoints   = app::Clusters::GroupcastCluster::kMaxCommandEndpoints;
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
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        data.key.ClearValue();
        for (int i = 1; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        }
        // Group 2
        data.groupID         = kGroup2;
        data.useAuxiliaryACL = MakeOptional(false);
        for (int i = 0; i < kIntervals; i++)
        {
            data.endpoints = DataModel::List<const EndpointId>(kEndpoints[i], kMaxEndpoints);
            result         = tester.Invoke(Commands::JoinGroup::Id, data);
            EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
        // JoinGroup for GroupID 2
        data.groupID = kGroup2;
        result       = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

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
    app::Clusters::GroupcastCluster ListenerAndSender{ { mFabricHelper.GetFabricTable(), mProvider, mMockTimerDelegate,
                                                         mAccessControl, mTesting },
                                                       BitFlags<Feature>{ Feature::kListener, Feature::kSender } };
    ASSERT_EQ(ListenerAndSender.Startup(*clusterContext), CHIP_NO_ERROR);
    chip::Testing::ClusterTester listenerAndSenderTester(ListenerAndSender);
    listenerAndSenderTester.SetFabricIndex(kTestFabricIndex);
    listenerAndSenderTester.SetSubjectDescriptor(kAdminSubjectDescriptor);
    {
        // JoinGroup for GroupID 3
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup3;
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints[0], 1);
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(true);
        auto result          = listenerAndSenderTester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Read Membership
        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(listenerAndSenderTester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

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
        auto result    = listenerAndSenderTester.Invoke(Commands::LeaveGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        app::Clusters::Groupcast::Attributes::Membership::TypeInfo::DecodableType memberships;
        ASSERT_EQ(listenerAndSenderTester.ReadAttribute(Attributes::Membership::Id, memberships), CHIP_NO_ERROR);

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
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Join groups
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroup1;
        data.keySetID        = kKeyset1;
        data.key             = MakeOptional(ByteSpan(key1));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        data.groupID  = kGroup2;
        data.keySetID = kKeyset2;
        data.key      = MakeOptional(ByteSpan(key2));
        result        = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::AlreadyExists));

        // Update to non-existing keyset (invalid)
        data.groupID  = kGroup2;
        data.keySetID = kKeyset3;
        data.key.ClearValue();
        result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::NotFound));

        // Update without key (always valid)
        data.groupID  = kGroup2;
        data.keySetID = kKeyset1;
        data.key.ClearValue();
        result = tester.Invoke(Commands::UpdateGroupKey::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        // Create a new key (valid, if i <= mProvider.GetMaxGroupKeysPerFabric())
        // 2 keysets already in use
        data.key = MakeOptional(ByteSpan(key1));
        for (uint16_t i = 3; i <= mProvider.GetMaxGroupKeysPerFabric() + 1; ++i)
        {
            data.keySetID = kKeyset2 + i;
            result        = tester.Invoke(Commands::UpdateGroupKey::Id, data);
            EXPECT_EQ(result.GetStatusCode(),
                      i > mProvider.GetMaxGroupKeysPerFabric() ? ClusterStatusCode(Status::ResourceExhausted)
                                                               : ClusterStatusCode(Status::Success));
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
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Join group
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupId;
        data.keySetID        = kKeyset;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(false);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
        sender_tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = true;

        auto result = sender_tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
    }

    // Update (false to true)
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = true;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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

    // Try to update for groupID 0 --> constraint error
    {
        tester.SetFabricIndex(kTestFabricIndex);
        tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = 0;
        data.useAuxiliaryACL = false;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
    }

    // Update (true to false)
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = false;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
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
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

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
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        // Command failed; should not have changed.
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);

        // Too large
        data.durationSeconds = MakeOptional(static_cast<uint16_t>(1201));
        result               = tester.Invoke(Commands::GroupcastTesting::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

        // Command failed; should not have changed from enabled state.
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable testing (no duration)
    {
        Commands::GroupcastTesting::Type data;
        data.testOperation = GroupcastTestingEnum::kEnableListenerTesting;

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);
    }

    // Disable testing should clear the fabric under test
    {
        Commands::GroupcastTesting::Type data;
        data.testOperation = GroupcastTestingEnum::kDisableTesting;

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable Listener Testing with duration
    {
        const uint16_t durationSeconds = 10;

        Commands::GroupcastTesting::Type data;
        data.testOperation   = GroupcastTestingEnum::kEnableListenerTesting;
        data.durationSeconds = MakeOptional(durationSeconds);

        auto result = tester.Invoke(Commands::GroupcastTesting::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));

        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);

        //  Testing should still be active
        mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(durationSeconds - 1));
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kTestFabricIndex);

        //  Testing should end after the duration
        mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(durationSeconds + 1));
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricUnderTest::Id, fabricUnderTest), CHIP_NO_ERROR);
        EXPECT_EQ(fabricUnderTest, kUndefinedFabricIndex);
    }

    // Enable Sender Testing with duration
}

TEST_F(TestGroupcastCluster, TestAuxiliaryAccessUpdatedEvent)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1, 2 };
    const GroupId kGroupId        = 0x1234;
    const GroupId kGroupIdNoAux   = 0x5678;
    const KeysetId kKeyset        = 0xabcd;

    ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    auto & logOnlyEvents = mTestContext.EventsGenerator();

    // 1. Test JoinGroup with useAuxiliaryACL = true -> Should generate event
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupId;
        data.keySetID        = kKeyset;
        data.key             = MakeOptional(ByteSpan(key));
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ(event.value().eventOptions.mPath.mClusterId, Clusters::AccessControl::Id);
        EXPECT_EQ(event.value().eventOptions.mPath.mEventId, Clusters::AccessControl::Events::AuxiliaryAccessUpdated::Id);

        Clusters::AccessControl::Events::AuxiliaryAccessUpdated::DecodableType decodedEvent;
        ASSERT_EQ(event.value().GetEventData(decodedEvent), CHIP_NO_ERROR);
        EXPECT_EQ(decodedEvent.fabricIndex, kTestFabricIndex);
    }

    // 2. Test ConfigureAuxiliaryACL with no change -> Should NOT generate event
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = true;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // 3. Test ConfigureAuxiliaryACL with change (true to false) -> Should generate event
    {
        Commands::ConfigureAuxiliaryACL::Type data;
        data.groupID         = kGroupId;
        data.useAuxiliaryACL = false;

        auto result = tester.Invoke(Commands::ConfigureAuxiliaryACL::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ(event.value().eventOptions.mPath.mEventId, Clusters::AccessControl::Events::AuxiliaryAccessUpdated::Id);
    }

    // 4. Test LeaveGroup with endpoints (partial remove) -> Should NOT generate event (as hasAuxiliaryACL is false)
    {
        Commands::LeaveGroup::Type data;
        data.groupID   = kGroupId;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints)));

        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // Re-join with auxiliary ACL for LeaveGroup test
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupId;
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(true);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent(); // Clear the event from JoinGroup
        ASSERT_TRUE(event.has_value());
    }

    // 5. Test LeaveGroup (whole group) -> Should generate event (as hasAuxiliaryACL is true)
    {
        Commands::LeaveGroup::Type data;
        data.groupID = kGroupId;

        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ(event.value().eventOptions.mPath.mEventId, Clusters::AccessControl::Events::AuxiliaryAccessUpdated::Id);
    }

    // 6. Test JoinGroup for a group WITHOUT auxiliary ACL -> Should NOT generate event
    {
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupIdNoAux;
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(false);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // 7. Test Add Endpoint to group WITHOUT auxiliary ACL -> Should NOT generate event
    {
        const EndpointId kEndpoints2[] = { 3 };
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupIdNoAux;
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(false);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints2, MATTER_ARRAY_SIZE(kEndpoints2));

        auto result = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // 8. Test LeaveGroup (partial) for group WITHOUT auxiliary ACL -> Should NOT generate event
    {
        const EndpointId kEndpoints2[] = { 2 };
        Commands::LeaveGroup::Type data;
        data.groupID   = kGroupIdNoAux;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(kEndpoints2, MATTER_ARRAY_SIZE(kEndpoints2)));

        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // 9. Test JoinGroup with different multicast policy (no updates resulting in ACL change) -> Should NOT generate event
    {
        // First, ensure group exists with Aux ACL, endpoint and IANA policy
        Commands::JoinGroup::Type data;
        data.groupID         = kGroupId;
        data.keySetID        = kKeyset;
        data.useAuxiliaryACL = MakeOptional(true);
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kIanaAddr);
        data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

        auto setupResult = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(setupResult.status, Protocols::InteractionModel::Status::Success);
        (void) logOnlyEvents.GetNextEvent(); // Clear event from setup

        // Now call JoinGroup again with same data but different multicast policy
        data.mcastAddrPolicy = MakeOptional(app::Clusters::Groupcast::MulticastAddrPolicyEnum::kPerGroup);
        auto result          = tester.Invoke(Commands::JoinGroup::Id, data);
        AssertStatus(result.status, Protocols::InteractionModel::Status::Success);

        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }
}

TEST_F(TestGroupcastCluster, TestMaxMembershipPerFabric)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };
    const KeysetId kKeyset        = 0xabcd;

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type data;
    data.keySetID        = kKeyset;
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

    constexpr uint16_t kMaxPerFabric = GroupDataProviderImpl::kMaxMembershipPerFabric;

    for (GroupId i = 1; i <= kMaxPerFabric; i++)
    {
        data.groupID = i;
        data.key     = (i == 1) ? MakeOptional(ByteSpan(key)) : Optional<ByteSpan>();
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success)) << "JoinGroup for group " << i << " should succeed";
    }

    // Per-fabric overflow
    data.groupID = kMaxPerFabric + 1;
    data.key.ClearValue();
    auto result = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));
}

TEST_F(TestGroupcastCluster, TestTotalMaxMembership)
{
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };
    const KeysetId kKeyset        = 0xabcd;

    constexpr uint16_t kMaxCount   = GroupDataProviderImpl::kMaxMembershipCount;
    constexpr uint16_t kMaxPerFab  = GroupDataProviderImpl::kMaxMembershipPerFabric;
    constexpr uint16_t kFab1Groups = kMaxPerFab - 1;
    constexpr uint16_t kRemaining  = kMaxCount - kFab1Groups;
    constexpr uint16_t kFab2Groups = kRemaining / 2;
    constexpr uint16_t kFab3Groups = kRemaining - kFab2Groups;

    // Set up two additional fabrics
    FabricIndex fabricIndex2 = 2;
    FabricIndex fabricIndex3 = 3;
    ASSERT_EQ(mFabricHelper.AddAdditionalTestFabric(fabricIndex2), CHIP_NO_ERROR);
    ASSERT_EQ(mFabricHelper.AddAdditionalTestFabric(fabricIndex3), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(mListener);
    SubjectDescriptor adminSubjectDescriptor = kAdminSubjectDescriptor;

    Commands::JoinGroup::Type data;
    data.keySetID        = kKeyset;
    data.useAuxiliaryACL = MakeOptional(true);
    data.endpoints       = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

    // Fabric 1: kFab1Groups (one below per-fabric limit)
    tester.SetFabricIndex(kTestFabricIndex);
    adminSubjectDescriptor.fabricIndex = kTestFabricIndex;
    tester.SetSubjectDescriptor(adminSubjectDescriptor);

    for (GroupId i = 1; i <= kFab1Groups; i++)
    {
        data.groupID = i;
        data.key     = (i == 1) ? MakeOptional(ByteSpan(key)) : Optional<ByteSpan>();
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }

    // Fabric 2: kFab2Groups
    tester.SetFabricIndex(fabricIndex2);
    adminSubjectDescriptor.fabricIndex = fabricIndex2;
    tester.SetSubjectDescriptor(adminSubjectDescriptor);

    for (GroupId i = 1; i <= kFab2Groups; i++)
    {
        data.groupID = i;
        data.key     = (i == 1) ? MakeOptional(ByteSpan(key)) : Optional<ByteSpan>();
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }

    // Fabric 3: kFab3Groups
    tester.SetFabricIndex(fabricIndex3);
    adminSubjectDescriptor.fabricIndex = fabricIndex3;
    tester.SetSubjectDescriptor(adminSubjectDescriptor);

    for (GroupId i = 1; i <= kFab3Groups; i++)
    {
        data.groupID = i;
        data.key     = (i == 1) ? MakeOptional(ByteSpan(key)) : Optional<ByteSpan>();
        auto result  = tester.Invoke(Commands::JoinGroup::Id, data);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    }

    // Node-wide total is now kMaxCount.
    // Fabric 1 has kFab1Groups (< kMaxPerFab) so the per-fabric check passes,
    // but the node-wide check should return ResourceExhausted.
    tester.SetFabricIndex(kTestFabricIndex);
    adminSubjectDescriptor.fabricIndex = kTestFabricIndex;
    tester.SetSubjectDescriptor(adminSubjectDescriptor);

    data.groupID = kFab1Groups + 1;
    data.key.ClearValue();
    auto result = tester.Invoke(Commands::JoinGroup::Id, data);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));
}

// A LeaveGroup command whose Endpoints list exceeds the maximum constraint ("1 to 20") is rejected
// with CONSTRAINT_ERROR, without writing past the fixed-size response buffer.
TEST_F(TestGroupcastCluster, TestLeaveGroupEndpointsExceedingMaxReturnsConstraintError)
{
    static constexpr uint16_t kMaxCommandEndpoints = app::Clusters::GroupcastCluster::kMaxCommandEndpoints;
    // exceeds kMaxMembershipEndpoints (response buffer sizing) and kMaxCommandEndpoints
    static constexpr uint16_t kEndpointsInGroup = kMaxMembershipEndpoints + 5;
    const GroupId kGroup                        = 0xab01;
    const KeysetId kKeyset                      = 0xabcd;
    const uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Join the group with kEndpointsInGroup distinct endpoints, kMaxCommandEndpoints at a time.
    {
        Commands::JoinGroup::Type data;
        data.groupID  = kGroup;
        data.keySetID = kKeyset;
        data.key      = MakeOptional(ByteSpan(key));

        EndpointId nextEndpoint = 1;
        for (uint16_t added = 0; added < kEndpointsInGroup;)
        {
            EndpointId chunk[kMaxCommandEndpoints];
            uint16_t count = 0;
            for (; count < kMaxCommandEndpoints && added < kEndpointsInGroup; count++, added++)
            {
                chunk[count] = nextEndpoint++;
            }
            data.endpoints = DataModel::List<const EndpointId>(chunk, count);
            auto result    = tester.Invoke(Commands::JoinGroup::Id, data);
            ASSERT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
            data.key.ClearValue();
        }
    }

    // Leaving the group while naming more than the maximum number of endpoints is rejected with
    // CONSTRAINT_ERROR; no response payload is produced.
    {
        EndpointId leaveEndpoints[kEndpointsInGroup];
        for (uint16_t i = 0; i < kEndpointsInGroup; i++)
        {
            leaveEndpoints[i] = static_cast<EndpointId>(i + 1);
        }

        Commands::LeaveGroup::Type data;
        data.groupID   = kGroup;
        data.endpoints = MakeOptional(DataModel::List<const EndpointId>(leaveEndpoints, kEndpointsInGroup));

        auto result = tester.Invoke(Commands::LeaveGroup::Id, data);
        EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
        EXPECT_FALSE(result.response.has_value());
    }
}

TEST_F(TestGroupcastCluster, TestJoinAndLeaveDistinctGroupsReclaimsKeyMapSlots)
{
    const KeysetId kKeyset = 0xabcd;
    const uint8_t key[]    = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    const uint16_t maxPerFabric = mProvider.GetMaxGroupsPerFabric();
    const uint16_t totalGroups  = static_cast<uint16_t>(maxPerFabric * 2 + 4);
    std::optional<uint16_t> sessionId;

    for (uint16_t step = 0; step < totalGroups; step++)
    {
        const GroupId groupId = static_cast<GroupId>(0x1000 + step);

        Commands::JoinGroup::Type joinData;
        joinData.groupID   = groupId;
        joinData.keySetID  = kKeyset;
        joinData.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
        if (step == 0)
        {
            joinData.key = MakeOptional(ByteSpan(key));
        }

        auto joinResult = tester.Invoke(Commands::JoinGroup::Id, joinData);
        ASSERT_EQ(joinResult.GetStatusCode(), ClusterStatusCode(Status::Success));

        if (!sessionId.has_value())
        {
            auto * keyContext = mProvider.GetKeyContext(kTestFabricIndex, groupId);
            ASSERT_NE(keyContext, nullptr);
            sessionId = keyContext->GetKeyHash();
            keyContext->Release();

            auto * activeSessionIter = mProvider.IterateGroupSessions(*sessionId);
            ASSERT_NE(activeSessionIter, nullptr);
            EXPECT_GT(activeSessionIter->Count(), 0u);
            activeSessionIter->Release();
        }

        Commands::LeaveGroup::Type leaveData;
        if ((step % 3) == 0)
        {
            // Whole-group LeaveGroup
            leaveData.groupID = groupId;
        }
        else if ((step % 3) == 1)
        {
            // Leave-all LeaveGroup(0)
            leaveData.groupID = kUndefinedGroupId;
        }
        else
        {
            // Listener-only last-endpoint LeaveGroup deletes the group and its key mapping
            leaveData.groupID   = groupId;
            leaveData.endpoints = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints)));
        }

        auto leaveResult = tester.Invoke(Commands::LeaveGroup::Id, leaveData);
        ASSERT_EQ(leaveResult.GetStatusCode(), ClusterStatusCode(Status::Success));
    }

    auto * keyIter = mProvider.IterateGroupKeys(kTestFabricIndex);
    ASSERT_NE(keyIter, nullptr);
    EXPECT_EQ(keyIter->Count(), 0u);
    keyIter->Release();

    ASSERT_TRUE(sessionId.has_value());
    auto * sessionIter = mProvider.IterateGroupSessions(*sessionId);
    ASSERT_NE(sessionIter, nullptr);
    EXPECT_EQ(sessionIter->Count(), 0u);
    GroupDataProvider::GroupSession staleSession;
    EXPECT_FALSE(sessionIter->Next(staleSession));
    sessionIter->Release();
}

TEST_F(TestGroupcastCluster, TestSenderEndpointLeaveRetainsKeyMapUntilWholeGroupLeave)
{
    const GroupId kGroupId = 0x3001;
    const KeysetId kKeyset = 0x3002;
    const uint8_t key[]    = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    const EndpointId kEndpoints[] = { 1 };

    app::Clusters::GroupcastCluster listenerAndSender{ { mFabricHelper.GetFabricTable(), mProvider, mMockTimerDelegate,
                                                         mAccessControl, mTesting },
                                                       BitFlags<Feature>{ Feature::kListener, Feature::kSender } };
    ASSERT_EQ(listenerAndSender.Startup(*clusterContext), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(listenerAndSender);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinData;
    joinData.groupID   = kGroupId;
    joinData.keySetID  = kKeyset;
    joinData.key       = MakeOptional(ByteSpan(key));
    joinData.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinData).GetStatusCode(), ClusterStatusCode(Status::Success));

    // Removing the last endpoint when Sender feature is supported keeps the group and its key mapping.
    Commands::LeaveGroup::Type endpointLeave;
    endpointLeave.groupID   = kGroupId;
    endpointLeave.endpoints = MakeOptional(DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints)));
    ASSERT_EQ(tester.Invoke(Commands::LeaveGroup::Id, endpointLeave).GetStatusCode(), ClusterStatusCode(Status::Success));

    GroupDataProvider::GroupInfo info;
    KeysetId retrievedKeyset = kInvalidKeysetId;
    EXPECT_EQ(mProvider.GetGroupInfo(kTestFabricIndex, kGroupId, info), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kGroupId, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kKeyset);

    // Whole-group LeaveGroup removes both GroupInfo and GroupKeyMap.
    Commands::LeaveGroup::Type wholeGroupLeave;
    wholeGroupLeave.groupID = kGroupId;
    ASSERT_EQ(tester.Invoke(Commands::LeaveGroup::Id, wholeGroupLeave).GetStatusCode(), ClusterStatusCode(Status::Success));

    EXPECT_EQ(mProvider.GetGroupInfo(kTestFabricIndex, kGroupId, info), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kGroupId, retrievedKeyset), CHIP_ERROR_NOT_FOUND);

    listenerAndSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestGroupcastCluster, TestStartupRemovesGroupKeysWithoutGroupInfoUnlessSender)
{
    const GroupId kActiveGroup = 0x4001;
    const KeysetId kKeyset     = 0x4002;
    const uint8_t key[]        = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F };
    const EndpointId kEndpoints[] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinData;
    joinData.groupID   = kActiveGroup;
    joinData.keySetID  = kKeyset;
    joinData.key       = MakeOptional(ByteSpan(key));
    joinData.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinData).GetStatusCode(), ClusterStatusCode(Status::Success));

    // Simulate reboot from unpatched firmware that leaked KeyMapData entries up to GetMaxGroupsPerFabric().
    mSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);
    mListener.Shutdown(app::ClusterShutdownType::kClusterShutdown);
    ASSERT_FALSE(mMockTimerDelegate.IsTimerActive(nullptr));

    const uint16_t maxPerFabric = mProvider.GetMaxGroupsPerFabric();
    for (uint16_t leakedOffset = 0; leakedOffset + 1 < maxPerFabric; leakedOffset++)
    {
        const GroupId leakedGroup = static_cast<GroupId>(0x4100 + leakedOffset);
        ASSERT_EQ(mProvider.SetGroupKey(kTestFabricIndex, leakedGroup, kKeyset), CHIP_NO_ERROR);
    }

    // A cluster with the Sender feature keeps mappings that have no GroupInfo: the node may send to those groups with
    // mappings written before Groupcast was enabled.
    ASSERT_EQ(mSender.Startup(*clusterContext), CHIP_NO_ERROR);
    auto * keyIter = mProvider.IterateGroupKeys(kTestFabricIndex);
    ASSERT_NE(keyIter, nullptr);
    EXPECT_EQ(keyIter->Count(), maxPerFabric);
    keyIter->Release();
    mSender.Shutdown(app::ClusterShutdownType::kClusterShutdown);

    // A cluster without the Sender feature removes them, and does not report the removals as Membership changes.
    ASSERT_EQ(mListener.Startup(*clusterContext), CHIP_NO_ERROR);
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(nullptr));
    ASSERT_EQ(mSender.Startup(*clusterContext), CHIP_NO_ERROR);

    keyIter = mProvider.IterateGroupKeys(kTestFabricIndex);
    ASSERT_NE(keyIter, nullptr);
    EXPECT_EQ(keyIter->Count(), 1u);
    keyIter->Release();

    KeysetId retrievedKeyset = kInvalidKeysetId;
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kActiveGroup, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kKeyset);

    // All reclaimed slots must now be usable for new JoinGroup commands.
    joinData.key.ClearValue();
    for (uint16_t slot = 1; slot < maxPerFabric; slot++)
    {
        joinData.groupID = static_cast<GroupId>(0x4200 + slot);
        ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinData).GetStatusCode(), ClusterStatusCode(Status::Success));
    }
}

TEST_F(TestGroupcastCluster, TestSetKeySetFailureRollsBackInlineKeyAndReturnsResourceExhausted)
{
    const KeysetId kSeedKeyset   = 0x5001;
    const KeysetId kInlineKeyset = 0x5002;
    const GroupId kTargetGroup   = 0x5100;
    const uint8_t key[] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };
    const EndpointId kEndpoints[] = { 1 };

    const FabricInfo * fabric = mFabricHelper.GetFabricTable().FindFabricWithIndex(kTestFabricIndex);
    ASSERT_NE(fabric, nullptr);
    uint8_t compressedFabricIdBuffer[sizeof(uint64_t)];
    MutableByteSpan compressedFabricId(compressedFabricIdBuffer);
    ASSERT_EQ(fabric->GetCompressedFabricIdBytes(compressedFabricId), CHIP_NO_ERROR);

    GroupDataProvider::KeySet seedKeySet(kSeedKeyset, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(seedKeySet.epoch_keys[0].key, key, sizeof(key));
    seedKeySet.epoch_keys[0].start_time = 1;
    ASSERT_EQ(mProvider.SetKeySet(kTestFabricIndex, compressedFabricId, seedKeySet), CHIP_NO_ERROR);

    // Fill GroupKeyMap capacity without adding GroupInfo so JoinGroup reaches SetKeySet -> SetGroupKey and hits
    // INVALID_LIST_LENGTH.
    const uint16_t maxPerFabric = mProvider.GetMaxGroupsPerFabric();
    for (uint16_t idx = 0; idx < maxPerFabric; idx++)
    {
        ASSERT_EQ(mProvider.SetGroupKey(kTestFabricIndex, static_cast<GroupId>(0x5200 + idx), kSeedKeyset), CHIP_NO_ERROR);
    }

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinData;
    joinData.groupID   = kTargetGroup;
    joinData.keySetID  = kInlineKeyset;
    joinData.key       = MakeOptional(ByteSpan(key));
    joinData.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));

    auto result = tester.Invoke(Commands::JoinGroup::Id, joinData);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));

    // Inline keyset creation must have been rolled back so retrying after freeing a slot does not fail with AlreadyExists.
    GroupDataProvider::KeySet outKeySet;
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kInlineKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);

    ASSERT_EQ(mProvider.RemoveGroupKeyAt(kTestFabricIndex, 0), CHIP_NO_ERROR);
    result = tester.Invoke(Commands::JoinGroup::Id, joinData);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::Success));
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kInlineKeyset, outKeySet), CHIP_NO_ERROR);
}

TEST_F(TestGroupcastCluster, TestJoinGroupRollsBackKeysWhenSetGroupInfoFails)
{
    const GroupId kExistingGroup   = 0x6001;
    const GroupId kFailedNewGroup  = 0x6002;
    const KeysetId kInitialKeyset  = 0x6101;
    const KeysetId kNewGroupKeyset = 0x6102;
    const KeysetId kUpdateKeyset   = 0x6103;
    const uint8_t key[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };
    const EndpointId kEndpoints[] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinExisting;
    joinExisting.groupID   = kExistingGroup;
    joinExisting.keySetID  = kInitialKeyset;
    joinExisting.key       = MakeOptional(ByteSpan(key));
    joinExisting.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinExisting).GetStatusCode(), ClusterStatusCode(Status::Success));

    FailWritesOf(DefaultStorageKeyAllocator::FabricGroup(kTestFabricIndex, kFailedNewGroup));
    FailWritesOf(DefaultStorageKeyAllocator::FabricGroup(kTestFabricIndex, kExistingGroup));

    // 1. Failing SetGroupInfo on a new group rolls back both the new KeyMapData and inline KeySet.
    Commands::JoinGroup::Type joinNew;
    joinNew.groupID   = kFailedNewGroup;
    joinNew.keySetID  = kNewGroupKeyset;
    joinNew.key       = MakeOptional(ByteSpan(key));
    joinNew.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinNew).GetStatusCode(), ClusterStatusCode(Status::Failure));

    KeysetId retrievedKeyset = kInvalidKeysetId;
    GroupDataProvider::KeySet outKeySet;
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kFailedNewGroup, retrievedKeyset), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kNewGroupKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);

    // 2. Failing SetGroupInfo when updating an existing group restores its prior keyset mapping and removes the new inline KeySet.
    joinExisting.keySetID        = kUpdateKeyset;
    joinExisting.key             = MakeOptional(ByteSpan(key));
    joinExisting.useAuxiliaryACL = MakeOptional(true);
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinExisting).GetStatusCode(), ClusterStatusCode(Status::Failure));

    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kExistingGroup, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kInitialKeyset);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kUpdateKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);

    // 3. Failing SetGroupInfo when updating an existing group that has no key mapping unlinks the newly written KeyMapData.
    mTestContext.StorageDelegate().ClearPoisonKeys();
    ASSERT_EQ(mProvider.RemoveKeySet(kTestFabricIndex, kInitialKeyset), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kExistingGroup, retrievedKeyset), CHIP_ERROR_NOT_FOUND);

    FailWritesOf(DefaultStorageKeyAllocator::FabricGroup(kTestFabricIndex, kExistingGroup));
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinExisting).GetStatusCode(), ClusterStatusCode(Status::Failure));
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kExistingGroup, retrievedKeyset), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kUpdateKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestGroupcastCluster, TestJoinGroupRollsBackWhenAddEndpointFails)
{
    const GroupId kNewGroup         = 0x7001;
    const GroupId kExistingGroup    = 0x7002;
    const KeysetId kNewInlineKeyset = 0x7101;
    const KeysetId kExistingKeyset  = 0x7102;
    const KeysetId kUpdatedKeyset   = 0x7103;
    const uint8_t key[] = { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F };
    const EndpointId kTwoEndpoints[] = { 1, 2 };
    const EndpointId kInitialEp[]    = { 1 };
    const EndpointId kAdditionalEp[] = { 2 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // 1. New group where AddEndpoint fails on the second endpoint rolls back endpoint #1, GroupInfo, KeyMapData, KeySet, and
    // AuxACL notification state.
    FailWritesOf(DefaultStorageKeyAllocator::FabricGroupEndpoint(kTestFabricIndex, kNewGroup, 2));

    Commands::JoinGroup::Type joinNew;
    joinNew.groupID         = kNewGroup;
    joinNew.keySetID        = kNewInlineKeyset;
    joinNew.key             = MakeOptional(ByteSpan(key));
    joinNew.useAuxiliaryACL = MakeOptional(true);
    joinNew.mcastAddrPolicy = MakeOptional(MulticastAddrPolicyEnum::kPerGroup);
    joinNew.endpoints       = DataModel::List<const EndpointId>(kTwoEndpoints, MATTER_ARRAY_SIZE(kTwoEndpoints));
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinNew).GetStatusCode(), ClusterStatusCode(Status::Failure));

    GroupDataProvider::GroupInfo info;
    KeysetId retrievedKeyset = kInvalidKeysetId;
    GroupDataProvider::KeySet outKeySet;
    EXPECT_EQ(mProvider.GetGroupInfo(kTestFabricIndex, kNewGroup, info), CHIP_ERROR_NOT_FOUND);
    EXPECT_FALSE(mProvider.HasEndpoint(kTestFabricIndex, kNewGroup, 1));
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kNewGroup, retrievedKeyset), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kNewInlineKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);
    EXPECT_FALSE(mProvider.ConsumeAuxAclNotificationNeeded());

    uint16_t usedMcastAddrCount = 0;
    ASSERT_EQ(tester.ReadAttribute(Attributes::UsedMcastAddrCount::Id, usedMcastAddrCount), CHIP_NO_ERROR);
    EXPECT_EQ(usedMcastAddrCount, 0u);

    // Retrying with working storage must succeed with the same KeySetID and inline key.
    mTestContext.StorageDelegate().ClearPoisonKeys();
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinNew).GetStatusCode(), ClusterStatusCode(Status::Success));
    EXPECT_EQ(mProvider.GetGroupInfo(kTestFabricIndex, kNewGroup, info), CHIP_NO_ERROR);
    EXPECT_TRUE(mProvider.HasEndpoint(kTestFabricIndex, kNewGroup, 1));
    EXPECT_TRUE(mProvider.HasEndpoint(kTestFabricIndex, kNewGroup, 2));
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kNewInlineKeyset, outKeySet), CHIP_NO_ERROR);

    // 2. Existing group where adding a new endpoint fails restores its prior GroupInfo flags, prior keyset, and rolls back the new
    // inline KeySet.
    Commands::JoinGroup::Type joinExisting;
    joinExisting.groupID         = kExistingGroup;
    joinExisting.keySetID        = kExistingKeyset;
    joinExisting.key             = MakeOptional(ByteSpan(key));
    joinExisting.useAuxiliaryACL = MakeOptional(false);
    joinExisting.endpoints       = DataModel::List<const EndpointId>(kInitialEp, MATTER_ARRAY_SIZE(kInitialEp));
    ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinExisting).GetStatusCode(), ClusterStatusCode(Status::Success));

    FailWritesOf(DefaultStorageKeyAllocator::FabricGroupEndpoint(kTestFabricIndex, kExistingGroup, 2));
    auto & generatedEvents = mTestContext.EventsGenerator();
    while (generatedEvents.GetNextEvent().has_value())
    {
    }

    joinExisting.keySetID        = kUpdatedKeyset;
    joinExisting.key             = MakeOptional(ByteSpan(key));
    joinExisting.useAuxiliaryACL = MakeOptional(true);
    joinExisting.endpoints       = DataModel::List<const EndpointId>(kAdditionalEp, MATTER_ARRAY_SIZE(kAdditionalEp));
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinExisting).GetStatusCode(), ClusterStatusCode(Status::Failure));

    EXPECT_EQ(mProvider.GetGroupInfo(kTestFabricIndex, kExistingGroup, info), CHIP_NO_ERROR);
    EXPECT_FALSE(info.HasAuxiliaryACL());
    EXPECT_TRUE(mProvider.HasEndpoint(kTestFabricIndex, kExistingGroup, 1));
    EXPECT_FALSE(mProvider.HasEndpoint(kTestFabricIndex, kExistingGroup, 2));
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kExistingGroup, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kExistingKeyset);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kUpdatedKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);

    // The auxiliary ACL flag of a group with an endpoint was changed and restored. The failed command reports that with
    // an event and leaves no notification pending for the next command.
    auto auxiliaryAccessEvent = generatedEvents.GetNextEvent();
    ASSERT_TRUE(auxiliaryAccessEvent.has_value());
    EXPECT_EQ(auxiliaryAccessEvent.value().eventOptions.mPath.mEventId,
              Clusters::AccessControl::Events::AuxiliaryAccessUpdated::Id);
    EXPECT_FALSE(generatedEvents.GetNextEvent().has_value());
    EXPECT_FALSE(mProvider.ConsumeAuxAclNotificationNeeded());
}

TEST_F(TestGroupcastCluster, TestFailedJoinGroupRestoresGroupKeyOfGroupWithoutGroupInfo)
{
    const GroupId kGroupId         = 0x7801;
    const KeysetId kPreviousKeyset = 0x7811;
    const KeysetId kInlineKeyset   = 0x7812;
    const uint8_t key[] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F };

    // A node that sends to a group can hold a mapping for it without GroupInfo, written before Groupcast was enabled.
    ASSERT_EQ(mProvider.SetGroupKeyAt(kTestFabricIndex, 0, GroupDataProvider::GroupKey(kGroupId, kPreviousKeyset)), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(mSender);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    FailWritesOf(DefaultStorageKeyAllocator::FabricGroup(kTestFabricIndex, kGroupId));

    Commands::JoinGroup::Type joinData;
    joinData.groupID  = kGroupId;
    joinData.keySetID = kInlineKeyset;
    joinData.key      = MakeOptional(ByteSpan(key));
    EXPECT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinData).GetStatusCode(), ClusterStatusCode(Status::Failure));

    KeysetId retrievedKeyset = kInvalidKeysetId;
    GroupDataProvider::KeySet outKeySet;
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kGroupId, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kPreviousKeyset);
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kInlineKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestGroupcastCluster, TestUpdateGroupKeyFailureRollsBackInlineKeyAndReturnsResourceExhausted)
{
    const GroupId kTargetGroup   = 0x8001;
    const KeysetId kInitialKey   = 0x8101;
    const KeysetId kSeedKey      = 0x8102;
    const KeysetId kInlineKeyset = 0x8103;
    const uint8_t key[] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F };
    const EndpointId kEndpoints[] = { 1 };

    chip::Testing::ClusterTester tester(mListener);
    tester.SetFabricIndex(kTestFabricIndex);
    tester.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Commands::JoinGroup::Type joinData;
    joinData.groupID   = kTargetGroup;
    joinData.keySetID  = kInitialKey;
    joinData.key       = MakeOptional(ByteSpan(key));
    joinData.endpoints = DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    ASSERT_EQ(tester.Invoke(Commands::JoinGroup::Id, joinData).GetStatusCode(), ClusterStatusCode(Status::Success));

    // Remove the group's keyset so GroupInfo exists without a KeyMapData entry, then fill GroupKeyMap capacity.
    ASSERT_EQ(mProvider.RemoveKeySet(kTestFabricIndex, kInitialKey), CHIP_NO_ERROR);

    const FabricInfo * fabric = mFabricHelper.GetFabricTable().FindFabricWithIndex(kTestFabricIndex);
    ASSERT_NE(fabric, nullptr);
    uint8_t compressedFabricIdBuffer[sizeof(uint64_t)];
    MutableByteSpan compressedFabricId(compressedFabricIdBuffer);
    ASSERT_EQ(fabric->GetCompressedFabricIdBytes(compressedFabricId), CHIP_NO_ERROR);

    GroupDataProvider::KeySet seedKeySet(kSeedKey, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(seedKeySet.epoch_keys[0].key, key, sizeof(key));
    seedKeySet.epoch_keys[0].start_time = 1;
    ASSERT_EQ(mProvider.SetKeySet(kTestFabricIndex, compressedFabricId, seedKeySet), CHIP_NO_ERROR);

    const uint16_t maxPerFabric = mProvider.GetMaxGroupsPerFabric();
    for (uint16_t idx = 0; idx < maxPerFabric; idx++)
    {
        ASSERT_EQ(mProvider.SetGroupKey(kTestFabricIndex, static_cast<GroupId>(0x8200 + idx), kSeedKey), CHIP_NO_ERROR);
    }

    Commands::UpdateGroupKey::Type updateData;
    updateData.groupID  = kTargetGroup;
    updateData.keySetID = kInlineKeyset;
    updateData.key      = MakeOptional(ByteSpan(key));
    EXPECT_EQ(tester.Invoke(Commands::UpdateGroupKey::Id, updateData).GetStatusCode(),
              ClusterStatusCode(Status::ResourceExhausted));

    GroupDataProvider::KeySet outKeySet;
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kInlineKeyset, outKeySet), CHIP_ERROR_NOT_FOUND);

    // Removing a leaked slot allows retrying UpdateGroupKey with the same inline keyset.
    ASSERT_EQ(mProvider.RemoveGroupKeyAt(kTestFabricIndex, 0), CHIP_NO_ERROR);
    EXPECT_EQ(tester.Invoke(Commands::UpdateGroupKey::Id, updateData).GetStatusCode(), ClusterStatusCode(Status::Success));
    EXPECT_EQ(mProvider.GetKeySet(kTestFabricIndex, kInlineKeyset, outKeySet), CHIP_NO_ERROR);
    KeysetId retrievedKeyset = kInvalidKeysetId;
    EXPECT_EQ(mProvider.GetGroupKey(kTestFabricIndex, kTargetGroup, retrievedKeyset), CHIP_NO_ERROR);
    EXPECT_EQ(retrievedKeyset, kInlineKeyset);
}

} // namespace
