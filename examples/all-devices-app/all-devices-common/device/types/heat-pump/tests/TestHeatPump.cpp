/*
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

#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/heat-pump/impl/OOBAccessors.h>
#include <device/types/heat-pump/impl/SimulatedHeatPump.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <binding-handler/SimpleBindingCommand.h>
#include <clusters/Binding/ClusterId.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/Identify/ClusterId.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/Thermostat/ClusterId.h>
#include <clusters/Thermostat/Commands.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/power-source/WiredPowerSource.h>
#include <devices/Types.h>
#include <lib/core/TLV.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <optional>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

using ThermostatControl = SimulatedHeatPump::ThermostatControl;

class NoopIdentifyDelegate : public Clusters::IdentifyDelegate
{
public:
    void OnIdentifyStart(Clusters::IdentifyCluster &) override {}
    void OnIdentifyStop(Clusters::IdentifyCluster &) override {}
    void OnTriggerEffect(Clusters::IdentifyCluster &) override {}
    bool IsTriggerEffectEnabled() const override { return false; }
};

class FailingEndpointIdAllocator : public EndpointIdAllocator
{
public:
    explicit FailingEndpointIdAllocator(size_t failureIndex) : mFailureIndex(failureIndex) {}

    EndpointId Allocate() override
    {
        if (mAllocationIndex++ == mFailureIndex)
        {
            return kInvalidEndpointId;
        }
        return mNextEndpoint++;
    }

private:
    const size_t mFailureIndex;
    size_t mAllocationIndex  = 0;
    EndpointId mNextEndpoint = 1;
};

std::vector<DataModel::EndpointEntry> GetEndpoints(CodeDrivenDataModelProvider & provider)
{
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
    EXPECT_SUCCESS(provider.Endpoints(builder));
    const auto buffer = builder.TakeBuffer();
    return std::vector<DataModel::EndpointEntry>(buffer.begin(), buffer.end());
}

std::vector<ClusterId> GetServerClusters(CodeDrivenDataModelProvider & provider, EndpointId endpoint)
{
    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    EXPECT_SUCCESS(provider.ServerClusters(endpoint, builder));
    std::vector<ClusterId> clusters;
    for (const auto & entry : builder.TakeBuffer())
    {
        clusters.push_back(entry.clusterId);
    }
    std::sort(clusters.begin(), clusters.end());
    return clusters;
}

std::vector<ClusterId> GetClientClusters(const DeviceInterface & device)
{
    ReadOnlyBufferBuilder<ClusterId> builder;
    EXPECT_SUCCESS(device.ClientClusters(builder));
    const auto buffer = builder.TakeBuffer();
    return std::vector<ClusterId>(buffer.begin(), buffer.end());
}

template <typename CommandType>
Platform::UniquePtr<BindingCommand> MakeCommand(const CommandType & command)
{
    return Platform::UniquePtr<BindingCommand>(Platform::New<SimpleBindingCommand<CommandType>>(command));
}

/// Builds the HeatPumpSetpointRaiseLower payload { 1: endpointId, 2: mode, 3: amount }, omitting unset fields.
class SetpointRaiseLowerPayload
{
public:
    SetpointRaiseLowerPayload(EndpointId endpointId, std::optional<uint8_t> mode, std::optional<int8_t> amount)
    {
        mError = Encode(endpointId, mode, amount);
    }

    ByteSpan Span() const
    {
        EXPECT_SUCCESS(mError);
        return ByteSpan(mBuffer, mLength);
    }

private:
    CHIP_ERROR Encode(EndpointId endpointId, std::optional<uint8_t> mode, std::optional<int8_t> amount)
    {
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
        if (mode.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *mode));
        }
        if (amount.has_value())
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *amount));
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

constexpr CharSpan kSetpointRaiseLowerAction = "HeatPumpSetpointRaiseLower"_span;

class TestHeatPump : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_SUCCESS(Platform::MemoryInit()); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    SimulatedHeatPump::Context MakeContext()
    {
        return SimulatedHeatPump::Context{
            .timerDelegate            = mTimerDelegate,
            .testEventTriggerDelegate = mTestEventTriggerDelegate,
            .identifyDelegate         = mIdentifyDelegate,
            .fabricTable              = mFabricTable,
            .groupDataProvider        = mGroupDataProvider,
            .platformManager          = DeviceLayer::PlatformMgr(),
            .bindingTable             = mBindingManager.GetBindingTable(),
            .bindingManager           = mBindingManager,
            .bindingHandler           = mBindingHandler,
        };
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    SimpleTestEventTriggerDelegate mTestEventTriggerDelegate;
    NoopIdentifyDelegate mIdentifyDelegate;
    FabricTable mFabricTable;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    Clusters::Binding::Manager mBindingManager;
    BindingHandler mBindingHandler;
};

TEST_F(TestHeatPump, ThermostatEndpointVariantComposesThermostatChild)
{
    SimulatedHeatPump heatPump(MakeContext(), ThermostatControl::kThermostatEndpoint);
    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_SUCCESS(heatPump.Register(allocator, mProvider));

    const EndpointId heatPumpEndpoint = heatPump.GetEndpointId();
    const auto endpoints              = GetEndpoints(mProvider);
    // Heat pump, power source, energy management, electrical sensor and thermostat.
    ASSERT_EQ(endpoints.size(), 5u);
    for (const auto & entry : endpoints)
    {
        if (entry.id != heatPumpEndpoint)
        {
            EXPECT_EQ(entry.parentId, heatPumpEndpoint);
        }
    }

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypes;
    ASSERT_SUCCESS(mProvider.DeviceTypes(heatPumpEndpoint, deviceTypes));
    const auto deviceTypeBuffer = deviceTypes.TakeBuffer();
    ASSERT_EQ(deviceTypeBuffer.size(), 1u);
    EXPECT_EQ(deviceTypeBuffer[0].deviceTypeId, Device::Type::kHeatPump.deviceTypeId);

    EXPECT_EQ(GetServerClusters(mProvider, heatPumpEndpoint),
              (std::vector<ClusterId>{ Clusters::Identify::Id, Clusters::Descriptor::Id }));
    EXPECT_TRUE(GetClientClusters(heatPump).empty());

    const auto thermostatEndpoint = std::find_if(endpoints.begin(), endpoints.end(), [this](const auto & entry) {
        const auto clusters = GetServerClusters(mProvider, entry.id);
        return std::find(clusters.begin(), clusters.end(), Clusters::Thermostat::Id) != clusters.end();
    });
    EXPECT_NE(thermostatEndpoint, endpoints.end());

    heatPump.Unregister(mProvider);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
    EXPECT_EQ(heatPump.GetEndpointId(), kInvalidEndpointId);
}

TEST_F(TestHeatPump, ThermostatClientVariantAddsBindingAndClient)
{
    SimulatedHeatPump heatPump(MakeContext(), ThermostatControl::kThermostatClient);
    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_SUCCESS(heatPump.Register(allocator, mProvider));

    // Heat pump, power source, energy management and electrical sensor.
    EXPECT_EQ(GetEndpoints(mProvider).size(), 4u);
    EXPECT_EQ(GetServerClusters(mProvider, heatPump.GetEndpointId()),
              (std::vector<ClusterId>{ Clusters::Identify::Id, Clusters::Descriptor::Id, Clusters::Binding::Id }));
    EXPECT_EQ(GetClientClusters(heatPump), (std::vector<ClusterId>{ Clusters::Thermostat::Id }));

    heatPump.Unregister(mProvider);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
}

TEST_F(TestHeatPump, RegisterFailsWithoutThermostatEndpointOrClient)
{
    // The spec requires one or more Thermostat endpoints or a Thermostat client; this heat pump has neither.
    HeatPump heatPump(
        HeatPump::Config{},
        HeatPump::Parts{
            .powerSource      = std::make_unique<WiredPowerSource>("grid"_span, Clusters::PowerSource::WiredCurrentTypeEnum::kAc),
            .energyManagement = std::make_unique<EnergyManagement>(mTimerDelegate),
            .electricalSensor = std::make_unique<SimulatedElectricalSensor>(mTimerDelegate, mTestEventTriggerDelegate),
        });
    ConsecutiveEndpointIdAllocator allocator;

    EXPECT_EQ(heatPump.Register(allocator, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
    EXPECT_EQ(heatPump.GetEndpointId(), kInvalidEndpointId);
}

TEST_F(TestHeatPump, FailedAllocationRollsBackAllEndpoints)
{
    for (ThermostatControl thermostatControl : { ThermostatControl::kThermostatEndpoint, ThermostatControl::kThermostatClient })
    {
        const size_t allocations = thermostatControl == ThermostatControl::kThermostatEndpoint ? 5 : 4;
        for (size_t failureIndex = 0; failureIndex < allocations; failureIndex++)
        {
            SimulatedHeatPump heatPump(MakeContext(), thermostatControl);
            FailingEndpointIdAllocator failingAllocator(failureIndex);

            EXPECT_NE(heatPump.Register(failingAllocator, mProvider), CHIP_NO_ERROR);
            EXPECT_TRUE(GetEndpoints(mProvider).empty());
            EXPECT_EQ(heatPump.GetEndpointId(), kInvalidEndpointId);

            // A clean rollback leaves the device registrable again.
            ConsecutiveEndpointIdAllocator allocator;
            ASSERT_SUCCESS(heatPump.Register(allocator, mProvider));
            heatPump.Unregister(mProvider);
        }
    }
}

TEST_F(TestHeatPump, DispatchRequiresRegistrationAndThermostatClient)
{
    const Clusters::Thermostat::Commands::SetpointRaiseLower::Type setpointRaiseLower{
        .mode   = Clusters::Thermostat::SetpointRaiseLowerModeEnum::kHeat,
        .amount = 10,
    };

    SimulatedHeatPump clientHeatPump(MakeContext(), ThermostatControl::kThermostatClient);
    EXPECT_EQ(clientHeatPump.Dispatch(MakeCommand(setpointRaiseLower)), CHIP_ERROR_INCORRECT_STATE);

    ConsecutiveEndpointIdAllocator clientAllocator;
    ASSERT_SUCCESS(clientHeatPump.Register(clientAllocator, mProvider));
    EXPECT_EQ(clientHeatPump.Dispatch(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(clientHeatPump.Dispatch(MakeCommand(Clusters::OnOff::Commands::On::Type{})), CHIP_ERROR_INVALID_ARGUMENT);
    clientHeatPump.Unregister(mProvider);

    // The thermostat endpoint variant has no Thermostat client, so it rejects Thermostat commands too.
    SimulatedHeatPump endpointHeatPump(MakeContext(), ThermostatControl::kThermostatEndpoint);
    ConsecutiveEndpointIdAllocator endpointAllocator;
    ASSERT_SUCCESS(endpointHeatPump.Register(endpointAllocator, mProvider));
    EXPECT_EQ(endpointHeatPump.Dispatch(MakeCommand(setpointRaiseLower)), CHIP_ERROR_INVALID_ARGUMENT);
    endpointHeatPump.Unregister(mProvider);
}

TEST_F(TestHeatPump, HasThermostatControlFindsClientOrThermostatEndpoint)
{
    SimulatedHeatPump clientHeatPump(MakeContext(), ThermostatControl::kThermostatClient);
    EXPECT_TRUE(clientHeatPump.HasThermostatControl(mProvider));

    // The Thermostat endpoint only counts once it is registered as a child of the heat pump endpoint.
    SimulatedHeatPump endpointHeatPump(MakeContext(), ThermostatControl::kThermostatEndpoint);
    EXPECT_FALSE(endpointHeatPump.HasThermostatControl(mProvider));

    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_SUCCESS(endpointHeatPump.Register(allocator, mProvider));
    EXPECT_TRUE(endpointHeatPump.HasThermostatControl(mProvider));

    endpointHeatPump.Unregister(mProvider);
    EXPECT_FALSE(endpointHeatPump.HasThermostatControl(mProvider));
}

TEST_F(TestHeatPump, SetpointRaiseLowerAccessorValidatesPayload)
{
    SimulatedHeatPump clientHeatPump(MakeContext(), ThermostatControl::kThermostatClient);
    ConsecutiveEndpointIdAllocator allocator;
    ASSERT_SUCCESS(clientHeatPump.Register(allocator, mProvider));
    const EndpointId endpoint = clientHeatPump.GetEndpointId();

    InMemoryOOBAccessorRegistry registry;
    RegisterOOBAccessors(clientHeatPump, registry);

    // Other endpoints are left to other accessors.
    EXPECT_EQ(registry.HandleAction(kSetpointRaiseLowerAction, SetpointRaiseLowerPayload(endpoint + 100, 0, 10).Span()),
              CHIP_ERROR_NOT_FOUND);
    // Mode 3 is outside SetpointRaiseLowerModeEnum (Heat, Cool, Both).
    EXPECT_EQ(registry.HandleAction(kSetpointRaiseLowerAction, SetpointRaiseLowerPayload(endpoint, 3, 10).Span()),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(registry.HandleAction(kSetpointRaiseLowerAction, SetpointRaiseLowerPayload(endpoint, 0, std::nullopt).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    clientHeatPump.Unregister(mProvider);

    // Without a Thermostat client the device has nothing to send the command with.
    SimulatedHeatPump endpointHeatPump(MakeContext(), ThermostatControl::kThermostatEndpoint);
    ConsecutiveEndpointIdAllocator endpointAllocator;
    ASSERT_SUCCESS(endpointHeatPump.Register(endpointAllocator, mProvider));
    InMemoryOOBAccessorRegistry endpointRegistry;
    RegisterOOBAccessors(endpointHeatPump, endpointRegistry);
    EXPECT_EQ(endpointRegistry.HandleAction(kSetpointRaiseLowerAction,
                                            SetpointRaiseLowerPayload(endpointHeatPump.GetEndpointId(), 0, 10).Span()),
              CHIP_ERROR_INVALID_ARGUMENT);
    endpointHeatPump.Unregister(mProvider);
}

} // namespace
