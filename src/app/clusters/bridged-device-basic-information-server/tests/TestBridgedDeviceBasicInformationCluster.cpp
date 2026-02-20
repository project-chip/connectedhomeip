/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationCluster.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationDelegate.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/BridgedDeviceBasicInformation/Enums.h>
#include <clusters/BridgedDeviceBasicInformation/Events.h>
#include <clusters/BridgedDeviceBasicInformation/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/Constants.h>
#include <string>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BridgedDeviceBasicInformation;
using namespace chip::Testing;

using chip::Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

class MockDelegate : public BridgedDeviceBasicInformationDelegate
{
public:
    void OnNodeLabelChanged(const std::string & newNodeLabel) override
    {
        mNodeLabelChangedCalled = true;
        mLastNodeLabel          = newNodeLabel;
    }

    bool mNodeLabelChangedCalled = false;
    std::string mLastNodeLabel;
};

class MockVersionConfigurationDelegate : public ConfigurationVersionDelegate
{
public:
    CHIP_ERROR IncreaseConfigurationVersion() override
    {
        mVersion++;
        return CHIP_NO_ERROR;
    }

    uint32_t mVersion = 0;
};

class TestBridgedDeviceIcdDelegate : public BridgedDeviceIcdDelegate
{
public:
    uint32_t mOnKeepActiveCalled = 0;
    Commands::KeepActive::DecodableType mLastKeepActiveRequest;

    Status OnKeepActive(const Commands::KeepActive::DecodableType & request) override
    {
        mOnKeepActiveCalled++;
        mLastKeepActiveRequest = request;
        return Status::Success;
    }
};

struct TestBridgedDeviceBasicInformationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TestServerClusterContext mContext;
    MockDelegate mDelegate;
    MockVersionConfigurationDelegate mMockVersionConfiguration;
};

