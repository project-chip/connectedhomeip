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
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/TestReadWriteAttribute.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMemString.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;

static constexpr const char * kVendorName            = "TestVendor";
static constexpr const char * kProductName           = "TestProduct";
static constexpr const char * kHardwareVersionString = "HW1.0";
static constexpr const char * kPartNumber            = "PART123";
static constexpr const char * kProductURL            = "http://example.com";
static constexpr const char * kProductLabel          = "Label123";
static constexpr const char * kSerialNumber          = "SN123456";
static constexpr uint16_t kVendorId                  = static_cast<uint16_t>(VendorId::TestVendor1);
static constexpr uint16_t kProductId                 = 0x5678;
static constexpr uint16_t kHardwareVersion           = 1;
static constexpr uint16_t kManufacturingYear         = 2023;
static constexpr uint8_t kManufacturingMonth         = 6;
static constexpr uint8_t kManufacturingDay           = 15;
static constexpr ProductFinishEnum kProductFinish    = ProductFinishEnum::kMatte;
static constexpr ColorEnum kProductPrimaryColor      = ColorEnum::kBlack;

// Helper function to safely copy strings and check for buffer size
CHIP_ERROR SafeCopyString(char * buf, size_t bufSize, const char * source)
{
    if (strlen(source) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    Platform::CopyString(buf, bufSize, source);
    return CHIP_NO_ERROR;
}
// Mock DeviceInstanceInfoProvider for testing
class MockDeviceInstanceInfoProvider : public DeviceLayer::DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kVendorName); }

    CHIP_ERROR GetVendorId(uint16_t & vendorId) override
    {
        vendorId = kVendorId;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kProductName); }

    CHIP_ERROR GetProductId(uint16_t & productId) override
    {
        productId = kProductId;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override
    {
        hardwareVersion = kHardwareVersion;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override
    {
        return SafeCopyString(buf, bufSize, kHardwareVersionString);
    }

    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override
    {
        year  = kManufacturingYear;
        month = kManufacturingMonth;
        day   = kManufacturingDay;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kPartNumber); }

    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kProductURL); }

    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kProductLabel); }

    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kSerialNumber); }

    CHIP_ERROR GetProductFinish(ProductFinishEnum * finish) override
    {
        *finish = kProductFinish;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetProductPrimaryColor(ColorEnum * color) override
    {
        *color = kProductPrimaryColor;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_NO_ERROR; }
};

static constexpr size_t kCountryCodeLength = 2;
static constexpr const char * kUniqueId    = "TEST_UNIQUE_ID_12345";
// Mock ConfigurationManager for testing
class MockConfigurationManager : public chip::DeviceLayer::ConfigurationManagerImpl
{
public:
    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize) override { return SafeCopyString(buf, bufSize, kUniqueId); }
    CHIP_ERROR StoreCountryCode(const char * countryCode, size_t countryCodeLen) override
    {
        VerifyOrReturnError(countryCodeLen == kCountryCodeLength, CHIP_ERROR_INVALID_ARGUMENT);
        Platform::CopyString(mCountryCode, sizeof(mCountryCode), countryCode);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & countryCodeLen) override
    {
        VerifyOrReturnError(bufSize > kCountryCodeLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(buf, bufSize, mCountryCode);
        countryCodeLen = kCountryCodeLength;
        return CHIP_NO_ERROR;
    }

    // The following methods does not have implementation on Linux, so we provide
    // a stub that returns success.
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVersion) override { return CHIP_NO_ERROR; }

private:
    char mCountryCode[kCountryCodeLength + 1] = "XX";
};

MockConfigurationManager gMockConfigurationManager;
// Static member definition
MockDeviceInstanceInfoProvider gMockDeviceInstanceInfoProvider;
// Test fixture
struct TestBasicInformationReadWrite : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        DeviceLayer::SetDeviceInstanceInfoProvider(&gMockDeviceInstanceInfoProvider);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::SetDeviceInstanceInfoProvider(nullptr);
        chip::Platform::MemoryShutdown();
    }

    TestBasicInformationReadWrite() : testContext(), context(testContext.Create()) {}

    void SetUp() override
    {
        DeviceLayer::SetConfigurationMgr(&gMockConfigurationManager);
        ASSERT_EQ(basicInformationClusterInstance.Startup(context), CHIP_NO_ERROR);
    }

    void TearDown() override { basicInformationClusterInstance.Shutdown(); }

    template <typename T>
    inline CHIP_ERROR ReadInformationClusterAttribute(AttributeId attr, T & val)
    {
        return chip::Test::ReadClusterAttribute(basicInformationClusterInstance,
                                                ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, attr), val);
    }

    template <typename T>
    inline CHIP_ERROR WriteInformationClusterAttribute(AttributeId attr, const T & val)
    {
        return chip::Test::WriteClusterAttribute(basicInformationClusterInstance,
                                                 ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, attr), val);
    }

    chip::Test::TestServerClusterContext testContext;
    ServerClusterContext context;
    static BasicInformationCluster & basicInformationClusterInstance;
};

