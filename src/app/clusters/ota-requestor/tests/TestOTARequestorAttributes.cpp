/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */


#include <app/clusters/ota-requestor/OTARequestorAttributes.h>
#include <pw_unit_test/framework.h>

#include <app/data-model/Nullable.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;

using chip::app::DataModel::Nullable;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestOTARequestorAttributes : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOTARequestorAttributes, SetChangeListenerRejectsInvalidEndpointId)
{
    chip::Testing::TestServerClusterContext context;
    OTARequestorAttributes attributes;

    EXPECT_NE(attributes.SetChangeListener(kInvalidEndpointId, context.ChangeListener()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateChangesValue)
{
    OTARequestorAttributes attributes;

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kIdle);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kQuerying);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kQuerying);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kApplying);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateMarksChangedWhenDifferent)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);
    ASSERT_EQ(attributes.SetChangeListener(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateProgressChangesValue)
{
    OTARequestorAttributes attributes;

    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), DataModel::NullNullable);

    EXPECT_EQ(attributes.SetUpdateStateProgress(0), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(0));

    EXPECT_EQ(attributes.SetUpdateStateProgress(1), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(1));

    EXPECT_EQ(attributes.SetUpdateStateProgress(50), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(50));

    EXPECT_EQ(attributes.SetUpdateStateProgress(99), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(99));

    EXPECT_EQ(attributes.SetUpdateStateProgress(100), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(100));
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateProgressMarksChangedWhenDifferent)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);
    ASSERT_EQ(attributes.SetChangeListener(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, InvalidUpdateStateProgressDoesNotChangeValueOrMarkChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);
    ASSERT_EQ(attributes.SetChangeListener(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_NE(attributes.SetUpdateStateProgress(200), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), DataModel::NullNullable);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStatePossibleChangesValue)
{
    OTARequestorAttributes attributes;

    attributes.SetUpdatePossible(true);
    EXPECT_TRUE(attributes.GetUpdatePossible());

    attributes.SetUpdatePossible(false);
    EXPECT_FALSE(attributes.GetUpdatePossible());
}

TEST_F(TestOTARequestorAttributes, SetUpdateStatePossibleMarksChangedWhenDifferent)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    attributes.SetUpdatePossible(false);
    ASSERT_EQ(attributes.SetChangeListener(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    attributes.SetUpdatePossible(true);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdatePossible::Id);

    changeListener.DirtyList().clear();
    attributes.SetUpdatePossible(true);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

} // namespace
