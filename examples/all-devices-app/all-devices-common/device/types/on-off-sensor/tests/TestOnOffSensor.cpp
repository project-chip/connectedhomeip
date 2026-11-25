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

#include <device/types/on-off-sensor/OOBAccessors.h>
#include <device/types/on-off-sensor/OnOffSensor.h>
#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/StatusResponse.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Binding/Ids.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/OnOff/Commands.h>
#include <credentials/GroupDataProviderImpl.h>
#include <devices/Types.h>
#include <lib/support/Defer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <algorithm>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kSensorEndpoint = 7;
constexpr EndpointId kPeerEndpoint   = 42;

class ActionPayload
{
public:
    ActionPayload(std::optional<EndpointId> endpointId, std::optional<uint32_t> tag2 = std::nullopt)
    {
        mError = Encode(endpointId, tag2);
    }

    ByteSpan Span() const
    {
        EXPECT_EQ(mError, CHIP_NO_ERROR);
        return ByteSpan(mBuffer, mLength);
    }

private:
    CHIP_ERROR Encode(std::optional<EndpointId> endpointId, std::optional<uint32_t> tag2)
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
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        mLength = writer.GetLengthWritten();
        return CHIP_NO_ERROR;
    }

    uint8_t mBuffer[32];
    size_t mLength    = 0;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class IdentifyDelegateMock : public IdentifyDelegate
{
public:
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override {}
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override {}
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return triggerEffectEnabled; }

    bool triggerEffectEnabled = false;
};

class FailingOnOffSensor : public OnOffSensor
{
public:
    using OnOffSensor::OnOffSensor;
    bool failRegistration = true;

private:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override
    {
        return failRegistration ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
    }
};

class InvokeRecorder : public Messaging::UnsolicitedMessageHandler, public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & header, ExchangeDelegate *& delegate) override
    {
        delegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchange, const PayloadHeader & header,
                                 System::PacketBufferHandle && payload) override
    {
        VerifyOrReturnError(header.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest),
                            CHIP_ERROR_INVALID_MESSAGE_TYPE);
        System::PacketBufferTLVReader reader;
        reader.Init(std::move(payload));
        InvokeRequestMessage::Parser request;
        ReturnErrorOnFailure(request.Init(reader));
        InvokeRequests::Parser commands;
        ReturnErrorOnFailure(request.GetInvokeRequests(&commands));
        ReturnErrorOnFailure(commands.Next());
        commands.GetReader(&reader);
        CommandDataIB::Parser command;
        ReturnErrorOnFailure(command.Init(reader));
        CommandPathIB::Parser path;
        ReturnErrorOnFailure(command.GetPath(&path));
        EndpointId endpoint;
        ClusterId cluster;
        CommandId commandId;
        ReturnErrorOnFailure(path.GetEndpointId(&endpoint));
        ReturnErrorOnFailure(path.GetClusterId(&cluster));
        ReturnErrorOnFailure(path.GetCommandId(&commandId));
        lastPath.emplace(endpoint, cluster, commandId);
        ReturnErrorOnFailure(command.GetFields(&reader));
        identifyTime.reset();
        if (cluster == Identify::Id)
        {
            Identify::Commands::Identify::DecodableType identify;
            ReturnErrorOnFailure(DataModel::Decode(reader, identify));
            identifyTime = identify.identifyTime;
        }
        else
        {
            TLV::TLVType outer;
            ReturnErrorOnFailure(reader.EnterContainer(outer));
            VerifyOrReturnError(reader.Next() == CHIP_END_OF_TLV, CHIP_ERROR_INVALID_TLV_ELEMENT);
        }
        ++count;
        // A failure response completes the exchange without requiring a peer cluster implementation.
        return StatusResponse::Send(Protocols::InteractionModel::Status::Failure, exchange, false);
    }

    void OnResponseTimeout(Messaging::ExchangeContext * exchange) override {}

    size_t count = 0;
    std::optional<ConcreteCommandPath> lastPath;
    std::optional<uint16_t> identifyTime;
};

class TestOnOffSensor : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());
        ExpireSessionBobToAlice();
        ExpireSessionAliceToBob();
        ASSERT_SUCCESS(CreateCASESessionBobToAlice());
        ASSERT_SUCCESS(CreateCASESessionAliceToBob());

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
        ASSERT_SUCCESS(
            GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, &mRecorder));
    }

    void TearDown() override
    {
        if (mSensor.GetEndpointId() != kInvalidEndpointId)
        {
            mSensor.Unregister(mProvider);
        }
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id));
        mBindingHandler.Shutdown();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    OnOffSensor::Context MakeContext()
    {
        return {
            .timerDelegate   = mTimerDelegate,
            .platformManager = DeviceLayer::PlatformMgr(),
            .bindingTable    = mBindingManager.GetBindingTable(),
            .bindingManager  = mBindingManager,
            .bindingHandler  = mBindingHandler,
        };
    }

    void AddBinding(EndpointId local, ClusterId cluster)
    {
        ASSERT_SUCCESS(mBindingManager.GetBindingTable().Add(
            Binding::TableEntry::ForNode(GetBobFabricIndex(), GetAliceFabric()->GetNodeId(), local, kPeerEndpoint, cluster)));
    }

    void ExpectCommand(ClusterId cluster, CommandId command, size_t count)
    {
        DrainAndServiceIO();
        ASSERT_EQ(mRecorder.count, count);
        ASSERT_TRUE(mRecorder.lastPath.has_value());
        EXPECT_EQ(mRecorder.lastPath->mEndpointId, kPeerEndpoint);
        EXPECT_EQ(mRecorder.lastPath->mClusterId, cluster);
        EXPECT_EQ(mRecorder.lastPath->mCommandId, command);
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    IdentifyDelegateMock mIdentifyDelegate;
    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Binding::Manager mBindingManager;
    BindingHandler mBindingHandler;
    OnOffSensor mSensor{ MakeContext(), mIdentifyDelegate };
    InvokeRecorder mRecorder;
};

