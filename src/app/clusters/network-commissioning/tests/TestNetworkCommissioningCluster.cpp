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
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/GeneralCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Metadata.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/NetworkCommissioning.h>
#include <vector>

#include "FakeDrivers.h"

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning::Attributes;

using chip::app::AttributeValueDecoder;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::kAdminSubjectDescriptor;
using chip::Testing::WriteOperation;

class NoopBreadcrumbTracker : public BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t v) override {}
};
// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestNetworkCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkCommissioningCluster, TestAttributes)
{
    NoopBreadcrumbTracker tracker;
    {
        Testing::FakeWiFiDriver fakeWifiDriver;

        NetworkCommissioningCluster cluster(kRootEndpointId, &fakeWifiDriver, tracker);

        // NOTE: this is AWKWARD: we pass in a wifi driver, yet attributes are still depending
        //       on device enabling. Ideally we should not allow compiling odd things at all.
        //       For now keep the logic as inherited from previous implementation.
        std::vector<AttributeEntry> expectedAttributes;
        expectedAttributes.push_back(MaxNetworks::kMetadataEntry);
        expectedAttributes.push_back(Networks::kMetadataEntry);
        expectedAttributes.push_back(InterfaceEnabled::kMetadataEntry);
        expectedAttributes.push_back(LastNetworkingStatus::kMetadataEntry);
        expectedAttributes.push_back(LastNetworkID::kMetadataEntry);
        expectedAttributes.push_back(LastConnectErrorValue::kMetadataEntry);

#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        expectedAttributes.push_back(ScanMaxTimeSeconds::kMetadataEntry);
        expectedAttributes.push_back(ConnectMaxTimeSeconds::kMetadataEntry);
        expectedAttributes.push_back(SupportedWiFiBands::kMetadataEntry);
#endif

        ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
    }

    // TODO: more tests for ethernet and thread should be added
}

TEST_F(TestNetworkCommissioningCluster, TestNotifyOnEnableInterface)
{
    Testing::FakeWiFiDriver fakeWifiDriver;
    NoopBreadcrumbTracker tracker;
    NetworkCommissioningCluster cluster(kRootEndpointId, &fakeWifiDriver, tracker);

    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    {
        WriteOperation writeOp(kRootEndpointId, NetworkCommissioning::Id, InterfaceEnabled::Id);
        writeOp.SetSubjectDescriptor(kAdminSubjectDescriptor);
        AttributeValueDecoder decoder = writeOp.DecoderFor(false);

        // no notification if enable fails
        context.ChangeListener().DirtyList().clear();
        fakeWifiDriver.SetEnabledAllowed(false);
        ASSERT_FALSE(cluster.WriteAttribute(writeOp.GetRequest(), decoder).IsSuccess());
        ASSERT_TRUE(context.ChangeListener().DirtyList().empty());
    }

    {
        WriteOperation writeOp(kRootEndpointId, NetworkCommissioning::Id, InterfaceEnabled::Id);
        writeOp.SetSubjectDescriptor(kAdminSubjectDescriptor);
        AttributeValueDecoder decoder = writeOp.DecoderFor(true);

        // Receive a notification if enable succeeds
        context.ChangeListener().DirtyList().clear();
        fakeWifiDriver.SetEnabledAllowed(true);
        ASSERT_TRUE(cluster.WriteAttribute(writeOp.GetRequest(), decoder).IsSuccess());
        ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
        ASSERT_EQ(context.ChangeListener().DirtyList()[0],
                  app::AttributePathParams(kRootEndpointId, NetworkCommissioning::Id, InterfaceEnabled::Id)

        );
    }

    cluster.Shutdown();
}

} // namespace
