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

#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/bridged-device-basic-information-server/BasicInformationClusterProxy.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationCluster.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationDelegate.h>
#include <app/clusters/bridged-device-basic-information-server/ConfigurationVersionDelegate.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/String.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/BridgedDeviceBasicInformation/Commands.h>
#include <clusters/BridgedDeviceBasicInformation/Enums.h>
#include <clusters/BridgedDeviceBasicInformation/Events.h>
#include <clusters/BridgedDeviceBasicInformation/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TimerDelegateMock.h>
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

class MockConfigurationManager : public DeviceLayer::ConfigurationManager
{
public:
    CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan & buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetConfigurationVersion(uint32_t & configurationVer) override
    {
        configurationVer = mConfigurationVersion;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetFirmwareBuildChipEpochTime(System::Clock::Seconds32 & buildTime) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR IncrementLifetimeCounter() override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetRotatingDeviceIdUniqueId(const ByteSpan & uniqueIdSpan) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    CHIP_ERROR GetRegulatoryLocation(uint8_t & location) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreSoftwareVersion(uint32_t softwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreConfigurationVersion(uint32_t configurationVer) override
    {
        if (mReturnError != CHIP_NO_ERROR)
        {
            return mReturnError;
        }
        mConfigurationVersion = configurationVer;
        mStoreCalled++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR StoreHardwareVersion(uint16_t hardwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreRegulatoryLocation(uint8_t location) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRebootCount(uint32_t & rebootCount) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreRebootCount(uint32_t rebootCount) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetBootReason(uint32_t & bootReason) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreBootReason(uint32_t bootReason) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreUniqueId(const char * uniqueId, size_t uniqueIdLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GenerateUniqueId(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetFailSafeArmed(bool & val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetFailSafeArmed(bool val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    bool IsFullyProvisioned() override { return true; }
    void InitiateFactoryReset() override {}
    void LogDeviceConfig() override {}
    bool IsCommissionableDeviceTypeEnabled() override { return false; }
    CHIP_ERROR GetDeviceTypeId(uint32_t & deviceType) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    bool IsCommissionableDeviceNameEnabled() override { return false; }
    CHIP_ERROR GetCommissionableDeviceName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    bool CanFactoryReset() override { return true; }
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    virtual void RunUnitTests() override {}

    uint32_t mConfigurationVersion = 10;
    uint32_t mStoreCalled          = 0;
    CHIP_ERROR mReturnError        = CHIP_NO_ERROR;
};

class MockDelegate : public BridgedDeviceBasicInformationDelegate
{
public:
    Status OnNodeLabelChanged(const std::string & newNodeLabel) override
    {
        mNodeLabelChangedCalled = true;
        mLastNodeLabel          = newNodeLabel;
        return mReturnStatus;
    }

    bool mNodeLabelChangedCalled = false;
    std::string mLastNodeLabel;
    Status mReturnStatus = Status::Success;
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

class MockDeviceInstanceInfoProvider : public DeviceLayer::DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductId(uint16_t & productId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

class TestBridgedDeviceIcdDelegate : public BridgedDeviceIcdDelegate
{
public:
    uint32_t mEnterCalled   = 0;
    uint32_t mExpiredCalled = 0;

    Status OnEnterPendingActive() override
    {
        mEnterCalled++;
        return Status::Success;
    }

    /// Called when the PendingActive state timer expires.
    void OnPendingActiveExpired() override { mExpiredCalled++; }
};

struct TestBridgedDeviceBasicInformationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    MockDeviceInstanceInfoProvider mDeviceInfoProvider;
    MockConfigurationManager mMockConfigManager;

    BasicInformationCluster::Context mBasicInfoContext = {
        .deviceInstanceInfoProvider = mDeviceInfoProvider,
        .configurationManager       = mMockConfigManager,
        .platformManager            = DeviceLayer::PlatformMgr(),
        .subscriptionsPerFabric     = 1,
    };

    TestServerClusterContext mContext;
    MockDelegate mDelegate;
    MockVersionConfigurationDelegate mMockVersionConfiguration;
    chip::TimerDelegateMock mMockTimer;
};

TEST_F(TestBridgedDeviceBasicInformationCluster, TestEmptyAttributes)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "foo-bar",
                                                     .reachable = true,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "foo-bar",
                                                     .reachable            = true,
                                                     .nodeLabel            = "mylabel",
                                                     .configurationVersion = 200u,
                                                 },
                                                 {
                                                     .partNumber = "010203",
                                                 },
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
        {
            .parentVersionConfiguration = mMockVersionConfiguration,
            .delegate                   = mDelegate,
            .timerDelegate              = mMockTimer,
        });
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
                                            Attributes::Reachable::kMetadataEntry,
                                            Attributes::UniqueID::kMetadataEntry,
                                            Attributes::ProductAppearance::kMetadataEntry,
                                            Attributes::ConfigurationVersion::kMetadataEntry,
                                        }));
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestAttributeReads)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "test-unique-id",
                                                     .reachable            = true,
                                                     .nodeLabel            = "TestLabel",
                                                     .configurationVersion = 5u,
                                                 },
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
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
    TestBridgedDeviceIcdDelegate icdDelegate;
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "icd-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                     .icdDelegate                = &icdDelegate,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    constexpr uint32_t kStayActiveDurationMs = 1000;

    Commands::KeepActive::Type request;
    request.stayActiveDuration = kStayActiveDurationMs;
    request.timeoutMs          = 30000;

    auto response = tester.Invoke<Commands::KeepActive::Type>(request);
    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(icdDelegate.mEnterCalled, 1u);
    EXPECT_EQ(icdDelegate.mExpiredCalled, 0u);

    EXPECT_EQ(cluster.GetRequestedStayActiveDurationMs(), kStayActiveDurationMs);

    EXPECT_TRUE(mMockTimer.IsTimerActive(&cluster));

    // notify device is active and see that the cluster handles it correctly
    cluster.NotifyDeviceActive();
    EXPECT_FALSE(cluster.GetRequestedStayActiveDurationMs().has_value());
    EXPECT_FALSE(mMockTimer.IsTimerActive(&cluster));

    // validate that an event is generated
    std::optional<LogOnlyEvents::EventInformation> eventInfo = mContext.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(eventInfo->eventOptions.mPath.mClusterId, Id);
    EXPECT_EQ(eventInfo->eventOptions.mPath.mEventId, Events::ActiveChanged::Id);

    Events::ActiveChanged::DecodableType decodedEvent;
    ASSERT_EQ(eventInfo->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.promisedActiveDuration, kStayActiveDurationMs);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestNotifyDeviceActiveWithoutRequestedDuration)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "no-icd",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Call NotifyDeviceActive when not in pending state
    // Should NOT generate an event and NOT crash
    cluster.NotifyDeviceActive();

