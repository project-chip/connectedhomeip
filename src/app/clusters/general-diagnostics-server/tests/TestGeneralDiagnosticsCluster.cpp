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

#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-logic.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/GeneralDiagnostics/Enums.h>
#include <clusters/GeneralDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

#include <cmath>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

struct TestGeneralDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestGeneralDiagnosticsCluster, CompileTest)
{
    const GeneralDiagnosticsEnabledAttributes enabledAttributes{
        .enableTotalOperationalHours = false,
        .enableBootReason            = false,
        .enableActiveHardwareFaults  = false,
        .enableActiveRadioFaults     = false,
        .enableActiveNetworkFaults   = false,
    };

    GeneralDiagnosticsCluster<DeviceLayerGeneralDiagnosticsLogic> cluster(enabledAttributes);
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, GeneralDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestGeneralDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };
        const GeneralDiagnosticsEnabledAttributes enabledAttributes{
            .enableTotalOperationalHours = false,
            .enableBootReason            = false,
            .enableActiveHardwareFaults  = false,
            .enableActiveRadioFaults     = false,
            .enableActiveNetworkFaults   = false,
        };
        NullProvider nullProvider;
        InjectedDiagnosticsGeneralDiagnosticsLogic injectedLogic(nullProvider, enabledAttributes);

        // Check mandatory commands are present
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(injectedLogic.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 2u);

        ASSERT_EQ(commands[0].commandId, GeneralDiagnostics::Commands::TestEventTrigger::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(commands[1].commandId, GeneralDiagnostics::Commands::TimeSnapshot::Id);
        ASSERT_EQ(commands[1].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry.GetInvokePrivilege());

        // Everything is unimplemented, so attributes are just the global and mandatory ones.
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(injectedLogic.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                      GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                      GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override
            {
                rebootCount = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override
            {
                totalOperationalHours = 456;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetBootReason(chip::app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason) override
            {
                bootReason = GeneralDiagnostics::BootReasonEnum::kSoftwareReset;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetActiveHardwareFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & hardwareFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetActiveRadioFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & radioFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetActiveNetworkFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & networkFaults) override
            {
                return CHIP_NO_ERROR;
            }
        };

        // Enable all the optional attributes
        const GeneralDiagnosticsEnabledAttributes enabledAttributes {
            .enableTotalOperationalHours = true,
            .enableBootReason = true,
            .enableActiveHardwareFaults = true,
            .enableActiveRadioFaults = true,
            .enableActiveNetworkFaults = true,
        };

        AllProvider allProvider;
        InjectedDiagnosticsGeneralDiagnosticsLogic injectedLogic(allProvider, enabledAttributes);

        // Check mandatory commands are present
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(injectedLogic.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 2u);

        ASSERT_EQ(commands[0].commandId, GeneralDiagnostics::Commands::TestEventTrigger::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(commands[1].commandId, GeneralDiagnostics::Commands::TimeSnapshot::Id);
        ASSERT_EQ(commands[1].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry.GetInvokePrivilege());

        // Everything is implemented, so attributes are the global ones and ALL optional ones as well.
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(injectedLogic.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                      GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                      GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TotalOperationalHours::kMetadataEntry,
                      GeneralDiagnostics::Attributes::BootReason::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveHardwareFaults::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveRadioFaults::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveNetworkFaults::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

        // Check proper read/write of values and returns
        uint16_t rebootCount = 0;
        uint32_t operationalHours = 0;
        GeneralDiagnostics::BootReasonEnum bootReasonValue;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> hardwareFaults;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> radioFaults;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> networkFaults;

        EXPECT_EQ(injectedLogic.GetRebootCount(rebootCount), CHIP_NO_ERROR);
        EXPECT_EQ(rebootCount, 123u);

        EXPECT_EQ(injectedLogic.GetTotalOperationalHours(operationalHours), CHIP_NO_ERROR);
        EXPECT_EQ(operationalHours, 456u);

        EXPECT_EQ(injectedLogic.GetBootReason(bootReasonValue), CHIP_NO_ERROR);
        EXPECT_EQ(bootReasonValue, GeneralDiagnostics::BootReasonEnum::kSoftwareReset);

        EXPECT_EQ(injectedLogic.GetActiveHardwareFaults(hardwareFaults), CHIP_NO_ERROR);
        EXPECT_EQ(injectedLogic.GetActiveRadioFaults(radioFaults), CHIP_NO_ERROR);
        EXPECT_EQ(injectedLogic.GetActiveNetworkFaults(networkFaults), CHIP_NO_ERROR);
    }
}

} // namespace
