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
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/AccessControl/Enums.h>
#include <clusters/AccessControl/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

// Simple DeviceTypeResolver for tests
class TestDeviceTypeResolver : public chip::Access::AccessControl::DeviceTypeResolver
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
class TestAccessRestrictionProvider : public chip::Access::AccessRestrictionProvider
{
public:
    // Override DoRequestFabricRestrictionReview to capture call parameters
    // and allow tests to control the return value.
    CHIP_ERROR DoRequestFabricRestrictionReview(const chip::FabricIndex fabricIndex, uint64_t token,
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
    chip::FabricIndex mLastFabricIndex = chip::kUndefinedFabricIndex;
    uint64_t mLastToken                = 0;
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
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        // Initialize AccessControl for reading mandatory attributes
        chip::Access::AccessControl::Delegate * delegate = chip::Access::Examples::GetAccessControlDelegate();
        ASSERT_EQ(chip::Access::GetAccessControl().Init(delegate, gTestDeviceTypeResolver), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::Access::GetAccessControl().Finish();
        chip::Platform::MemoryShutdown();
    }
};

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

    ReadOnlyBufferBuilder<chip::CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(accessControlPath, generatedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<chip::CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Check accepted commands
    ASSERT_EQ(acceptedCommands.size(), AccessControl::Commands::kAcceptedCommandsCount);
    ASSERT_EQ(acceptedCommands[0].commandId, AccessControl::Commands::ReviewFabricRestrictions::Id);
    ASSERT_EQ(acceptedCommands[0].GetInvokePrivilege(),
              AccessControl::Commands::ReviewFabricRestrictions::kMetadataEntry.GetInvokePrivilege());

    // Check generated commands
    ASSERT_EQ(generatedCommands.size(), AccessControl::Commands::kGeneratedCommandsCount);
    ASSERT_EQ(generatedCommands[0], AccessControl::Commands::ReviewFabricRestrictionsResponse::Id);
#else
    ASSERT_EQ(acceptedCommands.size(), (size_t) (0));
    ASSERT_EQ(generatedCommands.size(), (size_t) (0));
#endif
}

TEST_F(TestAccessControlCluster, AttributesTest)
{
    AccessControlCluster cluster;
    ConcreteClusterPath accessControlPath = ConcreteClusterPath(kRootEndpointId, AccessControl::Id);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
    ASSERT_EQ(cluster.Attributes(accessControlPath, attributesBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

    ASSERT_EQ(expectedBuilder.AppendElements({
#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
        AccessControl::Attributes::Extension::kMetadataEntry,
#endif

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
            AccessControl::Attributes::CommissioningARL::kMetadataEntry, AccessControl::Attributes::Arl::kMetadataEntry
#endif
    }),
              CHIP_NO_ERROR);
    ASSERT_EQ(expectedBuilder.AppendElements(AccessControl::Attributes::kMandatoryMetadata), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
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
    chip::Test::ClusterTester tester(cluster);

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
}

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_Success)
{
    // Set up a mock AccessRestrictionProvider to capture the review request
    // This allows us to verify that the command handler correctly processes
    // the command and calls the provider with the expected parameters.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    // Initialize the cluster and tester
    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Build a ReviewFabricRestrictions command request with ARL entries.
    // The request contains a list of CommissioningAccessRestrictionEntryStruct entries,
    // each specifying an endpoint, cluster, and list of restrictions.
    AccessControl::Commands::ReviewFabricRestrictions::Type request;

    // Create a single ARL entry for endpoint 1, OnOff cluster (0x0006)
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = 0x0006; // OnOff cluster ID

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
    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_GT(result.response->token, 0u);
    ASSERT_EQ(mockProvider.mRequestCount, 1u);
    ASSERT_EQ(mockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].endpointNumber, 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].clusterId, 0x0006u);

    ASSERT_EQ(mockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_TRUE(mockProvider.mLastArl[0].restrictions[0].id.HasValue());
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[0].id.Value(), 0x0000u);
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_EmptyArl)
{
    // Test the case where an empty ARL list is sent.
    // According to the spec, an empty list means "review all restrictions" for the fabric.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create a request with an empty ARL list
    // This signals a request to review all restrictions for the accessing fabric
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(nullptr, 0);

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_GT(result.response->token, 0u);
    ASSERT_EQ(mockProvider.mRequestCount, 1u);
    ASSERT_EQ(mockProvider.mLastArl.size(), 0u);
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_MultipleEntries)
{
    // Test the command with multiple ARL entries in a single request.
    // This verifies that the command handler correctly processes and forwards
    // all entries to the AccessRestrictionProvider.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create an array of ARL entries for different endpoints and clusters
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entries[2];

    // First entry: endpoint 1, OnOff cluster with attribute access restriction
    entries[0].endpoint = 1;
    entries[0].cluster  = 0x0006; // OnOff cluster
    AccessControl::Structs::AccessRestrictionStruct::Type restriction1;
    restriction1.type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restriction1.id.SetNonNull(0x0000);
    entries[0].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction1, 1);