TEST_F(TestBridgedDeviceBasicInformationCluster, TestEmptyAttributes)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "foo-bar", .reachable = true }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::UniqueID::kMetadataEntry,
                                            Attributes::Reachable::kMetadataEntry,
                                            Attributes::NodeLabel::kMetadataEntry,
                                            Attributes::ConfigurationVersion::kMetadataEntry,
                                        }));
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestPartialAttributes)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "foo-bar", .reachable = true, .nodeLabel = "mylabel", .configurationVersion = 200u },
        {
            .partNumber = "010203",
        },
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::UniqueID::kMetadataEntry,
                                            Attributes::Reachable::kMetadataEntry,
                                            Attributes::NodeLabel::kMetadataEntry,
                                            Attributes::PartNumber::kMetadataEntry,
                                            Attributes::ConfigurationVersion::kMetadataEntry,
                                        }));
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestAllAttributes)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "foo-bar", .reachable = true, .nodeLabel = "Remote", .configurationVersion = 123u },
        {
            .vendorName            = "ACME",
            .vendorId              = VendorId::Common,
            .productName           = "Bridge",
            .productId             = 0x1234,
            .hardwareVersion       = 0x1122,
            .hardwareVersionString = "NewVersion-A",
            .softwareVersion       = 0x11223344,
            .softwareVersionString = "FancyBuild",
            .manufacturingDate     = "010203",
            .partNumber            = "A-B-C",
            .productUrl            = "http://example.com",
            .productLabel          = "New",
            .serialNumber          = "SN123456",
            .productAppearance =
                Structs::ProductAppearanceStruct::Type{
                    .finish       = ProductFinishEnum::kPolished,
                    .primaryColor = ColorEnum::kFuchsia,
                },
        },
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::VendorName::kMetadataEntry,
                                            Attributes::VendorID::kMetadataEntry,
                                            Attributes::ProductName::kMetadataEntry,
                                            Attributes::ProductID::kMetadataEntry,
                                            Attributes::NodeLabel::kMetadataEntry,
                                            Attributes::HardwareVersion::kMetadataEntry,
                                            Attributes::HardwareVersionString::kMetadataEntry,
                                            Attributes::SoftwareVersion::kMetadataEntry,
                                            Attributes::SoftwareVersionString::kMetadataEntry,
                                            Attributes::ManufacturingDate::kMetadataEntry,
                                            Attributes::PartNumber::kMetadataEntry,
                                            Attributes::ProductURL::kMetadataEntry,
                                            Attributes::ProductLabel::kMetadataEntry,
                                            Attributes::SerialNumber::kMetadataEntry,
                                            Attributes::UniqueID::kMetadataEntry,
                                            Attributes::ProductAppearance::kMetadataEntry,
                                            Attributes::ConfigurationVersion::kMetadataEntry,
                                            Attributes::Reachable::kMetadataEntry,
                                        }));
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestAttributeReads)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "test-unique-id", .reachable = true, .nodeLabel = "TestLabel", .configurationVersion = 5u },
        {
            .vendorName            = "TestVendor",
            .vendorId              = VendorId::TestVendor1,
            .productName           = "TestProduct",
            .productId             = 0xABCD,
            .hardwareVersion       = 1,
            .hardwareVersionString = "v1.0",
            .softwareVersion       = 2,
            .softwareVersionString = "v2.0",
            .manufacturingDate     = "20240101",
            .partNumber            = "PN123",
            .productUrl            = "http://test.com",
            .productLabel          = "Test Product Label",
            .serialNumber          = "SN789",
            .productAppearance =
                Structs::ProductAppearanceStruct::Type{
                    .finish       = ProductFinishEnum::kMatte,
                    .primaryColor = ColorEnum::kRed,
                },
        },
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    ClusterTester tester(cluster);

    CharSpan charSpanVal;
    uint16_t u16Val;
    uint32_t u32Val;
    VendorId vendorIdVal;
    bool boolVal;

    EXPECT_EQ(tester.ReadAttribute(Attributes::UniqueID::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("test-unique-id")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::Reachable::Id, boolVal), CHIP_NO_ERROR);
    EXPECT_TRUE(boolVal);
    EXPECT_EQ(tester.ReadAttribute(Attributes::VendorName::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("TestVendor")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::VendorID::Id, vendorIdVal), CHIP_NO_ERROR);
    EXPECT_EQ(vendorIdVal, VendorId::TestVendor1);
    EXPECT_EQ(tester.ReadAttribute(Attributes::ProductName::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("TestProduct")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::ProductID::Id, u16Val), CHIP_NO_ERROR);
    EXPECT_EQ(u16Val, 0xABCD);
    EXPECT_EQ(tester.ReadAttribute(Attributes::NodeLabel::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("TestLabel")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::HardwareVersion::Id, u16Val), CHIP_NO_ERROR);
    EXPECT_EQ(u16Val, 1u);
    EXPECT_EQ(tester.ReadAttribute(Attributes::HardwareVersionString::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("v1.0")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::SoftwareVersion::Id, u32Val), CHIP_NO_ERROR);
    EXPECT_EQ(u32Val, 2u);
    EXPECT_EQ(tester.ReadAttribute(Attributes::SoftwareVersionString::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("v2.0")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::ManufacturingDate::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("20240101")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::PartNumber::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("PN123")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::ProductURL::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("http://test.com")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::ProductLabel::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("Test Product Label")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::SerialNumber::Id, charSpanVal), CHIP_NO_ERROR);
    EXPECT_TRUE(charSpanVal.data_equal(CharSpan::fromCharString("SN789")));
    EXPECT_EQ(tester.ReadAttribute(Attributes::ConfigurationVersion::Id, u32Val), CHIP_NO_ERROR);
    EXPECT_EQ(u32Val, 5u);

    Structs::ProductAppearanceStruct::Type productAppearance;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ProductAppearance::Id, productAppearance), CHIP_NO_ERROR);
    EXPECT_EQ(productAppearance.finish, ProductFinishEnum::kMatte);
    EXPECT_EQ(productAppearance.primaryColor, ColorEnum::kRed);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestKeepActiveCommand)
{
    TestBridgedDeviceIcdDelegate delegate;
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "icd-dev" }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate, .icdDelegate = &delegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::KeepActive::Type request;
    request.stayActiveDuration = 1000;
    request.timeoutMs          = 30000;

    auto response = tester.Invoke<Commands::KeepActive::Type>(request);
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ(delegate.mOnKeepActiveCalled, 1u);
    EXPECT_EQ(delegate.mLastKeepActiveRequest.stayActiveDuration, 1000u);
    EXPECT_EQ(delegate.mLastKeepActiveRequest.timeoutMs, 30000u);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestKeepActiveCommandTimeoutDomain)
{
    TestBridgedDeviceIcdDelegate delegate;
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "icd-dev" }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate, .icdDelegate = &delegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    constexpr uint32_t kInvalidTimeouts[] = { 123, 100000000 };

    for (uint32_t timeoutMs : kInvalidTimeouts)
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 1000;
        request.timeoutMs          = timeoutMs;

        auto response = tester.Invoke<Commands::KeepActive::Type>(request);
        EXPECT_FALSE(response.IsSuccess());
        EXPECT_EQ(response.status, Status::ConstraintError);
        EXPECT_EQ(delegate.mOnKeepActiveCalled, 0u);
    }
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestReachableChangedEvent)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "event-dev", .reachable = false }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    cluster.SetReachable(true);

    std::optional<LogOnlyEvents::EventInformation> eventInfo = mContext.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(eventInfo->eventOptions.mPath.mClusterId, Id);
    EXPECT_EQ(eventInfo->eventOptions.mPath.mEventId, Events::ReachableChanged::Id);

    Events::ReachableChanged::DecodableType decodedEvent;
    ASSERT_EQ(eventInfo->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_TRUE(decodedEvent.reachableNewValue);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestLeaveEvent)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "event-dev" }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    cluster.GenerateLeaveEvent();

    std::optional<LogOnlyEvents::EventInformation> eventInfo = mContext.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(eventInfo->eventOptions.mPath.mClusterId, Id);
    EXPECT_EQ(eventInfo->eventOptions.mPath.mEventId, Events::Leave::Id);
    // NOLINTEND(bugprone-unchecked-optional-access)
    // This event has no fields, so no data to decode.
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestActiveChangedEvent)
{
    TestBridgedDeviceIcdDelegate delegate;
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "icd-dev" }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate, .icdDelegate = &delegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    constexpr uint32_t promisedDuration = 5000;
    cluster.GenerateActiveChangedEvent(promisedDuration);

    std::optional<LogOnlyEvents::EventInformation> eventInfo = mContext.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(eventInfo->eventOptions.mPath.mClusterId, Id);
    EXPECT_EQ(eventInfo->eventOptions.mPath.mEventId, Events::ActiveChanged::Id);

    Events::ActiveChanged::DecodableType decodedEvent;
    ASSERT_EQ(eventInfo->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.promisedActiveDuration, promisedDuration);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestFeatureMap)
{
    // Test without ICD delegate
    {
        BridgedDeviceBasicInformationCluster cluster(
            kTestEndpointId, { .uniqueId = "no-icd" }, {},
            { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
        EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        BitFlags<Feature> featureMap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_FALSE(featureMap.Has(Feature::kBridgedICDSupport));
    }

    // Test with ICD delegate
    {
        TestBridgedDeviceIcdDelegate delegate;
        BridgedDeviceBasicInformationCluster cluster(
            kTestEndpointId, { .uniqueId = "icd" }, {},
            { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate, .icdDelegate = &delegate });
        EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        BitFlags<Feature> featureMap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_TRUE(featureMap.Has(Feature::kBridgedICDSupport));
    }
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetNodeLabel)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "test-unique-id", .configurationVersion = 1u }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetNodeLabel("NewLabel"_span), Status::Success);
    EXPECT_TRUE(mDelegate.mNodeLabelChangedCalled);
    EXPECT_EQ(mDelegate.mLastNodeLabel, "NewLabel");
    EXPECT_EQ(cluster.GetNodeLabel(), "NewLabel");

    CharSpan nodeLabel;
    EXPECT_EQ(tester.ReadAttribute(Attributes::NodeLabel::Id, nodeLabel), CHIP_NO_ERROR);
    EXPECT_TRUE(nodeLabel.data_equal(CharSpan::fromCharString("NewLabel")));

    // Test no change
    mDelegate.mNodeLabelChangedCalled = false;
    EXPECT_EQ(cluster.SetNodeLabel("NewLabel"_span), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
    EXPECT_FALSE(mDelegate.mNodeLabelChangedCalled);

    // Test set to empty/clear
    EXPECT_EQ(cluster.SetNodeLabel({}), Status::Success);
    EXPECT_TRUE(cluster.GetNodeLabel().empty());
    EXPECT_EQ(tester.ReadAttribute(Attributes::NodeLabel::Id, nodeLabel), CHIP_NO_ERROR);
    EXPECT_TRUE(nodeLabel.empty());

    // Test too long
    std::string tooLongLabel(33, 'b');
    EXPECT_EQ(cluster.SetNodeLabel(CharSpan::fromCharString(tooLongLabel.c_str())), Status::ConstraintError);
    EXPECT_TRUE(cluster.GetNodeLabel().empty()); // Should not have changed
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetConfigurationVersion)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "test-cfg-ver", .configurationVersion = 1u }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockVersionConfiguration.mVersion = 0;
    EXPECT_EQ(cluster.IncreaseConfigurationVersion(), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetConfigurationVersion(), 2u);  // we started with 1
    EXPECT_EQ(mMockVersionConfiguration.mVersion, 1u); // parent is also increased

    uint32_t configVersion;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ConfigurationVersion::Id, configVersion), CHIP_NO_ERROR);
    EXPECT_EQ(configVersion, 2u);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestWriteNodeLabel)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "test-unique-id", .configurationVersion = 1u }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute(Attributes::NodeLabel::Id, "WrittenLabel"_span), CHIP_NO_ERROR);
    EXPECT_TRUE(mDelegate.mNodeLabelChangedCalled);
    EXPECT_EQ(mDelegate.mLastNodeLabel, "WrittenLabel");
    EXPECT_EQ(cluster.GetNodeLabel(), "WrittenLabel");

    // Test max length
    std::string longLabel(32, 'a');
    EXPECT_EQ(tester.WriteAttribute(Attributes::NodeLabel::Id, CharSpan::fromCharString(longLabel.c_str())), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetNodeLabel(), longLabel);

    // Test too long
    std::string tooLongLabel(33, 'b');
    EXPECT_EQ(tester.WriteAttribute(Attributes::NodeLabel::Id, CharSpan::fromCharString(tooLongLabel.c_str())),
              Status::ConstraintError);
    EXPECT_EQ(cluster.GetNodeLabel(), longLabel); // Should not have changed
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestWriteConfigurationVersion)
{
    BridgedDeviceBasicInformationCluster cluster(
        kTestEndpointId, { .uniqueId = "test-unique-id", .configurationVersion = 1u }, {},
        { .parentVersionConfiguration = mMockVersionConfiguration, .delegate = mDelegate });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute<uint32_t>(Attributes::ConfigurationVersion::Id, 2u), Status::UnsupportedWrite);
    EXPECT_EQ(cluster.GetConfigurationVersion(), 1u);
}

} // namespace
