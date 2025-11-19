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
template <typename DecodableListType>
size_t CountListElements(DecodableListType & list)
{
    size_t count = 0;
    auto it      = list.begin();
    while (it.Next())
    {
        ++count;
    }
    return count;
}

TEST_F(TestAccessControlCluster, ReadAttributesTest)
{
    // Test that all available attributes (mandatory and optional) can be read
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
    ASSERT_EQ(CountListElements(extension), 0u);
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Read CommissioningARL attribute (optional)
    AccessControl::Attributes::CommissioningARL::TypeInfo::DecodableType commissioningArl;
    status = tester.ReadAttribute(AccessControl::Attributes::CommissioningARL::Id, commissioningArl);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_EQ(CountListElements(commissioningArl), 0u);

    // Read Arl attribute (optional)
    AccessControl::Attributes::Arl::TypeInfo::DecodableType arl;
    status = tester.ReadAttribute(AccessControl::Attributes::Arl::Id, arl);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_EQ(CountListElements(arl), 0u);
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
}

TEST_F(TestAccessControlCluster, WriteAttributesTest)
{
    // Test that writable attributes can be written
    AccessControlCluster cluster;
    chip::Test::ClusterTester tester(cluster);

    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Test writing Acl attribute (writable, mandatory)
    // Write an empty ACL list
    AccessControl::Attributes::Acl::TypeInfo::Type emptyAcl;
    auto status = tester.WriteAttribute(AccessControl::Attributes::Acl::Id, emptyAcl);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the write by reading it back
    AccessControl::Attributes::Acl::TypeInfo::DecodableType readAcl;
    status = tester.ReadAttribute(AccessControl::Attributes::Acl::Id, readAcl);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_EQ(CountListElements(readAcl), 0u);

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    // Test writing Extension attribute (writable, optional)
    AccessControl::Attributes::Extension::TypeInfo::Type emptyExtension;
    status = tester.WriteAttribute(AccessControl::Attributes::Extension::Id, emptyExtension);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the write by reading it back
    AccessControl::Attributes::Extension::TypeInfo::DecodableType readExtension;
    status = tester.ReadAttribute(AccessControl::Attributes::Extension::Id, readExtension);
    ASSERT_TRUE(status.IsSuccess());
    ASSERT_EQ(CountListElements(readExtension), 0u);
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS

    // Test writing to a non-writable attribute should fail
    // SubjectsPerAccessControlEntry is read-only
    uint16_t invalidValue = 10;
    status                = tester.WriteAttribute(AccessControl::Attributes::SubjectsPerAccessControlEntry::Id, invalidValue);
    ASSERT_FALSE(status.IsSuccess());
}

} // namespace
