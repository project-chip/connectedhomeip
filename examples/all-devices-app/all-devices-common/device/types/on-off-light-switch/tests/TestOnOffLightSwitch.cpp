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

#include <device/types/on-off-light-switch/OOBAccessors.h>
#include <device/types/on-off-light-switch/OnOffLightSwitch.h>
#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <binding-handler/BindingHandler.h>
#include <clusters/Identify/Enums.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Ids.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/core/TLV.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimerDelegateMock.h>
#include <messaging/tests/MessagingContext.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <platform/CHIPDeviceLayer.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kSwitchEndpoint = 1;
constexpr EndpointId kOtherEndpoint  = 2;
constexpr GroupId kGroupId           = 0x0101;

/// Builds the OOB TLV payload: { 1: endpointId, 2: tag2, 3: tag3 }, omitting the fields that are not set.
class ActionPayload
{
public:
    ActionPayload(std::optional<EndpointId> endpointId, std::optional<uint32_t> tag2 = std::nullopt,
                  std::optional<uint8_t> tag3 = std::nullopt)
    {
        mError = Encode(endpointId, tag2, tag3);
    }

    ByteSpan Span() const
    {
        EXPECT_EQ(mError, CHIP_NO_ERROR);
        return ByteSpan(mBuffer, mLength);
    }

private:
    CHIP_ERROR Encode(std::optional<EndpointId> endpointId, std::optional<uint32_t> tag2, std::optional<uint8_t> tag3)
    {
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        if (endpointId.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), *endpointId));
        }
        if (tag2.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *tag2));
        }
        if (tag3.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *tag3));
        }
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        mLength = writer.GetLengthWritten();
        return CHIP_NO_ERROR;
    }

    uint8_t mBuffer[32];
    size_t mLength    = 0;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

/**
 * OnOffLightSwitch fixture with a BindingHandler initialized on a real Binding::Manager, backed by a
 * loopback messaging stack and a real CASESessionManager.
 */
class TestOnOffLightSwitch : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        CASESessionManagerConfig config;
        config.sessionInitParams.sessionManager = &GetSecureSessionManager();
        config.sessionInitParams.exchangeMgr    = &GetExchangeManager();
        config.sessionInitParams.fabricTable    = &GetFabricTable();
        // Only validated for non-null: session establishment never gets far enough to use it in these tests.
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
        mRegistry.Clear();
        mBindingHandler.Shutdown();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    OnOffLightSwitch::Config MakeConfig(BindingHandler & bindingHandler)
    {
        return OnOffLightSwitch::Config{
            .timerDelegate   = mTimerDelegate,
            .platformManager = DeviceLayer::PlatformMgr(),
            .bindingTable    = mBindingManager.GetBindingTable(),
            .bindingManager  = mBindingManager,
            .bindingHandler  = bindingHandler,
        };
    }

    void AddGroupBinding(ClusterId clusterId)
    {
        ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                      Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kSwitchEndpoint, clusterId)),
                  CHIP_NO_ERROR);
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    InMemoryOOBAccessorRegistry mRegistry;

    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Binding::Manager mBindingManager;
    BindingHandler mBindingHandler;
};