BasicInformationCluster & TestBasicInformationReadWrite::basicInformationClusterInstance = BasicInformationCluster::Instance();

TEST_F(TestBasicInformationReadWrite, TestNodeLabelLoadAndSave)
{
    // 1. GIVEN: A mock storage with a pre-existing "Old Label".
    const char * oldLabel = "Old Label";
    CharSpan oldLabelSpan = CharSpan::fromCharString(oldLabel);

    // Initialize AttributePersistence with the provider from *this test's* context
    AttributePersistence persistence(context.attributeStorage);

    Storage::String<32> labelStorage;
    labelStorage.SetContent(oldLabelSpan);
    persistence.StoreString({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelStorage);

    // 2. WHEN: The BasicInformationCluster starts up.
    // We must shut down the one from SetUp and re-start it to force a load.
    basicInformationClusterInstance.Shutdown();
    ASSERT_EQ(basicInformationClusterInstance.Startup(context), CHIP_NO_ERROR);
    // 3. THEN: The cluster should have loaded "Old Label" into its memory.
    char readBuffer[32];
    CharSpan readSpan(readBuffer);

    // Read NodeLabel via macro
    ASSERT_EQ(ReadInformationClusterAttribute(Attributes::NodeLabel::Id, readSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(oldLabelSpan));

    // 4. WHEN: A "New Label" is written to the attribute.
    const char * newLabel = "New Label";
    CharSpan newLabelSpan = CharSpan::fromCharString(newLabel);

    // Write NodeLabel via macro
    ASSERT_EQ(WriteInformationClusterAttribute(Attributes::NodeLabel::Id, newLabelSpan), CHIP_NO_ERROR);

    // 5. THEN: The cluster's in-memory value should be updated to "New Label".
    // Read NodeLabel via macro
    ASSERT_EQ(ReadInformationClusterAttribute(Attributes::NodeLabel::Id, readSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(newLabelSpan));

    // 6. AND THEN: The "New Label" should have been saved back to persistent storage.
    Storage::String<32> persistedLabel;
    persistence.LoadString({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, persistedLabel);
    EXPECT_TRUE(persistedLabel.Content().data_equal(newLabelSpan));
}

TEST_F(TestBasicInformationReadWrite, TestAllAttributesSpecCompliance)
{
    using namespace chip::app::Clusters::BasicInformation;

    // VendorName
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::VendorName::Id, val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString(kVendorName)));
    }

    // VendorID
    {
        VendorId val = VendorId::NotSpecified;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::VendorID::Id, val), CHIP_NO_ERROR);
        EXPECT_EQ(val, kVendorId);
    }

    // ProductName
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::ProductName::Id, val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString(kProductName)));
    }
    // ProductID
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::ProductID::Id, val), CHIP_NO_ERROR);
        EXPECT_EQ(val, kProductId);
    }

    // NodeLabel (default empty on fresh boot)
    {
        char buf[32];
        CharSpan val(buf);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::NodeLabel::Id, val), CHIP_NO_ERROR);
        EXPECT_LE(val.size(), static_cast<size_t>(32));
    }

    // HardwareVersion
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::HardwareVersion::Id, val), CHIP_NO_ERROR);
        EXPECT_EQ(val, kHardwareVersion);
    }

    // HardwareVersionString
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::HardwareVersionString::Id, val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString(kHardwareVersionString)));
    }

    // SoftwareVersion is read from ConfigurationManager
    // Just ensure it decodes
    {
        uint32_t val = 0;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::SoftwareVersion::Id, val), CHIP_NO_ERROR);
    }

    // ManufacturingDate (YYYYMMDD from mock)
    {
        char buf[32];
        CharSpan val(buf);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::ManufacturingDate::Id, val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("20230615")));
    }

    // UniqueID (Mandatory in Rev 4+, so if it fails, cluster rev must be < 4)
    {
        // Read ClusterRevision first
        uint32_t clusterRev;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::ClusterRevision::Id, clusterRev), CHIP_NO_ERROR);
        char buf[256];
        CharSpan val(buf);
        CHIP_ERROR err = ReadInformationClusterAttribute(Attributes::UniqueID::Id, val);

        if (err != CHIP_NO_ERROR)
        {
            // If UniqueID cannot be read, the cluster MUST report a revision < 4.
            EXPECT_LT(clusterRev, 4U); // < 4
        }
        else
        {
            // If UniqueID is readable, it must match what the mock returns.
            EXPECT_TRUE(val.data_equal(CharSpan::fromCharString(kUniqueId)));
            EXPECT_EQ(clusterRev, BasicInformation::kRevision);
        }
    }

    // CapabilityMinima
    {
        Structs::CapabilityMinimaStruct::Type val;
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::CapabilityMinima::Id, val), CHIP_NO_ERROR);
        EXPECT_GE(val.caseSessionsPerFabric, 3);
        EXPECT_GE(val.subscriptionsPerFabric, 3);
    }
}

