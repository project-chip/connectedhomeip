/*
 *
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

#include <app/clusters/localization-configuration-server/localization-configuration-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/LocalizationConfiguration/Enums.h>
#include <clusters/LocalizationConfiguration/Ids.h>
#include <clusters/LocalizationConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// Mock DeviceInfoProvider for testing
class MockDeviceInfoProvider : public DeviceLayer::DeviceInfoProvider
{
public:
    MockDeviceInfoProvider()           = default;
    ~MockDeviceInfoProvider() override = default;

    void SetSupportedLocales(const std::vector<CharSpan> & locales) { mSupportedLocales = locales; }

    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override { return nullptr; }
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override { return nullptr; }
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override { return nullptr; }

    SupportedLocalesIterator * IterateSupportedLocales() override
    {
        return chip::Platform::New<MockSupportedLocalesIterator>(mSupportedLocales);
    }

protected:
    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override { return CHIP_NO_ERROR; }
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_NO_ERROR; }

private:
    class MockSupportedLocalesIterator : public SupportedLocalesIterator
    {
    public:
        MockSupportedLocalesIterator(const std::vector<CharSpan> & locales) : mLocales(locales) {}
        ~MockSupportedLocalesIterator() override = default;

        size_t Count() override { return mLocales.size(); }
        bool Next(CharSpan & output) override
        {
            if (mIndex < mLocales.size())
            {
                output = mLocales[mIndex++];
                return true;
            }
            return false;
        }
        void Release() override { chip::Platform::Delete(this); }

    private:
        std::vector<CharSpan> mLocales;
        size_t mIndex = 0;
    };

    std::vector<CharSpan> mSupportedLocales;
};

struct TestLocalizationConfigurationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        // Set up mock device info provider
        mDeviceInfoProvider = chip::Platform::New<MockDeviceInfoProvider>();
        DeviceLayer::SetDeviceInfoProvider(mDeviceInfoProvider);
    }

    void TearDown() override
    {
        if (mDeviceInfoProvider)
        {
            DeviceLayer::SetDeviceInfoProvider(nullptr);
            chip::Platform::Delete(mDeviceInfoProvider);
            mDeviceInfoProvider = nullptr;
        }
    }

    MockDeviceInfoProvider * mDeviceInfoProvider = nullptr;
};

TEST_F(TestLocalizationConfigurationCluster, TestReadAndWriteActiveLocale)
{
    // Set up mock supported locales.
    std::vector<CharSpan> supportedLocales = { CharSpan::fromCharString("en-US"), CharSpan::fromCharString("es-ES") };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    // Create cluster instance with an invalid locale.
    CharSpan initialLocale = CharSpan::fromCharString("de-DE");
    LocalizationConfigurationCluster cluster(initialLocale);

    // ActiveLocale should not be set.
    CharSpan actualLocale = cluster.GetLogic().GetActiveLocale();
    EXPECT_EQ(actualLocale.size(), size_t(0));

    // Test 1: Write a valid supported locale.
    CharSpan validLocale = CharSpan::fromCharString("es-ES");
    CHIP_ERROR error     = cluster.GetLogic().SetActiveLocale(validLocale);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    // Verify the valid locale was written correctly.
    actualLocale = cluster.GetLogic().GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal(validLocale));
    EXPECT_EQ(actualLocale.size(), validLocale.size());

    // Test 2: Write an invalid unsupported locale.
    CharSpan invalidLocale = CharSpan::fromCharString("de-DE");

    error = cluster.GetLogic().SetActiveLocale(invalidLocale);
    EXPECT_EQ(error, CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestLocalizationConfigurationCluster, TestReadAttributes)
{
    // Set up mock supported locales
    std::vector<CharSpan> supportedLocales = { CharSpan::fromCharString("en-US"), CharSpan::fromCharString("es-ES") };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    // Create cluster instance
    CharSpan initialLocale = CharSpan::fromCharString("en-US");
    LocalizationConfigurationCluster cluster(initialLocale);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, LocalizationConfiguration::Id), attributesBuilder),
              CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
    ASSERT_EQ(expectedBuilder.AppendElements({ LocalizationConfiguration::Attributes::ActiveLocale::kMetadataEntry,
                                               LocalizationConfiguration::Attributes::SupportedLocales::kMetadataEntry }),
              CHIP_NO_ERROR);

    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}
} // namespace
