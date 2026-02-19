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

#include <app/clusters/fixed-label-server/FixedLabelCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/FixedLabel/Attributes.h>
#include <clusters/FixedLabel/Metadata.h>

#include <clusters/FixedLabel/Structs.h>
#include <platform/DeviceInfoProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FixedLabel;
using namespace chip::app::Clusters::FixedLabel::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

// Mock DeviceInfoProvider for testing
class MockFixedLabelIterator : public DeviceLayer::DeviceInfoProvider::FixedLabelIterator
{
public:
    MockFixedLabelIterator()
    {
        mLabels[0].label = chip::CharSpan::fromCharString("test_label1");
        mLabels[0].value = chip::CharSpan::fromCharString("test_value1");
    }

    size_t Count() override { return 1; }

    bool Next(DeviceLayer::DeviceInfoProvider::FixedLabelType & output) override
    {
        if (mIndex < Count())
        {
            output = mLabels[mIndex++];
            return true;
        }
        return false;
    }

    void Release() override { delete this; }

private:
    size_t mIndex = 0;
    DeviceLayer::DeviceInfoProvider::FixedLabelType mLabels[1];
};

// In MockDeviceInfoProvider:
class MockDeviceInfoProvider : public DeviceLayer::DeviceInfoProvider
{
public:
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override { return new MockFixedLabelIterator(); }
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override { return nullptr; }
    SupportedLocalesIterator * IterateSupportedLocales() override { return nullptr; }
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override { return nullptr; }

protected:
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

struct TestFixedLabelCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(fixedLabel.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { fixedLabel.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestFixedLabelCluster() : fixedLabel(kRootEndpointId, mDeviceInfoProvider) {}

    TestServerClusterContext testContext;
    MockDeviceInfoProvider mDeviceInfoProvider;
    FixedLabelCluster fixedLabel;
};

} // namespace

TEST_F(TestFixedLabelCluster, AttributeTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(fixedLabel,
                                        {
                                            FixedLabel::Attributes::LabelList::kMetadataEntry,
                                        }));
}

TEST_F(TestFixedLabelCluster, ReadAttributeTest)
{
    ClusterTester tester(fixedLabel);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::LabelStruct::DecodableType> labelList;
    ASSERT_EQ(tester.ReadAttribute(LabelList::Id, labelList), CHIP_NO_ERROR);
    auto it = labelList.begin();
    ASSERT_TRUE(it.Next());
    auto label = it.GetValue();
    ASSERT_TRUE(label.label.data_equal(chip::CharSpan::fromCharString("test_label1")));
    ASSERT_TRUE(label.value.data_equal(chip::CharSpan::fromCharString("test_value1")));
    ASSERT_FALSE(it.Next());
}
