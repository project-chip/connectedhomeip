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

#include <access/AccessControl.h>
#include <access/examples/ExampleAccessControlDelegate.h>
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
#include <access/AccessRestrictionProvider.h>
#endif
#include <app/clusters/access-control-server/access-control-cluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/AccessControl/Enums.h>
#include <clusters/AccessControl/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <vector>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
#include <app-common/zap-generated/cluster-objects.h>
#endif

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;
// Simple DeviceTypeResolver for tests
class TestDeviceTypeResolver : public Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return false; }
};

static TestDeviceTypeResolver gTestDeviceTypeResolver;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
// Mock AccessRestrictionProvider for testing ReviewFabricRestrictions command.
// This mock captures the parameters passed to RequestFabricRestrictionReview
// and allows tests to verify that the command handler correctly processes
// the request and calls the provider with the expected data.
class TestAccessRestrictionProvider : public Access::AccessRestrictionProvider
{
public:
    // Override DoRequestFabricRestrictionReview to capture call parameters
    // and allow tests to control the return value.
    CHIP_ERROR DoRequestFabricRestrictionReview(const FabricIndex fabricIndex, uint64_t token,
                                                const std::vector<Entry> & arl) override
    {
        // Store the parameters passed to this method so tests can verify them
        mLastFabricIndex = fabricIndex;
        mLastToken       = token;
        mLastArl         = arl;
        mRequestCount++;

        // Return the configured error code (defaults to CHIP_NO_ERROR)
        return mReturnError;
    }

    // Test verification fields - these are populated when DoRequestFabricRestrictionReview is called
    FabricIndex mLastFabricIndex = kUndefinedFabricIndex;
    uint64_t mLastToken          = 0;
    std::vector<Entry> mLastArl;
    size_t mRequestCount = 0;

    // Control field - set this to simulate different error conditions
    CHIP_ERROR mReturnError = CHIP_NO_ERROR;
};
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

struct TestAccessControlCluster : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        // Initialize AccessControl for reading mandatory attributes
        Access::AccessControl::Delegate * delegate = Access::Examples::GetAccessControlDelegate();
        ASSERT_EQ(Access::GetAccessControl().Init(delegate, gTestDeviceTypeResolver), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        Access::GetAccessControl().Finish();
        Platform::MemoryShutdown();
    }
};

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
// Test fixture for tests that require a mock AccessRestrictionProvider.
// This fixture sets up the mock provider, cluster, and tester in SetUp()
// and restores the previous provider in TearDown().
struct TestAccessControlClusterWithMockProvider : public TestAccessControlCluster
{
    TestAccessControlClusterWithMockProvider() : mTester(mCluster) {}

    void SetUp() override
    {
        // Save the previous provider to restore it after the test
        mPreviousProvider = Access::GetAccessControl().GetAccessRestrictionProvider();
        Access::GetAccessControl().SetAccessRestrictionProvider(&mMockProvider);
        // Initialize the cluster and tester
        ASSERT_EQ(mCluster.Startup(mTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster.Shutdown();
        // Restore the previous provider to avoid use-after-free issues
        Access::GetAccessControl().SetAccessRestrictionProvider(mPreviousProvider);
    }

    TestAccessRestrictionProvider mMockProvider;
    AccessControlCluster mCluster;
    Testing::ClusterTester mTester;

private:
    Access::AccessRestrictionProvider * mPreviousProvider = nullptr;
};
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

TEST_F(TestAccessControlCluster, CompileTest)
{
    AccessControlCluster cluster;
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, AccessControl::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestAccessControlCluster, CommandsTest)
{
    AccessControlCluster cluster;
    ConcreteClusterPath accessControlPath = ConcreteClusterPath(kRootEndpointId, AccessControl::Id);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsBuilder;
    ASSERT_EQ(cluster.AcceptedCommands(accessControlPath, acceptedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = acceptedCommandsBuilder.TakeBuffer();

    ReadOnlyBufferBuilder<CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(accessControlPath, generatedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Check accepted commands
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  AccessControl::Commands::ReviewFabricRestrictions::kMetadataEntry,
                                              }));

    // Check generated commands
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               {
                                                   AccessControl::Commands::ReviewFabricRestrictionsResponse::Id,
                                               }));
#else
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, {}));
#endif
}

