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

#include <vector>

#include <app/clusters/ota-requestor/OTARequestorStorage.h>
#include <app/common/CompatEnumNames.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/EventIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;

using chip::app::DataModel::Nullable;
using chip::app::DataModel::NullNullable;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct MockOTARequestorStorage : public OTARequestorStorage
{
    ~MockOTARequestorStorage() override = default;

    CHIP_ERROR StoreDefaultProviders(const ProviderLocationList & providers) override
    {
        storeDefaultProvidersCalled = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LoadDefaultProviders(ProviderLocationList & providers) override
    {
        loadDefaultProvidersCalled = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LoadCurrentUpdateState(OTAUpdateStateEnum & currentUpdateState) override
    {
        loadCurrentUpdateStateCalled = true;
        return CHIP_NO_ERROR;
    }

    // Unrelated methods.
    CHIP_ERROR StoreCurrentProviderLocation(const ProviderLocationType & provider) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClearCurrentProviderLocation() override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadCurrentProviderLocation(ProviderLocationType & provider) override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreUpdateToken(ByteSpan updateToken) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadUpdateToken(MutableByteSpan & updateToken) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClearUpdateToken() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreCurrentUpdateState(OTAUpdateStateEnum currentUpdateState) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClearCurrentUpdateState() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreTargetVersion(uint32_t targetVersion) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadTargetVersion(uint32_t & targetVersion) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClearTargetVersion() override { return CHIP_NO_ERROR; }

    bool storeDefaultProvidersCalled  = false;
    bool loadDefaultProvidersCalled   = false;
    bool loadCurrentUpdateStateCalled = false;
};

struct MockAttributeChangeListener : public OTARequestorAttributes::AttributeChangeListener
{
    void AttributeChanged(AttributeId attributeId) override { dirtyList.push_back(attributeId); }

    std::vector<AttributeId> dirtyList;
};

struct TestOTARequestorAttributes : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOTARequestorAttributes, SetChangeListenerRejectsInvalidEndpointId)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;
    OTARequestorAttributes attributes;

    EXPECT_NE(attributes.SetInteractionModelContext(kInvalidEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateChangesValue)
{
    OTARequestorAttributes attributes;
    OTAChangeReasonEnum reason = OTAChangeReasonEnum::kSuccess;

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kIdle);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kQuerying, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kQuerying);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnQuery);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDownloading, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kApplying);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnApply);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kRollingBack);

    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent, reason, NullNullable);
    EXPECT_EQ(attributes.GetUpdateState(), OTARequestorAttributes::OTAUpdateStateEnum::kDelayedOnUserConsent);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateMarksChangedWhenDifferent)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;
    OTAChangeReasonEnum reason = OTAChangeReasonEnum::kSuccess;

    OTARequestorAttributes attributes;
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown, reason, NullNullable);
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle, reason, NullNullable);
    ASSERT_EQ(changeListener.dirtyList.size(), 1u);
    EXPECT_EQ(changeListener.dirtyList[0], UpdateState::Id);

    changeListener.dirtyList.clear();
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle, reason, NullNullable);
    EXPECT_EQ(changeListener.dirtyList.size(), 0u);
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateGeneratesEventWhenDifferent)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;
    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();

    OTARequestorAttributes attributes;
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kUnknown, OTAChangeReasonEnum::kUnknown, NullNullable);
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, eventsGenerator), CHIP_NO_ERROR);
    ASSERT_FALSE(eventsGenerator.GetNextEvent().has_value());

    // Verify setting a state with a target version generates a matching event.
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess,
                              Nullable<uint32_t>(1234u));
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
    EXPECT_EQ(event->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, Events::StateTransition::Id));
    Events::StateTransition::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.previousState, OTARequestorAttributes::OTAUpdateStateEnum::kUnknown);
    EXPECT_EQ(decodedEvent.newState, OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);
    EXPECT_EQ(decodedEvent.reason, OTAChangeReasonEnum::kSuccess);
    EXPECT_EQ(decodedEvent.targetSoftwareVersion, Nullable<uint32_t>(1234u));

    // Verify setting a state without a target version generates a matching event.
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kTimeOut, NullNullable);
    event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
    EXPECT_EQ(event->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, Events::StateTransition::Id));
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.previousState, OTARequestorAttributes::OTAUpdateStateEnum::kDownloading);
    EXPECT_EQ(decodedEvent.newState, OTARequestorAttributes::OTAUpdateStateEnum::kIdle);
    EXPECT_EQ(decodedEvent.reason, OTAChangeReasonEnum::kTimeOut);
    EXPECT_TRUE(decodedEvent.targetSoftwareVersion.IsNull());

    // Verify setting the same state doesn't generate an event.
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess, NullNullable);
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
}

