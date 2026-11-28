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

#include <device/types/window-covering-controller/OOBAccessors.h>
#include <device/types/window-covering-controller/WindowCoveringController.h>
#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/data-model-provider/Provider.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <binding-handler/BindingHandler.h>
#include <binding-handler/SimpleBindingCommand.h>
#include <clusters/Binding/Ids.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/Groups/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/WindowCovering/Commands.h>
#include <clusters/WindowCovering/Ids.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/core/TLV.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
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

/// OOB action fields, encoded as context tags 1 and 2 (see OOBAccessors.h). Unset fields are omitted.
struct ActionFields
{
    std::optional<EndpointId> endpointId = kControllerEndpoint;
    std::optional<uint16_t> percent100ths;
};

class ActionPayload
{
public:
    explicit ActionPayload(const ActionFields & fields = {}) { mError = Encode(fields); }

    ByteSpan Span() const
    {
        EXPECT_SUCCESS(mError);
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
        ReturnErrorOnFailure(PutIfSet(writer, 2, fields.percent100ths));
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

bool HasServerCluster(DataModel::Provider & provider, EndpointId endpointId, ClusterId clusterId)
{
    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    if (provider.ServerClusters(endpointId, builder) != CHIP_NO_ERROR)
    {
        return false;
    }
    for (const auto & entry : builder.TakeBuffer())
    {
        if (entry.clusterId == clusterId)
        {
            return true;
        }
    }
    return false;
}

/**
 * WindowCoveringController fixture with a BindingHandler initialized on a real Binding::Manager, backed by a
 * loopback messaging stack and a real CASESessionManager.
 */
class TestWindowCoveringController : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        mGroupDataProvider.SetStorageDelegate(&mGroupStorage);
        mGroupDataProvider.SetSessionKeystore(&mKeystore);
        ASSERT_SUCCESS(mGroupDataProvider.Init());

        CASESessionManagerConfig config;
        config.sessionInitParams.sessionManager    = &GetSecureSessionManager();
        config.sessionInitParams.exchangeMgr       = &GetExchangeManager();
        config.sessionInitParams.fabricTable       = &GetFabricTable();
        config.sessionInitParams.groupDataProvider = &mGroupDataProvider;
        config.clientPool                          = &mCASEClientPool;
        config.sessionSetupPool                    = &mSessionSetupPool;
        ASSERT_SUCCESS(mCASESessionManager.Init(&GetSystemLayer(), config));

        ASSERT_SUCCESS(
            mBindingHandler.Init(mBindingManager, { &GetFabricTable(), &mCASESessionManager, &mStorage }, GetExchangeManager()));
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
    WindowCoveringController::Config MakeConfig(BindingHandler & bindingHandler, bool withOptionalClusters = true)
    {
        return WindowCoveringController::Config{
            .bindingTable       = mBindingManager.GetBindingTable(),
            .bindingManager     = mBindingManager,
            .platformManager    = DeviceLayer::PlatformMgr(),
            .bindingHandler     = bindingHandler,
            .timerDelegate      = mTimerDelegate,
            .withIdentifyServer = withOptionalClusters,
            .withIdentifyClient = withOptionalClusters,
            .withGroupsClient   = withOptionalClusters,
        };
    }

    /// Provisions group keys for the Bob fabric so that group commands reach the loopback transport.
    void ProvisionGroupKeys()
    {
        Credentials::SetGroupDataProvider(&mGroupDataProvider);
        uint8_t buffer[sizeof(CompressedFabricId)];
        MutableByteSpan compressedFabricId(buffer);
        ASSERT_SUCCESS(GetBobFabric()->GetCompressedFabricIdBytes(compressedFabricId));
        ASSERT_SUCCESS(GroupTesting::InitData(&mGroupDataProvider, GetBobFabricIndex(), compressedFabricId));
    }

    void AddGroupBinding(ClusterId clusterId)
    {
        ASSERT_SUCCESS(mBindingManager.GetBindingTable().Add(
            Binding::TableEntry::ForGroup(GetBobFabricIndex(), GetFriendsGroupId(), kControllerEndpoint, clusterId)));
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
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

TEST_F(TestWindowCoveringController, RegisterAndUnregister)
{
    WindowCoveringController controller(MakeConfig(mBindingHandler));

    EXPECT_EQ(controller.Dispatch(MakeCommand<WindowCovering::Commands::StopMotion::Type>()), CHIP_ERROR_INCORRECT_STATE);

    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));
    EXPECT_EQ(controller.GetEndpointId(), kControllerEndpoint);
    EXPECT_EQ(controller.Register(kOtherEndpoint, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::StopMotion::Type>()));

    controller.Unregister(mProvider);
    EXPECT_EQ(controller.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(controller.Dispatch(MakeCommand<WindowCovering::Commands::StopMotion::Type>()), CHIP_ERROR_INCORRECT_STATE);

    ASSERT_SUCCESS(controller.Register(kOtherEndpoint, mProvider));
    EXPECT_EQ(controller.GetEndpointId(), kOtherEndpoint);
    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, ClientClustersFollowConfig)
{
    WindowCoveringController minimal(MakeConfig(mBindingHandler, false /* withOptionalClusters */));
    ReadOnlyBufferBuilder<ClusterId> minimalBuilder;
    ASSERT_SUCCESS(minimal.ClientClusters(minimalBuilder));
    auto minimalClusters = minimalBuilder.TakeBuffer();
    ASSERT_EQ(minimalClusters.size(), 1U);
    EXPECT_EQ(minimalClusters[0], WindowCovering::Id);

    WindowCoveringController full(MakeConfig(mBindingHandler));
    ReadOnlyBufferBuilder<ClusterId> fullBuilder;
    ASSERT_SUCCESS(full.ClientClusters(fullBuilder));
    auto fullClusters = fullBuilder.TakeBuffer();
    ASSERT_EQ(fullClusters.size(), 3U);
    EXPECT_EQ(fullClusters[0], WindowCovering::Id);
    EXPECT_EQ(fullClusters[1], Identify::Id);
    EXPECT_EQ(fullClusters[2], Groups::Id);
}

TEST_F(TestWindowCoveringController, DispatchRejectsNullAndDisabledClusters)
{
    WindowCoveringController controller(MakeConfig(mBindingHandler, false /* withOptionalClusters */));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));