    EXPECT_EQ(mContext.EventsGenerator().GetNextEvent(), std::nullopt);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestShutdownCancelsTimer)
{
    TestBridgedDeviceIcdDelegate icdDelegate;
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "icd-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                     .icdDelegate                = &icdDelegate,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::KeepActive::Type request;
    request.stayActiveDuration = 1000;
    request.timeoutMs          = 30000;

    auto response = tester.Invoke<Commands::KeepActive::Type>(request);
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_TRUE(mMockTimer.IsTimerActive(&cluster));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    EXPECT_FALSE(mMockTimer.IsTimerActive(&cluster));
    EXPECT_FALSE(cluster.GetRequestedStayActiveDurationMs().has_value());
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestKeepActiveTimerNoRegression)
{
    TestBridgedDeviceIcdDelegate icdDelegate;
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "icd-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                     .icdDelegate                = &icdDelegate,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Initial request: timeout 60s
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 1000;
        request.timeoutMs          = 60000;
        EXPECT_TRUE(tester.Invoke<Commands::KeepActive::Type>(request).IsSuccess());
    }

    // Advance 10s. Remaining is 50s.
    mMockTimer.AdvanceClock(System::Clock::Milliseconds32(10000));

    // Second request: timeout 30s. This should be ignored for timer extension because 30s < 50s.
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 1000;
        request.timeoutMs          = 30000;
        EXPECT_TRUE(tester.Invoke<Commands::KeepActive::Type>(request).IsSuccess());
    }

    // Advance another 35s (total 45s from start). Timer should still be active.
    mMockTimer.AdvanceClock(System::Clock::Milliseconds32(35000));
    EXPECT_TRUE(mMockTimer.IsTimerActive(&cluster));

    // Advance another 20s (total 65s from start). Timer should have fired.
    mMockTimer.AdvanceClock(System::Clock::Milliseconds32(20000));
    EXPECT_FALSE(mMockTimer.IsTimerActive(&cluster));
    EXPECT_EQ(icdDelegate.mExpiredCalled, 1u);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestKeepActiveCommandMultipleRequests)
{
    TestBridgedDeviceIcdDelegate icdDelegate;
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "icd-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                     .icdDelegate                = &icdDelegate,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Initial request: stay active for 1000ms, timeout 30s
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 1000;
        request.timeoutMs          = 30000;

        auto response = tester.Invoke<Commands::KeepActive::Type>(request);
        ASSERT_TRUE(response.IsSuccess());
        EXPECT_EQ(cluster.GetRequestedStayActiveDurationMs(), 1000u);
    }

    // Second request: stay active for 500ms (less), timeout 40s (more)
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 500;
        request.timeoutMs          = 40000;

        auto response = tester.Invoke<Commands::KeepActive::Type>(request);
        ASSERT_TRUE(response.IsSuccess());
        // Max stay active duration should be kept
        EXPECT_EQ(cluster.GetRequestedStayActiveDurationMs(), 1000u);
    }

    // Third request: stay active for 2000ms (more), timeout 35s (less than current 40s)
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 2000;
        request.timeoutMs          = 35000;

        auto response = tester.Invoke<Commands::KeepActive::Type>(request);
        ASSERT_TRUE(response.IsSuccess());
        // New max duration
        EXPECT_EQ(cluster.GetRequestedStayActiveDurationMs(), 2000u);
    }

    // Verify timer firing:
    // After 39s, timer should still be active (40s was the max timeout)
    mMockTimer.AdvanceClock(System::Clock::Milliseconds32(39000));
    EXPECT_TRUE(mMockTimer.IsTimerActive(&cluster));
    EXPECT_EQ(icdDelegate.mExpiredCalled, 0u);

    // After another 2s (total 41s), timer should have fired
    mMockTimer.AdvanceClock(System::Clock::Milliseconds32(2000));
    EXPECT_FALSE(mMockTimer.IsTimerActive(&cluster));
    EXPECT_EQ(icdDelegate.mExpiredCalled, 1u);
    EXPECT_FALSE(cluster.GetRequestedStayActiveDurationMs().has_value());
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestKeepActiveCommandTimeoutDomain)
{
    TestBridgedDeviceIcdDelegate icdDelegate;
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "icd-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                     .icdDelegate                = &icdDelegate,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    constexpr uint32_t kInvalidTimeouts[] = { 123, 100000000 };

    for (uint32_t timeoutMs : kInvalidTimeouts)
    {
        Commands::KeepActive::Type request;
        request.stayActiveDuration = 1000;
        request.timeoutMs          = timeoutMs;

        auto response = tester.Invoke<Commands::KeepActive::Type>(request);
        ASSERT_FALSE(response.IsSuccess());
        EXPECT_EQ(response.status, Status::ConstraintError);
        EXPECT_EQ(icdDelegate.mEnterCalled, 0u);
        EXPECT_EQ(icdDelegate.mExpiredCalled, 0u);
    }
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestReachableChangedEvent)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "event-dev",
                                                     .reachable = false,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId = "event-dev",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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

