/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/closure-controller/ClosureController.h>
#include <device/types/closure-controller/OOBAccessors.h>
#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <binding-handler/BindingHandler.h>
#include <binding-handler/SimpleBindingCommand.h>
#include <clusters/ClosureControl/Commands.h>
#include <clusters/ClosureControl/Ids.h>
#include <clusters/ClosureDimension/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/core/TLV.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/tests/MessagingContext.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <platform/CHIPDeviceLayer.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kControllerEndpoint = 1;
constexpr EndpointId kOtherEndpoint      = 2;

/// OOB action fields, encoded as context tags 1 to 5 (see OOBAccessors.h). Unset fields are omitted.
struct ActionFields
{
    std::optional<EndpointId> endpointId = kControllerEndpoint;
    std::optional<uint8_t> tag2;
    std::optional<uint8_t> tag3;
    std::optional<uint16_t> tag4;
    std::optional<bool> tag5;
};

class ActionPayload
{
public:
    explicit ActionPayload(const ActionFields & fields = {}) { mError = Encode(fields); }

    ByteSpan Span() const
    {
        EXPECT_EQ(mError, CHIP_NO_ERROR);
        return ByteSpan(mBuffer, mLength);
    }

private:
    template <typename T>
    static CHIP_ERROR PutIfSet(TLV::TLVWriter & writer, uint8_t tagNum, const std::optional<T> & value)
    {
        VerifyOrReturnError(value.has_value(), CHIP_NO_ERROR);
        return writer.Put(TLV::ContextTag(tagNum), *value);
    }

    CHIP_ERROR Encode(const ActionFields & fields)
    {
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(PutIfSet(writer, 1, fields.endpointId));
        ReturnErrorOnFailure(PutIfSet(writer, 2, fields.tag2));
        ReturnErrorOnFailure(PutIfSet(writer, 3, fields.tag3));
        ReturnErrorOnFailure(PutIfSet(writer, 4, fields.tag4));
        ReturnErrorOnFailure(PutIfSet(writer, 5, fields.tag5));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        mLength = writer.GetLengthWritten();
        return CHIP_NO_ERROR;
    }

    uint8_t mBuffer[32];
    size_t mLength    = 0;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

template <typename CommandType>
Platform::UniquePtr<BindingCommand> MakeCommand(CommandType command = {})
{
    return Platform::UniquePtr<BindingCommand>(Platform::New<SimpleBindingCommand<CommandType>>(std::move(command)));
}

/**
 * ClosureController fixture with a BindingHandler initialized on a real Binding::Manager, backed by a
 * loopback messaging stack and a real CASESessionManager.
 */
class TestClosureController : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        mGroupDataProvider.SetStorageDelegate(&mGroupStorage);
        mGroupDataProvider.SetSessionKeystore(&mKeystore);
        ASSERT_EQ(mGroupDataProvider.Init(), CHIP_NO_ERROR);

        CASESessionManagerConfig config;
        config.sessionInitParams.sessionManager    = &GetSecureSessionManager();
        config.sessionInitParams.exchangeMgr       = &GetExchangeManager();
        config.sessionInitParams.fabricTable       = &GetFabricTable();
        config.sessionInitParams.groupDataProvider = &mGroupDataProvider;
        config.clientPool                          = &mCASEClientPool;
        config.sessionSetupPool                    = &mSessionSetupPool;
        ASSERT_EQ(mCASESessionManager.Init(&GetSystemLayer(), config), CHIP_NO_ERROR);

        ASSERT_EQ(
            mBindingHandler.Init(mBindingManager, { &GetFabricTable(), &mCASESessionManager, &mStorage }, GetExchangeManager()),
            CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        DrainAndServiceIO();
        mRegistry.Clear();
        mBindingHandler.Shutdown();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        Credentials::SetGroupDataProvider(nullptr);
        mGroupDataProvider.Finish();
        LoopbackMessagingContext::TearDown();
    }

protected:
    ClosureController::Config MakeConfig(BindingHandler & bindingHandler, bool withOptionalClients = true)
    {
        return ClosureController::Config{
            .bindingTable               = mBindingManager.GetBindingTable(),
            .bindingManager             = mBindingManager,
            .platformManager            = DeviceLayer::PlatformMgr(),
            .bindingHandler             = bindingHandler,
            .withIdentifyClient         = withOptionalClients,
            .withClosureDimensionClient = withOptionalClients,
        };
    }