    // Second entry: endpoint 2, Level Control cluster (0x0008) with command restriction
    entries[1].endpoint = 2;
    entries[1].cluster  = 0x0008; // Level Control cluster
    AccessControl::Structs::AccessRestrictionStruct::Type restriction2;
    restriction2.type = AccessControl::Enums::AccessRestrictionTypeEnum::kCommandForbidden;
    restriction2.id.SetNonNull(0x0000);
    entries[1].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction2, 1);

    // Build the request with both entries
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(entries, 2);

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_EQ(mockProvider.mRequestCount, 1u);
    ASSERT_EQ(mockProvider.mLastArl.size(), 2u);
    ASSERT_EQ(mockProvider.mLastArl[0].endpointNumber, 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].clusterId, 0x0006u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_EQ(mockProvider.mLastArl[1].endpointNumber, 2u);
    ASSERT_EQ(mockProvider.mLastArl[1].clusterId, 0x0008u);
    ASSERT_EQ(mockProvider.mLastArl[1].restrictions.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[1].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kCommandForbidden);
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_WildcardRestriction)
{
    // Test the command with a wildcard restriction (null ID).
    // A null ID means the restriction applies to all attributes/commands/events
    // of the specified type for the given cluster and endpoint.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create a request with a wildcard restriction
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = 0x0006; // OnOff cluster

    // Create a restriction with null ID (wildcard)
    // This means "forbid access to all attributes" on this cluster/endpoint
    AccessControl::Structs::AccessRestrictionStruct::Type restriction;
    restriction.type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restriction.id.SetNull(); // Wildcard - applies to all attributes

    entry.restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restriction, 1);
    request.arl        = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(&entry, 1);

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_EQ(mockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions.size(), 1u);
    ASSERT_FALSE(mockProvider.mLastArl[0].restrictions[0].id.HasValue());
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_MultipleRestrictionsPerEntry)
{
    // Test the command with an ARL entry containing multiple restrictions.
    // A single entry can have multiple restrictions of different types.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create an entry with multiple restrictions
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint = 1;
    entry.cluster  = 0x0006; // OnOff cluster

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

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_EQ(mockProvider.mLastArl.size(), 1u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions.size(), 3u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[1].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeWriteForbidden);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[2].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kCommandForbidden);
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_ProviderError)
{
    // Test error handling when the AccessRestrictionProvider returns an error.
    // The command handler should propagate the error from the provider.
    TestAccessRestrictionProvider mockProvider;

    // Configure the mock to return an error when RequestFabricRestrictionReview is called
    // This simulates a failure scenario (e.g., provider unable to process the request)
    mockProvider.mReturnError = CHIP_ERROR_INTERNAL;

    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create a valid request
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entry;
    entry.endpoint     = 1;
    entry.cluster      = 0x0006;
    entry.restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(nullptr, 0);
    request.arl        = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(&entry, 1);

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_FALSE(result.IsSuccess());
    ASSERT_EQ(mockProvider.mRequestCount, 1u);
}

TEST_F(TestAccessControlCluster, ReviewFabricRestrictionsCommand_AllRestrictionTypes)
{
    // Test that all restriction types are correctly handled by the command.
    // This verifies that the command handler correctly converts enum values
    // from the Matter data model to the internal AccessRestrictionProvider types.
    TestAccessRestrictionProvider mockProvider;
    chip::Access::GetAccessControl().SetAccessRestrictionProvider(&mockProvider);

    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Create entries for each restriction type
    AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type entries[4];
    AccessControl::Structs::AccessRestrictionStruct::Type restrictions[4];

    // Entry 0: AttributeAccessForbidden
    entries[0].endpoint  = 1;
    entries[0].cluster   = 0x0006;
    restrictions[0].type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeAccessForbidden;
    restrictions[0].id.SetNonNull(0x0000);
    entries[0].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restrictions[0], 1);

    // Entry 1: AttributeWriteForbidden
    entries[1].endpoint  = 1;
    entries[1].cluster   = 0x0006;
    restrictions[1].type = AccessControl::Enums::AccessRestrictionTypeEnum::kAttributeWriteForbidden;
    restrictions[1].id.SetNonNull(0x0001);
    entries[1].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restrictions[1], 1);

    // Entry 2: CommandForbidden
    entries[2].endpoint  = 1;
    entries[2].cluster   = 0x0006;
    restrictions[2].type = AccessControl::Enums::AccessRestrictionTypeEnum::kCommandForbidden;
    restrictions[2].id.SetNonNull(0x0000);
    entries[2].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restrictions[2], 1);

    // Entry 3: EventForbidden
    entries[3].endpoint  = 1;
    entries[3].cluster   = 0x0006;
    restrictions[3].type = AccessControl::Enums::AccessRestrictionTypeEnum::kEventForbidden;
    restrictions[3].id.SetNonNull(0x0000);
    entries[3].restrictions = DataModel::List<const AccessControl::Structs::AccessRestrictionStruct::Type>(&restrictions[3], 1);

    // Build request with all entries
    AccessControl::Commands::ReviewFabricRestrictions::Type request;
    request.arl = DataModel::List<const AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type>(entries, 4);

    auto result = tester.Invoke(AccessControl::Commands::ReviewFabricRestrictions::Id, request);

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    ASSERT_EQ(mockProvider.mLastArl.size(), 4u);
    ASSERT_EQ(mockProvider.mLastArl[0].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeAccessForbidden);
    ASSERT_EQ(mockProvider.mLastArl[1].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kAttributeWriteForbidden);
    ASSERT_EQ(mockProvider.mLastArl[2].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kCommandForbidden);
    ASSERT_EQ(mockProvider.mLastArl[3].restrictions[0].restrictionType,
              chip::Access::AccessRestrictionProvider::Type::kEventForbidden);
}
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

} // namespace