TEST_F(TestOnOffSensor, RegistersOnlyMandatoryClusters)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> typesBuilder;
    ASSERT_SUCCESS(mProvider.DeviceTypes(kSensorEndpoint, typesBuilder));
    auto types = typesBuilder.TakeBuffer();
    ASSERT_EQ(types.size(), 1u);
    EXPECT_EQ(types[0].deviceTypeId, Device::Type::kOnOffSensor.deviceTypeId);
    EXPECT_EQ(types[0].deviceTypeRevision, Device::Type::kOnOffSensor.deviceTypeRevision);

    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> serversBuilder;
    ASSERT_SUCCESS(mProvider.ServerClusters(kSensorEndpoint, serversBuilder));
    auto servers = serversBuilder.TakeBuffer();
    ASSERT_EQ(servers.size(), 3u);
    for (ClusterId id : { Descriptor::Id, Identify::Id, Binding::Id })
    {
        EXPECT_NE(std::find_if(servers.begin(), servers.end(), [id](const auto & entry) { return entry.clusterId == id; }),
                  servers.end());
    }

    ReadOnlyBufferBuilder<ClusterId> clientsBuilder;
    ASSERT_SUCCESS(mProvider.ClientClusters(kSensorEndpoint, clientsBuilder));
    auto clients = clientsBuilder.TakeBuffer();
    ASSERT_EQ(clients.size(), 2u);
    EXPECT_EQ(clients[0], Identify::Id);
    EXPECT_EQ(clients[1], OnOff::Id);
}

