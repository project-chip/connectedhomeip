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
#include <pw_unit_test/framework.h>

#include <app/clusters/user-label-server/UserLabelCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/UserLabel/Attributes.h>
#include <clusters/UserLabel/Enums.h>
#include <clusters/UserLabel/Metadata.h>
#include <clusters/UserLabel/Structs.h>
#include <platform/DeviceInfoProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UserLabel;
using namespace chip::app::Clusters::UserLabel::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

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
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override
    {
        val = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override { return CHIP_NO_ERROR; }
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_NO_ERROR; }
};

struct TestUserLabelCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        DeviceLayer::SetDeviceInfoProvider(&mDeviceInfoProvider);
        ASSERT_EQ(userLabel.Startup(testContext.Get()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        userLabel.Shutdown(ClusterShutdownType::kClusterShutdown);
        DeviceLayer::SetDeviceInfoProvider(nullptr);
    }

    TestUserLabelCluster() : userLabel(kRootEndpointId) {}

    TestServerClusterContext testContext;
    UserLabelCluster userLabel;
    MockDeviceInfoProvider mDeviceInfoProvider;
};

} // namespace

TEST_F(TestUserLabelCluster, AttributeTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(userLabel,
                                        {
                                            UserLabel::Attributes::LabelList::kMetadataEntry,
                                        }));
}

TEST_F(TestUserLabelCluster, ReadAttributeTest)
{
    ClusterTester tester(userLabel);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::LabelStruct::DecodableType> labelList;
    ASSERT_EQ(tester.ReadAttribute(LabelList::Id, labelList), CHIP_NO_ERROR);
    auto it = labelList.begin();
    while (it.Next())
    {
        ASSERT_GT(it.GetValue().label.size(), 0u);
    }
}

TEST_F(TestUserLabelCluster, WriteValidLabelListTest)
{
    ClusterTester tester(userLabel);
    Structs::LabelStruct::Type labels[] = {
        { .label = "room"_span, .value = "bedroom 2"_span },
        { .label = "orientation"_span, .value = "North"_span },
    };
    ASSERT_EQ(tester.WriteAttribute(LabelList::Id, DataModel::List(labels)), CHIP_NO_ERROR);
}

TEST_F(TestUserLabelCluster, WriteLabelWithLabelTooLongTest)
{
    ClusterTester tester(userLabel);
    constexpr auto tooLongLabel = "this_label_is_way_too_long"_span;
    static_assert(tooLongLabel.size() > UserLabelCluster::kMaxLabelSize);
    Structs::LabelStruct::Type labels[] = {
        { .label = tooLongLabel, .value = "value"_span },
    };
    ASSERT_EQ(tester.WriteAttribute(LabelList::Id, DataModel::List(labels)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestUserLabelCluster, WriteLabelWithValueTooLongTest)
{
    ClusterTester tester(userLabel);
    constexpr auto tooLongValue = "this_value_is_way_too_long"_span;
    static_assert(tooLongValue.size() > UserLabelCluster::kMaxValueSize);
    Structs::LabelStruct::Type labels[] = {
        { .label = "room"_span, .value = tooLongValue },
    };
    ASSERT_EQ(tester.WriteAttribute(LabelList::Id, DataModel::List(labels)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestUserLabelCluster, WriteEmptyLabelsTest)
{
    ClusterTester tester(userLabel);
    Structs::LabelStruct::Type labels[] = {
        { .label = ""_span, .value = ""_span }, // empty label and value are allowed per spec
    };
    ASSERT_EQ(tester.WriteAttribute(LabelList::Id, DataModel::List(labels)), CHIP_NO_ERROR);
}

TEST_F(TestUserLabelCluster, WriteMaxSizeLabelListTest)
{
    ClusterTester tester(userLabel);
    std::array<Structs::LabelStruct::Type, chip::DeviceLayer::kMaxUserLabelListLength> labels;
    for (size_t i = 0; i < labels.size(); i++)
    {
        labels[i].label = "label"_span;
        labels[i].value = "value"_span;
    }
    ASSERT_EQ(tester.WriteAttribute(LabelList::Id, DataModel::List(labels.data(), labels.size())), CHIP_NO_ERROR);
}
