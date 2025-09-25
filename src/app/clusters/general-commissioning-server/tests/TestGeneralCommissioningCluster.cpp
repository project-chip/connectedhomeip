/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <clusters/GeneralCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestGeneralCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestGeneralCommissioningCluster, TestAttributes)
{
    {
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(GeneralCommissioningCluster::Instance().Attributes({ kRootEndpointId, GeneralCommissioning::Id }, builder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Breadcrumb::kMetadataEntry,
                      BasicCommissioningInfo::kMetadataEntry,
                      RegulatoryConfig::kMetadataEntry,
                      LocationCapability::kMetadataEntry,
                      SupportsConcurrentConnection::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        ASSERT_EQ(expectedBuilder.AppendElements({
                      TCAcceptedVersion::kMetadataEntry,
                      TCMinRequiredVersion::kMetadataEntry,
                      TCAcknowledgements::kMetadataEntry,
                      TCAcknowledgementsRequired::kMetadataEntry,
                      TCUpdateDeadline::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
#endif
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

TEST_F(TestGeneralCommissioningCluster, TestAcceptedCommands)
{
    {
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(GeneralCommissioningCluster::Instance().AcceptedCommands({ kRootEndpointId, GeneralCommissioning::Id }, builder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::ArmFailSafe::kMetadataEntry,
                      Commands::SetRegulatoryConfig::kMetadataEntry,
                      Commands::CommissioningComplete::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::SetTCAcknowledgements::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
#endif

        EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

} // namespace