TEST_F(TestBridgedDeviceBasicInformationCluster, TestFeatureMap)
{
    // Test without ICD delegate
    {
        BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                     {
                                                         .uniqueId = "no-icd",
                                                     },
                                                     {},
                                                     {
                                                         .parentVersionConfiguration = mMockVersionConfiguration,
                                                         .delegate                   = mDelegate,
                                                         .timerDelegate              = mMockTimer,
                                                     });
        EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        BitFlags<Feature> featureMap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_FALSE(featureMap.Has(Feature::kBridgedICDSupport));
    }

    // Test with ICD delegate
    {
        TestBridgedDeviceIcdDelegate icdDelegate;
        BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                     {
                                                         .uniqueId = "icd",
                                                     },
                                                     {},
                                                     {
                                                         .parentVersionConfiguration = mMockVersionConfiguration,
                                                         .delegate                   = mDelegate,
                                                         .timerDelegate              = mMockTimer,
                                                         .icdDelegate                = &icdDelegate,
                                                     });
        EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        BitFlags<Feature> featureMap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_TRUE(featureMap.Has(Feature::kBridgedICDSupport));
    }
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetNodeLabel)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "test-label",
                                                     .configurationVersion = 1u,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
    mContext.ChangeListener().DirtyList().clear();
    EXPECT_EQ(cluster.SetNodeLabel("NewLabel"_span), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
    EXPECT_FALSE(mDelegate.mNodeLabelChangedCalled);
    EXPECT_FALSE(mContext.ChangeListener().IsDirty({ kTestEndpointId, Id, Attributes::NodeLabel::Id }));

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

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetNodeLabelEmpty)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "label-test",
                                                     .nodeLabel = "Initial",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });

    // Test setting to empty
    EXPECT_EQ(cluster.SetNodeLabel(""_span), Status::Success);
    EXPECT_EQ(cluster.GetNodeLabel(), "");
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetConfigurationVersion)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "test-cfg-ver",
                                                     .configurationVersion = 1u,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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