TEST_F(TestOTARequestorAttributes, SetUpdateStateProgressChangesValue)
{
    OTARequestorAttributes attributes;

    EXPECT_EQ(attributes.SetUpdateStateProgress(NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), NullNullable);

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
    MockAttributeChangeListener changeListener;

    OTARequestorAttributes attributes;
    EXPECT_EQ(attributes.SetUpdateStateProgress(NullNullable), CHIP_NO_ERROR);
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.dirtyList.size(), 1u);
    EXPECT_EQ(changeListener.dirtyList[0], UpdateStateProgress::Id);

    changeListener.dirtyList.clear();
    EXPECT_EQ(attributes.SetUpdateStateProgress(42), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.dirtyList.size(), 0u);
}

TEST_F(TestOTARequestorAttributes, InvalidUpdateStateProgressDoesNotChangeValueOrMarkChanged)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;

    OTARequestorAttributes attributes;
    EXPECT_EQ(attributes.SetUpdateStateProgress(NullNullable), CHIP_NO_ERROR);
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    EXPECT_NE(attributes.SetUpdateStateProgress(200), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.GetUpdateStateProgress(), NullNullable);
    EXPECT_EQ(changeListener.dirtyList.size(), 0u);
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
    MockAttributeChangeListener changeListener;

    OTARequestorAttributes attributes;
    attributes.SetUpdatePossible(false);
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    attributes.SetUpdatePossible(true);
    ASSERT_EQ(changeListener.dirtyList.size(), 1u);
    EXPECT_EQ(changeListener.dirtyList[0], UpdatePossible::Id);

    changeListener.dirtyList.clear();
    attributes.SetUpdatePossible(true);
    EXPECT_EQ(changeListener.dirtyList.size(), 0u);
}

TEST_F(TestOTARequestorAttributes, CanAddAndIterateProviders)
{
    OTARequestorAttributes attributes;

    OTARequestorAttributes::ProviderLocationType new_location;
    new_location.providerNodeID = 0x1234;
    new_location.endpoint       = 10;
    new_location.fabricIndex    = 1;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    new_location.providerNodeID = 0x5678;
    new_location.endpoint       = 20;
    new_location.fabricIndex    = 2;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    new_location.providerNodeID = 0x90ab;
    new_location.endpoint       = 30;
    new_location.fabricIndex    = 3;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    // Verify that all the providers were added.
    ProviderLocationList::Iterator iterator = attributes.GetDefaultOtaProviderListIterator();
    bool found_first                        = false;
    bool found_second                       = false;
    bool found_third                        = false;
    while (iterator.Next())
    {
        auto location = iterator.GetValue();
        if (location.providerNodeID == 0x1234 && location.endpoint == 10 && location.fabricIndex == 1)
        {
            found_first = true;
        }
        else if (location.providerNodeID == 0x5678 && location.endpoint == 20 && location.fabricIndex == 2)
        {
            found_second = true;
        }
        else if (location.providerNodeID == 0x90ab && location.endpoint == 30 && location.fabricIndex == 3)
        {
            found_third = true;
        }
        else
        {
            ADD_FAILURE() << "Unexpected location: " << location.providerNodeID << ", " << location.endpoint << ", "
                          << location.fabricIndex;
        }
    }
    EXPECT_TRUE(found_first);
    EXPECT_TRUE(found_second);
    EXPECT_TRUE(found_third);
}