TEST_F(TestAccessControlCluster, AttributesTest)
{
    AccessControlCluster cluster;

    std::vector<DataModel::AttributeEntry> expectedAttributes(AccessControl::Attributes::kMandatoryMetadata.begin(),
                                                              AccessControl::Attributes::kMandatoryMetadata.end());

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    expectedAttributes.push_back(AccessControl::Attributes::Extension::kMetadataEntry);
#endif

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    expectedAttributes.push_back(AccessControl::Attributes::CommissioningARL::kMetadataEntry);
    expectedAttributes.push_back(AccessControl::Attributes::Arl::kMetadataEntry);
#endif

    ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
}

// Helper function to count elements in a decodable list
// Returns CHIP_ERROR to allow callers to use ASSERT_EQ/EXPECT_EQ for better error messages
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

// Test that all available attributes (mandatory and optional) can be read
TEST_F(TestAccessControlCluster, ReadAttributesTest)
{
    AccessControlCluster cluster;
    Testing::ClusterTester tester(cluster);

    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Test mandatory attributes (always available)
    // According to Matter spec, minimum values are:
    // - SubjectsPerAccessControlEntry: >= 4
    // - TargetsPerAccessControlEntry: >= 3
    // - AccessControlEntriesPerFabric: >= 4
    uint16_t subjectsPerEntry = 0;
    auto status = tester.ReadAttribute(AccessControl::Attributes::SubjectsPerAccessControlEntry::Id, subjectsPerEntry);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_GE(subjectsPerEntry, 4u);

    uint16_t targetsPerEntry = 0;
    status                   = tester.ReadAttribute(AccessControl::Attributes::TargetsPerAccessControlEntry::Id, targetsPerEntry);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_GE(targetsPerEntry, 3u);

    uint16_t entriesPerFabric = 0;
    status = tester.ReadAttribute(AccessControl::Attributes::AccessControlEntriesPerFabric::Id, entriesPerFabric);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_GE(entriesPerFabric, 4u);

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    // Read Extension attribute (optional)
    AccessControl::Attributes::Extension::TypeInfo::DecodableType extension;
    status = tester.ReadAttribute(AccessControl::Attributes::Extension::Id, extension);
    ASSERT_TRUE(status.IsSuccess());
    size_t extensionCount = 0;
    ASSERT_EQ(CountListElements(extension, extensionCount), CHIP_NO_ERROR);
    ASSERT_EQ(extensionCount, 0u);
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Read CommissioningARL attribute (optional)
    AccessControl::Attributes::CommissioningARL::TypeInfo::DecodableType commissioningArl;
    status = tester.ReadAttribute(AccessControl::Attributes::CommissioningARL::Id, commissioningArl);
    ASSERT_TRUE(status.IsSuccess());
    size_t commissioningArlCount = 0;
    ASSERT_EQ(CountListElements(commissioningArl, commissioningArlCount), CHIP_NO_ERROR);
    ASSERT_EQ(commissioningArlCount, 0u);

    // Read Arl attribute (optional)
    AccessControl::Attributes::Arl::TypeInfo::DecodableType arl;
    status = tester.ReadAttribute(AccessControl::Attributes::Arl::Id, arl);
    ASSERT_TRUE(status.IsSuccess());
    size_t arlCount = 0;
    ASSERT_EQ(CountListElements(arl, arlCount), CHIP_NO_ERROR);
    ASSERT_EQ(arlCount, 0u);
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

    cluster.Shutdown();
}

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_Success)
{

    // Build a ReviewFabricRestrictions command request with ARL entries.
    // The request contains a list of CommissioningAccessRestrictionEntryStruct entries,
    // each specifying an endpoint, cluster, and list of restrictions.
    AccessControl::Commands::ReviewFabricRestrictions::Type request;

    // Create a single ARL entry for endpoint 1, OnOff cluster
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = OnOff::Id;

    // Create a restriction that forbids access to attribute 0x0000
    // This restriction type means attribute reads/writes are forbidden
    AccessControl::Structs::AccessRestrictionStruct::Type restriction;
    restriction.type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restriction.id.SetNonNull(0x0000); // Attribute ID

    // Wrap the restriction in a DataModel::List as required by the struct
    entry.restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction, 1);

    // Wrap the entry in a DataModel::List for the request
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(&entry, 1);

    // Invoke the ReviewFabricRestrictions command
    // The command handler should process the request, call the AccessRestrictionProvider,
    // and return a response with a token.
    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_GT(result.response->token, 0u);
    ASSERT_EQ(mMockProvider.mRequestCount, 1u);
    ASSERT_EQ(mMockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].endpointNumber, 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].clusterId, OnOff::Id);

    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[0].restrictionType,
              Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_TRUE(mMockProvider.mLastArl[0].restrictions[0].id.HasValue());
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[0].id.Value(), 0x0000u);
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_EmptyArl)
{
    // Test the case where an empty ARL list is sent.
    // According to the spec, an empty list means "review all restrictions" for the fabric.

    // Create a request with an empty ARL list
    // This signals a request to review all restrictions for the accessing fabric
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(nullptr, 0);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_GT(result.response->token, 0u);
    ASSERT_EQ(mMockProvider.mRequestCount, 1u);
    ASSERT_EQ(mMockProvider.mLastArl.size(), 0u);
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_MultipleEntries)
{
    // Test the command with multiple ARL entries in a single request.
    // This verifies that the command handler correctly processes and forwards
    // all entries to the AccessRestrictionProvider.

    // Create an array of ARL entries for different endpoints and clusters
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entries[2];

    // First entry: endpoint 1, OnOff cluster with attribute access restriction
    entries[0].endpoint = 1;
    entries[0].cluster  = OnOff::Id;
    AccessControl::Structs::AccessRestrictionStruct::Type restriction1;
    restriction1.type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restriction1.id.SetNonNull(0x0000);
    entries[0].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction1, 1);

    // Second entry: endpoint 2, Level Control cluster with command restriction
    entries[1].endpoint = 2;
    entries[1].cluster  = LevelControl::Id;
    AccessControl::Structs::AccessRestrictionStruct::Type restriction2;
    restriction2.type = AccessControl::Enums::AccessRestrictionTypeEnum::kCommandForbidden;
    restriction2.id.SetNonNull(0x0000);
    entries[1].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction2, 1);

    // Build the request with both entries
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(entries, 2);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(mMockProvider.mRequestCount, 1u);
    ASSERT_EQ(mMockProvider.mLastArl.size(), 2u);
    ASSERT_EQ(mMockProvider.mLastArl[0].endpointNumber, 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].clusterId, OnOff::Id);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[0].restrictionType,
              Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_EQ(mMockProvider.mLastArl[1].endpointNumber, 2u);
    ASSERT_EQ(mMockProvider.mLastArl[1].clusterId, LevelControl::Id);
    ASSERT_EQ(mMockProvider.mLastArl[1].restrictions.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[1].restrictions[0].restrictionType,
              Access::AccessRestrictionProvider::Type::kCommandForbidden);
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_WildcardRestriction)
{
    // Test the command with a wildcard restriction (null ID).
    // A null ID means the restriction applies to all attributes/commands/events
    // of the specified type for the given cluster and endpoint.

    // Create a request with a wildcard restriction
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = OnOff::Id;

    // Create a restriction with null ID (wildcard)
    // This means "forbid access to all attributes" on this cluster/endpoint
    AccessControl::Structs::AccessRestrictionStruct::Type restriction;
    restriction.type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restriction.id.SetNull(); // Wildcard - applies to all attributes

    entry.restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction, 1);
    request.arl        = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(&entry, 1);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(mMockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_FALSE(mMockProvider.mLastArl[0].restrictions[0].id.HasValue());
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_MultipleRestrictionsPerEntry)
{
    // Test the command with an ARL entry containing multiple restrictions.
    // A single entry can have multiple restrictions of different types.

    // Create an entry with multiple restrictions
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = OnOff::Id;

    // Create multiple restrictions for the same entry
    AccessControl::Structs::AccessRestrictionStruct::Type restrictions[3];

    // First restriction: forbid attribute access for attribute 0x0000
    restrictions[0].type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restrictions[0].id.SetNonNull(0x0000);

    // Second restriction: forbid attribute writes for attribute 0x0001
    restrictions[1].type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeWriteForbidden;
    restrictions[1].id.SetNonNull(0x0001);

    // Third restriction: forbid command invocation for command 0x0000
    restrictions[2].type = AccessControl::Enums::AccessRestrictionTypeEnum::kCommandForbidden;
    restrictions[2].id.SetNonNull(0x0000);

    // Wrap all restrictions in a list
    entry.restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(restrictions, 3);

    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(&entry, 1);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(mMockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions.size(), 3u);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[0].restrictionType,
              Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[1].restrictionType,
              Access::AccessRestrictionProvider::Type::kAttributeWriteForbidden);
    ASSERT_EQ(mMockProvider.mLastArl[0].restrictions[2].restrictionType,
              Access::AccessRestrictionProvider::Type::kCommandForbidden);
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_ProviderError)
{
    // Test error handling when the AccessRestrictionProvider returns an error.
    // The command handler should propagate the error from the provider.

    // Configure the mock to return an error when RequestFabricRestrictionReview is called
    // This simulates a failure scenario (e.g., provider unable to process the request)
    mMockProvider.mReturnError = CHIP_ERROR_INTERNAL;

    // Create a minimal valid request with empty ARL list
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(nullptr, 0);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.status.has_value());
    ASSERT_EQ(result.status->GetStatus(), CHIP_ERROR_INTERNAL);
    ASSERT_EQ(mMockProvider.mRequestCount, 1u);
}