TEST_F(TestOnOffSensor, RejectsInvalidAndRepeatedRegistrationAndAllowsReuse)
{
    EXPECT_EQ(mSensor.Register(kInvalidEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mSensor.GetEndpointId(), kInvalidEndpointId);
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    EXPECT_EQ(mSensor.Register(2, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.GetEndpointId(), kSensorEndpoint);
    mSensor.Unregister(mProvider);
    EXPECT_EQ(mSensor.GetEndpointId(), kInvalidEndpointId);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpoints;
    ASSERT_SUCCESS(mProvider.Endpoints(endpoints));
    EXPECT_TRUE(endpoints.TakeBuffer().empty());
    ASSERT_SUCCESS(mSensor.Register(2, mProvider));
    EXPECT_EQ(mSensor.GetEndpointId(), 2);
}

TEST_F(TestOnOffSensor, RollsBackPartialRegistration)
{
    FailingOnOffSensor sensor(MakeContext(), mIdentifyDelegate);
    EXPECT_EQ(sensor.Register(kSensorEndpoint, mProvider), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(sensor.GetEndpointId(), kInvalidEndpointId);
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpoints;
    ASSERT_SUCCESS(mProvider.Endpoints(endpoints));
    EXPECT_TRUE(endpoints.TakeBuffer().empty());

    sensor.failRegistration = false;
    ASSERT_SUCCESS(sensor.Register(kSensorEndpoint, mProvider));
    auto unregister = MakeDefer([&] { sensor.Unregister(mProvider); });
    EXPECT_EQ(sensor.GetEndpointId(), kSensorEndpoint);
}

TEST_F(TestOnOffSensor, SendsRequireRegistration)
{
    EXPECT_EQ(mSensor.SendOn(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendOff(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendToggle(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendIdentify(10), CHIP_ERROR_INCORRECT_STATE);
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    mSensor.Unregister(mProvider);
    EXPECT_EQ(mSensor.SendOn(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendOff(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendToggle(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendIdentify(10), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestOnOffSensor, SendsWithoutMatchingBindingAreSuccessfulNoOps)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    AddBinding(2, OnOff::Id);
    VerifyOrReturn(!HasFailure());
    EXPECT_SUCCESS(mSensor.SendOn());
    EXPECT_SUCCESS(mSensor.SendOff());
    EXPECT_SUCCESS(mSensor.SendToggle());
    EXPECT_SUCCESS(mSensor.SendIdentify(10));
    DrainAndServiceIO();
    EXPECT_EQ(mRecorder.count, 0u);
}

TEST_F(TestOnOffSensor, SendsCommandsToBoundPeer)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    AddBinding(kSensorEndpoint, OnOff::Id);
    VerifyOrReturn(!HasFailure());
    ASSERT_SUCCESS(mSensor.SendOn());
    ExpectCommand(OnOff::Id, OnOff::Commands::On::Id, 1);
    ASSERT_SUCCESS(mSensor.SendOff());
    ExpectCommand(OnOff::Id, OnOff::Commands::Off::Id, 2);
    ASSERT_SUCCESS(mSensor.SendToggle());
    ExpectCommand(OnOff::Id, OnOff::Commands::Toggle::Id, 3);

    // The On/Off binding must not receive an Identify command.
    ASSERT_SUCCESS(mSensor.SendIdentify(10));
    DrainAndServiceIO();
    EXPECT_EQ(mRecorder.count, 3u);
    AddBinding(kSensorEndpoint, Identify::Id);
    VerifyOrReturn(!HasFailure());
    ASSERT_SUCCESS(mSensor.SendIdentify(123));
    ExpectCommand(Identify::Id, Identify::Commands::Identify::Id, 4);
    EXPECT_EQ(mRecorder.identifyTime, std::optional<uint16_t>(123));
}

TEST_F(TestOnOffSensor, PropagatesBindingErrors)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    mBindingHandler.Shutdown();
    EXPECT_EQ(mSensor.SendOn(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendOff(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendToggle(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mSensor.SendIdentify(10), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestOnOffSensor, InjectedDelegateControlsIdentifyCommandsWithoutAddingClusters)
{
    for (bool triggerEffectEnabled : { false, true })
    {
        mIdentifyDelegate.triggerEffectEnabled = triggerEffectEnabled;
        OnOffSensor sensor(MakeContext(), mIdentifyDelegate);
        ASSERT_SUCCESS(sensor.Register(kSensorEndpoint, mProvider));
        auto unregister = MakeDefer([&] { sensor.Unregister(mProvider); });
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> servers;
        ASSERT_SUCCESS(mProvider.ServerClusters(kSensorEndpoint, servers));
        EXPECT_EQ(servers.TakeBuffer().size(), 3u);
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
        ASSERT_SUCCESS(sensor.IdentifyCluster().AcceptedCommands({ kSensorEndpoint, Identify::Id }, commands));
        auto accepted = commands.TakeBuffer();
        ASSERT_EQ(accepted.size(), triggerEffectEnabled ? 2u : 1u);
        EXPECT_EQ(accepted[0].commandId, Identify::Commands::Identify::Id);
        if (triggerEffectEnabled)
        {
            EXPECT_EQ(accepted[1].commandId, Identify::Commands::TriggerEffect::Id);
        }
    }
}

TEST_F(TestOnOffSensor, OOBDispatchesOnlyToSelectedSensor)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    InMemoryOOBAccessorRegistry registry;
    RegisterOOBAccessors(mSensor, registry);
    AddBinding(kSensorEndpoint, OnOff::Id);
    AddBinding(kSensorEndpoint, Identify::Id);
    VerifyOrReturn(!HasFailure());

    EXPECT_EQ(registry.HandleAction("SensorOn"_span, ActionPayload(2).Span()), CHIP_ERROR_NOT_FOUND);
    ASSERT_SUCCESS(registry.HandleAction("SensorOn"_span, ActionPayload(kSensorEndpoint).Span()));
    ExpectCommand(OnOff::Id, OnOff::Commands::On::Id, 1);
    ASSERT_SUCCESS(registry.HandleAction("SensorOff"_span, ActionPayload(kSensorEndpoint).Span()));
    ExpectCommand(OnOff::Id, OnOff::Commands::Off::Id, 2);
    ASSERT_SUCCESS(registry.HandleAction("SensorToggle"_span, ActionPayload(kSensorEndpoint).Span()));
    ExpectCommand(OnOff::Id, OnOff::Commands::Toggle::Id, 3);
    ASSERT_SUCCESS(registry.HandleAction("SensorIdentify"_span, ActionPayload(kSensorEndpoint, 60).Span()));
    ExpectCommand(Identify::Id, Identify::Commands::Identify::Id, 4);
    EXPECT_EQ(mRecorder.identifyTime, std::optional<uint16_t>(60));
}

TEST_F(TestOnOffSensor, OOBRejectsInvalidArgumentsAndPropagatesErrors)
{
    ASSERT_SUCCESS(mSensor.Register(kSensorEndpoint, mProvider));
    InMemoryOOBAccessorRegistry registry;
    RegisterOOBAccessors(mSensor, registry);
    EXPECT_EQ(registry.HandleAction("Unknown"_span, ActionPayload(kSensorEndpoint).Span()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(registry.HandleAction("SensorOn"_span, ActionPayload(std::nullopt).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registry.HandleAction("SensorIdentify"_span, ActionPayload(kSensorEndpoint).Span()), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(registry.HandleAction("SensorIdentify"_span, ActionPayload(kSensorEndpoint, 65536).Span()),
              CHIP_ERROR_INVALID_INTEGER_VALUE);
    mBindingHandler.Shutdown();
    EXPECT_EQ(registry.HandleAction("SensorOn"_span, ActionPayload(kSensorEndpoint).Span()), CHIP_ERROR_INCORRECT_STATE);
}

} // namespace
