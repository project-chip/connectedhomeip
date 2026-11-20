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

#include <app/clusters/descriptor/DescriptorCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Descriptor/Enums.h>
#include <clusters/Descriptor/Metadata.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/DiagnosticDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor::Attributes;
using namespace chip::app::DataModel;
using chip::Testing::IsAttributesListEqualTo;

struct TestDescriptorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestDescriptorCluster, CompileTest)
{
    DescriptorCluster cluster(1, DescriptorCluster::OptionalAttributesSet(0), {});
    ASSERT_EQ(cluster.GetClusterFlags({ 1, Descriptor::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestDescriptorCluster, AttributesTest)
{
    // Cluster configuration with only mandatory attributes
    DescriptorCluster cluster(1, DescriptorCluster::OptionalAttributesSet(0), {});

    chip::Testing::TestServerClusterContext context;
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Descriptor::Attributes::DeviceTypeList::kMetadataEntry,
                                            Descriptor::Attributes::ServerList::kMetadataEntry,
                                            Descriptor::Attributes::ClientList::kMetadataEntry,
                                            Descriptor::Attributes::PartsList::kMetadataEntry,
                                        }));
}

TEST_F(TestDescriptorCluster, OnEndpointChangedTest)
{
    Testing::TestServerClusterContext mContext;
    auto & listener = mContext.ReleaseListener();

    SpanEndpoint rootEndpoint = SpanEndpoint::Builder().Build();
    SpanEndpoint endpoint1    = SpanEndpoint::Builder().Build();
    SpanEndpoint endpoint2    = SpanEndpoint::Builder().Build();
    SpanEndpoint endpoint3    = SpanEndpoint::Builder().Build();
    SpanEndpoint endpoint4    = SpanEndpoint::Builder().Build();

    // Deliberately set the composition patter to tree, it still should behave like full-family.
    EndpointInterfaceRegistration rootEndpointRegistration(
        rootEndpoint,
        { .id                 = kRootEndpointId,
          .parentId           = kInvalidEndpointId,
          .compositionPattern = chip::app::DataModel::EndpointCompositionPattern::kTree });

    EndpointInterfaceRegistration endpointRegistration1(
        endpoint1,
        { .id                 = 1,
          .parentId           = rootEndpointRegistration.GetEndpointEntry().id,
          .compositionPattern = chip::app::DataModel::EndpointCompositionPattern::kFullFamily });

    EndpointInterfaceRegistration endpointRegistration2(
        endpoint2,
        { .id                 = 2,
          .parentId           = endpointRegistration1.GetEndpointEntry().id,
          .compositionPattern = chip::app::DataModel::EndpointCompositionPattern::kTree });

    EndpointInterfaceRegistration endpointRegistration3(
        endpoint3,
        { .id                 = 3,
          .parentId           = endpointRegistration2.GetEndpointEntry().id,
          .compositionPattern = chip::app::DataModel::EndpointCompositionPattern::kTree });

    EndpointInterfaceRegistration endpointRegistration4(
        endpoint4,
        { .id                 = 4,
          .parentId           = endpointRegistration3.GetEndpointEntry().id,
          .compositionPattern = chip::app::DataModel::EndpointCompositionPattern::kTree });

    DescriptorCluster rootDescriptor(rootEndpointRegistration.GetEndpointEntry().id, {}, {});
    DescriptorCluster descriptor1(endpointRegistration1.GetEndpointEntry().id, {}, {});
    DescriptorCluster descriptor2(endpointRegistration2.GetEndpointEntry().id, {}, {});
    DescriptorCluster descriptor3(endpointRegistration3.GetEndpointEntry().id, {}, {});
    DescriptorCluster descriptor4(endpointRegistration4.GetEndpointEntry().id, {}, {});

    ServerClusterRegistration rootDescriptorEntry(rootDescriptor);
    ServerClusterRegistration descriptorEntry1(descriptor1);
    ServerClusterRegistration descriptorEntry2(descriptor2);
    ServerClusterRegistration descriptorEntry3(descriptor3);
    ServerClusterRegistration descriptorEntry4(descriptor4);

    CodeDrivenDataModelProvider provider(mContext.StorageDelegate(), mContext.AttributePersistenceProvider());
    ASSERT_SUCCESS(provider.Startup(mContext.ImContext()));

    ASSERT_SUCCESS(provider.AddCluster(rootDescriptorEntry));
    ASSERT_SUCCESS(provider.AddCluster(descriptorEntry1));
    ASSERT_SUCCESS(provider.AddCluster(descriptorEntry2));
    ASSERT_SUCCESS(provider.AddCluster(descriptorEntry3));
    ASSERT_SUCCESS(provider.AddCluster(descriptorEntry4));

    ASSERT_SUCCESS(provider.AddEndpoint(rootEndpointRegistration));
    ASSERT_SUCCESS(provider.AddEndpoint(endpointRegistration1));
    ASSERT_SUCCESS(provider.AddEndpoint(endpointRegistration2));
    ASSERT_SUCCESS(provider.AddEndpoint(endpointRegistration3));

    // Start testing, enable the notifications.
    provider.RegisterAttributeChangeListener(listener);

    EXPECT_SUCCESS(provider.AddEndpoint(endpointRegistration4));

    // Root endpoint, endpoint1 because it is full-family, endpoint3 because it is the parent endpoint
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ rootEndpointRegistration.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ endpointRegistration1.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ endpointRegistration3.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));

    EXPECT_EQ(mContext.ChangeListener().DirtyList().size(), 3u);

    listener.DirtyList().clear();

    EXPECT_SUCCESS(provider.RemoveEndpoint(endpointRegistration4.GetEndpointEntry().id));

    // Root endpoint, endpoint1 because it is full-family, endpoint3 because it is the parent endpoint
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ rootEndpointRegistration.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ endpointRegistration1.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));
    EXPECT_TRUE(mContext.ChangeListener().IsDirty(
        ConcreteAttributePath{ endpointRegistration3.GetEndpointEntry().id, Descriptor::Id, PartsList::Id }));

    EXPECT_EQ(mContext.ChangeListener().DirtyList().size(), 3u);

    EXPECT_SUCCESS(provider.Shutdown());
}

} // namespace
