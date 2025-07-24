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

#include <app/AttributeValueEncoder.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-cluster.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-logic.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/SoftwareDiagnostics/Enums.h>
#include <clusters/SoftwareDiagnostics/Metadata.h>
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

template <class T>
class ScopedDiagnosticsProvider
{
public:
    ScopedDiagnosticsProvider()
    {
        mOldProvider = &DeviceLayer::GetDiagnosticDataProvider();
        DeviceLayer::SetDiagnosticDataProvider(&mProvider);
    }
    ~ScopedDiagnosticsProvider() { DeviceLayer::SetDiagnosticDataProvider(mOldProvider); }

    ScopedDiagnosticsProvider(const ScopedDiagnosticsProvider &)             = delete;
    ScopedDiagnosticsProvider & operator=(const ScopedDiagnosticsProvider &) = delete;
    ScopedDiagnosticsProvider(ScopedDiagnosticsProvider &&)                  = delete;
    ScopedDiagnosticsProvider & operator=(ScopedDiagnosticsProvider &&)      = delete;

private:
    DeviceLayer::DiagnosticDataProvider * mOldProvider;
    T mProvider;
};

struct TestSoftwareDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestSoftwareDiagnosticsCluster, CompileTest)
{
    const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
        .enableThreadMetrics     = false,
        .enableCurrentHeapFree   = false,
        .enableCurrentHeapUsed   = false,
        .enableCurrentWatermarks = false,
    };

    // The cluster should compile for any logic
    SoftwareDiagnosticsServerCluster cluster(enabledAttributes);

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, SoftwareDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestSoftwareDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };
        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = false,
            .enableCurrentHeapFree   = false,
            .enableCurrentHeapUsed   = false,
            .enableCurrentWatermarks = false,
        };
        ScopedDiagnosticsProvider<NullProvider> nullProvider;
        SoftwareDiagnosticsLogic diag(enabledAttributes);

        // without watermarks, no commands are accepted
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);
        ASSERT_EQ(commandsBuilder.TakeBuffer().size(), 0u);

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{});

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), DefaultServerCluster::GlobalAttributes()));
    }

    {
        class WatermarksProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            bool SupportsWatermarks() override { return true; }
        };

        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = false,
            .enableCurrentHeapFree   = false,
            .enableCurrentHeapUsed   = false,
            .enableCurrentWatermarks = true,
        };

        ScopedDiagnosticsProvider<WatermarksProvider> watermarksProvider;
        SoftwareDiagnosticsLogic diag(enabledAttributes);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, SoftwareDiagnostics::Commands::ResetWatermarks::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  SoftwareDiagnostics::Commands::ResetWatermarks::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks });

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({ SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            bool SupportsWatermarks() override { return true; }

            CHIP_ERROR GetCurrentHeapFree(uint64_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetCurrentHeapUsed(uint64_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & v) override
            {
                v = 456;
                return CHIP_NO_ERROR;
            }
        };

        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = true,
            .enableCurrentHeapFree   = true,
            .enableCurrentHeapUsed   = true,
            .enableCurrentWatermarks = true,
        };

        ScopedDiagnosticsProvider<AllProvider> allProvider;
        SoftwareDiagnosticsLogic diag(enabledAttributes);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, SoftwareDiagnostics::Commands::ResetWatermarks::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  SoftwareDiagnostics::Commands::ResetWatermarks::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks });

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::CurrentHeapFree::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::CurrentHeapUsed::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::ThreadMetrics::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

        // assert values are read correctly
        uint64_t value = 0;

        EXPECT_EQ(diag.GetCurrentHeapFree(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 123u);

        EXPECT_EQ(diag.GetCurrentHeapUsed(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 234u);

        EXPECT_EQ(diag.GetCurrentHighWatermark(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 456u);
    }
}

TEST_F(TestSoftwareDiagnosticsCluster, ReadAttributeTest)
{
    // Test provider that supports all diagnostics
    class TestProvider : public DeviceLayer::DiagnosticDataProvider
    {
    public:
        bool SupportsWatermarks() override { return true; }

        CHIP_ERROR GetCurrentHeapFree(uint64_t & v) override
        {
            v = 1000;
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR GetCurrentHeapUsed(uint64_t & v) override
        {
            v = 2000;
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & v) override
        {
            v = 3000;
            return CHIP_NO_ERROR;
        }
    };

    ScopedDiagnosticsProvider<TestProvider> testProvider;

    // Helper function to test attribute reading
    auto testAttributeRead = [](SoftwareDiagnosticsServerCluster & cluster, AttributeId attributeId,
                                Protocols::InteractionModel::Status expectedStatus, const char * attributeName) {
        uint8_t encoderBuffer[128];
        TLV::TLVWriter writer;
        writer.Init(encoderBuffer);

        AttributeReportIBs::Builder attributeReportIBsBuilder;
        attributeReportIBsBuilder.Init(&writer);
        Access::SubjectDescriptor subject;
        const ConcreteAttributePath path(kRootEndpointId, SoftwareDiagnostics::Id, attributeId);
        AttributeEncodeState state;

        AttributeValueEncoder encoder(attributeReportIBsBuilder, subject, path, 0, false, state);

        DataModel::ReadAttributeRequest request;
        request.path = { kRootEndpointId, SoftwareDiagnostics::Id, attributeId };

        DataModel::ActionReturnStatus status = cluster.ReadAttribute(request, encoder);
        EXPECT_EQ(status, expectedStatus) << "Failed for attribute: " << attributeName;
    };

    // Test case 1: All attributes disabled - should return UnsupportedAttribute
    {
        const SoftwareDiagnosticsEnabledAttributes disabledAttributes{
            .enableThreadMetrics     = false,
            .enableCurrentHeapFree   = false,
            .enableCurrentHeapUsed   = false,
            .enableCurrentWatermarks = false,
        };

        SoftwareDiagnosticsServerCluster cluster(disabledAttributes);

        // Table of attributes to test when disabled
        struct AttributeTestCase
        {
            AttributeId attributeId;
            const char * name;
        };

        const AttributeTestCase disabledTestCases[] = {
            { SoftwareDiagnostics::Attributes::CurrentHeapFree::Id, "CurrentHeapFree" },
            { SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id, "CurrentHeapUsed" },
            { SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id, "CurrentHeapHighWatermark" },
            { SoftwareDiagnostics::Attributes::ThreadMetrics::Id, "ThreadMetrics" },
        };

        for (const auto & testCase : disabledTestCases)
        {
            testAttributeRead(cluster, testCase.attributeId, Protocols::InteractionModel::Status::UnsupportedAttribute,
                              testCase.name);
        }
    }

    // Test case 2: All attributes enabled - should return proper values
    {
        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = true,
            .enableCurrentHeapFree   = true,
            .enableCurrentHeapUsed   = true,
            .enableCurrentWatermarks = true,
        };

        SoftwareDiagnosticsServerCluster cluster(enabledAttributes);

        // Table of attributes to test when enabled
        struct AttributeTestCase
        {
            AttributeId attributeId;
            const char * name;
            Protocols::InteractionModel::Status expectedStatus;
        };

        const AttributeTestCase enabledTestCases[] = {
            { SoftwareDiagnostics::Attributes::CurrentHeapFree::Id, "CurrentHeapFree",
              Protocols::InteractionModel::Status::Success },
            { SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id, "CurrentHeapUsed",
              Protocols::InteractionModel::Status::Success },
            { SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id, "CurrentHeapHighWatermark",
              Protocols::InteractionModel::Status::Success },
            { SoftwareDiagnostics::Attributes::ThreadMetrics::Id, "ThreadMetrics", Protocols::InteractionModel::Status::Success },
            { Globals::Attributes::FeatureMap::Id, "FeatureMap", Protocols::InteractionModel::Status::Success },
            { Globals::Attributes::ClusterRevision::Id, "ClusterRevision", Protocols::InteractionModel::Status::Success },
        };

        for (const auto & testCase : enabledTestCases)
        {
            testAttributeRead(cluster, testCase.attributeId, testCase.expectedStatus, testCase.name);
        }
    }

    // Test case 3: Unknown attribute - should return UnsupportedAttribute
    {
        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = true,
            .enableCurrentHeapFree   = true,
            .enableCurrentHeapUsed   = true,
            .enableCurrentWatermarks = true,
        };

        SoftwareDiagnosticsServerCluster cluster(enabledAttributes);

        testAttributeRead(cluster, 0xFFFF, Protocols::InteractionModel::Status::UnsupportedAttribute, "InvalidAttribute");
    }
}

} // namespace
