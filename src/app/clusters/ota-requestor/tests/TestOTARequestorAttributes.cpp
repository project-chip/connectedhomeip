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

TEST_F(TestOTARequestorAttributes, CannotSetWithoutInitialization)
{
    OTARequestorAttributes attributes;

    EXPECT_NE(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle), CHIP_NO_ERROR);
    EXPECT_NE(attributes.SetUpdateStateProgress(50), CHIP_NO_ERROR);
    EXPECT_NE(attributes.SetUpdatePossible(false), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorAttributes, RejectsInvalidEndpointId)
{
    chip::Testing::TestServerClusterContext context;
    OTARequestorAttributes attributes;

    EXPECT_NE(attributes.Init(kInvalidEndpointId, context.ChangeListener()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateChangesValueAndMarksChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set the current state so it won't match the first value tested below.
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kIdle);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kQuerying), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kQuerying);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDownloading), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kApplying);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateToCurrentValueDoesNotMarkChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set a specific state.
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying), CHIP_NO_ERROR);

    // Set the state again and expect that it doesn't mark the attribute changed.
    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateProgressChangesValueAndMarksChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set the current value so it won't match the first value tested below.
    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(0), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(0));
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(1), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(1));
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(50), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(50));
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(99), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(99));
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(100), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), Nullable<uint8_t>(100));
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), DataModel::NullNullable);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);
}

TEST_F(TestOTARequestorAttributes, InvalidUpdateStateProgressDoesNotChangeValue)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set the current value to later verify it hasn't changed.
    EXPECT_EQ(attributes.SetUpdateStateProgress(DataModel::NullNullable), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_NE(attributes.SetUpdateStateProgress(200), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), DataModel::NullNullable);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateProgressToCurrentValueDoesNotMarkChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set a specific value.
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);

    // Set the value again and expect that it doesn't mark the attribute changed.
    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStatePossibleChangesValueAndMarksChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set the current value so it won't match the first value tested below.
    EXPECT_EQ(attributes.SetUpdatePossible(false), CHIP_NO_ERROR);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdatePossible(true), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdatePossible(), true);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdatePossible::Id);

    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdatePossible(false), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdatePossible(), false);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdatePossible::Id);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStatePossibleToCurrentValueDoesNotMarkChanged)
{
    chip::Testing::TestServerClusterContext context;
    auto & changeListener = context.ChangeListener();

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.Init(kTestEndpointId, changeListener), CHIP_NO_ERROR);

    // Set a specific value.
    EXPECT_EQ(attributes.SetUpdatePossible(false), CHIP_NO_ERROR);

    // Set the value again and expect that it doesn't mark the attribute changed.
    changeListener.DirtyList().clear();
    EXPECT_EQ(attributes.SetUpdatePossible(false), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

} // namespace