    EXPECT_EQ(controller.Dispatch(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(controller.Dispatch(MakeCommand<OnOff::Commands::Toggle::Type>()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(controller.Dispatch(MakeCommand<Identify::Commands::Identify::Type>()), CHIP_ERROR_INVALID_ARGUMENT);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, DispatchFailsWhenBindingHandlerIsNotInitialized)
{
    BindingHandler uninitializedHandler;
    WindowCoveringController controller(MakeConfig(uninitializedHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));

    EXPECT_EQ(controller.Dispatch(MakeCommand<WindowCovering::Commands::StopMotion::Type>()), CHIP_ERROR_INCORRECT_STATE);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, IdentifyServerFollowsConfig)
{
    WindowCoveringController minimal(MakeConfig(mBindingHandler, false /* withOptionalClusters */));
    ASSERT_SUCCESS(minimal.Register(kControllerEndpoint, mProvider));
    EXPECT_TRUE(HasServerCluster(mProvider, kControllerEndpoint, Descriptor::Id));
    EXPECT_TRUE(HasServerCluster(mProvider, kControllerEndpoint, Binding::Id));
    EXPECT_FALSE(HasServerCluster(mProvider, kControllerEndpoint, Identify::Id));
    minimal.Unregister(mProvider);

    WindowCoveringController full(MakeConfig(mBindingHandler));
    ASSERT_SUCCESS(full.Register(kControllerEndpoint, mProvider));
    EXPECT_TRUE(HasServerCluster(mProvider, kControllerEndpoint, Descriptor::Id));
    EXPECT_TRUE(HasServerCluster(mProvider, kControllerEndpoint, Binding::Id));
    EXPECT_TRUE(HasServerCluster(mProvider, kControllerEndpoint, Identify::Id));
    full.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, GroupBindingSendsCommands)
{
    ProvisionGroupKeys();
    AddGroupBinding(WindowCovering::Id);

    WindowCoveringController controller(MakeConfig(mBindingHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));
    auto & loopback = GetLoopback();

    WindowCovering::Commands::GoToLiftPercentage::Type goToLift;
    goToLift.liftPercent100thsValue = 5000;
    WindowCovering::Commands::GoToTiltPercentage::Type goToTilt;
    goToTilt.tiltPercent100thsValue = 2500;

    uint32_t sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::UpOrOpen::Type>()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::DownOrClose::Type>()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::StopMotion::Type>()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::GoToLiftPercentage::Type>(goToLift)));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::GoToTiltPercentage::Type>(goToTilt)));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, OOBAccessorDispatchesActions)
{
    ProvisionGroupKeys();
    AddGroupBinding(WindowCovering::Id);

    WindowCoveringController controller(MakeConfig(mBindingHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));
    RegisterOOBAccessors(controller, mRegistry);
    auto & loopback = GetLoopback();

    uint32_t sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(mRegistry.HandleAction("WindowCoveringUpOrOpen"_span, ActionPayload().Span()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(mRegistry.HandleAction("WindowCoveringDownOrClose"_span, ActionPayload().Span()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(mRegistry.HandleAction("WindowCoveringStopMotion"_span, ActionPayload().Span()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(
        mRegistry.HandleAction("WindowCoveringGoToLiftPercentage"_span, ActionPayload({ .percent100ths = 5000 }).Span()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    sentBefore = loopback.mSentMessageCount;
    EXPECT_SUCCESS(
        mRegistry.HandleAction("WindowCoveringGoToTiltPercentage"_span, ActionPayload({ .percent100ths = 10000 }).Span()));
    EXPECT_GT(loopback.mSentMessageCount, sentBefore);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, OOBAccessorRejectsInvalidPayloads)
{
    WindowCoveringController controller(MakeConfig(mBindingHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));
    RegisterOOBAccessors(controller, mRegistry);

    // Unknown actions and other endpoints are left to other accessors.
    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload().Span()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringStopMotion"_span, ActionPayload({ .endpointId = kOtherEndpoint }).Span()),
              CHIP_ERROR_NOT_FOUND);

    // Missing mandatory fields.
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringStopMotion"_span, ActionPayload({ .endpointId = std::nullopt }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringGoToLiftPercentage"_span, ActionPayload().Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringGoToTiltPercentage"_span, ActionPayload().Span()), CHIP_ERROR_INVALID_ARGUMENT);

    // Out-of-range values.
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringGoToLiftPercentage"_span, ActionPayload({ .percent100ths = 10001 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringGoToTiltPercentage"_span, ActionPayload({ .percent100ths = 10001 }).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, OOBAccessorPropagatesSendErrors)
{
    BindingHandler uninitializedHandler;
    WindowCoveringController controller(MakeConfig(uninitializedHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));
    RegisterOOBAccessors(controller, mRegistry);

    EXPECT_EQ(mRegistry.HandleAction("WindowCoveringStopMotion"_span, ActionPayload().Span()), CHIP_ERROR_INCORRECT_STATE);

    controller.Unregister(mProvider);
}

TEST_F(TestWindowCoveringController, UnicastBindingQueuesCommandUntilSessionIsEstablished)
{
    // The binding refers to a fabric the device does not know, so session setup cannot start and the
    // binding manager keeps the command queued instead of sending it.
    constexpr FabricIndex kUnknownFabricIndex = 200;
    ASSERT_SUCCESS(mBindingManager.GetBindingTable().Add(Binding::TableEntry::ForNode(
        kUnknownFabricIndex, 0x1234 /* node */, kControllerEndpoint, 1 /* remote */, WindowCovering::Id)));

    WindowCoveringController controller(MakeConfig(mBindingHandler));
    ASSERT_SUCCESS(controller.Register(kControllerEndpoint, mProvider));

    const uint32_t sentBefore = GetLoopback().mSentMessageCount;
    EXPECT_SUCCESS(controller.Dispatch(MakeCommand<WindowCovering::Commands::UpOrOpen::Type>()));
    EXPECT_EQ(GetLoopback().mSentMessageCount, sentBefore);

    controller.Unregister(mProvider);
}

} // namespace
