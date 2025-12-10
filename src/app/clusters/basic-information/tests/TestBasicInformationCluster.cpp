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
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>
#include <platform/DeviceInfoProvider.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// Mock DeviceInfoProvider for testing
class MockDeviceInfoProvider : public DeviceLayer::DeviceInfoProvider
{
public:
    MockDeviceInfoProvider()           = default;
    ~MockDeviceInfoProvider() override = default;

    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override { return nullptr; }
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override { return nullptr; }
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override { return nullptr; }
    SupportedLocalesIterator * IterateSupportedLocales() override { return nullptr; }

protected:
    // Simple no-op implementations - we only need these to return success
    // so that the cluster's validation logic can be tested
    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override{ return CHIP_NO_ERROR; }
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_NO_ERROR; }
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestBasicInformationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        DeviceLayer::SetDeviceInfoProvider(&mDeviceInfoProvider);
    }

    MockDeviceInfoProvider mDeviceInfoProvider;
};

TEST_F(TestBasicInformationCluster, TestAttributes)
{

    // save and restore the flags at the end of the test
    struct SaveFlags
    {
        SaveFlags() { mFlags = BasicInformationCluster::Instance().OptionalAttributes(); }
        ~SaveFlags() { BasicInformationCluster::Instance().OptionalAttributes() = mFlags; }

    private:
        BasicInformationCluster::OptionalAttributesSet mFlags;
    } scopedFlagsSave;

    // check without optional attributes
    {
        BasicInformationCluster::Instance().OptionalAttributes() =
            BasicInformationCluster::OptionalAttributesSet().Set<UniqueID::Id>();

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(
            BasicInformationCluster::Instance(),
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
        // UniqueID is EXPLICITLY NOT SET
        BasicInformationCluster::Instance().OptionalAttributes() = BasicInformationCluster::OptionalAttributesSet();

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(BasicInformationCluster::Instance(),
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
        BasicInformationCluster::Instance().OptionalAttributes() = //
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

        EXPECT_TRUE(Testing::IsAttributesListEqualTo(BasicInformationCluster::Instance(),

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