    /// Provisions group keys for the Bob fabric so that group commands reach the loopback transport.
    void ProvisionGroupKeys()
    {
        Credentials::SetGroupDataProvider(&mGroupDataProvider);
        uint8_t buffer[sizeof(CompressedFabricId)];
        MutableByteSpan compressedFabricId(buffer);
        ASSERT_EQ(GetBobFabric()->GetCompressedFabricIdBytes(compressedFabricId), CHIP_NO_ERROR);
        ASSERT_EQ(GroupTesting::InitData(&mGroupDataProvider, GetBobFabricIndex(), compressedFabricId), CHIP_NO_ERROR);
    }

    void AddGroupBinding(ClusterId clusterId)
    {
        ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                      Binding::TableEntry::ForGroup(GetBobFabricIndex(), GetFriendsGroupId(), kControllerEndpoint, clusterId)),
                  CHIP_NO_ERROR);
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    InMemoryOOBAccessorRegistry mRegistry;

    TestPersistentStorageDelegate mStorage;
    TestPersistentStorageDelegate mGroupStorage;
    Crypto::DefaultSessionKeystore mKeystore;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Binding::Manager mBindingManager;
    BindingHandler mBindingHandler;
};

TEST_F(TestClosureController, RegisterAndUnregister)
{
    ClosureController controller(MakeConfig(mBindingHandler));

    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Stop::Type>()), CHIP_ERROR_INCORRECT_STATE);

    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(controller.GetEndpointId(), kControllerEndpoint);
    EXPECT_EQ(controller.Register(kOtherEndpoint, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Stop::Type>()), CHIP_NO_ERROR);

    controller.Unregister(mProvider);
    EXPECT_EQ(controller.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Stop::Type>()), CHIP_ERROR_INCORRECT_STATE);

    ASSERT_EQ(controller.Register(kOtherEndpoint, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(controller.GetEndpointId(), kOtherEndpoint);
    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, ClientClustersFollowConfig)
{
    ClosureController minimal(MakeConfig(mBindingHandler, false /* withOptionalClients */));
    ReadOnlyBufferBuilder<ClusterId> minimalBuilder;
    ASSERT_EQ(minimal.ClientClusters(minimalBuilder), CHIP_NO_ERROR);
    auto minimalClusters = minimalBuilder.TakeBuffer();
    ASSERT_EQ(minimalClusters.size(), 1U);
    EXPECT_EQ(minimalClusters[0], ClosureControl::Id);

    ClosureController full(MakeConfig(mBindingHandler));
    ReadOnlyBufferBuilder<ClusterId> fullBuilder;
    ASSERT_EQ(full.ClientClusters(fullBuilder), CHIP_NO_ERROR);
    auto fullClusters = fullBuilder.TakeBuffer();
    ASSERT_EQ(fullClusters.size(), 3U);
    EXPECT_EQ(fullClusters[0], ClosureControl::Id);
    EXPECT_EQ(fullClusters[1], Identify::Id);
    EXPECT_EQ(fullClusters[2], ClosureDimension::Id);
}

TEST_F(TestClosureController, DispatchRejectsNullAndDisabledClusters)
{
    ClosureController controller(MakeConfig(mBindingHandler, false /* withOptionalClients */));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(controller.Dispatch(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(controller.Dispatch(MakeCommand<OnOff::Commands::Toggle::Type>()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(controller.Dispatch(MakeCommand<Identify::Commands::Identify::Type>()), CHIP_ERROR_INVALID_ARGUMENT);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, DispatchFailsWhenBindingHandlerIsNotInitialized)
{
    BindingHandler uninitializedHandler;
    ClosureController controller(MakeConfig(uninitializedHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Stop::Type>()), CHIP_ERROR_INCORRECT_STATE);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, UnicastBindingQueuesCommandUntilSessionIsEstablished)
{
    // The binding refers to a fabric the device does not know, so session setup cannot start and the
    // binding manager keeps the command queued instead of sending it.
    constexpr FabricIndex kUnknownFabricIndex = 200;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(Binding::TableEntry::ForNode(
                  kUnknownFabricIndex, 0x1234 /* node */, kControllerEndpoint, 1 /* remote */, ClosureControl::Id)),
              CHIP_NO_ERROR);

    ClosureController controller(MakeConfig(mBindingHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);

    const uint32_t sentBefore = GetLoopback().mSentMessageCount;
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::MoveTo::Type>()), CHIP_NO_ERROR);
    EXPECT_EQ(GetLoopback().mSentMessageCount, sentBefore);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, GroupBindingSendsOnlyUntimedCommands)
{
    ProvisionGroupKeys();
    AddGroupBinding(ClosureControl::Id);

    ClosureController controller(MakeConfig(mBindingHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    auto & loopback = GetLoopback();

    uint32_t sentBefore = loopback.mSentMessageCount;
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Stop::Type>()), CHIP_NO_ERROR);
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    // MoveTo and Calibrate require a timed invoke, and group invokes cannot be timed.
    sentBefore = loopback.mSentMessageCount;
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::MoveTo::Type>()), CHIP_NO_ERROR);
    EXPECT_EQ(controller.Dispatch(MakeCommand<ClosureControl::Commands::Calibrate::Type>()), CHIP_NO_ERROR);
    EXPECT_EQ(loopback.mSentMessageCount, sentBefore);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, OOBAccessorDispatchesActions)
{
    ClosureController controller(MakeConfig(mBindingHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(controller, mRegistry);
    AddGroupBinding(ClosureControl::Id);
    AddGroupBinding(ClosureDimension::Id);
    AddGroupBinding(Identify::Id);

    EXPECT_EQ(mRegistry.HandleAction("ClosureMoveTo"_span, ActionPayload().Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureGroupedMoveTo"_span, ActionPayload({ .tag2 = 1, .tag3 = 2, .tag5 = true }).Span()),
              CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureSetTarget"_span, ActionPayload({ .tag4 = 5000 }).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(
        mRegistry.HandleAction("ClosureGroupedSetTarget"_span, ActionPayload({ .tag3 = 1, .tag4 = 10000, .tag5 = false }).Span()),
        CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStep"_span, ActionPayload({ .tag2 = 0, .tag4 = 2 }).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureGroupedStep"_span, ActionPayload({ .tag2 = 1, .tag3 = 3, .tag4 = 1 }).Span()),
              CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStop"_span, ActionPayload().Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureCalibrate"_span, ActionPayload().Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureIdentify"_span, ActionPayload({ .tag4 = 10 }).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("ClosureTriggerEffect"_span, ActionPayload({ .tag2 = 0, .tag3 = 0 }).Span()), CHIP_NO_ERROR);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, OOBAccessorRejectsInvalidPayloads)
{
    ClosureController controller(MakeConfig(mBindingHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(controller, mRegistry);

    // Unknown actions and other endpoints are left to other accessors.
    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload().Span()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStop"_span, ActionPayload({ .endpointId = kOtherEndpoint }).Span()),
              CHIP_ERROR_NOT_FOUND);

    EXPECT_EQ(mRegistry.HandleAction("ClosureStop"_span, ActionPayload({ .endpointId = std::nullopt }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Unknown enum values.
    EXPECT_EQ(mRegistry.HandleAction("ClosureMoveTo"_span, ActionPayload({ .tag2 = 0xFF }).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureMoveTo"_span, ActionPayload({ .tag3 = 0xFF }).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStep"_span, ActionPayload({ .tag2 = 0xFF, .tag4 = 1 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureTriggerEffect"_span, ActionPayload({ .tag2 = 3, .tag3 = 0 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureTriggerEffect"_span, ActionPayload({ .tag2 = 0, .tag3 = 1 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Out-of-range values.
    EXPECT_EQ(mRegistry.HandleAction("ClosureSetTarget"_span, ActionPayload({ .tag4 = 10001 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStep"_span, ActionPayload({ .tag2 = 0, .tag4 = 0 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Missing mandatory fields.
    EXPECT_EQ(mRegistry.HandleAction("ClosureStep"_span, ActionPayload({ .tag4 = 1 }).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureStep"_span, ActionPayload({ .tag2 = 0 }).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureIdentify"_span, ActionPayload().Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureTriggerEffect"_span, ActionPayload({ .tag2 = 0 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, OOBAccessorRejectsDisabledClientClusters)
{
    ClosureController controller(MakeConfig(mBindingHandler, false /* withOptionalClients */));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(controller, mRegistry);

    EXPECT_EQ(mRegistry.HandleAction("ClosureSetTarget"_span, ActionPayload().Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("ClosureIdentify"_span, ActionPayload({ .tag4 = 10 }).Span()), CHIP_ERROR_INVALID_ARGUMENT);

    controller.Unregister(mProvider);
}

TEST_F(TestClosureController, OOBAccessorPropagatesSendErrors)
{
    BindingHandler uninitializedHandler;
    ClosureController controller(MakeConfig(uninitializedHandler));
    ASSERT_EQ(controller.Register(kControllerEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(controller, mRegistry);

    EXPECT_EQ(mRegistry.HandleAction("ClosureStop"_span, ActionPayload().Span()), CHIP_ERROR_INCORRECT_STATE);

    controller.Unregister(mProvider);
}

} // namespace
