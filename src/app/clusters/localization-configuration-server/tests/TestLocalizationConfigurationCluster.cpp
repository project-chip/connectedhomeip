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

#include <app/clusters/localization-configuration-server/LocalizationConfigurationCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/LocalizationConfiguration/Enums.h>
#include <clusters/LocalizationConfiguration/Ids.h>
#include <clusters/LocalizationConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;
using namespace chip::Protocols::InteractionModel;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::IsAttributesListEqualTo;

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

class MockLocalizationConfigurationCluster : public LocalizationConfigurationCluster
{
public:
    MockLocalizationConfigurationCluster(DeviceLayer::DeviceInfoProvider & deviceInfoProvider, CharSpan activeLocale) :
        LocalizationConfigurationCluster(deviceInfoProvider, activeLocale)
    {}

    bool GetDefaultLocale(MutableCharSpan & outLocale) override
    {
        bool found = false;
        DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;
        if ((it = mDeviceInfoProvider.IterateSupportedLocales()))
        {
            CharSpan tempLocale;
            found = it->Next(tempLocale);
            it->Release();
            VerifyOrReturnValue(CopyCharSpanToMutableCharSpan(tempLocale, outLocale) == CHIP_NO_ERROR, false);
        }
        return found;
    }
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
    std::vector<CharSpan> supportedLocales = { "en-US"_span, "es-ES"_span };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    // Create cluster instance with an invalid locale.
    CharSpan initialLocale = "de-DE"_span;
    MockLocalizationConfigurationCluster cluster(*mDeviceInfoProvider, initialLocale);

    // ActiveLocale should be set to the default locale which is the first supported locale in this case.
    CharSpan actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal(supportedLocales[0]));
    EXPECT_EQ(actualLocale.size(), supportedLocales[0].size());

    // Test 1: Write a valid supported locale.
    CharSpan validLocale                 = "es-ES"_span;
    DataModel::ActionReturnStatus status = cluster.SetActiveLocale(validLocale);
    EXPECT_EQ(status, Status::Success);

    // Verify the valid locale was written correctly.
    actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal(validLocale));
    EXPECT_EQ(actualLocale.size(), validLocale.size());

    // Test 2: Write an invalid unsupported locale.
    CharSpan invalidLocale = "de-DE"_span;

    status = cluster.SetActiveLocale(invalidLocale);
    EXPECT_EQ(status, Status::ConstraintError);
}

TEST_F(TestLocalizationConfigurationCluster, TestReadAttributes)
{
    // Set up mock supported locales
    std::vector<CharSpan> supportedLocales = { "en-US"_span, "es-ES"_span };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    // Create cluster instance
    CharSpan initialLocale = "en-US"_span;
    LocalizationConfigurationCluster cluster(*mDeviceInfoProvider, initialLocale);

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            LocalizationConfiguration::Attributes::ActiveLocale::kMetadataEntry,
                                            LocalizationConfiguration::Attributes::SupportedLocales::kMetadataEntry,
                                        }));
}

TEST_F(TestLocalizationConfigurationCluster, WriteAttributeNotifiesSubscribers)
{
    std::vector<CharSpan> supportedLocales = { "en-US"_span, "es-ES"_span, "fr-FR"_span };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    MockLocalizationConfigurationCluster cluster(*mDeviceInfoProvider, "en-US"_span);

    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Testing::WriteOperation writeOp(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id);
    writeOp.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);

    AttributeValueDecoder decoder        = writeOp.DecoderFor("es-ES"_span);
    DataModel::ActionReturnStatus status = cluster.WriteAttribute(writeOp.GetRequest(), decoder);
    EXPECT_EQ(status, Status::Success);

    CharSpan actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal("es-ES"_span));

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    EXPECT_TRUE(
        context.ChangeListener().IsDirty(ConcreteAttributePath(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id)));

    context.ChangeListener().DirtyList().clear();
    DataVersion versionAfterFirstWrite = cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id });

    chip::Testing::WriteOperation writeOp2(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id);
    writeOp2.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
    AttributeValueDecoder decoder2 = writeOp2.DecoderFor("fr-FR"_span);
    status                         = cluster.WriteAttribute(writeOp2.GetRequest(), decoder2);
    EXPECT_EQ(status, Status::Success);

    actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal("fr-FR"_span));

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    EXPECT_TRUE(
        context.ChangeListener().IsDirty(ConcreteAttributePath(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id)));
    EXPECT_NE(cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id }), versionAfterFirstWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLocalizationConfigurationCluster, WriteAttributeFailureDoesNotNotify)
{
    std::vector<CharSpan> supportedLocales = { "en-US"_span, "es-ES"_span };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    MockLocalizationConfigurationCluster cluster(*mDeviceInfoProvider, "en-US"_span);

    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    DataVersion versionBefore = cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id });

    chip::Testing::WriteOperation writeOp(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id);
    writeOp.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
    AttributeValueDecoder decoder        = writeOp.DecoderFor("de-DE"_span);
    DataModel::ActionReturnStatus status = cluster.WriteAttribute(writeOp.GetRequest(), decoder);
    EXPECT_EQ(status, Status::ConstraintError);

    CharSpan actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal("en-US"_span));

    EXPECT_TRUE(context.ChangeListener().DirtyList().empty());
    EXPECT_EQ(cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id }), versionBefore);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLocalizationConfigurationCluster, WriteSameLocaleIsNoOp)
{
    std::vector<CharSpan> supportedLocales = { "en-US"_span, "es-ES"_span };
    mDeviceInfoProvider->SetSupportedLocales(supportedLocales);

    MockLocalizationConfigurationCluster cluster(*mDeviceInfoProvider, "en-US"_span);

    chip::Testing::TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    DataVersion versionBefore = cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id });

    chip::Testing::WriteOperation writeOp(kRootEndpointId, LocalizationConfiguration::Id, ActiveLocale::Id);
    writeOp.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
    AttributeValueDecoder decoder        = writeOp.DecoderFor("en-US"_span);
    DataModel::ActionReturnStatus status = cluster.WriteAttribute(writeOp.GetRequest(), decoder);
    EXPECT_EQ(status, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    CharSpan actualLocale = cluster.GetActiveLocale();
    EXPECT_TRUE(actualLocale.data_equal("en-US"_span));

    EXPECT_TRUE(context.ChangeListener().DirtyList().empty());
    EXPECT_EQ(cluster.GetDataVersion({ kRootEndpointId, LocalizationConfiguration::Id }), versionBefore);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
} // namespace