TEST_F(TestOnOffLightSwitch, UnregisteredSendsFail)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));

    // Prior to registration there is no endpoint to send from, even though the binding handler is initialized.
    EXPECT_EQ(lightSwitch.SendOn(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(lightSwitch.SendOff(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(lightSwitch.SendToggle(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(lightSwitch.SendIdentify(10), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(lightSwitch.SendTriggerEffect(Identify::EffectIdentifierEnum::kBlink, Identify::EffectVariantEnum::kDefault),
              CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestOnOffLightSwitch, RegistrationExposesClientClusters)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));

    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.GetEndpointId(), kSwitchEndpoint);

    ReadOnlyBufferBuilder<ClusterId> clientClusters;
    EXPECT_EQ(lightSwitch.ClientClusters(clientClusters), CHIP_NO_ERROR);
    auto buffer = clientClusters.TakeBuffer();
    ASSERT_EQ(buffer.size(), 2U);
    EXPECT_EQ(buffer[0], OnOff::Id);
    EXPECT_EQ(buffer[1], Identify::Id);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, SendsWithoutBindingSucceed)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);

    // With no binding configured there is nothing to send: every dispatch is a successful no-op.
    EXPECT_EQ(lightSwitch.SendOn(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendOff(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendToggle(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendIdentify(10), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendTriggerEffect(Identify::EffectIdentifierEnum::kBlink, Identify::EffectVariantEnum::kDefault),
              CHIP_NO_ERROR);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, SendsDispatchToGroupBindings)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);

    // No group keys are provisioned, so the group sends themselves fail and are only logged.
    AddGroupBinding(OnOff::Id);
    AddGroupBinding(Identify::Id);

    EXPECT_EQ(lightSwitch.SendOn(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendOff(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendToggle(), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendIdentify(10), CHIP_NO_ERROR);
    EXPECT_EQ(lightSwitch.SendTriggerEffect(Identify::EffectIdentifierEnum::kOkay, Identify::EffectVariantEnum::kDefault),
              CHIP_NO_ERROR);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, SendsFailWhenBindingHandlerIsNotInitialized)
{
    BindingHandler uninitializedHandler;
    OnOffLightSwitch lightSwitch(MakeConfig(uninitializedHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(lightSwitch.SendToggle(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(lightSwitch.SendIdentify(10), CHIP_ERROR_INCORRECT_STATE);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, OOBAccessorDispatchesActions)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(lightSwitch, mRegistry);
    AddGroupBinding(OnOff::Id);
    AddGroupBinding(Identify::Id);

    const uint32_t kBlink = static_cast<uint32_t>(Identify::EffectIdentifierEnum::kBlink);

    EXPECT_EQ(mRegistry.HandleAction("SwitchOn"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("SwitchOff"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("SwitchIdentify"_span, ActionPayload(kSwitchEndpoint, 10).Span()), CHIP_NO_ERROR);
    EXPECT_EQ(mRegistry.HandleAction("SwitchTriggerEffect"_span, ActionPayload(kSwitchEndpoint, kBlink, 0).Span()), CHIP_NO_ERROR);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, OOBAccessorRejectsInvalidPayloads)
{
    OnOffLightSwitch lightSwitch(MakeConfig(mBindingHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(lightSwitch, mRegistry);

    // Unknown actions and other endpoints are left to other accessors.
    EXPECT_EQ(mRegistry.HandleAction("SetOnOff"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload(kOtherEndpoint).Span()), CHIP_ERROR_NOT_FOUND);

    // Missing EndpointId.
    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload(std::nullopt).Span()), CHIP_ERROR_INVALID_ARGUMENT);

    // Missing or out-of-range arguments.
    EXPECT_EQ(mRegistry.HandleAction("SwitchIdentify"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("SwitchIdentify"_span, ActionPayload(kSwitchEndpoint, 0x10000).Span()),
              CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(mRegistry.HandleAction("SwitchTriggerEffect"_span, ActionPayload(kSwitchEndpoint, 0).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mRegistry.HandleAction("SwitchTriggerEffect"_span, ActionPayload(kSwitchEndpoint, 0x100, 0).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);

    lightSwitch.Unregister(mProvider);
}

TEST_F(TestOnOffLightSwitch, OOBAccessorPropagatesSendErrors)
{
    BindingHandler uninitializedHandler;
    OnOffLightSwitch lightSwitch(MakeConfig(uninitializedHandler));
    ASSERT_EQ(lightSwitch.Register(kSwitchEndpoint, mProvider), CHIP_NO_ERROR);
    RegisterOOBAccessors(lightSwitch, mRegistry);

    EXPECT_EQ(mRegistry.HandleAction("SwitchToggle"_span, ActionPayload(kSwitchEndpoint).Span()), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mRegistry.HandleAction("SwitchIdentify"_span, ActionPayload(kSwitchEndpoint, 5).Span()), CHIP_ERROR_INCORRECT_STATE);

    lightSwitch.Unregister(mProvider);
}

} // namespace
