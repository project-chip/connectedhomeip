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

#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

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

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestBasicInformationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    MockDeviceInstanceInfoProvider mDeviceInfoProvider;
};

TEST_F(TestBasicInformationCluster, TestAttributes)
{

    // check without optional attributes
    {
        const BasicInformationCluster::OptionalAttributesSet optionalAttributeSet;
        BasicInformationCluster cluster(optionalAttributeSet, &mDeviceInfoProvider);

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(
            cluster,
            {

                DataModelRevision::kMetadataEntry, VendorName::kMetadataEntry, VendorID::kMetadataEntry,
                ProductName::kMetadataEntry, ProductID::kMetadataEntry, NodeLabel::kMetadataEntry, Location::kMetadataEntry,
                HardwareVersion::kMetadataEntry, HardwareVersionString::kMetadataEntry, SoftwareVersion::kMetadataEntry,
                SoftwareVersionString::kMetadataEntry, CapabilityMinima::kMetadataEntry, SpecificationVersion::kMetadataEntry,
                MaxPathsPerInvoke::kMetadataEntry, ConfigurationVersion::kMetadataEntry,
                UniqueID::kMetadataEntry, // required in latest spec
            }));
    }

    // Check that disabling unique id works
    {
        const BasicInformationCluster::OptionalAttributesSet optionalAttributeSet;
        BasicInformationCluster cluster(optionalAttributeSet, &mDeviceInfoProvider);

        // UniqueID is EXPLICITLY NOT SET
        cluster.OptionalAttributes() = BasicInformationCluster::OptionalAttributesSet();

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(cluster,
                                                     {
                                                         DataModelRevision::kMetadataEntry,
                                                         VendorName::kMetadataEntry,
                                                         VendorID::kMetadataEntry,
                                                         ProductName::kMetadataEntry,
                                                         ProductID::kMetadataEntry,
                                                         NodeLabel::kMetadataEntry,
                                                         Location::kMetadataEntry,
                                                         HardwareVersion::kMetadataEntry,
                                                         HardwareVersionString::kMetadataEntry,
                                                         SoftwareVersion::kMetadataEntry,
                                                         SoftwareVersionString::kMetadataEntry,
                                                         CapabilityMinima::kMetadataEntry,
                                                         SpecificationVersion::kMetadataEntry,
                                                         MaxPathsPerInvoke::kMetadataEntry,
                                                         ConfigurationVersion::kMetadataEntry,
                                                     }));
    }

    // All attributes
    {
        const BasicInformationCluster::OptionalAttributesSet optionalAttributeSet =
            BasicInformationCluster::OptionalAttributesSet()
                .Set<ManufacturingDate::Id>()
                .Set<PartNumber::Id>()
                .Set<ProductURL::Id>()
                .Set<ProductLabel::Id>()
                .Set<SerialNumber::Id>()
                .Set<LocalConfigDisabled::Id>()
                .Set<Reachable::Id>()
                .Set<ProductAppearance::Id>()
                .Set<UniqueID::Id>();

        BasicInformationCluster cluster(optionalAttributeSet, &mDeviceInfoProvider);

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(cluster,

                                                     {
                                                         DataModelRevision::kMetadataEntry,
                                                         VendorName::kMetadataEntry,
                                                         VendorID::kMetadataEntry,
                                                         ProductName::kMetadataEntry,
                                                         ProductID::kMetadataEntry,
                                                         NodeLabel::kMetadataEntry,
                                                         Location::kMetadataEntry,
                                                         HardwareVersion::kMetadataEntry,
                                                         HardwareVersionString::kMetadataEntry,
                                                         SoftwareVersion::kMetadataEntry,
                                                         SoftwareVersionString::kMetadataEntry,
                                                         CapabilityMinima::kMetadataEntry,
                                                         SpecificationVersion::kMetadataEntry,
                                                         MaxPathsPerInvoke::kMetadataEntry,
                                                         ConfigurationVersion::kMetadataEntry,
                                                         UniqueID::kMetadataEntry,
                                                         ManufacturingDate::kMetadataEntry,
                                                         PartNumber::kMetadataEntry,
                                                         ProductURL::kMetadataEntry,
                                                         ProductLabel::kMetadataEntry,
                                                         SerialNumber::kMetadataEntry,
                                                         LocalConfigDisabled::kMetadataEntry,
                                                         Reachable::kMetadataEntry,
                                                         ProductAppearance::kMetadataEntry,
                                                     }));
    }
}

} // namespace
