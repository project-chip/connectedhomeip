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

#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
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

#include "FakeWifiDriver.h"

namespace {

using namespace chip;
using namespace chip::app::Clusters;

using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestNetworkCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkCommissioningCluster, TestAttributes)
{
    {
        Testing::FakeWiFiDriver fakeWifiDriver;
        NetworkCommissioningCluster cluster(kRootEndpointId, &fakeWifiDriver);

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, NetworkCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      NetworkCommissioning::Attributes::MaxNetworks::kMetadataEntry,
                      NetworkCommissioning::Attributes::Networks::kMetadataEntry,
                      NetworkCommissioning::Attributes::InterfaceEnabled::kMetadataEntry,
                      NetworkCommissioning::Attributes::LastNetworkingStatus::kMetadataEntry,
                      NetworkCommissioning::Attributes::LastNetworkID::kMetadataEntry,
                      NetworkCommissioning::Attributes::LastConnectErrorValue::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        // NOTE: this is AWKWARD: we pass in a wifi driver, yet attributes are still depending
        //       on device enabling. Ideally we should not allow compiling odd things at all.
        //       For now keep the logic as inherited from previous implementation.
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        ASSERT_EQ(expectedBuilder.AppendElements({
                      NetworkCommissioning::Attributes::ScanMaxTimeSeconds::kMetadataEntry,
                      NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::kMetadataEntry,
                      NetworkCommissioning::Attributes::SupportedWiFiBands::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
#endif

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    // TODO: more tests for ethernet and thread should be added
}

} // namespace