TEST_F(TestOTARequestorAttributes, ClearingProvidersRemovesFromList)
{
    OTARequestorAttributes attributes;

    // Add some locations.
    OTARequestorAttributes::ProviderLocationType new_location;
    new_location.providerNodeID = 0x1234;
    new_location.endpoint       = 10;
    new_location.fabricIndex    = 1;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    new_location.providerNodeID = 0x5678;
    new_location.endpoint       = 20;
    new_location.fabricIndex    = 2;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    new_location.providerNodeID = 0x90ab;
    new_location.endpoint       = 30;
    new_location.fabricIndex    = 3;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(new_location), CHIP_NO_ERROR);

    // Remove the locations one at a time and verify they've been removed.
    EXPECT_EQ(attributes.RemoveDefaultOtaProvider(2), CHIP_NO_ERROR);
    ProviderLocationList::Iterator iterator = attributes.GetDefaultOtaProviderListIterator();
    int count                               = 0;
    while (iterator.Next())
    {
        auto & location = iterator.GetValue();
        EXPECT_NE(location.fabricIndex, 2);
        ++count;
    }
    EXPECT_EQ(count, 2);

    EXPECT_EQ(attributes.RemoveDefaultOtaProvider(3), CHIP_NO_ERROR);
    iterator = attributes.GetDefaultOtaProviderListIterator();
    count    = 0;
    while (iterator.Next())
    {
        auto & location = iterator.GetValue();
        EXPECT_NE(location.fabricIndex, 3);
        ++count;
    }
    EXPECT_EQ(count, 1);

    EXPECT_EQ(attributes.RemoveDefaultOtaProvider(1), CHIP_NO_ERROR);
    iterator = attributes.GetDefaultOtaProviderListIterator();
    count    = 0;
    while (iterator.Next())
    {
        auto & location = iterator.GetValue();
        EXPECT_NE(location.fabricIndex, 1);
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST_F(TestOTARequestorAttributes, ChangingProvidersMarksChanged)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;

    OTARequestorAttributes attributes;
    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    OTARequestorAttributes::ProviderLocationType location;
    location.providerNodeID = 0x1234;
    location.endpoint       = 10;
    location.fabricIndex    = 1;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(location), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.dirtyList.size(), 1u);
    EXPECT_EQ(changeListener.dirtyList[0], DefaultOTAProviders::Id);

    changeListener.dirtyList.clear();
    EXPECT_EQ(attributes.RemoveDefaultOtaProvider(1), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.dirtyList.size(), 1u);
    EXPECT_EQ(changeListener.dirtyList[0], DefaultOTAProviders::Id);
}

TEST_F(TestOTARequestorAttributes, CannotAddProviderForFabricWithProvider)
{
    chip::Testing::TestServerClusterContext context;
    MockAttributeChangeListener changeListener;

    OTARequestorAttributes attributes;

    OTARequestorAttributes::ProviderLocationType location;
    location.providerNodeID = 0x1234;
    location.endpoint       = 10;
    location.fabricIndex    = 1;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(location), CHIP_NO_ERROR);

    ASSERT_EQ(attributes.SetInteractionModelContext(kTestEndpointId, changeListener, context.EventsGenerator()), CHIP_NO_ERROR);

    changeListener.dirtyList.clear();
    location.providerNodeID = 0x5678;
    location.endpoint       = 20;
    location.fabricIndex    = 1;
    EXPECT_NE(attributes.AddDefaultOtaProvider(location), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.dirtyList.size(), 0u);
}

TEST_F(TestOTARequestorAttributes, DefaultProvidersAreLoadedWhenSettingStorage)
{
    MockOTARequestorStorage storage;
    OTARequestorAttributes attributes;

    storage.loadDefaultProvidersCalled = false;
    EXPECT_EQ(attributes.SetStorageAndLoadAttributes(storage), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.loadDefaultProvidersCalled);
}

TEST_F(TestOTARequestorAttributes, UpdateStateIsLoadedWhenSettingStorage)
{
    MockOTARequestorStorage storage;
    OTARequestorAttributes attributes;

    storage.loadCurrentUpdateStateCalled = false;
    EXPECT_EQ(attributes.SetStorageAndLoadAttributes(storage), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.loadCurrentUpdateStateCalled);
}

TEST_F(TestOTARequestorAttributes, AddingProviderUpdatesDefaultProviders)
{
    MockOTARequestorStorage storage;
    OTARequestorAttributes attributes;
    OTARequestorAttributes::ProviderLocationType location;
    location.providerNodeID = 0x1234;
    location.endpoint       = 10;
    location.fabricIndex    = 1;
    ASSERT_EQ(attributes.SetStorageAndLoadAttributes(storage), CHIP_NO_ERROR);

    storage.storeDefaultProvidersCalled = false;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(location), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.storeDefaultProvidersCalled);
}

TEST_F(TestOTARequestorAttributes, RemovingProviderUpdatesDefaultProviders)
{
    MockOTARequestorStorage storage;
    OTARequestorAttributes attributes;
    OTARequestorAttributes::ProviderLocationType location;
    location.providerNodeID = 0x1234;
    location.endpoint       = 10;
    location.fabricIndex    = 1;
    ASSERT_EQ(attributes.SetStorageAndLoadAttributes(storage), CHIP_NO_ERROR);
    EXPECT_EQ(attributes.AddDefaultOtaProvider(location), CHIP_NO_ERROR);

    storage.storeDefaultProvidersCalled = false;
    EXPECT_EQ(attributes.RemoveDefaultOtaProvider(1), CHIP_NO_ERROR);
    EXPECT_TRUE(storage.storeDefaultProvidersCalled);
}

} // namespace