TEST_F(TestBasicInformationReadWrite, TestWriteNodeLabel)
{
    // 1. ARRANGE: Define a new valid label
    const char * newLabelStr = "My Awesome Hub";
    CharSpan newLabel        = CharSpan::fromCharString(newLabelStr);
    char readBuffer[32];
    CharSpan readSpan(readBuffer);

    // 2. ACT: Write the new label to the attribute via macro
    ASSERT_EQ(WriteInformationClusterAttribute(Attributes::NodeLabel::Id, newLabel), CHIP_NO_ERROR);

    // 3. ASSERT: Read the attribute back and verify it matches the new label
    ASSERT_EQ(ReadInformationClusterAttribute(Attributes::NodeLabel::Id, readSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(newLabel));
}

TEST_F(TestBasicInformationReadWrite, TestWriteLocation)
{
    // --- Test Case 1: Write a valid 2-character location ---
    {
        const char * validLocationStr = "US";
        CharSpan validLocation        = CharSpan::fromCharString(validLocationStr);
        char readBuffer[8];
        CharSpan readSpan(readBuffer);
        ASSERT_EQ(WriteInformationClusterAttribute(Attributes::Location::Id, validLocation), CHIP_NO_ERROR);
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::Location::Id, readSpan), CHIP_NO_ERROR);
        EXPECT_TRUE(readSpan.data_equal(validLocation));
    }

    // --- Test Case 2: Write an invalid location (not 2 characters) ---
    {
        // Define an invalid 3-character location. Spec requires exactly 2.
        const char * invalidLocationStr = "USA";
        CharSpan invalidLocation        = CharSpan::fromCharString(invalidLocationStr);

        // Attempt to write the invalid location and confirm it fails
        CHIP_ERROR writeErr = WriteInformationClusterAttribute(Attributes::Location::Id, invalidLocation);
        EXPECT_NE(writeErr, CHIP_NO_ERROR); // Expect a failure (ConstraintError)
    }
}

TEST_F(TestBasicInformationReadWrite, TestWriteLocalConfigDisabled)
{
    bool readValue;

    // --- Test Case 1: Write 'true' ---
    {
        // The default value is false, so we'll write true
        constexpr bool newValue = true;
        // Write 'true' to the attribute via macro
        ASSERT_EQ(WriteInformationClusterAttribute(Attributes::LocalConfigDisabled::Id, newValue), CHIP_NO_ERROR);
        // Read the value back and confirm it is now true
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::LocalConfigDisabled::Id, readValue), CHIP_NO_ERROR);
        EXPECT_EQ(readValue, newValue);
    }

    // --- Test Case 2: Write 'false' back ---
    {
        // Write false back
        constexpr bool finalValue = false;
        // Write 'false' to the attribute via macro
        ASSERT_EQ(WriteInformationClusterAttribute(Attributes::LocalConfigDisabled::Id, finalValue), CHIP_NO_ERROR);
        // Read the value back and confirm it is now false
        ASSERT_EQ(ReadInformationClusterAttribute(Attributes::LocalConfigDisabled::Id, readValue), CHIP_NO_ERROR);
        EXPECT_EQ(readValue, finalValue);
    }
}

} // namespace
