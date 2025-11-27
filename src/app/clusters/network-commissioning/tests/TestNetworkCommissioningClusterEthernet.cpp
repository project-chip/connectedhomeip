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

#include <app/AttributePathParams.h>
#include <app/AttributeValueDecoder.h>
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/GeneralCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Metadata.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

#include "FakeDrivers.h"

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning;

using chip::app::AttributeValueDecoder;
using chip::app::DataModel::AttributeEntry;
using chip::app::Testing::kAdminSubjectDescriptor;
using chip::app::Testing::WriteOperation;

class NoopBreadcrumbTracker : public BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t v) override {}
};
// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestNetworkCommissioningClusterEthernet : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkCommissioningClusterEthernet, TestAttributes)
{
    NoopBreadcrumbTracker tracker;
    Testing::FakeEthernetDriver fakeEthernetDriver;
    ByteSpan testInterfaceName(Uint8::from_const_char("eth0_test"), 9);
    NetworkCommissioningCluster cluster(kRootEndpointId, &fakeEthernetDriver, tracker);
    chip::Test::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Init(), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Cluster Revision
    {
        Attributes::ClusterRevision::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::ClusterRevision::Id, value).IsSuccess());
        ASSERT_EQ(value, NetworkCommissioning::kRevision);
    }

    // Feature Map
    {
        Attributes::FeatureMap::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::FeatureMap::Id, value).IsSuccess());
        ASSERT_EQ(value, BitFlags<Feature>{ Feature::kEthernetNetworkInterface }.Raw());
    }

    // Attribute List
    ASSERT_TRUE(Testing::IsAttributesListEqualTo(cluster,
                                                 {
                                                     Attributes::MaxNetworks::kMetadataEntry,
                                                     Attributes::Networks::kMetadataEntry,
                                                     Attributes::InterfaceEnabled::kMetadataEntry,
                                                     Attributes::LastNetworkingStatus::kMetadataEntry,
                                                     Attributes::LastNetworkID::kMetadataEntry,
                                                     Attributes::LastConnectErrorValue::kMetadataEntry,
                                                 }));

    // Accepted Commands List
    // No commands when ethernet only
    ASSERT_TRUE(Testing::IsAcceptedCommandsListEqualTo(cluster, {}));

    // Generated Commands List
    ASSERT_TRUE(Testing::IsGeneratedCommandsListEqualTo(cluster, {}));

    // Max Networks
    {
        Attributes::MaxNetworks::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::MaxNetworks::Id, value).IsSuccess());
        ASSERT_EQ(value, fakeEthernetDriver.GetMaxNetworks());
    }

    // ---- No Ethernet Connected ----
    fakeEthernetDriver.SetNoNetwork();

    // Networks
    {
        Attributes::Networks::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::Networks::Id, value).IsSuccess());
        auto it = value.begin();
        EXPECT_FALSE(it.Next()); // no networks
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    }

    // Last Networking Status
    {
        Attributes::LastNetworkingStatus::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastNetworkingStatus::Id, value).IsSuccess());
        EXPECT_TRUE(value.IsNull());
    }

    // Last Network ID
    {
        Attributes::LastNetworkID::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastNetworkID::Id, value).IsSuccess());
        EXPECT_TRUE(value.IsNull());
    }

    // Last Connect Error Value
    {
        Attributes::LastConnectErrorValue::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastConnectErrorValue::Id, value).IsSuccess());
        EXPECT_TRUE(value.IsNull());
    }

    // ---- With Ethernet Connected ----
    fakeEthernetDriver.SetNetwork(testInterfaceName);
    fakeEthernetDriver.SetNetworkConnected(true);

    // Networks
    {
        Attributes::Networks::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::Networks::Id, value).IsSuccess());
        auto it = value.begin();
        ASSERT_TRUE(it.Next());
        const auto & network = it.GetValue();
        EXPECT_TRUE(testInterfaceName.data_equal(network.networkID));
        EXPECT_TRUE(network.connected);
        EXPECT_FALSE(it.Next());
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    }

    // Last Networking Status
    {
        Attributes::LastNetworkingStatus::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastNetworkingStatus::Id, value).IsSuccess());
        ASSERT_FALSE(value.IsNull());
        EXPECT_EQ(value.Value(), NetworkCommissioningStatusEnum::kSuccess);
    }

    // Last Network ID
    {
        Attributes::LastNetworkID::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastNetworkID::Id, value).IsSuccess());
        ASSERT_FALSE(value.IsNull());
        EXPECT_TRUE(testInterfaceName.data_equal(value.Value()));
    }

    // Last Connect Error Value
    {
        Attributes::LastConnectErrorValue::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::LastConnectErrorValue::Id, value).IsSuccess());
        ASSERT_FALSE(value.IsNull());
        EXPECT_EQ(value.Value(), fakeEthernetDriver.testErrorValue.Value());
    }

    // -------------------------

    // Interface Enabled
    {
        Attributes::InterfaceEnabled::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::InterfaceEnabled::Id, value).IsSuccess());
        EXPECT_TRUE(value); // default enabled
    }

    // Try enabling the interface when it is alread enabled and disabling is not supported
    // This should be success
    fakeEthernetDriver.EnableDisabling(false);
    ASSERT_TRUE(tester.WriteAttribute(Attributes::InterfaceEnabled::Id, true).IsSuccess());

    // Try disabling the interface when it is not supported
    // This should fail with InvalidAction
    ASSERT_EQ(tester.WriteAttribute(Attributes::InterfaceEnabled::Id, false).GetStatusCode().GetStatus(),
              Protocols::InteractionModel::Status::InvalidAction);

    // Now try disabling when it is supported
    // This should succeed and mark the attribute as dirty
    fakeEthernetDriver.EnableDisabling(true);
    ASSERT_TRUE(tester.WriteAttribute(Attributes::InterfaceEnabled::Id, false).IsSuccess());
    EXPECT_FALSE(tester.GetDirtyList().empty());

    // Verify persistence
    {
        Attributes::InterfaceEnabled::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::InterfaceEnabled::Id, value).IsSuccess());
        EXPECT_FALSE(value); // disabled
    }

    cluster.Shutdown();
    cluster.Deinit();

    ASSERT_EQ(cluster.Init(), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    {
        Attributes::InterfaceEnabled::TypeInfo::DecodableType value;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::InterfaceEnabled::Id, value).IsSuccess());
        EXPECT_FALSE(value); // stays disabled
    }

    cluster.Shutdown();
    cluster.Deinit();
}

} // namespace