TEST_F(TestAccessControlClusterWithMockProvider, ReviewFabricRestrictionsCommand_AllRestrictionTypes)
{
    // Test that all restriction types are correctly handled by the command.
    // This verifies that the command handler correctly converts enum values
    // from the Matter data model to the internal AccessRestrictionProvider types.

    constexpr size_t kNumRestrictionTypes = 4;
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entries[kNumRestrictionTypes];
    AccessControl::Structs::AccessRestrictionStruct::Type restrictions[kNumRestrictionTypes];

    const struct
    {
        AccessControl::Enums::AccessRestrictionTypeEnum dmType;
        Access::AccessRestrictionProvider::Type internalType;
    } typeMappings[kNumRestrictionTypes] = {
        { AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden,
          Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden },
        { AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeWriteForbidden,
          Access::AccessRestrictionProvider::Type::kAttributeWriteForbidden },
        { AccessControl::Enums::AccessRestrictionTypeEnum::kCommandForbidden,
          Access::AccessRestrictionProvider::Type::kCommandForbidden },
        { AccessControl::Enums::AccessRestrictionTypeEnum::kEventForbidden,
          Access::AccessRestrictionProvider::Type::kEventForbidden },
    };

    for (size_t i = 0; i < kNumRestrictionTypes; ++i)
    {
        entries[i].endpoint  = 1;
        entries[i].cluster   = OnOff::Id;
        restrictions[i].type = typeMappings[i].dmType;
        restrictions[i].id.SetNonNull(static_cast<uint32_t>(i));
        entries[i].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restrictions[i], 1);
    }

    // Build request with all entries
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(
        entries, kNumRestrictionTypes);

    auto result = mTester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(mMockProvider.mLastArl.size(), kNumRestrictionTypes);

    for (size_t i = 0; i < kNumRestrictionTypes; ++i)
    {
        ASSERT_EQ(mMockProvider.mLastArl[i].restrictions[0].restrictionType, typeMappings[i].internalType);
    }
}
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

} // namespace
