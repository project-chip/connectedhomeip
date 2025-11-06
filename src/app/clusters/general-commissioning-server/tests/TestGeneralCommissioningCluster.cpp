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
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <clusters/GeneralCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <platform/DeviceControlServer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;

using chip::app::AttributeListBuilder;
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
    // test without optional attributes
    {
        GeneralCommissioningCluster cluster(
            GeneralCommissioningCluster::Context {
                .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(), //
                    .configurationManager   = DeviceLayer::ConfigurationMgr(),                       //
                    .deviceControlServer    = DeviceLayer::DeviceControlServer::DeviceControlSvr(),  //
                    .fabricTable            = Server::GetInstance().GetFabricTable(),                //
                    .failsafeContext        = Server::GetInstance().GetFailSafeContext(),            //
                    .platformManager        = DeviceLayer::PlatformMgr(),                            //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                    .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            },
            GeneralCommissioningCluster::OptionalAttributes(0));

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, GeneralCommissioning::Id }, builder), CHIP_NO_ERROR);

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
        // This is both define AND feature map dependent. The feature map is hardcoded
        // based on compile-time defines, so we check the define directly.
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

    // test with optional attributes
    {
        GeneralCommissioningCluster cluster(
            GeneralCommissioningCluster::Context {
                .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(), //
                    .configurationManager   = DeviceLayer::ConfigurationMgr(),                       //
                    .deviceControlServer    = DeviceLayer::DeviceControlServer::DeviceControlSvr(),  //
                    .fabricTable            = Server::GetInstance().GetFabricTable(),                //
                    .failsafeContext        = Server::GetInstance().GetFailSafeContext(),            //
                    .platformManager        = DeviceLayer::PlatformMgr(),                            //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                    .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            },
            GeneralCommissioningCluster::OptionalAttributes().Set<IsCommissioningWithoutPower::Id>());

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, GeneralCommissioning::Id }, builder), CHIP_NO_ERROR);

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
        // This is both define AND feature map dependent. The feature map is hardcoded
        // based on compile-time defines, so we check the define directly.
        ASSERT_EQ(expectedBuilder.AppendElements({
                      TCAcceptedVersion::kMetadataEntry,
                      TCMinRequiredVersion::kMetadataEntry,
                      TCAcknowledgements::kMetadataEntry,
                      TCAcknowledgementsRequired::kMetadataEntry,
                      TCUpdateDeadline::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
#endif

        // Add the optional attribute since it's now enabled
        ASSERT_EQ(expectedBuilder.AppendElements({
                      IsCommissioningWithoutPower::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

TEST_F(TestGeneralCommissioningCluster, TestAcceptedCommands)
{
    GeneralCommissioningCluster cluster(
        GeneralCommissioningCluster::Context {
            .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(), //
                .configurationManager   = DeviceLayer::ConfigurationMgr(),                       //
                .deviceControlServer    = DeviceLayer::DeviceControlServer::DeviceControlSvr(),  //
                .fabricTable            = Server::GetInstance().GetFabricTable(),                //
                .failsafeContext        = Server::GetInstance().GetFailSafeContext(),            //
                .platformManager        = DeviceLayer::PlatformMgr(),                            //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        GeneralCommissioningCluster::OptionalAttributes());

    {
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, GeneralCommissioning::Id }, builder), CHIP_NO_ERROR);

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