TEST_F(TestBridgedDeviceBasicInformationCluster, TestAcceptedCommands)
{
    // Without ICD support
    {
        BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                     {
                                                         .uniqueId = "no-icd",
                                                     },
                                                     {},
                                                     {
                                                         .parentVersionConfiguration = mMockVersionConfiguration,
                                                         .delegate                   = mDelegate,
                                                         .timerDelegate              = mMockTimer,
                                                     });

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
        EXPECT_EQ(cluster.AcceptedCommands(cluster.GetPaths()[0], builder), CHIP_NO_ERROR);
        EXPECT_EQ(builder.TakeBuffer().size(), 0u);
    }

    // With ICD support
    {
        TestBridgedDeviceIcdDelegate icdDelegate;
        BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                     {
                                                         .uniqueId = "icd",
                                                     },
                                                     {},
                                                     {
                                                         .parentVersionConfiguration = mMockVersionConfiguration,
                                                         .delegate                   = mDelegate,
                                                         .timerDelegate              = mMockTimer,
                                                         .icdDelegate                = &icdDelegate,
                                                     });

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
        EXPECT_EQ(cluster.AcceptedCommands(cluster.GetPaths()[0], builder), CHIP_NO_ERROR);
        EXPECT_EQ(builder.TakeBuffer().size(), 1u);
    }
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestReachableChangedSuppression)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "reachable-test",
                                                     .reachable = true,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Setting it to same value should NOT generate event
    cluster.SetReachable(true);
    EXPECT_EQ(mContext.EventsGenerator().GetNextEvent(), std::nullopt);

    // Setting it to different value SHOULD generate event
    cluster.SetReachable(false);
    EXPECT_NE(mContext.EventsGenerator().GetNextEvent(), std::nullopt);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestWriteNodeLabel)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "test-unique-id",
                                                     .configurationVersion = 1u,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
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
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId             = "test-unique-id",
                                                     .configurationVersion = 1u,
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute<uint32_t>(Attributes::ConfigurationVersion::Id, 2u), Status::UnsupportedWrite);
    EXPECT_EQ(cluster.GetConfigurationVersion(), 1u);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestSetNodeLabelDelegateError)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "test-label",
                                                     .nodeLabel = "Initial",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Set delegate to return error
    mDelegate.mReturnStatus = Status::ConstraintError;

    EXPECT_EQ(cluster.SetNodeLabel("NewLabel"_span), Status::ConstraintError);
    EXPECT_TRUE(mDelegate.mNodeLabelChangedCalled);
    EXPECT_EQ(cluster.GetNodeLabel(), "Initial"); // Should NOT have changed
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestNodeLabelPersistence)
{
    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "test-persistence",
                                                     .nodeLabel = "Initial",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Set new label
    EXPECT_EQ(cluster.SetNodeLabel("PersistentLabel"_span), Status::Success);

    // Verify it is in persistence
    AttributePersistence persistence(mContext.Get().attributeStorage);
    Storage::String<32> storedLabel;
    EXPECT_TRUE(persistence.LoadString({ kTestEndpointId, Id, Attributes::NodeLabel::Id }, storedLabel));
    EXPECT_TRUE(storedLabel.Content().data_equal("PersistentLabel"_span));
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestBasicInformationClusterProxy)
{
    BasicInformationCluster basicInfo(BasicInformationCluster::OptionalAttributesSet(), mBasicInfoContext);

    // Initial value in our mock is 10
    EXPECT_EQ(mMockConfigManager.mConfigurationVersion, 10u);
    EXPECT_EQ(mMockConfigManager.mStoreCalled, 0u);

    BasicInformationClusterProxy proxy(basicInfo);
    EXPECT_EQ(proxy.IncreaseConfigurationVersion(), CHIP_NO_ERROR);

    EXPECT_EQ(mMockConfigManager.mConfigurationVersion, 11u);
    EXPECT_EQ(mMockConfigManager.mStoreCalled, 1u);

    // Test failure
    mMockConfigManager.mReturnError = CHIP_ERROR_INTERNAL;
    EXPECT_EQ(proxy.IncreaseConfigurationVersion(), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(mMockConfigManager.mConfigurationVersion, 11u);
    EXPECT_EQ(mMockConfigManager.mStoreCalled, 1u);
}

TEST_F(TestBridgedDeviceBasicInformationCluster, TestStartupPersistence)
{
    // Prime persistence with a value
    {
        AttributePersistence persistence(mContext.AttributePersistenceProvider());
        Storage::String<32> storedLabel;
        storedLabel.SetContent("StoredLabel"_span);
        EXPECT_EQ(persistence.StoreString({ kTestEndpointId, Id, Attributes::NodeLabel::Id }, storedLabel), CHIP_NO_ERROR);
    }

    BridgedDeviceBasicInformationCluster cluster(kTestEndpointId,
                                                 {
                                                     .uniqueId  = "test-startup",
                                                     .nodeLabel = "ConstructorLabel",
                                                 },
                                                 {},
                                                 {
                                                     .parentVersionConfiguration = mMockVersionConfiguration,
                                                     .delegate                   = mDelegate,
                                                     .timerDelegate              = mMockTimer,
                                                 });

    // Startup should load from persistence
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Verify it was loaded
    EXPECT_EQ(cluster.GetNodeLabel(), "StoredLabel");

    // Delegate should have been called because stored value != constructor value
    EXPECT_TRUE(mDelegate.mNodeLabelChangedCalled);
    EXPECT_EQ(mDelegate.mLastNodeLabel, "StoredLabel");
}

} // namespace
